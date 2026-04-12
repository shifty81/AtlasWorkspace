# Project Status

Current Phase: **Phase A — Truth and Cleanup Lock** (Not Started)

## Build Status

- **Passing**
- All 25 test suites build and pass (4125 test cases)
- Tests require online fetch of Catch2 (gated behind `ATLAS_ENABLE_ONLINE_DEPS`)
- Project validator: 79/79 checks passing

## Repo Condition

### What Works (Real, Tested, Wired)

- **Infrastructure contracts:** WorkspaceShell, ToolRegistry, PanelRegistry, ProjectRegistry
- **UI rendering pipeline:** DrawList → DrawListDispatcher → UIRenderer → GDIBackend (Windows)
- **Panel chrome:** 8 core panels render labeled UI layouts with placeholder content
- **8 hosted tools:** SceneEditor, AssetEditor, MaterialEditor, AnimationEditor, DataEditor, VisualLogic, BuildTool, AtlasAI — all registered and render tool views
- **Command system:** CommandRegistry, CommandHistory, ActionMap, ShortcutRouter — infrastructure exists with 3 stub commands (Save, Command Palette, Build)
- **Event system:** WorkspaceEventBus, NotificationBus, WorkspaceEventQueue — all functional
- **Project loading:** .atlas file parsing, NovaForgeAdapter selection, panel descriptor registration
- **Asset catalog:** AssetCatalogPopulator with 50+ extension classification, catalog population
- **Settings/preferences:** SettingsStore (3-layer), PreferenceRegistry, PreferenceController
- **Undo/redo:** UndoStack, UndoManager, UndoTransaction
- **Persistence:** WorkspaceProjectFile, ProjectSerializer, AssetCatalogSerializer, SettingsStore serialization
- **Session management:** SessionManager, SessionHistory
- **Search:** SearchEngine, SearchIndex
- **Scripting:** ScriptEngine, AutomationTask
- **Plugin system:** PluginRegistry, PluginSandbox
- **Diagnostics:** DiagnosticCollector, TelemetryCollector
- **Naming:** AtlasAI canon complete — zero Arbiter references in active paths
- **Viewport contracts:** ViewportHostContract, ViewportHostRegistry, ViewportFrameLoop
- **NovaForge panel factories:** 6 panels instantiable via ProjectSystemsTool

### What Does NOT Work (Stubs, Shells, Not Wired)

- **Viewport rendering:** Shows placeholder grid, not a real scene. No 3D rendering.
- **Panel content:** All panels display chrome (titles, fake rows) but edit nothing real.
- **NovaForge panels:** Have `projectRoot` and `ready` flag, but no schemas, no data, no save targets.
- **Project load depth:** Loading .atlas selects the adapter and registers descriptors, but does not load asset registries, gameplay data, or documents.
- **PCG pipeline → editor:** Pipeline exists as file-based event system but is completely disconnected from viewport and panels.
- **D3D11 backend:** Architecturally complete headers but not executing — GDI is still the active renderer.
- **Play-In-Editor:** Does not exist. No embedded runtime, no PIE lifecycle.
- **Preferences UI:** Infrastructure exists but no user-facing preferences window.
- **Keybind UI:** No keybinding editor panel. Shortcuts are hardcoded stubs.
- **Command palette:** Ctrl+P defined but handler is empty.
- **Layout persistence to disk:** LayoutPersistenceManager exists but not wired to actual disk I/O.
- **Real document editing:** No document model. No dirty tracking in practice. No save/apply path from panels.

## Honest Summary

The repository has excellent infrastructure — contracts, registries, event buses,
serialization, undo/redo, and a clean module architecture. But the editor does not
yet edit anything. Panels render chrome. The viewport shows a placeholder. Projects
load as adapter labels. PCG is disconnected. There is no play mode.

The new roadmap (Phases A–H) addresses all of these gaps with a clear end goal:
Atlas Workspace v1.0.

## Phase Summary

| Phase | Title | Status |
|-------|-------|--------|
| Checkpoint | Foundation (Phases 0–71) | ✅ Complete |
| A | Truth and Cleanup Lock | ⬚ Not Started |
| B | Real Project Load | ⬚ Not Started |
| C | Panels Edit Real Data | ⬚ Not Started |
| D | Runtime-Backed Viewport | ⬚ Not Started |
| E | Shared PCG Preview Pipeline | ⬚ Not Started |
| F | Play-In-Editor (PIE) | ⬚ Not Started |
| G | Full Tool Wiring | ⬚ Not Started |
| H | UX Completion | ⬚ Not Started |

## Next Milestone

Phase A — Truth and Cleanup Lock:
- Archive forwarding shims
- Remove stale EditorApp references
- Rename ArbiterReasoner.cpp
- Slim main.cpp
- Correct documentation to reflect reality
