# Master Roadmap

This is the execution ladder. Every line is tied to a real milestone. No brainstorm clutter.

---

## Phase 0 – Canon Reset and Consolidation

**Status: Done**

- [x] Add `.gitattributes` for line-ending normalization
- [x] Tighten `.gitignore`
- [x] Gate online test dependencies (`ATLAS_ENABLE_ONLINE_DEPS`)
- [x] Rewrite README with current canon
- [x] Create Canon docs
- [x] Create Roadmap docs
- [x] Create Inventory docs
- [x] Active-path Arbiter → AtlasAI rename
- [x] Workspace bootstrap naming cleanup
- [x] GDI/OpenGL fallback marking + D3D11/DirectWrite stubs
- [x] Editor umbrella include reduction (Editor.h → EditorSharedPanels.h + EditorToolRegistry.h)
- [x] Editor inventory and consolidation plan
- [x] Archive legacy tools (ArbiterAI, SwissAgent, build_verify)
- [x] NovaForge adapter contract (IGameProjectAdapter, ProjectSystemsTool)
- [x] CONTRIBUTING.md and Docs/README.md doc index

**Success Criteria:**
- README rewritten
- Canon docs in place
- Roadmap reset
- Inventory docs created
- Stale docs archived
- Active-path naming scrub complete
- Repo hygiene rules documented

---

## Phase 1 – Workspace Core Stabilization

**Status: Done**

- [x] IHostedTool interface and ToolDescriptor
- [x] ToolRegistry — tool registration, lookup, lifecycle
- [x] PanelRegistry — shared panel registration and context binding
- [x] WorkspaceShell — composition root owning registries, managers, project adapter
- [x] Wire WorkspaceShell into EditorApp bootstrap (via Editor.h umbrella)
- [x] Project adapter loading through WorkspaceShell
- [x] Tests for WorkspaceShell, ToolRegistry, PanelRegistry (42 tests, 141 assertions)
- [x] Remove project-specific leakage from workspace core (via Phase 3 — Source/Workspace/ module is tool-agnostic)

**Success Criteria:**
- Host bootstrap is clean and deterministic
- Tool registry is rationalized
- Project adapter contract exists
- No game-specific logic in workspace core

---

## Phase 2 – AtlasUI Backend Strategy

**Status: Done**

- [x] Create backend selector contract (UIBackendSelector.h)
- [x] Mark GDI as fallback only
- [x] Add D3D11 backend stub
- [x] Add DirectWrite text backend stub (ITextBackend interface)
- [x] Isolate legacy OpenGL/GLFW paths (Compat/ subdirectory + compat markers)
- [x] Formalize backend interface split (IFrameBackend, IGeometryBackend, ITextRenderBackend, ITextureBackend — IUIBackendInterfaces.h)
- [x] Implement D3D11 backend (architecturally complete: HLSL shaders, COM handle structure, IFrameBackend+IGeometryBackend+ITextureBackend, text delegation, diagnostics)
- [x] Implement DirectWrite text backend (architecturally complete: IDWriteFactory hierarchy, glyph atlas strategy, ITextRenderBackend with FontKey cache)

**Success Criteria:**
- Backend selector contract exists ✓
- GDI explicitly fallback-only ✓
- D3D11/DirectWrite path formally targeted ✓
- Legacy compatibility paths isolated (Compat/) ✓
- IUIBackendInterfaces.h formalises the backend split ✓
- D3D11Backend implements split interfaces with full Windows COM structure ✓
- DirectWriteTextBackend implements ITextRenderBackend with full DWrite hierarchy ✓
- UIBackendSelector has priority chain and BackendCapabilities query ✓
- NF_UIBackendTests: 30+ interface contract tests ✓

---

## Phase 3 – Editor Consolidation

**Status: DONE**

> ⚠️ STOP: No new V1 stub headers or S-story test expansions.
> Stories S4–S189 produced 400+ header-only stubs. That pattern is closed.
> All S-story test files and non-core V1 stubs have been moved to Legacy/.
> Phase 3 is complete: real hosted tools, workspace/editor separation, shared panels.

