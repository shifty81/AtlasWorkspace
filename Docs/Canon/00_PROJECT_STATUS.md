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
| D | Runtime-Backed Viewport | 🔄 In Progress (D.5 partial) |
| E | Shared PCG Preview Pipeline | ✅ Complete |
| F | Play-In-Editor (PIE) | 🔄 In Progress (F.1–F.4 contract layer done) |
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
- [x] All test suites build and pass on clean checkout (4469 test cases, all green)

### A.3 — Documentation Correction
- [x] Update project status phase marker to "In Progress"
- [x] Mark stale phase-specific roadmap docs (01–07) as historical
- [x] Add "panel edits real data" criteria to `09_DEFINITION_OF_DONE.md`

## Phase B Progress (Complete)
- 40+ tests covering B.1–B.4, all green (test_phase_b.cpp)

## Phase C Progress (Complete)
- 58 tests, 104 assertions, all green (test_phase_c.cpp)

## Phase D Progress (In Progress — D.1–D.4 complete, D.5 pending)
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
- [x] Collider editing: `ColliderDescriptor`, `setCollider*()` methods
- [x] Socket editing: `SocketDescriptor`, addSocket/removeSocket/setSocketTransform
- [x] Anchor editing: `AnchorDescriptor`, addAnchor/removeAnchor/setAnchorTransform
- [x] PCG metadata: `AssetPCGMetadata`, setPlacementTag/addGenerationTag/setPCGScaleRange etc.

### D.4 — Material Preview Viewport ✅
- [x] `NovaForgeMaterialPreview` — IViewportSceneProvider, bindMaterial/clearMaterial
- [x] Preview meshes: Sphere/Cube/Plane via `setPreviewMesh()`
- [x] Material parameters: setParameter/removeParameter/resetParameterToDefault/resetAllParametersToDefault
- [x] apply()/revert() baseline management; properties() for inspector
- [x] `MaterialEditorTool::attachMaterialPreviewProvider()` + `provideScene()` delegation

### D.5 — D3D11 Backend Activation (pending)
- [ ] Activate D3D11 backend as primary
- [ ] Wire DirectWrite text rendering
- 161 D completion tests (79 test cases, 155 assertions), all green (test_phase_d_completion.cpp)
- D.5 partial: `createD3D11WithDirectWrite()` factory + contract tests (NF_PhaseD5Tests), real GPU init deferred

## Phase E Progress (Complete ✅)
### E.1 — Shared NovaForge PCG Core ✅
- [x] `PCGRuleSet` — typed rule container (add/set/remove/reset/filter, dirty tracking)
- [x] `PCGGeneratorService` — stateless deterministic generator (rules + seed → placements)
- [x] `PCGDeterministicSeedContext` — universe seed + domain derivation + child contexts + pinning

### E.2 — Editor PCG Preview Service ✅
- [x] `PCGPreviewService` — bindRuleSet/forceRegenerate/autoRegenerate/stats/lastResult
- [x] `populatePreviewWorld()` — writes PCG placements into NovaForgePreviewWorld

### E.3 — PCG Rule Editing ✅
- [x] `PCGPreviewService::setRuleValue()` + `resetRules()` with auto-regen
- [x] Change callback (`setOnRegenerateCallback()`)
- [x] Domain override (`setDomainOverride()`)
- [x] `ProcGenRuleEditorPanel` — bindDocument/editRule/resetRule/resetAll/save/revert/attachPreviewService
- [x] Save-back: `save()` commits snapshot; `revert()` restores last-saved state

### E.4 — Asset PCG Metadata ✅
- [x] `NovaForgeAssetPreview::AssetPCGMetadata` — placementTag/generationTags/scale/density
- [x] `PCGPreviewService::populatePreviewWorld()` uses asset/placement tags for entity mesh tags
- [x] Event-driven: `setPlacementTagAndNotify()` / `addGenerationTagAndNotify()` / `setPCGDensityAndNotify()` etc. auto-trigger `forceRegenerate()`
- [x] `attachPCGPreviewService()` / `pcgRegenTriggerCount()` — wired event loop
- 82 Phase E core tests + 46 E-completion tests, all green

## Phase F Progress (In Progress — F.1–F.4 contract layer complete)
### F.1 — Embedded PIE Runtime ✅ (contract layer)
- [x] `PIEService` — enter/exit/pause/resume/step/reset with full state machine
- [x] `PIEState` (Stopped/Playing/Paused) + `PIEDiagnosticSeverity` + `PIEPerformanceCounters`
- [x] `PIESessionRecord` — sessionId/durationMs/totalTicks/errorCount/events
- [x] `pushDiagnostic()` / `diagnosticCount()` / `errorCount()` / `countBySeverity()`
- [x] `tickFrame()` updates perf counters (no-op when not Playing)
- [x] All lifecycle callbacks: onEnter/onExit/onPause/onResume/onStep/onReset/onDiagnostic
- [ ] Runtime runs in real viewport panel (Phase G)

### F.2 — PIE Input Mode ✅ (contract layer)
- [x] `PIEInputRouter` — routeToGame/routeToEditor + mode query
- [x] processKey/processMouseButton/processMouseMove dispatch to mode-correct sink
- [x] `isExitKey()` — escape detection in game mode (configurable `exitKeyCode`)
- [x] `modeSwitchCount` / `keyEventCount` / event counters; `onModeChange` callback
- [ ] PIE toolbar buttons wired to PIEService (Phase H)

### F.3 — External Game Launch ✅ (contract layer)
- [x] `PIEExternalLaunch` — launch/terminate/simulateExit lifecycle
- [x] `PIELaunchConfig` — executablePath/projectFilePath/args/buildConfiguration
- [x] `pushStdoutLine()` / `onStdoutLine` — console output routing
- [x] `onLaunched` / `onExited` callbacks; `launchCount` / `processId` / `lastExitCode`
- [ ] Real process launch (CreateProcess/fork+exec) — platform-specific activation (Phase G)

### F.4 — PIE Diagnostics ✅ (contract layer)
- [x] Severity-filtered diagnostics, session history capture, onDiagnostic callback
- [ ] Performance counters UI panel (Phase G/H)
- All Phase F tests: 76 test cases, all green (test_phase_f.cpp)
