# Project Status

Current Phase: **Phase A — Truth and Cleanup Lock** (In Progress)

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
| A | Truth and Cleanup Lock | 🔄 In Progress |
| B | Real Project Load | ✅ Complete |
| C | Panels Edit Real Data | ✅ Complete |
| D | Runtime-Backed Viewport | 🔄 In Progress |
| E | Shared PCG Preview Pipeline | ⬚ Not Started |
| F | Play-In-Editor (PIE) | ⬚ Not Started |
| G | Full Tool Wiring | ⬚ Not Started |
| H | UX Completion | ⬚ Not Started |

## Phase A Progress

### A.1 — Repo Cleanup
- [x] Rename `ArbiterReasoner.cpp` → `AtlasAIReasoner.cpp` in pipeline
- [x] Archive 37 forwarding shims from `Source/Editor/include/NF/Editor/` → `Deprecated/`
- [x] Update all consumers to use canonical `NF/Workspace/` include paths
- [x] Remove stale `EditorApp` include from `Editor.h` (umbrella no longer pulls legacy app)
- [x] Update stale EditorApp comment in ViewportPanel.h
- [x] Extract `LocalProjectAdapter` from `main.cpp` → dedicated `LocalProjectAdapter.h`
- [x] Validate — all 4125 tests pass; `validate_project.sh` passes 79/79
- [ ] Add CI smoke build entry for `ATLAS_ENABLE_ONLINE_DEPS=ON`

### A.2 — Build and CI Hygiene
- [x] All test suites build and pass on clean checkout (4308 test cases, all green)

### A.3 — Documentation Correction
- [x] Update project status phase marker to "In Progress"
- [x] Mark stale phase-specific roadmap docs (01–07) as historical
- [x] Add "panel edits real data" criteria to `09_DEFINITION_OF_DONE.md`

## Phase B Progress (Complete)
- 40+ tests covering B.1–B.4, all green (test_phase_b.cpp)

## Phase C Progress (Complete)
- 58 tests, 104 assertions, all green (test_phase_c.cpp)

## Phase D Progress (In Progress)
### D.1 — NovaForge Preview Runtime Bridge ✅
- [x] `NovaForgePreviewWorld` — entity lifecycle, transform, mesh/material, selection, dirty tracking
- [x] `NovaForgePreviewRuntime` — IViewportSceneProvider, fly-camera, gizmo state, inspector data, hierarchy order

### D.2 — Scene Editor Viewport ✅
- [x] `SceneEditorTool::attachSceneProvider()` — delegates `provideScene()` to attached provider
- [x] Fly-camera: `processCameraInput()` — WASD movement + mouse-look with pitch clamp
- [x] Gizmo state: `gizmoState()` — reflects selected entity position
- [x] Inspector data: `selectedEntityProperties()` — flat property map for selected entity
- [x] Hierarchy order: `hierarchyOrder()` — BFS parent-before-child traversal

### D.3 — Asset Preview Viewport ✅
- [x] `NovaForgeAssetPreview` — IViewportSceneProvider, bind/edit/apply/revert
- [x] `AssetEditorTool::attachAssetPreviewProvider()` — delegates `provideScene()` to asset preview
- [ ] Collider/socket/anchor editing
- [ ] PCG tag and placement metadata editing

### D.4–D.5 — Material Preview + D3D11 Backend
- [ ] MaterialEditorTool test-mesh preview scene
- [ ] D3D11 backend activation
- 84 new tests, 165 assertions, all green (test_phase_d.cpp)