- [x] Create EditorToolRegistry and EditorSharedPanels headers
- [x] Host NovaForge gameplay panels through adapter
- [x] Archive S-story stub tests (test_s4–test_s189) to Tests/Editor/Legacy/
- [x] Archive non-core V1 stub headers to Source/Editor/include/NF/Editor/Legacy/
- [x] Remove archived tests from active CMakeLists build
- [x] Implement SceneEditorTool as first real NF::IHostedTool
- [x] Implement AssetEditorTool as NF::IHostedTool
- [x] Implement MaterialEditorTool as NF::IHostedTool
- [x] Implement AnimationEditorTool as NF::IHostedTool
- [x] Implement DataEditorTool as NF::IHostedTool
- [x] Implement VisualLogicEditorTool as NF::IHostedTool
- [x] Implement BuildTool as NF::IHostedTool
- [x] Implement AtlasAITool as NF::IHostedTool
- [x] Wire all primary tools into WorkspaceShell at bootstrap
- [x] Remove one-off tools from active registry
  - [x] Archive 29 project-specific game editors to Legacy/
  - [x] Group 143 one-off editors into tool sub-directories (Scene/, Asset/, Material/, Animation/, Data/, Logic/, Build/, AI/, Infra/, ProjectSystems/)
  - [x] Create Source/Workspace/ module (NF::Workspace) as the OS-like host layer
  - [x] Decouple WorkspaceShell from hardcoded tool includes (factory-based registration)
  - [x] Create CoreToolRoster.h for primary tool registration
- [x] Extract shared panels (Outliner, Inspector, ContentBrowser) from standalone editors
  - [x] Create ISharedPanel interface in NF::Workspace
  - [x] Extend PanelRegistry with factory-based panel creation and lifecycle
  - [x] Implement 6 shared panels as ISharedPanel (ContentBrowser, ComponentInspector, Diagnostics, MemoryProfiler, PipelineMonitor, NotificationCenter)
  - [x] Register panel factories in WorkspaceShell::registerDefaultPanels()
  - [x] Add 16 tests for shared panel system (1118 total tests pass)

**Success Criteria:**
- Primary tool roster (~10 tools) all implemented as real NF::IHostedTool ✓
- All tools registered with WorkspaceShell via ToolRegistry at boot ✓
- Shared panels owned by workspace core, not duplicated per tool ✓
- NovaForge gameplay panels hosted through adapter ✓
- No new one-off standalone editor headers added to active build ✓
- Workspace shell is tool-agnostic (no hardcoded tool includes) ✓

---

## Phase 4 – AtlasAI and Codex Integration

**Status: Done**

- [x] Complete AtlasAI naming migration
- [x] Formalize broker flow (BrokerFlowController: broker→reasoner→action surface→notifications)
- [x] Wire build-log routing into AtlasAI (BuildLogRouter: log sink→classify→AtlasAI analysis)
- [x] Define Codex mirroring, validation, deduplication (SnippetPromotionRules: FNV-1a content hashing, validation limits)
- [x] Define snippet promotion rules (PromotionRule: Manual/AutoOnSave/AutoOnUse/AutoOnReview triggers)

**Success Criteria:**
- BrokerFlowController wires WorkspaceBroker→AtlasAIReasoner→AIActionSurface→NotificationSystem ✓
- BuildLogRouter captures build errors/warnings and routes to AtlasAI ✓
- CodexSnippetMirror has validation (body/title limits, tag limits) and dedup (FNV-1a) ✓
- Snippet promotion rules with 4 trigger types and configurable criteria ✓
- 67 Phase 4 tests pass (163 assertions) ✓

---

## Phase 5 – Hosted Project Support

**Status: Done**

- [x] Project loading contracts (ProjectLoadContract: state, validation, build-readiness)
- [x] ProjectRegistry — multi-project factory model, load/unload lifecycle
- [x] Build gating for hosted projects (BuildGateController: rules, blocking errors, status)
- [x] Plugin/project model for future projects (factory-based ProjectRegistry)
- [x] 63 Phase 5 tests pass (158 assertions)

