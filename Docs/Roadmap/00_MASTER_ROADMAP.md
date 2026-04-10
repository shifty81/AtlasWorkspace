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

---

## Phase 9 – Asset Pipeline and Content Routing

**Status: Done**

- [x] Create `AssetCatalog.h` — authoritative asset registry
  - [x] AssetTypeTag (15 tags: Unknown/Texture/Mesh/Audio/Script/Shader/Scene/Font/Video/Archive/Project/Material/Animation/Prefab/Custom) with name helper
  - [x] AssetImportState (Unknown/Staged/Importing/Imported/Dirty/Error/Excluded) with name helper
  - [x] AssetMetadata — key-value bag (MAX_ENTRIES=64): set/get/getOr/has/remove/clear
  - [x] AssetDescriptor — id+sourcePath+catalogPath+displayName+typeTag+importState+metadata; isValid/isImported/needsReimport/extension
  - [x] AssetCatalog — add/remove/find/findByPath/contains/countByState/countByType/query/all/clear; duplicate catalogPath rejected
- [x] Create `AssetTransformer.h` — typed import step chain
  - [x] TransformStepStatus (Ok/Skip/Error) + factory helpers (ok/skip/error)
  - [x] TransformContext — assetId/sourcePath/outputPath/typeTag/progress/metadata/scratchData (setScratch/getScratch)
  - [x] TransformStep — name+fn+enabled, isValid()
  - [x] TransformChain — addStep/removeStep/enableStep/run; run aborts on Error, continues on Skip
  - [x] TransformResult — succeeded/errorStep/errorMessage/stepsRun/stepsSkipped/finalProgress
  - [x] AssetTransformer — registerChain per-type + setDefaultChain; transform() validates ctx, routes to chain, tracks totalTransforms/Succeeded/Failed
- [x] Create `ContentRouter.h` — file-type to tool routing rules
  - [x] ContentRouterPolicy (Reject/UseDefault/Prompt) with name helper
  - [x] RouteResult — matched/toolId/ruleName/needsPrompt
  - [x] RoutingRule — name/toolId/typeTag(wildcard=Unknown)/sourceFilter/priority/enabled; matches()
  - [x] ContentRouter — addRule/removeRule/enableRule/clearRules; rules sorted by priority descending; route(tag)/route(descriptor)/route(intakeItem); Reject/UseDefault/Prompt policies; routeCount/missCount
- [x] Create `AssetWatcher.h` — logical file-change detection with debounce
  - [x] ChangeType (Created/Modified/Deleted/Renamed) with name helper
  - [x] ChangeEvent — watchId/path/newPath/type/timestamp; isValid()
  - [x] WatchEntry — id/path/recursive/enabled/eventCount; isValid()
  - [x] AssetWatcher — addWatch/removeWatch/removeWatchByPath/enableWatch; notifyChanged (dedup pending); tick(nowMs, debounceMs) delivers settled events; subscribe/clearCallbacks; clearPending; totalDelivered
- [x] Add `Tests/Workspace/test_phase9_asset_pipeline.cpp` — 71 test cases / 236 assertions:
  - [x] AssetCatalog (18 tests) — type/state names, metadata, descriptor validity, add/find/findByPath, duplicate rejection, remove, setImportState/setImportError, markDirty, setMetadata, countByState/countByType, query, all, clear
  - [x] AssetTransformer (17 tests) — status names, step result factories, context validity, scratch data, step validity, chain add/remove/run, skip/error/disabled handling, transformer routing/stats, default chain, missing chain, invalid context, hasChainFor
  - [x] ContentRouter (14 tests) — policy names, rule validity/matching, wildcard/source-filter, add/route basic, Reject/UseDefault/Prompt policies, priority ordering, remove/enable, hasRule, route by descriptor, route by intake item, clearRules
  - [x] AssetWatcher (17 tests) — change type names, event validity, addWatch/isWatching, duplicate dedup, empty path, removeWatch/byPath, enableWatch, notifyChanged queuing, ignore unregistered, tick debounce, event dedup, recursive matching, non-recursive, multi-callback, clearCallbacks, clearPending, eventCount
  - [x] Integration (3 tests) — intake→route→catalog, transform chain updates catalog metadata, watcher dirties catalog entry on file change
