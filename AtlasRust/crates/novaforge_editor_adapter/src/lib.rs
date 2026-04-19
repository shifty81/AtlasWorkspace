//! novaforge_editor_adapter — Bridge between the AtlasWorkspace editor and
//! the NovaForge game/server processes.
//!
//! Provides the launch contract (how the workspace spawns game processes),
//! the in-process adapter used when the game is embedded, and helpers for
//! routing editor events into the running game.

use serde::{Deserialize, Serialize};
use std::{path::PathBuf, process};

use atlas_engine::{Engine, Event};

// ── Launch contract ───────────────────────────────────────────────────────────

/// Mirrors the C++ WorkspaceLaunchContract.
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct LaunchContract {
    /// Path to the `.atlas` project file.
    pub project_path:    String,
    /// Workspace root directory.
    pub workspace_root:  String,
    /// Opaque session identifier.
    pub session_id:      String,
    /// If true, run headless (no window).
    pub headless:        bool,
    /// Override for the game binary path. Empty = use workspace default.
    pub game_binary:     String,
}

impl LaunchContract {
    pub fn new(project_path: impl Into<String>) -> Self {
        Self { project_path: project_path.into(), ..Default::default() }
    }

    /// Serialise to a command-line argument list.
    pub fn to_args(&self) -> Vec<String> {
        let mut args = vec![
            format!("--project={}", self.project_path),
            format!("--workspace-root={}", self.workspace_root),
            format!("--session-id={}", self.session_id),
        ];
        if self.headless { args.push("--headless".into()); }
        args.push("--hosted".into());
        args
    }
}

// ── AdapterMode ───────────────────────────────────────────────────────────────

/// How the game is connected to the editor.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
pub enum AdapterMode {
    /// Game runs as a separate process; editor communicates via pipeline files.
    #[default]
    OutOfProcess,
    /// Game is embedded in the editor process (single binary, edit mode).
    InProcess,
}

// ── EditorGameBridge ──────────────────────────────────────────────────────────

/// Manages the connection between the editor and a live game session.
pub struct EditorGameBridge {
    pub mode:            AdapterMode,
    pub contract:        LaunchContract,
    child_pid:           Option<u32>,
    connected:           bool,
}

impl EditorGameBridge {
    pub fn new(mode: AdapterMode, contract: LaunchContract) -> Self {
        Self { mode, contract, child_pid: None, connected: false }
    }

    /// Launch the game as a child process using the contract.
    pub fn launch_game(&mut self, binary: &str) -> std::io::Result<()> {
        let args = self.contract.to_args();
        tracing::info!(target: "EditorAdapter", "Launching game: {} {:?}", binary, args);
        let child = process::Command::new(binary)
            .args(&args)
            .spawn()?;
        self.child_pid = Some(child.id());
        self.connected = true;
        tracing::info!(target: "EditorAdapter", "Game PID={:?}", self.child_pid);
        Ok(())
    }

    /// Terminate the child game process.
    pub fn stop_game(&mut self) {
        if let Some(_pid) = self.child_pid.take() {
            tracing::info!(target: "EditorAdapter", "Stopping game process");
            // Platform kill logic would go here in a full implementation.
            self.connected = false;
        }
    }

    pub fn is_connected(&self) -> bool { self.connected }
    pub fn child_pid(&self) -> Option<u32> { self.child_pid }

    /// Emit a play-mode start event into an in-process engine.
    pub fn begin_play(&self, engine: &mut Engine) {
        let mut evt = Event::new("EDITOR_BEGIN_PLAY");
        evt.str_param = self.contract.session_id.clone();
        engine.events.publish(evt);
        tracing::info!(target: "EditorAdapter", "BEGIN_PLAY sent to engine");
    }

    /// Emit a play-mode stop event into an in-process engine.
    pub fn end_play(&self, engine: &mut Engine) {
        engine.events.publish(Event::new("EDITOR_END_PLAY"));
        tracing::info!(target: "EditorAdapter", "END_PLAY sent to engine");
    }
}