**Success Criteria:**
- ProjectLoadContract captures project identity, load state, validation errors, inventory ✓
- ProjectRegistry supports factory registration, load/unload, one-active constraint ✓
- BuildGateController gates builds on contract validity and custom rules ✓
- Plugin/project model: any project registers a factory; registry is project-agnostic ✓

---

## Phase 6 – Build, Patch, and Release Pipeline

**Status: Done**

- [x] Stabilize build presets and dependency policy
  - [x] Add `ci-release-tests` configure/build/test preset (Release + tests for CI smoke-testing)
  - [x] Add `DependencyPolicy.h` — dependency tier (Required/Optional/Forbidden), acquisition source (Vendored/FetchContent/vcpkg/System), evaluation with online-dep gating, canonical workspace policy
- [x] Finalize patch apply/remove workflow
  - [x] Add `PatchApplier.h` — PatchRecord, PatchState machine, dependency-ordered apply/remove, reset-on-failure, file entry inventory
- [x] Improve repo audit tooling
  - [x] Add `RepoAuditReport.h` — programmatic audit result: pass/warn/fail/skip checks, category classification, summary counters, CI-consumable failures list
- [x] Define packaging and release path
  - [x] Add `ReleaseManifest.h` — SemanticVersion, ReleaseTarget+ReleaseArtifact, ReleaseManifestValidator with gate callables
- [x] 74 Phase 6 tests pass (187 assertions)

**Success Criteria:**
- `ci-release-tests` preset enables release+tests on CI without a separate build step ✓
- DependencyPolicy classifies and evaluates all workspace deps including ATLAS_ENABLE_ONLINE_DEPS gating ✓
- PatchApplier enforces ordered apply/remove and dependency constraints ✓
- RepoAuditReport is a typed programmatic audit consumable by CI and the workspace Diagnostics panel ✓
- ReleaseManifest + validator gates on stable version, target presence, and custom rules ✓

---

## Phase 7 – Workspace Integration Surfaces

**Status: Done**

- [x] Create `ViewportHostContract.h` — formal contract for 3D viewport surface hosting
  - [x] ViewportHandle, ViewportBounds, ViewportState, ViewportRenderMode enums
  - [x] ViewportCameraDescriptor, ViewportGridDescriptor
  - [x] ViewportSlot — live slot owned by tool with activate/pause/resume lifecycle
  - [x] ViewportHostRegistry — slot allocator (requestSlot/releaseSlot/activate/pause/setCamera/setRenderMode)
- [x] Create `TypographySystem.h` — workspace-wide typography enforcement
  - [x] FontWeight, TextRole (13 semantic roles: Heading1-3, Body/BodySmall, Label/LabelSmall, Caption, Code/CodeSmall, Data, Icon, Badge)
  - [x] TypefaceDescriptor — (family, size, weight, italic, lineHeight, letterSpacing)
  - [x] TypographyRegistry — role→descriptor map with loadDefaults/setRole/applyScale/validate
  - [x] TypographyEnforcer — validates registry: size minimums, heading hierarchy, monospace code roles
  - [x] TypographyEnforcementReport — typed violation list
- [x] Add `Tests/Workspace/test_phase7_workspace_integration.cpp` — 79 test cases / 269 assertions covering:
  - [x] FileIntakePipeline (8 tests) — enums, type detection, ingest, handler reject, batch, findById, clearPending
  - [x] DropTargetHandler (7 tests) — state names, drag enter/over/leave/drop, pipeline binding, reject unknown
  - [x] NotificationWorkflow (12 tests) — action names, WorkflowRule matches, RateLimiter throttle/reset, PriorityQueue ordering, WorkflowEngine defaults/rules/suppress
  - [x] DockTreeSerializer (8 tests) — addNode, duplicates, removeNode, kind names, TabStack roundtrip, Split roundtrip, empty fails
  - [x] PanelStateSerializer (5 tests) — set/get types, roundtrip, invalid skip, empty fails
  - [x] LayoutPersistence (10 tests) — LayoutPreset validity/modified, save/find/overwrite/load/remove/built-in/rename/autoSave
  - [x] ViewportHostContract (14 tests) — bounds, contains, state/mode names, camera validity, request/activate/pause/release/setRenderMode/setCamera/frameCount/updateBounds
  - [x] TypographySystem (15 tests) — role/weight names, descriptor validity, lineHeight, loadDefaults, getRole, setRole, applyScale, enforce pass/fail cases