- [x] Wire `NF_Phase9Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- AssetCatalog is the single typed registry for all workspace assets ✓
- AssetTransformer provides a testable, composable step-chain for import transforms ✓
- ContentRouter routes any asset type or intake item to the correct tool ✓
- AssetWatcher delivers debounced change events without filesystem dependencies ✓
- Integration tests verify the pipeline end-to-end ✓
- 71 test cases pass (236 assertions) ✓
- Total test suite: 1592 tests passing ✓

---

## Phase 10 – Project Persistence and Serialization

**Status: Done**

- [x] Create `WorkspaceProjectFile.h` — .atlasproject file schema
  - [x] ProjectFileVersion (major.minor.patch): toString/parse/isCompatible/current()
  - [x] ProjectFileSection — named k/v section (MAX_ENTRIES=256): set/get/getOr/has/remove/clear/entries
  - [x] WorkspaceProjectFile — root document; project identity (id/name/contentRoot/version); section management (section/findSection/hasSection/removeSection); isValid(); serialize() / static parse()
  - [x] Wire format: `#atlasproject:<version>\nproject.id=…\n[SectionName]\nkey=value\n`
- [x] Create `ProjectSerializer.h` — WorkspaceShell snapshot serialization
  - [x] WorkspaceShellSnapshot — projectId/projectName/contentRoot/activeToolId/registeredToolIds/visiblePanelIds/fileVersion; isValid()
  - [x] SerializeResult — succeeded/errorMessage; ok()/fail() factory helpers
  - [x] ProjectSerializer::serialize — writes Core+Tools+Panels sections into project file
  - [x] ProjectSerializer::deserialize — restores snapshot from Core+Tools+Panels sections
  - [x] ProjectSerializer::roundTrip — serialize→text→parse→deserialize helper
- [x] Create `AssetCatalogSerializer.h` — AssetCatalog persistence
  - [x] CatalogSerializeResult — succeeded/assetCount/errorMessage; ok(n)/fail()
  - [x] AssetCatalogSerializer::serialize — writes one record per asset into "AssetCatalog" section; pipe-delimited fields with escape (`\P` for literal `|`)
  - [x] AssetCatalogSerializer::deserialize — reads back all asset records; reconstructs descriptors and metadata
  - [x] AssetCatalogSerializer::roundTrip — helper for self-contained round-trip testing
  - [x] Metadata round-trips losslessly (asset.<n>.meta.<i>.k/v)
  - [x] Pipe characters in field values are escaped/unescaped transparently
- [x] Create `SettingsStore.h` — layered typed settings (Default < Project < User)
  - [x] SettingsLayer enum (Default/Project/User) with name helper
  - [x] set/get/getOr/getBool/getInt32/getFloat — typed read/write with layer parameter
  - [x] setDefault — convenience for populating Default layer at startup
  - [x] Layer-aware getFromLayer / hasInLayer / remove / clearLayer / countInLayer / totalCount
  - [x] addObserver/clearObservers — SettingsChangeCallback (key, value, layer)
  - [x] serializeLayer / deserializeLayer / serializeAll / deserializeAll — WorkspaceProjectFile integration using "Settings.User/Project/Default" sections
- [x] Add `Tests/Workspace/test_phase10_persistence.cpp` — 62 test cases / 200 assertions:
  - [x] WorkspaceProjectFile (17 tests): version parse/compat/current, section CRUD, identity, isValid, serialize magic, round-trip identity/sections, parse rejections, version in version struct
  - [x] ProjectSerializer (12 tests): snapshot isValid, result factories, serialize fields, Core/Tools/Panels sections, invalid snapshot rejection, deserialize identity/tools/panels, round-trip, empty tool list
  - [x] AssetCatalogSerializer (10 tests): result factories, empty catalog, section key population, deserialize descriptor, missing section, round-trip 3 assets, import state, metadata, pipe escape
  - [x] SettingsStore (20 tests): layer name strings, set/get, getOr, bool/int32/float typed accessors, layer precedence (User>Project>Default), getFromLayer, hasInLayer, remove, clearLayer, count, observer notifications, clearObservers, serializeLayer, deserializeLayer, round-trip all layers, missing section
  - [x] Integration (3 tests): full cycle (snapshot+catalog+settings serialize→text→parse→restore), version round-trip + newer-minor incompatibility, settings override precedence