// ── WorldSyncAdapter ──────────────────────────────────────────────────────────

/// Synchronises world state snapshots between editor and game (out-of-process).
#[derive(Debug)]
pub struct WorldSyncAdapter {
    /// Directory where world-state snapshots are written.
    pub worlds_dir: PathBuf,
    snapshot_index: u64,
}

impl WorldSyncAdapter {
    pub fn new(worlds_dir: impl Into<PathBuf>) -> Self {
        Self { worlds_dir: worlds_dir.into(), snapshot_index: 0 }
    }

    pub fn write_snapshot(&mut self, json_data: &str) -> std::io::Result<()> {
        std::fs::create_dir_all(&self.worlds_dir)?;
        let path = self.worlds_dir.join(format!("world_{:06}.json", self.snapshot_index));
        self.snapshot_index += 1;
        std::fs::write(path, json_data)
    }

    pub fn latest_snapshot(&self) -> Option<String> {
        if self.snapshot_index == 0 { return None; }
        let path = self.worlds_dir.join(format!("world_{:06}.json", self.snapshot_index - 1));
        std::fs::read_to_string(path).ok()
    }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn launch_contract_to_args() {
        let contract = LaunchContract {
            project_path:   "/projects/MyGame".into(),
            workspace_root: "/ws".into(),
            session_id:     "sess-1".into(),
            headless:       false,
            game_binary:    String::new(),
        };
        let args = contract.to_args();
        assert!(args.iter().any(|a| a.contains("/projects/MyGame")));
        assert!(args.iter().any(|a| a.contains("/ws")));
        assert!(args.iter().any(|a| a.contains("sess-1")));
    }

    #[test]
    fn bridge_initially_disconnected() {
        let contract = LaunchContract::new("/projects/Test");
        let bridge = EditorGameBridge::new(AdapterMode::OutOfProcess, contract);
        assert!(!bridge.is_connected());
        assert!(bridge.child_pid().is_none());
    }

    #[test]
    fn bridge_in_process_begin_play() {
        let contract = LaunchContract::new("/projects/Test");
        let bridge = EditorGameBridge::new(AdapterMode::InProcess, contract);
        let mut engine = atlas_engine::Engine::new(atlas_engine::EngineConfig {
            headless: true,
            ..Default::default()
        });
        engine.init_core();
        bridge.begin_play(&mut engine);
        assert!(engine.is_running());
    }

    #[test]
    fn bridge_in_process_end_play() {
        let contract = LaunchContract::new("/projects/Test");
        let bridge = EditorGameBridge::new(AdapterMode::InProcess, contract);
        let mut engine = atlas_engine::Engine::new(atlas_engine::EngineConfig {
            headless: true,
            ..Default::default()
        });
        engine.init_core();
        bridge.begin_play(&mut engine);
        // Subscribe to EDITOR_END_PLAY so we can verify it was published
        let received = std::sync::Arc::new(std::sync::Mutex::new(false));
        let r2 = received.clone();
        engine.events.subscribe("EDITOR_END_PLAY", move |_| { *r2.lock().unwrap() = true; });
        bridge.end_play(&mut engine);
        assert!(*received.lock().unwrap(), "end_play must publish EDITOR_END_PLAY event");
    }

    #[test]
    fn world_sync_snapshot_round_trip() {
        let dir = std::env::temp_dir()
            .join(format!("atlas_world_sync_{}", std::process::id()));
        std::fs::create_dir_all(&dir).ok();
        let mut adapter = WorldSyncAdapter::new(&dir);
        assert!(adapter.latest_snapshot().is_none(), "no snapshot before first write");
        adapter.write_snapshot(r#"{"entities":42}"#).expect("must write snapshot");
        let snap = adapter.latest_snapshot().expect("must find snapshot");
        assert!(snap.contains("42"));
        std::fs::remove_dir_all(&dir).ok();
    }
}
