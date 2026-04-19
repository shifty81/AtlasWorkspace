//! NovaForge voxel world editor — host entry point.
//!
//! The editor embeds an AtlasWorkspace shell and connects to the game via the
//! novaforge_editor_adapter bridge.
//!
//! Launch arguments:
//!   --project=<path>          open a specific .atlas project
//!   --workspace-root=<path>   override the workspace root directory
//!   --headless                run headless (no window, one tick)

use std::{env, path::PathBuf};
use tracing_subscriber::EnvFilter;

use atlas_core::NF_VERSION_STRING;
use atlas_editor::EditorContext;
use atlas_engine::{Engine, EngineConfig, EngineMode};
use atlas_workspace::{WorkspaceShell, WorkspaceTool};
use novaforge_editor_adapter::{AdapterMode, EditorGameBridge, LaunchContract};

fn main() {
    tracing_subscriber::fmt()
        .with_env_filter(EnvFilter::from_default_env().add_directive("info".parse().unwrap()))
        .with_target(true)
        .init();

    // ── CLI ───────────────────────────────────────────────────────────────────
    let mut project_path   = String::new();
    let mut workspace_root = String::from(".");
    let mut headless       = false;

    for arg in env::args().skip(1) {
        if let Some(v) = arg.strip_prefix("--project=") {
            project_path = v.to_string();
        } else if let Some(v) = arg.strip_prefix("--workspace-root=") {
            workspace_root = v.to_string();
        } else if arg == "--headless" {
            headless = true;
        }
    }

    tracing::info!(target: "NovaForgeEditor", "=== NovaForge Editor v{NF_VERSION_STRING} ===");

    // ── Workspace shell ────────────────────────────────────────────────────────
    let mut shell = WorkspaceShell::new(&workspace_root);
    shell.init();

    if !project_path.is_empty() {
        shell.load_project(&PathBuf::from(&project_path));
    }

    shell.register_tool(WorkspaceTool {
        id: "NovaForgeGame".into(), display_name: "NovaForge Game".into(),
        icon: "🎮".into(), active: false,
    });
    shell.activate_tool("NovaForgeGame");

    // ── Editor context ─────────────────────────────────────────────────────────
    let editor = EditorContext::new();
    editor.init();

    // ── Game adapter ───────────────────────────────────────────────────────────
    let contract = LaunchContract {
        project_path:   project_path.clone(),
        workspace_root: workspace_root.clone(),
        session_id:     "editor-session".into(),
        headless:       false,
        game_binary:    String::new(),
    };
    let bridge = EditorGameBridge::new(AdapterMode::OutOfProcess, contract);
    tracing::info!(target: "NovaForgeEditor", "Editor bridge mode: {:?}", bridge.mode);

    // ── Engine ─────────────────────────────────────────────────────────────────
    let mut engine = Engine::new(EngineConfig {
        mode: EngineMode::Editor,
        headless,
        ..Default::default()
    });
    engine.init_core();
    engine.register_system("EditorAdapter");

    if headless {
        engine.run_headless(1);
    } else {
        // In a full build, the egui/winit window would start here.
        engine.run_headless(10);
    }

    tracing::info!(target: "NovaForgeEditor", "Editor shutdown");
}