- [x] Wire `NF_Phase10Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- .atlasproject file format is documented, versioned, and human-readable ✓
- WorkspaceShell state survives a full serialize/text/parse/deserialize round-trip ✓
- AssetCatalog (with metadata and import state) round-trips losslessly ✓
- SettingsStore resolves User > Project > Default with correct precedence ✓
- Integration test exercises all four components in one pipeline ✓
- 62 test cases pass (200 assertions) ✓
- Total test suite: 1654 tests passing ✓

---

## Phase 11 – Command Bus and Action System

**Status: Done**

- [x] Create `WorkspaceCommand.h` — typed command descriptor
  - [x] CommandCategory enum (File/Edit/View/Selection/Tools/Window/Help/Custom) with name helper
  - [x] CommandState struct (enabled/visible/checked) with equality operators
  - [x] WorkspaceCommand — id/label/tooltip/shortcut/iconKey/category; state getters/setters; setHandler/setUndoHandler; execute() (gated by enabled+handler); undo(); isReversible(); isValid()
- [x] Create `CommandRegistry.h` — command store with execution and hooks
  - [x] ExecuteStatus enum (Success/NotFound/Disabled/NoHandler/HandlerFailed) with name helper
  - [x] CommandExecuteResult (status/commandId; succeeded()/failed(); factory helpers)
  - [x] CommandPreHook / CommandPostHook type aliases
  - [x] CommandRegistry — registerCommand/unregisterCommand/isRegistered/count; findById/findByShortcut/findByCategory/all; setEnabled/setVisible/setChecked; execute() with pre/post hook dispatch; setPreHook/setPostHook/clearHooks; enableAll/disableAll/clear
  - [x] Hooks do NOT fire for NotFound/Disabled/NoHandler — only on actual dispatch attempt
- [x] Create `CommandHistory.h` — linear undo/redo with group support
  - [x] UndoRedoStatus enum (Success/NothingToUndo/NothingToRedo/HandlerFailed); UndoRedoResult with factory helpers
  - [x] HistoryEntry (commandId/label/undoFn/isGroupEntry)
  - [x] CommandGroup (name + sub-entries vector)
  - [x] CommandHistory — push(commandId, label, undoFn); beginGroup/endGroup/discardGroup; undo/redo; canUndo/canRedo/undoDepth/redoDepth/maxDepth; nextUndoLabel/nextRedoLabel; undoLabels(); clearHistory(); setMaxDepth()
  - [x] endGroup packs sub-entries into a single HistoryEntry whose undoFn undoes all in reverse order
  - [x] MAX_DEPTH (default 128) enforced; oldest entry trimmed; new push clears redo stack
- [x] Create `ActionBinding.h` — gesture-to-command bindings
  - [x] GestureType enum (Keyboard/Toolbar/MenuItem) with name helper
  - [x] ActionBinding struct (commandId/gestureType/gestureKey; isValid(); equality)
  - [x] ActionMap — addBinding/addKeyboardBinding/addMenuBinding/addToolbarBinding; removeBindingsForCommand/removeBinding; resolveGesture/resolveKeyboard/resolveMenu/resolveToolbar; bindingsForCommand/bindingsByType/hasBinding/count/empty/all; serialize/deserialize (pipe-delimited text); clear()
  - [x] Duplicate bindings (same commandId+type+key) rejected
  - [x] Multiple bindings per command allowed (different type or key)
- [x] Add `Tests/Workspace/test_phase11_command_bus.cpp` — 81 test cases / 207 assertions:
  - [x] WorkspaceCommand (13 tests): category name, state equality, validity, setters, default state, execute gating, undo handler
  - [x] CommandRegistry (20 tests): status names, result factories, empty registry, register/reject/duplicate/unregister, findById/findByShortcut/findByCategory, state mutation, execute outcomes, pre/post hooks, hooks not fired for NotFound, clearHooks, enableAll/disableAll, clear
  - [x] CommandHistory (18 tests): result factories, empty stack, push/reject, undo/redo, new push clears redo, nextUndo/RedoLabel, undoLabels newest-first, maxDepth trim, clearHistory, setMaxDepth, group beginGroup/endGroup, double-open rejection, empty group, discardGroup, openGroupName/Size, push-to-group defers depth
  - [x] ActionBinding/ActionMap (18 tests): gestureTypeName, ActionBinding.isValid, empty map, addKeyboard/Menu/Toolbar, duplicate rejection, multiple bindings per command, resolve*, removeBindingsForCommand, removeBinding, bindingsByType, serialize/deserialize round-trip, empty input rejection, clear
  - [x] Integration (5 tests): register→execute→history→undo, keyboard shortcut→lookup→execute, group undo collapses 3 actions in reverse, hook logging with status, actionMap serialize→deserialize→resolve→execute
- [x] Wire `NF_Phase11Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- WorkspaceCommand executes/undos via handler closures, gated by enabled flag ✓
- CommandRegistry dispatches commands and calls pre/post hooks only on actual dispatch ✓
- CommandHistory linear undo/redo with group transactions and max-depth trimming ✓
- ActionMap resolves all three gesture types; duplicate rejection; serialize/deserialize lossless ✓
- Integration tests: keyboard→command, group undo, hook logging, full serialize pipeline ✓
- 81 test cases pass (207 assertions) ✓
- Total test suite: 1735 tests passing ✓

