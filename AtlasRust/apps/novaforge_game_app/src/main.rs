//! NovaForge — Voxel game client entry point.
//!
//! Launch arguments (forward-compatible with the C++ WorkspaceLaunchContract):
//!   --headless          no window; exits after one tick (CI / server-side testing)
//!   --project=<path>    path to project directory or .atlas file
//!   --workspace-root=<path>
//!   --session-id=<id>
//!
//! Controls (in-game):
//!   Left-click          capture cursor (FPS mode)
//!   Escape              release cursor
//!   WASD                move
//!   Space / LCtrl       ascend / descend
//!   LShift              sprint (3× speed)
//!   Mouse               look around

use bevy::{
    log::LogPlugin,
    prelude::*,
};
use bevy::ecs::prelude::MessageWriter;
use novaforge_game::GamePlugin;

fn main() {
    // ── CLI argument parsing ───────────────────────────────────────────────────
    let mut headless      = false;
    let mut project_path  = String::new();
    let mut workspace_root = String::new();
    let mut session_id    = String::new();

    for arg in std::env::args().skip(1) {
        if arg == "--headless" {
            headless = true;
        } else if arg == "--hosted" {
            // launched by AtlasWorkspace; no-op
        } else if let Some(v) = arg.strip_prefix("--project=") {
            project_path = v.to_string();
        } else if let Some(v) = arg.strip_prefix("--workspace-root=") {
            workspace_root = v.to_string();
        } else if let Some(v) = arg.strip_prefix("--session-id=") {
            session_id = v.to_string();
        }
        // Unknown args are silently ignored for forward-compat.
    }

    // ── Bevy App ───────────────────────────────────────────────────────────────
    let mut app = App::new();

    if headless {
        // Headless mode: no window, run one update then exit.
        app.add_plugins(
            DefaultPlugins
                .build()
                .disable::<bevy::winit::WinitPlugin>()
                .set(LogPlugin {
                    level: bevy::log::Level::INFO,
                    ..default()
                }),
        )
        .add_plugins(GamePlugin)
        .add_systems(Update, exit_after_one_frame);
    } else {
        app.add_plugins(
            DefaultPlugins
                .build()
                .set(WindowPlugin {
                    primary_window: Some(Window {
                        title: format!(
                            "NovaForge v{}{}{}",
                            atlas_core::NF_VERSION_STRING,
                            if project_path.is_empty() { "".to_string() } else { format!(" — {project_path}") },
                            if session_id.is_empty() { "".to_string() } else { format!(" [{session_id}]") },
                        ),
                        resolution: (1280_u32, 720_u32).into(),
                        ..default()
                    }),
                    ..default()
                })
                .set(LogPlugin {
                    level: bevy::log::Level::INFO,
                    ..default()
                }),
        )
        .add_plugins(GamePlugin);
    }

    // Log startup context (mirrors C++ NF_LOG_INFO calls)
    tracing::info!(target: "Main", "=== NovaForge Game ===");
    tracing::info!(target: "Main", "Version: {}", atlas_core::NF_VERSION_STRING);
    if !project_path.is_empty() {
        tracing::info!(target: "Main", "Project: {project_path}");
    }
    if !workspace_root.is_empty() {
        tracing::info!(target: "Main", "Workspace root: {workspace_root}");
    }
    if !session_id.is_empty() {
        tracing::info!(target: "Main", "Session: {session_id}");
    }
    if headless {
        tracing::info!(target: "Main", "Mode: headless");
    }

    app.run();
}

/// Headless mode: exit cleanly after the first Update tick.
fn exit_after_one_frame(mut exit: MessageWriter<AppExit>) {
    tracing::info!(target: "Main", "Headless tick complete — exiting");
    exit.write(AppExit::Success);
}