- [x] Wire `NF_Phase7Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ViewportHostContract formally defines the 3D viewport hosting contract ✓
- TypographySystem defines all 13 text roles with enforcement rules ✓
- All previously untested workspace integration surfaces now have test coverage ✓
- 79 test cases pass (269 assertions) ✓

---

## Phase 8 – Runtime Wiring and First Real Tool Loop

**Status: Done**

- [x] Create `WorkspaceBootstrap.h` — typed workspace startup configuration and bootstrap sequence
  - [x] WorkspaceStartupMode (Hosted/Headless/Preview) with name helper
  - [x] WorkspaceWindowConfig — width/height/title/fullscreen/resizable, isValid(), aspectRatio()
  - [x] WorkspaceBackendChoice (Auto/D3D11/OpenGL/GDI/Null) with name helper
  - [x] WorkspaceBootstrapConfig — mode + window + backend + toolFactories + startupMessages
  - [x] WorkspaceBootstrapError + WorkspaceBootstrapResult — typed result with errorName()
  - [x] WorkspaceBootstrap — stateless runner: validates config, checks shell phase, registers factories, initializes shell, posts startup notifications
- [x] Create `WorkspaceFrameController.h` — frame pacing, dt smoothing, budget tracking
  - [x] FrameBudget — totalMs/updateMs/renderMs with isValid()
  - [x] FrameResult — smoothed dt, rawDt, wasSkipped, frameNumber
  - [x] FrameStatistics — totalFrames, fps, avgDtMs, min/max, lastUpdate/Render ms, skippedFrames, budgetUtilization()
  - [x] WorkspaceFrameController — setTargetFPS/setMaxDeltaTime/setEMAAlpha/setBudget, beginFrame/markUpdateDone/markRenderDone/endFrame, shouldSleep/sleepMs, resetStats
- [x] Add `Tests/Workspace/test_phase8_runtime_wiring.cpp` — 78 test cases / 251 assertions covering:
  - [x] WorkspaceBootstrap (15 tests) — mode/backend/error names, window config validity, headless success, invalid config, already-initialized, factory invocation, startup messages, runCount
  - [x] WorkspaceFrameController (17 tests) — defaults, setTargetFPS, ignore invalid fps, maxDt, EMA alpha, beginFrame frame numbers, dt clamping, zero dt, EMA smoothing, endFrame stats, FPS tracking, over-budget detection, wasSkipped, shouldSleep/sleepMs, resetStats, FrameBudget, budgetUtilization
  - [x] WorkspaceAppRegistry (9 tests) — appName, descriptor validity, displayLabel, register/find, duplicate rejection, invalid rejection, unregister, findByName, projectScopedApps filtering
  - [x] WorkspaceLaunchContract (9 tests) — launch/status mode names, context validity, toArgs, optional-args omission, result helpers, NullLaunchService success/AppNotFound/InvalidContext/shutdown
  - [x] ConsoleCommandBus (10 tests) — scope/argType/execResult names, command accessors, register+execute, duplicate rejection, NotFound, PermissionDenied, unregister, countByScope/hidden/enabled
  - [x] SelectionService (8 tests) — empty state, select/deselect, toggleSelect, multi-select, clearSelection, selectExclusive, version tracking, primary fallback
  - [x] EditorEventBus (10 tests) — priority names, event helpers, default state, subscribe+flush, wildcard, priority filter, suspend/resume, clearQueue, cancel subscription, non-matching topic
- [x] Wire `NF_Phase8Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- WorkspaceBootstrap provides a single testable entry point for workspace initialization ✓
- WorkspaceFrameController decouples frame pacing from the OS message loop ✓
- WorkspaceLaunchContract, AppRegistry, ConsoleCommandBus, SelectionService, EditorEventBus all have direct test coverage ✓
- 78 test cases pass (251 assertions) ✓
- Total test suite: 1521 tests passing ✓