---

## Phase 12 – Event Bus and Workspace Notifications

**Status: Done**

- [x] Create `WorkspaceEventBus.h` — workspace-level event infrastructure
  - [x] WorkspaceEventType enum (Tool/Panel/Project/Asset/Command/Selection/Layout/Notification/AI/System/Custom) with name helper
  - [x] WorkspaceEventPriority enum (Low/Normal/High/Critical) with name helper
  - [x] WorkspaceEvent — typed event descriptor: eventType/source/payload/timestampToken/priority; isValid/isHighPriority/isCritical; static make() factory
  - [x] WorkspaceEventSubscription — id/type/sourceFilter/handler/active/wildcard; matches()/deliver()/cancel(); deliveryCount tracking
  - [x] WorkspaceEventBus — subscribe/subscribeAll/unsubscribe/publish; per-type subscriber dispatch; wildcard subscriptions; find/countByType; totalPublished/totalDispatches stats; clear()
  - [x] WorkspaceEventQueue — deferred event accumulation; enqueue/drain; priority-sorted drain (Critical>High>Normal>Low); tick-based drain with configurable interval; pending()/clearQueue(); totalDrained tracking
  - [x] WsNotificationSeverity enum (Info/Success/Warning/Error/Critical) with name helper
  - [x] WorkspaceNotificationEntry — id/title/message/source/severity/timestampMs/read; markRead/isValid/isError/isCritical/isUnread
  - [x] WorkspaceNotificationBus — layered on WorkspaceEventBus; notify/info/success/warning/error/critical; markRead/markAllRead; find/unreadCount/countBySeverity/errorCount; history management (MAX_HISTORY=256); clearHistory
- [x] Add `Tests/Workspace/test_phase12_event_bus.cpp` — 50 test cases / 168 assertions:
  - [x] WorkspaceEventType (2 tests): event type names, priority names
  - [x] WorkspaceEvent (4 tests): default invalid, make factory, priority queries, empty source invalid
  - [x] WorkspaceEventBus (14 tests): empty state, subscribe, publish/dispatch, non-matching type, source filter, wildcard, unsubscribe, unknown unsubscribe, invalid publish, multiple subscribers, find by id, countByType, deliveryCount, clear
  - [x] WorkspaceEventQueue (10 tests): empty state, enqueue, reject invalid, drain, priority sort, empty drain, tick-based drain, tick empty, clearQueue, pending view, interval defaults
  - [x] WorkspaceNotificationBus (15 tests): severity names, entry validity, markRead, isError/isCritical, empty bus, notify stores history, publishes on bus, convenience helpers, markRead/markAllRead, errorCount, clearHistory, priority escalation, default source
  - [x] Integration (5 tests): multi-type dispatch, queue accumulate+drain, notification bus events, tick-based mixed priority, full pipeline
