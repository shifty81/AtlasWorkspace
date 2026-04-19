//! AtlasWorkspace — IDE host application entry point.
//!
//! Launch arguments:
//!   --project=<path>          open a specific .atlas project file
//!   --workspace-root=<path>   override the workspace root directory
//!   --headless                no UI, run one tick and exit (CI / testing)

use std::{env, path::PathBuf};
use tracing_subscriber::EnvFilter;

use atlas_core::NF_VERSION_STRING;
use atlas_editor::EditorContext;
use atlas_engine::{Engine, EngineConfig, EngineMode};
use atlas_workspace::{WorkspaceShell, WorkspaceTool};

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

    tracing::info!(target: "AtlasWorkspace", "=== AtlasWorkspace v{NF_VERSION_STRING} ===");
    tracing::info!(target: "AtlasWorkspace", "Workspace root: {workspace_root}");

    // ── Workspace shell ────────────────────────────────────────────────────────
    let mut shell = WorkspaceShell::new(&workspace_root);
    shell.init();

    if !project_path.is_empty() {
        let path = PathBuf::from(&project_path);
        if !shell.load_project(&path) {
            tracing::warn!(target: "AtlasWorkspace", "Could not load project: {project_path}");
        }
    }

    // Register built-in tools
    for (id, name, icon) in [
        ("WorldEditor",     "World Editor",     "🌍"),
        ("ScriptEditor",    "Script Editor",    "📝"),
        ("AssetBrowser",    "Asset Browser",    "📦"),
        ("AnimationEditor", "Animation Editor", "🎬"),
        ("PatchManager",    "Patch Manager",    "🔧"),
    ] {
        shell.register_tool(WorkspaceTool {
            id: id.to_string(), display_name: name.to_string(),
            icon: icon.to_string(), active: false,
        });
    }
    shell.activate_tool("WorldEditor");

    // ── Editor context ─────────────────────────────────────────────────────────
    let editor = EditorContext::new();
    editor.init();

    // ── Engine ─────────────────────────────────────────────────────────────────
    let mut engine = Engine::new(EngineConfig {
        mode:     EngineMode::Editor,
        headless,
        ..Default::default()
    });
    engine.init_core();

    if headless {
        tracing::info!(target: "AtlasWorkspace", "Headless mode — running 1 tick then exiting");
        engine.run_headless(1);
    } else {
        tracing::info!(target: "AtlasWorkspace", "AtlasWorkspace ready ({} tools, {} panels)",
            shell.tools().len(), editor.panels.panels().len());
        // In a full build, the egui/winit event loop would start here.
        // For now, run a brief headless loop to exercise all systems.
        engine.run_headless(10);
    }

    tracing::info!(target: "AtlasWorkspace", "Shutdown complete");
}