- [x] Wire `NF_Phase12Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- WorkspaceEventBus provides synchronous pub/sub with per-type dispatch and wildcard subscriptions ✓
- WorkspaceEventQueue accumulates events and drains with priority ordering (Critical first) ✓
- Tick-based drain enables frame-aligned event delivery ✓
- WorkspaceNotificationBus layers notification semantics on EventBus with history management ✓
- Error/Critical notifications auto-escalate to High/Critical bus priority ✓
- Integration tests verify the full pipeline: bus + queue + notifications ✓
- 50 test cases pass (168 assertions) ✓
- Total test suite: 1785 tests passing ✓

---

## Phase 13 – Workspace Preferences and Configuration

**Status: Done**

- [x] Create `WorkspacePreferences.h` — workspace preference infrastructure
  - [x] PreferenceCategory enum (General/Appearance/Keybindings/Editor/Build/AI/Plugin/Custom) with name helper
  - [x] PreferenceType enum (String/Bool/Int/Float) with name helper
  - [x] PreferenceEntry — key/displayName/description/defaultValue/category/type/min/max/hasRange/readOnly; isValid(); validate(); static factories (makeString/makeBool/makeInt/makeFloat)
  - [x] PreferenceRegistry — registerEntry/unregisterEntry/find/isRegistered/findByCategory/countByCategory/validate; populateDefaults(); loadWorkspaceDefaults(); MAX_ENTRIES=512
  - [x] PreferenceController — coordinated access binding Registry+SettingsStore+EventBus; set(with validation)/get/getOr/getBool/getInt/getFloat; resetToDefault/resetAll; initialize(); fires System events on change
  - [x] PreferenceSerializeResult — succeeded/entryCount/errorMessage; ok()/fail() factories
  - [x] PreferenceSerializer — serializeRegistry/deserializeRegistry to WorkspaceProjectFile "Preferences.Registry" section; roundTrip() helper
- [x] Add `Tests/Workspace/test_phase13_preferences.cpp` — 42 test cases / 157 assertions:
  - [x] PreferenceCategory/PreferenceType (2 tests): enum name strings
  - [x] PreferenceEntry (10 tests): default invalid, makeString/makeBool/makeInt/makeFloat, validate Bool/Int/Float/String, empty always valid
  - [x] PreferenceRegistry (10 tests): empty state, register+find, duplicate rejection, invalid rejection, unregister, findByCategory, countByCategory, validate delegation, populateDefaults, loadWorkspaceDefaults, clear
  - [x] PreferenceController (10 tests): set+get, reject unregistered, reject readOnly, validate before set, typed getters, resetToDefault, resetAll, EventBus on set, EventBus on reset, getOr fallback
  - [x] PreferenceSerializer (7 tests): result factories, serialize writes section, deserialize reads entries, missing section fails, roundTrip preserves entries, roundTrip preserves readOnly
  - [x] Integration (3 tests): full lifecycle, serialization round-trip, preferences + event bus + notification bus
- [x] Wire `NF_Phase13Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- PreferenceEntry validates typed values with optional range constraints ✓
- PreferenceRegistry provides centralized preference registration with category organization ✓
- PreferenceController coordinates validated access with EventBus change notifications ✓
- PreferenceSerializer round-trips registry through WorkspaceProjectFile losslessly ✓
- 13 workspace-default preferences auto-registered by loadWorkspaceDefaults ✓
- Integration tests: lifecycle, serialization, and multi-system pipeline ✓
- 42 test cases pass (157 assertions) ✓
- Total test suite: 1827 tests passing ✓
