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

---

## Phase 14 – Workspace Plugin System

**Status: Done**

- [x] Create `WorkspacePluginSystem.h` — workspace plugin infrastructure
  - [x] PluginState enum (Unloaded/Discovered/Loaded/Activated/Deactivated/Error) with name helper
  - [x] PluginCapability enum (ReadSettings/WriteSettings/RegisterTools/RegisterPanels/FileSystem/Network/EventBus/Commands) with name helper
  - [x] PluginVersion — semver with comparison operators, parse(), toString(), isValid()
  - [x] PluginDescriptor — id/displayName/author/description/version/dependencies/requiredCapabilities; isValid/dependsOn/requiresCapability
  - [x] PluginInstance — lifecycle state machine: load/activate/deactivate/unload; activate/deactivate handlers; setError
  - [x] PluginSandbox — capability-based permissions: grant/revoke/hasCapability; grantRequired(descriptor); revokeAll; countFor
  - [x] PluginRegistry — registerPlugin/unregisterPlugin/find/isRegistered; loadPlugin/activatePlugin/deactivatePlugin/unloadPlugin; areDependenciesMet (dependency check); recursive cascading deactivation; activeCount/findByState; MAX_PLUGINS=128
- [x] Add `Tests/Workspace/test_phase14_plugin_system.cpp` — 42 test cases / 127 assertions:
  - [x] PluginState/PluginCapability (2 tests): enum name strings
  - [x] PluginVersion (5 tests): make/toString, zero invalid, comparison operators, parse
  - [x] PluginDescriptor (4 tests): default invalid, valid construction, dependsOn, requiresCapability
  - [x] PluginInstance (9 tests): initial state, lifecycle (load→activate→deactivate→unload), no activate without load, handler failure→Error, handlers called, unload from active, reactivation, setError
  - [x] PluginSandbox (8 tests): empty state, grant+check, duplicate rejection, revoke, grantRequired, revokeAll, countFor, clear
  - [x] PluginRegistry (10 tests): empty state, register+find, duplicate rejection, invalid rejection, load+activate, dependency check, cascade deactivation, unregister active fails, unregister inactive, findByState, areDependenciesMet, clear
  - [x] Integration (4 tests): full lifecycle with sandbox, dependency chain A→B→C with recursive cascade, plugin handlers, version compatibility
- [x] Wire `NF_Phase14Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- PluginInstance lifecycle state machine: Discovered→Loaded→Activated→Deactivated→Unloaded ✓
- Activate/deactivate handlers called at correct lifecycle points ✓
- PluginSandbox capability-based permissions with grant/revoke/check ✓
- PluginRegistry dependency checking blocks activation of unmet dependencies ✓
- Recursive cascading deactivation (A→B→C chain) ✓
- Integration tests verify full plugin pipeline with sandbox and handlers ✓
- 42 test cases pass (127 assertions) ✓
- Total test suite: 1869 tests passing ✓

---

## Phase 15 – Workspace Diagnostics and Telemetry

**Status: Done**

- [x] Create `WorkspaceDiagnostics.h` — workspace diagnostics and telemetry infrastructure
  - [x] DiagnosticSeverity enum (Info/Warning/Error/Fatal) with name helper
  - [x] DiagnosticCategory enum (Build/Asset/Plugin/Project/Tool/Render/Performance/IO/Network/System/Custom) with name helper
  - [x] DiagnosticEntry — structured diagnostic record (id/category/severity/source/message/detail/timestampMs/acknowledged); isValid/isError; equality
  - [x] DiagnosticCollector — submit/submitInfo/submitWarning/submitError; query (findById/findByCategory/findBySeverity/findBySource); countBySeverity/countByCategory/errorCount/unacknowledgedCount/hasErrors; acknowledge/acknowledgeAll; observer callbacks; clear; MAX_ENTRIES=4096
  - [x] TelemetryEventType enum (FeatureUsage/Performance/Error/Navigation/Session/Command/Asset/Plugin/Custom) with name helper
  - [x] TelemetryEvent — name/type/source/timestampMs/durationMs; Property bag (setProperty/getProperty/hasProperty, MAX_PROPERTIES=32); isValid
  - [x] TelemetryCollector — session lifecycle (beginSession/endSession/isActive); record/recordFeature/recordPerformance/recordError; query (findByType/findBySource/findByName/countByType); observer callbacks; clear; MAX_EVENTS=8192
  - [x] DiagnosticSnapshot — point-in-time capture of DiagnosticCollector state (total/info/warning/error/fatal/unacknowledged counts)
  - [x] TelemetrySnapshot — point-in-time capture of TelemetryCollector state (session/active/total/feature/perf/error counts)
- [x] Add `Tests/Workspace/test_phase15_diagnostics.cpp` — 52 test cases / 200 assertions:
  - [x] Enum name strings (3 tests): severity, category, telemetry event type
  - [x] DiagnosticEntry (5 tests): default invalid, valid construction, isError for Error/Fatal, equality, validation rules
  - [x] DiagnosticCollector (16 tests): empty state, submit/count, reject invalid, findById, findByCategory, findBySeverity, findBySource, countBySeverity/countByCategory, hasErrors/errorCount, acknowledge, acknowledgeAll, observer, clearObservers, clear, all
  - [x] TelemetryEvent (5 tests): default invalid, valid construction, property bag, property overwrite, reject empty key, properties()
  - [x] TelemetryCollector (15 tests): inactive state, beginSession/endSession, reject inactive, reject invalid, record/count, findByType, findBySource, findByName, countByType, observer, clearObservers, clear, all, session restart, performance duration
  - [x] DiagnosticSnapshot (2 tests): capture with entries, empty collector
  - [x] TelemetrySnapshot (2 tests): capture with events, inactive collector
  - [x] Integration (4 tests): diagnostic→telemetry wiring, snapshot accuracy, full lifecycle with acknowledge, session restart
- [x] Wire `NF_Phase15Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- DiagnosticEntry provides structured diagnostic records with severity/category classification ✓
- DiagnosticCollector accumulates and queries diagnostics with filtering and acknowledgment ✓
- TelemetryEvent supports property bags and typed event classification ✓
- TelemetryCollector provides session-scoped telemetry accumulation ✓
- Snapshot types capture point-in-time state for UI display ✓
- Integration tests verify diagnostic→telemetry wiring and lifecycle ✓
- 52 test cases pass (200 assertions) ✓
- Total test suite: 1921 tests passing ✓

---

## Phase 16 – Workspace Scripting and Automation

**Status: Done**

- [x] Create `WorkspaceScripting.h` — workspace scripting and automation infrastructure
  - [x] ScriptParamType enum (Void/Bool/Int/Float/String/Path/Id/Custom) with name helper
  - [x] ScriptParam — typed parameter descriptor (name/type/defaultValue/required); isValid; equality
  - [x] ScriptBinding — typed function binding: name/description/params/handler/returnType; addParam/findParam/requiredParamCount; invoke; isValid
  - [x] ScriptContext — execution environment: variable scope (set/get/getOr/has/remove/clear, MAX_VARIABLES=512); output capture (append/clear); error state (set/has/clear); full reset
  - [x] ScriptExecStatus enum (Success/NotFound/InvalidArgs/HandlerFailed/BindingInvalid) with name helper
  - [x] ScriptExecResult — status/bindingId/errorMessage; succeeded/failed; ok/fail factories
  - [x] ScriptEngine — registerBinding/unregisterBinding/isRegistered/findBinding/allBindings; execute with arg validation and handler dispatch; totalExecutions/successfulExecutions; clear; MAX_BINDINGS=1024
  - [x] AutomationStepStatus enum (Pending/Running/Succeeded/Failed/Skipped) with name helper
  - [x] AutomationStep — named step with handler, status tracking, reset
  - [x] AutomationTaskState enum (Idle/Running/Completed/Failed/Aborted) with name helper
  - [x] AutomationTask — named sequence: addStep/removeStep/enableStep/findStep; run with abort-on-failure; step counters (run/succeeded/failed/skipped); reset; MAX_STEPS=256
- [x] Add `Tests/Workspace/test_phase16_scripting.cpp` — 62 test cases / 194 assertions:
  - [x] Enum name strings (4 tests): paramType, execStatus, stepStatus, taskState
  - [x] ScriptParam (4 tests): default invalid, valid construction, void invalid, equality
  - [x] ScriptBinding (9 tests): default invalid, valid with handler, addParam/findParam, duplicate rejection, invalid rejection, invoke, invoke without handler, returnType, params()
  - [x] ScriptContext (11 tests): empty state, set/get/has, missing key, getOr, overwrite, empty key rejection, remove, clearVariables, output, error state, reset
  - [x] ScriptExecResult (2 tests): ok/fail factories
  - [x] ScriptEngine (12 tests): empty, register/find, duplicate rejection, invalid rejection, unregister, execute success, NotFound, HandlerFailed, InvalidArgs, sufficient args, allBindings, clear
  - [x] AutomationStep (3 tests): default invalid, valid construction, reset
  - [x] AutomationTask (13 tests): default state, addStep/findStep, duplicate rejection, invalid rejection, removeStep, enableStep, run all succeed, abort on failure, continue on failure, skip disabled, reset, steps(), run empty
  - [x] Integration (4 tests): engine+context output, automation+engine pipeline, abort on engine failure, context variable persistence
- [x] Wire `NF_Phase16Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ScriptBinding provides typed function descriptors with parameter validation ✓
- ScriptContext provides isolated execution environment with variable scope ✓
- ScriptEngine dispatches bindings with arg validation and error handling ✓
- AutomationTask executes step sequences with abort-on-failure and step skip support ✓
- Integration tests verify engine→context→task pipeline end-to-end ✓
- 62 test cases pass (194 assertions) ✓
- Total test suite: 1983 tests passing ✓

---

## Phase 17 – Workspace Search and Indexing

**Status: Done**

- [x] Create `WorkspaceSearch.h` — workspace search and indexing infrastructure
  - [x] SearchScope enum (All/Project/Assets/Tools/Panels/Commands/Settings/Plugins/Scripts/Custom) with name helper
  - [x] SearchResultType enum (File/Asset/Tool/Panel/Command/Setting/Plugin/Script/Text/Symbol/Custom) with name helper
  - [x] SearchMatchKind enum (Exact/Prefix/Contains/Fuzzy) with name helper
  - [x] SearchQuery — typed query: text/scope/caseSensitive/maxResults; type filters (add/has/clear); sourceFilter; equality; isValid
  - [x] SearchResult — ranked result: id/title/description/source/context/type/matchKind/score/matchStart/matchLen; isValid; sorted by score descending; equality by id+source
  - [x] SearchIndex — in-memory content index: Entry (id/title/content/description/type); addEntry/removeEntry/updateEntry/findEntry; query with exact/prefix/contains/content matching; case-insensitive by default; type filter; maxResults; sorted results; clear; MAX_ENTRIES=16384
  - [x] SearchEngine — registerIndex/unregisterIndex/isRegistered/findIndex; cross-index search with scope filter and source filter; maxResults enforcement; totalSearches/totalResults/totalEntries stats; allIndices; clear; MAX_INDICES=64
- [x] Add `Tests/Workspace/test_phase17_search.cpp` — 51 test cases / 164 assertions:
  - [x] Enum name strings (3 tests): scope, resultType, matchKind
  - [x] SearchQuery (7 tests): default invalid, valid construction, case sensitivity, maxResults, type filters with duplicate rejection, source filter, equality
  - [x] SearchResult (4 tests): default invalid, valid construction, sorting by score, equality by id+source
  - [x] SearchIndex (17 tests): default state, unnamed invalid, addEntry/findEntry, duplicate rejection, invalid rejection, removeEntry, updateEntry, entries(), exact match, prefix match, title contains, content match, case insensitive, case sensitive, no match, type filter, invalid query, maxResults, sorted results, clear
  - [x] SearchEngine (12 tests): empty state, register/find, duplicate rejection, invalid rejection, unregister, cross-index search, scope filter, source filter, invalid query, maxResults across indices, totalEntries, allIndices, clear
  - [x] Integration (4 tests): multi-index ranking, add-after-register, scope-filtered mixed indices, statistics accumulation
- [x] Wire `NF_Phase17Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- SearchQuery provides typed queries with scope/type/source filters ✓
- SearchResult ranks matches with Exact>Prefix>Contains>Content scoring ✓
- SearchIndex provides in-memory content indexing with case-insensitive search ✓
- SearchEngine dispatches cross-index queries with scope and source filtering ✓
- Integration tests verify multi-index ranking and scoped search pipelines ✓
- 51 test cases pass (164 assertions) ✓
- Total test suite: 2034 tests passing ✓

---

## Phase 18 – Workspace Undo/Redo Stack

**Status: Done**

- [x] Create `WorkspaceUndoRedo.h` — workspace undo/redo infrastructure
  - [x] UndoActionType enum (Generic/Property/Create/Delete/Move/Transform/Reparent/Command/Batch/Custom) with name helper
  - [x] UndoAction — reversible action: label/type/doHandler/undoHandler/targetId; execute/undo; isValid; equality
  - [x] UndoTransaction — grouped action sequence: label/addAction/actions/actionCount; execute (with rollback on failure); undo (reverse order); MAX_ACTIONS=256
  - [x] UndoStack — linear undo/redo: push/pushTransaction; undo/redo; canUndo/canRedo; nextUndoLabel/nextRedoLabel; undoLabels/redoLabels; beginTransaction/addToTransaction/commitTransaction/discardTransaction; depth/undoDepth/redoDepth; isDirty/markClean; maxDepth with trim; statistics (totalPushes/Undos/Redos); clear; DEFAULT_MAX_DEPTH=128
  - [x] UndoManager — workspace-scoped: registerStack/unregisterStack/setActiveStack/findStack; push/undo/redo/canUndo/canRedo on active stack; observer callbacks (addObserver/removeObserver/clearObservers); stackNames; clear; MAX_STACKS=64; MAX_OBSERVERS=32
- [x] Add `Tests/Workspace/test_phase18_undo_redo.cpp` — 45 test cases / 189 assertions:
  - [x] Enum name strings (1 test): undoActionType
  - [x] UndoAction (5 tests): default invalid, valid construction, execute/undo, without handler fails, targetId, equality
  - [x] UndoTransaction (6 tests): default state, valid construction, addAction, reject invalid, execute all, undo reverse order, execute rollback on failure
  - [x] UndoStack (15 tests): empty state, push/undo, redo, push clears redo, reject invalid, labels, maxDepth trim, dirty/markClean, transaction grouping, transaction atomic undo, reject double begin, discard transaction, commit empty fails, statistics, clear, undoDepth/redoDepth
  - [x] UndoManager (10 tests): empty state, register/find, reject duplicate, reject empty name, unregister, set active, push/undo/redo, stackNames, observers, clear, push without stack fails
  - [x] Integration (4 tests): multi-step property undo, transaction atomic undo with manager, multi-stack manager, observer notifications across operations
- [x] Wire `NF_Phase18Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- UndoAction provides reversible do/undo handlers with type classification ✓
- UndoTransaction groups actions for atomic execute/undo with rollback on failure ✓
- UndoStack provides linear undo/redo with transaction grouping and dirty tracking ✓
- UndoManager provides workspace-scoped multi-stack undo with observer notifications ✓
- Integration tests verify multi-step undo, atomic transactions, multi-stack isolation, and observer logging ✓
- 45 test cases pass (189 assertions) ✓
- Total test suite: 2079 tests passing ✓

---

## Phase 19 – Workspace Session Management

**Status: Done**

- [x] Create `WorkspaceSession.h` — workspace session lifecycle infrastructure
  - [x] SessionState enum (Idle/Starting/Running/Saving/Closing/Closed) with `sessionStateName()` helper
  - [x] RecentItem — path/label/type/timestamp; `isValid()` (path non-empty); equality by path
  - [x] SessionRecord — id/name/state/startTime/endTime; `addTool`/`hasTool`; `duration()` (endTime-startTime or 0); equality by id
  - [x] SessionHistory — `addItem` with front-dedup by path (MAX_ITEMS=64); `removeItem`/`findItem`; `addRecord`/`findRecord` (MAX_RECORDS=32); `clear()`
  - [x] SessionManager — `start`/`stop`/`save` lifecycle; `currentRecord`/`isRunning`; `addRecentItem`/`recentItems`/`clearRecent`; `history()`; observer callbacks (addObserver/removeObserver/clearObservers, MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase19_session.cpp` — 40 test cases / 101 assertions:
  - [x] Enum name strings (1 test): all SessionState values
  - [x] RecentItem (3 tests): default invalid, valid construction, equality by path
  - [x] SessionRecord (6 tests): default invalid, valid construction, addTool/hasTool, duration, equality, state field
  - [x] SessionHistory (10 tests): empty state, addItem/findItem, dedup moves to front, invalid reject, removeItem, addRecord/findRecord, invalid record reject, MAX_ITEMS enforcement, clear
  - [x] SessionManager (15 tests): initial state, start, start-while-running fails, stop, stop-while-idle fails, save-while-running, save-while-idle fails, isRunning, currentRecord name, recentItems, clearRecent, history after stop, observer on start, observer on stop, removeObserver, clearObservers
  - [x] Integration (5 tests): full lifecycle, multiple sessions, recent item dedup, observer all states, name preserved in record
- [x] Wire `NF_Phase19Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- SessionState provides lifecycle classification with name helpers ✓
- RecentItem provides path-keyed items with front-dedup ✓
- SessionRecord tracks tools and duration with id-based equality ✓
- SessionHistory manages capped items and records ✓
- SessionManager provides start/stop/save lifecycle with observer notifications ✓
- Integration tests verify full lifecycle, multi-session history, and observer tracking ✓
- 40 test cases pass (101 assertions) ✓
- Total test suite: 2119 tests passing ✓

---

## Phase 20 – Workspace Clipboard System

**Status: Done**

- [x] Create `WorkspaceClipboard.h` — workspace clipboard infrastructure
  - [x] ClipboardFormat enum (None/Text/RichText/Path/EntityId/JsonBlob/Binary/Custom) with `clipboardFormatName()` helper
  - [x] ClipboardEntry — format/data/timestamp; `isValid()` (format != None); `isEmpty()` (data empty); equality (format+data)
  - [x] ClipboardBuffer — newest-first ring (push_front); `push`/`peek`/`peekAt`/`pop`/`count`/`capacity`/`clear`; capacity capped at MAX_SLOTS=32
  - [x] ClipboardChannel — named buffer wrapper: `push`/`peek`/`pop`/`count`/`clear`; `isValid()` (name non-empty)
  - [x] ClipboardManager — `registerChannel`/`unregisterChannel`/`isRegistered`/`findChannel` (MAX_CHANNELS=16); `push`/`peek`/`pop`; typed helpers `copyText`/`copyPath`/`copyEntity`/`copyJson`; `allChannels`/`clear`; observer callbacks on push (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase20_clipboard.cpp` — 44 test cases / 132 assertions:
  - [x] Enum name strings (1 test): all ClipboardFormat values
  - [x] ClipboardEntry (4 tests): default invalid, valid text, isEmpty, equality
  - [x] ClipboardBuffer (10 tests): empty state, push/peek, invalid reject, pop, peekAt, count, capacity enforcement, clear, push after clear, newest-first order
  - [x] ClipboardChannel (5 tests): default invalid, valid construction, push/peek, pop, count
  - [x] ClipboardManager (16 tests): empty state, registerChannel, duplicate reject, empty name reject, unregisterChannel, isRegistered, findChannel, push/peek, pop, copyText, copyPath, copyEntity, copyJson, allChannels, clear, push-unknown fails
  - [x] Integration (8 tests): multi-channel isolation, lifecycle, observer notification, removeObserver, copyJson round-trip, capacity drop, multi-format, clearObservers
- [x] Wire `NF_Phase20Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ClipboardFormat provides typed format classification with name helpers ✓
- ClipboardBuffer provides newest-first ring with capacity enforcement ✓
- ClipboardChannel provides named buffer wrappers ✓
- ClipboardManager provides multi-channel clipboard with typed copy helpers and observers ✓
- Integration tests verify channel isolation, capacity behavior, and observer pipelines ✓
- 44 test cases pass (132 assertions) ✓
- Total test suite: 2163 tests passing ✓

---

## Phase 21 – Workspace Focus and Context Tracking

**Status: Done**

- [x] Create `WorkspaceFocus.h` — workspace focus and context tracking infrastructure
  - [x] FocusLayer enum (Background/Base/Overlay/Modal/Popup) with `focusLayerName()` helper
  - [x] FocusTarget — id/displayName/panelId/toolId/layer; `isValid()` (id non-empty); equality by id
  - [x] FocusRecord — target/timestamp/gained; `isValid()` (target.isValid())
  - [x] FocusStack — `push`/`pop`/`current`/`depth`/`hasTarget`/`clear` (MAX_DEPTH=64); chronological history (MAX_HISTORY=256) with gain/lose records on push/pop; `clearHistory()`
  - [x] FocusManager — `registerTarget`/`unregisterTarget`/`isRegistered`/`findTarget` (MAX_TARGETS=256); `requestFocus`/`releaseFocus`/`currentFocus`/`canFocus`; `allTargets`/`stack`/`clear`; observer callbacks (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase21_focus.cpp` — 47 test cases / 120 assertions:
  - [x] Enum name strings (1 test): all FocusLayer values
  - [x] FocusTarget (4 tests): default invalid, valid all fields, equality by id, layer field
  - [x] FocusRecord (3 tests): default invalid, valid gained, valid lost
  - [x] FocusStack (12 tests): empty state, push/current, invalid reject, pop, depth, hasTarget, multiple layers, pop restores previous, clear, history on push, history on pop, clearHistory, MAX_DEPTH enforcement
  - [x] FocusManager (18 tests): empty state, registerTarget, duplicate reject, invalid reject, unregisterTarget, isRegistered, findTarget, requestFocus, requestFocus-unknown fails, releaseFocus, releaseFocus-non-current fails, currentFocus, canFocus, allTargets, clear, observer on request, observer on release, removeObserver
  - [x] Integration (8 tests): multi-target sequence, request+release lifecycle, observer chain, modal isolation, allTargets after unregister, canFocus after request, history accumulates, clearObservers
- [x] Wire `NF_Phase21Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- FocusLayer provides layered classification with name helpers ✓
- FocusTarget provides id-based equality with layer fields ✓
- FocusRecord captures timestamped gain/lose events ✓
- FocusStack provides push/pop management with chronological history ✓
- FocusManager provides workspace-scoped focus lifecycle with observer notifications ✓
- Integration tests verify multi-target sequences, modal isolation, and observer tracking ✓
- 47 test cases pass (120 assertions) ✓
- Total test suite: 2210 tests passing ✓

---

## Phase 22 – Workspace Drag and Drop System

**Status: Done**

- [x] Create `WorkspaceDragDrop.h` — workspace drag-and-drop coordination
  - [x] DragPayloadType enum (None/Text/Path/Asset/Entity/Json/Custom) with `dragPayloadTypeName()` helper
  - [x] DragPayload — type + content string; `isValid()`; equality
  - [x] DragSessionState enum (Idle/Active/Hovering/Dropped/Cancelled) with `dragSessionStateName()` helper
  - [x] DragSession — lifecycle state machine: `begin()`/`setHovering()`/`drop()`/`cancel()`/`reset()`; `isActive()`/`isCompleted()`; payload/sourceZoneId/hoverZoneId accessors
  - [x] DropZone — id/label + accepted-type bitmask; `accepts(DragPayloadType)`; `tryAccept(DragSession&)`; `lastAccepted()`/`acceptCount()`/`clear()`
  - [x] DragDropManager — `registerZone`/`unregisterZone`/`findZone`/`allZoneIds` (MAX_ZONES=64); `beginDrag`/`cancelDrag`/`commitDrop`; `hasActiveSession`/`activeSession`/`dropCount`; observer callbacks (MAX_OBSERVERS=16); `clear`
- [x] Add `Tests/Workspace/test_phase22_drag_drop.cpp` — 44 test cases:
  - [x] DragPayloadType enum (1 test): all 7 values
  - [x] DragPayload (5 tests): default invalid, valid text, None+content invalid, valid type+empty invalid, equality
  - [x] DragSessionState enum (1 test): all 5 values
  - [x] DragSession (10 tests): default Idle, begin→Active, begin fails invalid payload, begin fails if active, setHovering, drop from Active, drop from Hovering, cancel from Active, cancel from Dropped fails, reset
  - [x] DropZone (7 tests): default invalid, valid construction, accepts mask, tryAccept matching, tryAccept rejects incompatible, tryAccept rejects inactive, clear
  - [x] DragDropManager (13 tests): empty state, registerZone, duplicate reject, invalid reject, unregisterZone, unregister unknown fails, beginDrag, beginDrag fails active, cancelDrag, cancelDrag no session fails, commitDrop, commitDrop unknown zone, commitDrop incompatible type, observer on begin, observer on cancel, removeObserver, clear
  - [x] Integration (6 tests): full pipeline, multiple zones type isolation, cancel no dropCount, sequential drags, allZoneIds, clearObservers
- [x] Wire `NF_Phase22Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- DragPayloadType provides typed format classification with name helpers ✓
- DragSession provides Idle→Active→Hovering→Dropped/Cancelled state machine ✓
- DropZone provides bitmask-based type filtering with tryAccept ✓
- DragDropManager orchestrates sessions with zone registry and observers ✓
- Integration tests verify pipeline, type isolation, and sequential drags ✓
- 44 test cases pass ✓
- Total test suite: ~2254 tests passing ✓

---

## Phase 23 – Workspace Hotkey Manager

**Status: Done**

- [x] Create `WorkspaceHotkeys.h` — workspace keyboard shortcut management
  - [x] ModifierFlags bitmask enum (None/Ctrl/Alt/Shift/Meta) with `|`/`&` operators, `hasModifier()`, `modifierFlagsString()`
  - [x] HotkeyChord — modifiers + key string; `toString()`; `isValid()`; equality
  - [x] HotkeyBinding — id + chord + commandId + scopeId + enabled; `isValid()`; equality by id
  - [x] HotkeyConflict — bindingIdA + bindingIdB + chord + scopeId; `isValid()`
  - [x] HotkeyManager — `registerBinding`/`unregisterBinding`/`isRegistered`/`findById` (MAX_BINDINGS=512); `findByChord` (scope-exact then global fallback); `findByCommand`; `detectConflicts`; `enableBinding`/`disableBinding`; `activate` (dispatches observers); `allBindingIds`/`bindingCount`/`clear`; observer callbacks (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase23_hotkeys.cpp` — 40 test cases:
  - [x] ModifierFlags (5 tests): None string, Ctrl string, Ctrl+Shift string, hasModifier, all four bits
  - [x] HotkeyChord (6 tests): default invalid, valid key-only, toString with modifiers, toString Ctrl+Shift+Z, equality
  - [x] HotkeyBinding (5 tests): default invalid, valid construction, invalid without id, invalid without commandId, equality by id
  - [x] HotkeyConflict (2 tests): default invalid, valid construction
  - [x] HotkeyManager (18 tests): empty state, register, duplicate reject, invalid reject, unregister, unregister unknown, findById, findByChord global, findByChord scope-exact, findByChord global fallback, findByCommand, detectConflicts, no conflict different scopes, enable/disable, activate observer, activate unknown fails, removeObserver, allBindingIds, clear
  - [x] Integration (6 tests): full dispatch pipeline, scope isolation, multi-conflict detection, disabled not activated, clearObservers, multiple observers
- [x] Wire `NF_Phase23Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ModifierFlags provides composable bitmask with string helpers ✓
- HotkeyChord provides chord identity with toString and equality ✓
- HotkeyBinding maps chord to command with scope and enabled state ✓
- HotkeyManager provides scoped lookup with global fallback and conflict detection ✓
- Integration tests verify dispatch, scope isolation, and observer pipelines ✓
- 40 test cases pass ✓
- Total test suite: ~2294 tests passing ✓

---

## Phase 24 – Workspace Tooltip and Help System

**Status: Done**

- [x] Create `WorkspaceTooltip.h` — workspace tooltip lifecycle and content management
  - [x] TooltipTrigger enum (Hover/Focus/Manual) with `tooltipTriggerName()` helper
  - [x] TooltipPosition enum (Auto/Top/Bottom/Left/Right) with `tooltipPositionName()` helper
  - [x] TooltipEntry — id + title + body + targetElementId + trigger + position + enabled; `isValid()` (id + body non-empty); equality by id
  - [x] TooltipState — entryId + visible + showTimestamp; `isValid()` (entryId non-empty)
  - [x] TooltipManager — `registerTooltip`/`unregisterTooltip`/`isRegistered`/`findTooltip` (MAX_TOOLTIPS=256); `show`/`hide`/`hideAll`; `isVisible`/`currentVisible`; `enableTooltip`/`disableTooltip`; `allTooltipIds`/`tooltipCount`/`clear`; observer callbacks on show/hide (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase24_tooltip.cpp` — 43 test cases:
  - [x] TooltipTrigger enum (1 test): all 3 values
  - [x] TooltipPosition enum (1 test): all 5 values
  - [x] TooltipEntry (5 tests): default invalid, valid all fields, invalid without id, invalid without body, equality by id
  - [x] TooltipState (2 tests): default invalid, valid with entryId
  - [x] TooltipManager (26 tests): empty state, register, duplicate reject, invalid reject, unregister, unregister unknown, findTooltip, show, show unknown fails, show disabled fails, hide, hide non-visible fails, show second hides first, hideAll, enable/disable, disable hides visible, unregister hides visible, observer on show, observer on hide, removeObserver, allTooltipIds, clear
  - [x] Integration (8 tests): full pipeline, multiple one-at-a-time, observer for auto-replaced tooltip, hideAll fires observer, disabled re-enable, clearObservers, showTimestamp increments
- [x] Wire `NF_Phase24Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- TooltipTrigger and TooltipPosition provide typed enum classification with name helpers ✓
- TooltipEntry provides content metadata with id-based equality ✓
- TooltipState tracks current visibility with monotonic timestamp ✓
- TooltipManager enforces single-visible constraint with enable/disable and observer notifications ✓
- Integration tests verify pipeline, auto-hide, observer sequencing, and timestamp ordering ✓
- 43 test cases pass ✓
- Total test suite: ~2337 tests passing ✓

---

## Phase 25 – Workspace Status Bar System

**Status: Done**

- [x] Create `WorkspaceStatusBar.h` — workspace status bar item management
  - [x] StatusBarSide enum (Left/Center/Right) with `statusBarSideName()` helper
  - [x] StatusBarItem — id + label + tooltip + icon + priority + enabled; `isValid()`; equality by id
  - [x] StatusBarSection — ordered priority-sorted collection (MAX_ITEMS=64); `add`/`remove`/`update`/`find`/`contains`/`count`/`empty`/`items`/`clear`; stable-sort by priority
  - [x] StatusBarManager — three-section registry (Left/Center/Right); `addItem`/`removeItem`/`updateItem`/`findItem`/`contains`/`sectionOf`; `enableItem`/`disableItem`; `clear`; observer callbacks on change (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase25_status_bar.cpp` — 33 test cases / 80 assertions:
  - [x] StatusBarSide enum (1 test): all 3 values
  - [x] StatusBarItem (4 tests): default invalid, valid construction, invalid without id, equality by id
  - [x] StatusBarSection (11 tests): empty state, add, duplicate fails, invalid rejected, remove, remove unknown fails, find, priority sorting, update re-sorts, update unknown fails, clear
  - [x] StatusBarManager (12 tests): addItem left, addItem center+right, removeItem, removeItem unknown, updateItem, findItem, enable/disable, sectionOf, observer on add, observer on remove, removeObserver, clear
  - [x] Integration (5 tests): full pipeline all three sides, priority sorting preserved, update+observer, clearObservers, multiple observers
- [x] Wire `NF_Phase25Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- StatusBarSide provides three placement zones with name helpers ✓
- StatusBarSection maintains stable priority order on add/update ✓
- StatusBarManager routes items into three independent sections ✓
- Observer notifies on every structural change (add/remove/update/enable) ✓
- 33 test cases pass (80 assertions) ✓
- Total test suite: ~2370 tests passing ✓

---

## Phase 26 – Workspace Context Menu System

**Status: Done**

- [x] Create `WorkspaceContextMenu.h` — workspace context menu definition and lifecycle
  - [x] MenuItemKind enum (Action/Separator/Submenu) with `menuItemKindName()` helper
  - [x] ContextMenuItem — id + label + kind + enabled + shortcut + icon; `isValid()` (id non-empty; label required for non-Separator); `separator()` factory; equality by id
  - [x] ContextMenu — id + ordered item list (MAX_ITEMS=128); `addItem`/`removeItem`/`updateItem`/`findItem`/`contains`/`itemCount`/`empty`/`items`/`clear`; `attachSubmenu`/`findSubmenu`
  - [x] ContextMenuManager — named menu registry (MAX_MENUS=64); `registerMenu`/`unregisterMenu`/`isRegistered`/`findMenu`/`allMenuIds`; `openMenu`/`closeMenu`/`isOpen`/`hasOpenMenu`/`openMenuId` (one-open constraint, auto-close on second open); `activateItem` (action-only, enabled-only); `clear`; action observers + lifecycle observers (MAX_OBSERVERS=16 each); `removeObserver`/`clearObservers`
- [x] Add `Tests/Workspace/test_phase26_context_menu.cpp` — 46 test cases / 98 assertions:
  - [x] MenuItemKind enum (1 test): all 3 values
  - [x] ContextMenuItem (7 tests): default invalid, valid action, invalid without id, invalid action without label, separator valid, separator invalid, equality by id
  - [x] ContextMenu (14 tests): default invalid, valid construction, addItem, duplicate fails, invalid rejected, removeItem, removeItem unknown, updateItem, updateItem unknown, findItem, separator added, attachSubmenu, attachSubmenu fails non-Submenu kind, clear
  - [x] ContextMenuManager (18 tests): empty state, register, duplicate fails, invalid rejected, unregister, unregister unknown, openMenu, openMenu unknown, openMenu same twice, closeMenu, closeMenu nothing open, opening second closes first, unregister closes open, activateItem observer, disabled item fails, separator fails, lifecycle observer, removeObserver, allMenuIds, clear
  - [x] Integration (5 tests): full pipeline, submenu tree preserved, second open auto-closes first with events, clearObservers
- [x] Wire `NF_Phase26Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- MenuItemKind provides typed classification with name helpers ✓
- ContextMenuItem provides id-based equality with separator factory ✓
- ContextMenu maintains ordered item list with submenu tree support ✓
- ContextMenuManager enforces single-open constraint with auto-close on second open ✓
- Action/lifecycle observer pipelines independently notified ✓
- 46 test cases pass (98 assertions) ✓
- Total test suite: ~2416 tests passing ✓

---

## Phase 27 – Workspace Badge and Icon Registry

**Status: Done**

- [x] Create `WorkspaceBadge.h` — workspace badge overlay and icon asset management
  - [x] BadgeKind enum (Info/Warning/Error/Success/Count/Custom) with `badgeKindName()` helper
  - [x] Badge — id + targetId + kind + label + count + visible; `isValid()` (id + targetId non-empty); equality by id
  - [x] BadgeRegistry — `attach`/`detach`/`update`/`isAttached`/`findById`/`findByTarget`/`findByKind` (MAX_BADGES=512); `setVisible`/`setCount` (Count-kind only); `totalCount`/`empty`/`clear`; observer callbacks (MAX_OBSERVERS=16)
  - [x] IconEntry — id + path + alias + category + size; `isValid()` (id + path non-empty); equality by id
  - [x] IconRegistry — `registerIcon`/`unregisterIcon`/`isRegistered`/`findById`/`findByAlias`/`find` (id-first then alias); `findByCategory`; `allIds`/`count`/`empty`/`clear` (MAX_ICONS=1024)
- [x] Add `Tests/Workspace/test_phase27_badge.cpp` — 47 test cases / 104 assertions:
  - [x] BadgeKind enum (1 test): all 6 values
  - [x] Badge (6 tests): default invalid, valid construction, invalid without id, invalid without targetId, equality by id, Count kind with numeric count
  - [x] BadgeRegistry (17 tests): empty state, attach, duplicate fails, invalid rejected, detach, detach unknown fails, update, update unknown fails, findByTarget, findByKind, setVisible, setVisible unknown fails, setCount, setCount non-Count fails, observer on attach, observer on detach, removeObserver, clear
  - [x] IconEntry (5 tests): default invalid, valid construction, invalid without id, invalid without path, equality by id
  - [x] IconRegistry (12 tests): empty state, registerIcon, duplicate fails, invalid rejected, unregisterIcon, unregister unknown fails, findById, findByAlias, find id-or-alias, findByCategory, allIds, clear
  - [x] Integration (6 tests): full badge pipeline, multi-target queries, alias lookup, clearObservers, multiple observers
- [x] Wire `NF_Phase27Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- BadgeKind provides semantic overlay classification with name helpers ✓
- BadgeRegistry supports multi-target and multi-kind queries with setCount for Count badges ✓
- IconRegistry provides id-first-then-alias lookup with category grouping ✓
- Observer notifies on every badge structural change ✓
- 47 test cases pass (104 assertions) ✓
- Total test suite: ~2463 tests passing ✓

---

## Phase 28 – Workspace Minimap / Overview

**Status: Done**

- [x] Create `WorkspaceMinimap.h` — workspace minimap region and viewport tracking
  - [x] MinimapRect — normalized [0,1] float rectangle; `isValid()` (w>0 && h>0); equality
  - [x] MinimapRegion — id + label + rect + color + visible; `isValid()` (id non-empty + rect valid); equality by id
  - [x] MinimapViewport — rect + locked flag; `isValid()` delegates to rect
  - [x] MinimapManager — region registry (MAX_REGIONS=256); `addRegion`/`removeRegion`/`updateRegion`/`findRegion`/`isRegistered`/`setVisible`/`visibleRegions`; `setViewport`/`scrollViewport` (clamped to [0, 1-w]/[0, 1-h])/`lockViewport`/`unlockViewport`; separate region observers + viewport observers (MAX_OBSERVERS=16 each)
- [x] Add `Tests/Workspace/test_phase28_minimap.cpp` — 37 test cases / 71 assertions:
  - [x] MinimapRect (4 tests): default invalid, valid with positive size, invalid zero width, equality
  - [x] MinimapRegion (5 tests): default invalid, valid construction, invalid without id, invalid zero rect, equality by id
  - [x] MinimapViewport (2 tests): default invalid, valid with positive rect
  - [x] MinimapManager (21 tests): empty state, addRegion, duplicate fails, invalid rejected, removeRegion, remove unknown, updateRegion, update unknown, setVisible, visibleRegions filter, setViewport, setViewport invalid, scrollViewport, scroll clamped, scroll locked, lock/unlock, region observer add, region observer remove, viewport observer setViewport, viewport observer scroll, removeObserver, clear
  - [x] Integration (5 tests): full pipeline, visible filter, clearObservers, multiple observers
- [x] Wire `NF_Phase28Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- MinimapRect provides normalized float rect with validity check ✓
- MinimapRegion provides labeled colored area with id-based equality ✓
- MinimapManager tracks three-zone visibility and viewport with scroll clamping ✓
- Region and viewport observer pipelines independently notified ✓
- 37 test cases pass (71 assertions) ✓
- Total test suite: ~2500 tests passing ✓

---

## Phase 29 – Workspace Annotation System

**Status: Done**

- [x] Create `WorkspaceAnnotation.h` — workspace annotation anchoring and lifecycle
  - [x] AnnotationKind enum (Note/Warning/Todo/Bookmark/Review) with `annotationKindName()` helper
  - [x] AnnotationAnchor — targetId + contextKey + x/y position; `isValid()` (targetId non-empty)
  - [x] Annotation — id + kind + author + body + anchor + resolved + timestamp; `isValid()` (id + body + anchor valid); equality by id
  - [x] AnnotationManager — registry (MAX_ANNOTATIONS=1024); `add`/`remove`/`update`/`resolve`/`reopen`/`findById`; filter: `findByTarget`/`findByAuthor`/`findByKind`/`unresolved`/`resolved`/`allIds`; monotonic timestamp assigned on add; observer callbacks (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase29_annotation.cpp` — 37 test cases / 78 assertions:
  - [x] AnnotationKind enum (1 test): all 5 values
  - [x] AnnotationAnchor (3 tests): default invalid, valid with targetId, invalid without targetId
  - [x] Annotation (7 tests): default invalid, valid construction, invalid without id, invalid without body, invalid without anchor target, equality by id
  - [x] AnnotationManager (23 tests): empty state, add, duplicate fails, invalid rejected, timestamps increment, remove, remove unknown, update, update unknown, resolve, resolve already resolved, reopen, reopen already open, findByTarget, findByAuthor, findByKind, unresolved/resolved filters, allIds, observer on add, observer on remove, observer on resolve, removeObserver, clear
  - [x] Integration (4 tests): full pipeline, filter subsets, timestamps monotonic, clearObservers
- [x] Wire `NF_Phase29Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- AnnotationKind provides semantic category with name helpers ✓
- AnnotationAnchor ties annotations to workspace elements ✓
- AnnotationManager supports resolve/reopen lifecycle with monotonic timestamps ✓
- Filter queries by target, author, and kind work independently ✓
- 37 test cases pass (78 assertions) ✓
- Total test suite: ~2537 tests passing ✓

---

## Phase 30 – Workspace Filter and Search Index

**Status: Done**

- [x] Create `WorkspaceFilterIndex.h` — workspace searchable item index with tag and field filters
  - [x] IndexedItemKind enum (Asset/Panel/Tool/Node/Command/Custom) with `indexedItemKindName()` helper
  - [x] IndexedItem — id + kind + label + tags + fields (string map); `isValid()` (id + label non-empty); `hasTag`/`hasField`/`fieldValue`; equality by id
  - [x] FilterQuery — text (case-insensitive label substring) + filterKind/kind + requiredTags (all must match) + requiredFields (all keys must exist); `matchesItem()` combines all predicates
  - [x] WorkspaceFilterIndex — item registry (MAX_ITEMS=4096); `addItem`/`removeItem`/`updateItem`/`findById`/`isIndexed`; query: `query(FilterQuery)`/`findByKind`/`findByTag`/`allIds`; observer callbacks (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase30_filter_index.cpp` — 32 test cases / 78 assertions:
  - [x] IndexedItemKind enum (1 test): all 6 values
  - [x] IndexedItem (5 tests): default invalid, valid with tags+fields, invalid without id, invalid without label, equality by id
  - [x] FilterQuery (6 tests): empty matches all, text case-insensitive, kind filter, required tags, required fields, combined all predicates
  - [x] WorkspaceFilterIndex (16 tests): empty state, addItem, duplicate fails, invalid rejected, removeItem, remove unknown, updateItem, update unknown, query by text, findByKind, findByTag, allIds, observer on add, observer on remove, removeObserver, clear
  - [x] Integration (4 tests): full pipeline, combined filter, clearObservers, multiple observers
- [x] Wire `NF_Phase30Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- IndexedItemKind provides typed categorization with name helpers ✓
- IndexedItem supports tag set and arbitrary field map with helper accessors ✓
- FilterQuery combines text/kind/tag/field predicates independently ✓
- WorkspaceFilterIndex returns filtered item lists without mutation ✓
- 32 test cases pass (78 assertions) ✓
- Total test suite: ~2569 tests passing ✓

---

## Phase 31 – Workspace Theme System

**Status: Done**

- [x] Create `WorkspaceTheme.h` — workspace-wide theme management
  - [x] ThemeSlot enum (14 semantic color roles: Background/Surface/Border/Accent/AccentHover/AccentActive/TextPrimary/TextSecondary/TextDisabled/IconPrimary/IconSecondary/SelectionHighlight/ErrorColor/WarningColor) with `themeSlotName()` helper
  - [x] ThemeColorMap — slot-indexed RRGGBBAA color table; set/get/isDefined/reset/resetAll/definedCount/allDefined
  - [x] ThemeDescriptor — id + displayName + author + colorMap + isBuiltIn; isValid()
  - [x] ThemeViolation + ThemeEnforcementReport — typed violation list; passed flag + violationCount
  - [x] ThemeEnforcer — validates descriptor: checks all slots defined, invalid descriptor reports violation
  - [x] ThemeRegistry — named theme store (MAX_THEMES=64); registerTheme/unregisterTheme/find/contains/applyTheme/activeThemeId/activeTheme/allIds/clear; observer callbacks (MAX_OBSERVERS=16); cannot unregister active theme
- [x] Add `Tests/Workspace/test_phase31_theme.cpp` — 36 test cases / 92 assertions:
  - [x] ThemeSlot (1 test): all 14 values + kThemeSlotCount
  - [x] ThemeColorMap (7 tests): default, set+get, isDefined false for unset, reset single, resetAll, allDefined, definedCount
  - [x] ThemeDescriptor (5 tests): default invalid, valid, invalid without id, invalid without displayName, isBuiltIn flag
  - [x] ThemeEnforcer (4 tests): pass fully-defined, missing slots, invalid descriptor, violation carries slot info
  - [x] ThemeRegistry (16 tests): empty, register, duplicate fails, invalid fails, find, find null, apply, apply unknown, unregister, cannot unregister active, unregister unknown, allIds, observer on apply, observer on switch, clearObservers, clear
  - [x] Integration (3 tests): full pipeline, invalid rejected by enforcer, multiple observers
- [x] Wire `NF_Phase31Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ThemeSlot provides 14 semantic color roles with name helpers ✓
- ThemeColorMap tracks defined state per slot independently ✓
- ThemeEnforcer detects any undefined slot as a violation ✓
- ThemeRegistry prevents active-theme removal and fires observers on apply ✓
- 36 test cases pass (92 assertions) ✓
- Total test suite: ~2607 tests passing ✓

---

## Phase 32 – Workspace Tour / Onboarding System

**Status: Done**

- [x] Create `WorkspaceTour.h` — guided onboarding tour
  - [x] TourStepKind enum (Highlight/Tooltip/Modal/Action/Pause) with `tourStepKindName()` helper
  - [x] TourState enum (Idle/Running/Paused/Completed/Cancelled) with `tourStateName()` helper
  - [x] TourStep — id + kind + targetId + title + body + actionLabel; isValid()
  - [x] TourSequence — ordered step list (MAX_STEPS=128); id + name; addStep/stepAt/stepCount; isValid()
  - [x] TourProgress — sequenceId + stepIndex + totalSteps; isActive(); fraction()
  - [x] TourController — load/start/next/prev/pause/resume/cancel/complete/reset; currentStep(); observer callbacks (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase32_tour.cpp` — 43 test cases / 78 assertions:
  - [x] TourStepKind (1 test): all 5 values
  - [x] TourState (1 test): all 5 values
  - [x] TourStep (5 tests): default invalid, valid, invalid without id, invalid without title, stores kind
  - [x] TourSequence (6 tests): default invalid, invalid with no steps, valid, addStep invalid, stepAt, stepAt out-of-range
  - [x] TourProgress (4 tests): default inactive, isActive, fraction, isActive false past-end
  - [x] TourController (20 tests): default Idle, load valid, load invalid, start Running, start no sequence, start already running, next advances, next last completes, next not Running, prev goes back, prev fails at first, pause+resume, pause not Running, resume not Paused, cancel from Running, cancel from Paused, cancel from Idle, currentStep, currentStep null Idle, reset
  - [x] Integration (3 tests): full walk-through, pause mid-tour, progress fraction increases
- [x] Wire `NF_Phase32Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- TourStepKind and TourState provide typed enums with name helpers ✓
- TourSequence enforces non-empty valid steps ✓
- TourController enforces state machine transitions (cannot start if running, cannot pause if idle, etc.) ✓
- TourProgress.fraction() increases linearly through the sequence ✓
- 43 test cases pass (78 assertions) ✓
- Total test suite: ~2647 tests passing ✓

---

## Phase 33 – Workspace Split View / Tab Groups

**Status: Done**

- [x] Create `WorkspaceSplitView.h` — split-view layout with panes and tab groups
  - [x] SplitOrientation enum (None/Horizontal/Vertical) with `splitOrientationName()` helper
  - [x] TabEntry — id + label + closeable; isValid(); equality by id
  - [x] TabGroup — ordered tab list (MAX_TABS=64); groupId; addTab/removeTab/setActiveTab/hasTab/tabCount/empty; active tab fallback on remove
  - [x] SplitPane — id + orientation + tabGroup + splitRatio + first/second children; isLeaf/isBranch/isValid()
  - [x] SplitViewController — root pane tree; containsPane/findPane/setActivePane; addTab/removeTab/setActiveTab; splitPane (leaf→branch with two children)/collapsePane (branch→leaf keeping first child); observer callbacks (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase33_split_view.cpp` — 46 test cases / 82 assertions:
  - [x] SplitOrientation (1 test): all 3 values
  - [x] TabEntry (5 tests): default invalid, valid, invalid without id, invalid without label, equality by id
  - [x] TabGroup (11 tests): empty state, isValid, addTab sets active, duplicate fails, invalid fails, removeTab+active fallback, removeTab unknown, setActiveTab, setActiveTab unknown, hasTab, empty after all removed
  - [x] SplitPane (4 tests): default leaf, invalid without id, valid, branch when orientation set
  - [x] SplitViewController (20 tests): init root leaf, containsPane, setActivePane, setActivePane unknown, addTab, duplicate fails, unknown pane fails, removeTab, removeTab unknown, setActiveTab, splitPane horizontal, splitPane vertical, None fails, duplicate secondId fails, splitPane on branch fails, tabs preserved in first child, collapsePane, collapsePane on leaf fails, observer on addTab, observer on splitPane, clearObservers
  - [x] Integration (4 tests): split + add tabs both panes, switch active pane, collapse keeps first-child tabs, multiple observers
- [x] Wire `NF_Phase33Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- SplitOrientation provides 3 layout modes with name helpers ✓
- TabGroup maintains stable active-tab selection with fallback on remove ✓
- SplitPane cleanly models leaf/branch duality ✓
- SplitViewController enforces split constraints (no duplicate IDs, no splitting branches, None orientation rejected) ✓
- collapsePane restores leaf state and preserves first-child tab group ✓
- 46 test cases pass (82 assertions) ✓
- Total test suite: ~2688 tests passing ✓

---

## Phase 34 – Workspace Frame Controller

**Status: Done**

- [x] Use `WorkspaceFrameController.h` — frame pacing, delta-time smoothing, and budget tracking
  - [x] FrameBudget — totalMs/updateMs/renderMs; `isValid()` (all positive, sum within 150% total)
  - [x] FrameResult — dt (smoothed) + rawDt (clamped) + wasSkipped + frameNumber
  - [x] FrameStatistics — totalFrames/fps/avgDtMs/minDtMs/maxDtMs/lastUpdateMs/lastRenderMs/skippedFrames; `reset()`; `budgetUtilization()`
  - [x] WorkspaceFrameController — setTargetFPS/setMaxDeltaTime/setEMAAlpha/setBudget; beginFrame (clamp+EMA); markUpdateDone/markRenderDone/endFrame; shouldSleep/sleepMs; resetStats
- [x] Add `Tests/Workspace/test_phase34_frame_controller.cpp` — 31 test cases / 55 assertions:
  - [x] FrameBudget (3 tests): valid default, invalid zero total, invalid zero update
  - [x] FrameResult (1 test): default zeroed state
  - [x] FrameStatistics (4 tests): default state, reset, budgetUtilization zero budget, budgetUtilization ratio
  - [x] WorkspaceFrameController (20 tests): default 60 FPS, default maxDtSec, setTargetFPS updates budget, setTargetFPS ignores non-positive, setMaxDeltaTime, setEMAAlpha valid, setEMAAlpha invalid, setBudget valid, setBudget invalid, beginFrame increments frameNumber, clamps negative dt, clamps dt above maxDtSec, first frame seeds EMA, markUpdateDone+markRenderDone, endFrame increments totalFrames, skippedFrames over budget, wasSkipped next frame, shouldSleep, sleepMs, resetStats
  - [x] Integration (3 tests): 10-frame loop accumulates stats, budget overrun detected, EMA converges
- [x] Wire `NF_Phase34Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- FrameBudget validates positive component constraints ✓
- beginFrame clamps delta-time and applies EMA smoothing ✓
- endFrame detects budget overrun and increments skippedFrames ✓
- wasSkipped propagated to the next frame's FrameResult ✓
- shouldSleep/sleepMs provide frame pacing helpers ✓
- 31 test cases pass (55 assertions) ✓
- Total test suite: ~2719 tests passing ✓

---

## Phase 35 – Workspace Project File

**Status: Done**

- [x] Use `WorkspaceProjectFile.h` — canonical .atlasproject file schema
  - [x] ProjectFileVersion — major.minor.patch; `toString()`; `isCompatible()` (same major, file minor ≤ reader minor); `parse()`; `current()`; equality operators
  - [x] ProjectFileSection — named key-value store (MAX_ENTRIES=256); `set`/`get`/`getOr`/`has`/`remove`/`clear`/`count`/`empty`/`entries()`
  - [x] WorkspaceProjectFile — projectId/projectName/contentRoot/version setters; `isValid()` (non-empty id+name, major>0); `section()` create-on-demand; `findSection`/`hasSection`/`removeSection`/`sectionCount`; `serialize()`/`parse()` round-trip
- [x] Add `Tests/Workspace/test_phase35_project_file.cpp` — 38 test cases / 77 assertions:
  - [x] ProjectFileVersion (10 tests): default 1.0.0, toString, isCompatible same version, isCompatible minor older, incompatible minor newer, incompatible major, parse valid, parse invalid, current returns 1.0.0, equality
  - [x] ProjectFileSection (11 tests): default empty, set+get, set overwrites, get nullptr missing, getOr default, getOr existing, has, remove existing, remove missing, clear, count/empty
  - [x] WorkspaceProjectFile (10 tests): default invalid, valid with id+name, invalid without id, invalid without name, setContentRoot, section create on demand, section returns existing, findSection nullptr, removeSection, removeSection unknown
  - [x] Serialization (5 tests): serialize produces magic header, serialize+parse round-trip, parse with sections, parse fails no magic, parse fails empty
  - [x] Integration (3 tests): full round-trip, version compatibility, multiple sections independent
- [x] Wire `NF_Phase35Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ProjectFileVersion enforces same-major / file-minor-≤-reader-minor compatibility rule ✓
- ProjectFileSection MAX_ENTRIES=256 enforced; duplicate key overwrites ✓
- serialize/parse round-trip is lossless for identity fields and all named sections ✓
- parse rejects missing magic header and empty input ✓
- 38 test cases pass (77 assertions) ✓
- Total test suite: ~2757 tests passing ✓

---

## Phase 36 – AI Assistant Panel and AI Panel Session

**Status: Done**

- [x] Use `AIAssistantPanel.h` — AtlasAI assistant chat infrastructure
  - [x] ChatRole enum (User/Assistant/System) with `chatRoleName()` helper
  - [x] ChatMessage — id + role + content + timestamp + pending; `isUser()`/`isAssistant()`/`isSystem()`
  - [x] ChatSession — addUserMessage/addAssistantMessage/addSystemMessage/addMessage; `lastMessage()`/`messageCount()`/`empty()`/`id()`/`clear()`
- [x] Use `AIPanelSession.h` — AI panel session with context lifecycle
  - [x] AISessionContextType enum (None/File/Selection/Error/Notification/Diff/Log) with `aiSessionContextTypeName()` helper
  - [x] AISessionContext — type + label + content + pinned; `isValid()` (type≠None and content non-empty)
  - [x] AIPanelSession — `addContext`/`removeContext`/`clearUnpinnedContexts`/`findContext`/`contextCount`; `submitUserTurn`/`receiveAssistantResponse`/`turnCount`/`messageCount`/`isEmpty`; `setTitle`/`title`; `reset`
- [x] Add `Tests/Workspace/test_phase36_ai_panel.cpp` — 34 test cases / 80 assertions:
  - [x] ChatRole (1 test): all 3 name helpers
  - [x] ChatMessage (4 tests): default User role, isAssistant, isSystem, pending flag
  - [x] ChatSession (9 tests): default empty, addUserMessage, addAssistantMessage, addSystemMessage, lastMessage, lastMessage null, id preserved, clear, unique message ids
  - [x] AISessionContextType (1 test): all 7 values
  - [x] AISessionContext (4 tests): default invalid, valid, invalid no content, pinned defaults false
  - [x] AIPanelSession (12 tests): default empty, sessionId, submitUserTurn, receiveAssistantResponse, addContext valid, addContext invalid, removeContext, removeContext unknown, findContext, clearUnpinnedContexts, setTitle/title, reset
  - [x] Integration (3 tests): multi-turn conversation, pinned context survives clearUnpinned, reset and restart
- [x] Wire `NF_Phase36Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ChatRole name helpers cover all 3 roles ✓
- ChatSession assigns unique auto-generated ids to each message ✓
- AISessionContext.isValid() rejects None type and empty content ✓
- AIPanelSession.turnCount() increments only on submitUserTurn, not receiveAssistantResponse ✓
- clearUnpinnedContexts removes only non-pinned contexts ✓
- Integration: multi-turn conversation, reset+restart, pinned context persistence ✓
- 34 test cases pass (80 assertions) ✓
- Total test suite: ~2791 tests passing ✓

---

## Phase 37 – Console Command Bus

**Status: Done**

- [x] Use `ConsoleCommandBus.h` — console command palette backend
  - [x] ConsoleCmdScope enum (Global/Editor/Game/Server/Client/Plugin) with `consoleCmdScopeName()` helper
  - [x] ConsoleCmdArgType enum (None/Bool/Int/Float/String/Enum) with `consoleCmdArgTypeName()` helper
  - [x] ConsoleCmdExecResult enum (Ok/NotFound/InvalidArgs/PermissionDenied/Error) with `consoleCmdExecResultName()` helper
  - [x] ConsoleCommand — name + scope + argType + description + enabled + hidden; setters/getters
  - [x] ConsoleCommandBus — MAX_COMMANDS=1024; `registerCommand`/`unregisterCommand`/`findCommand`; `execute()` → ConsoleCmdExecResult; `lastExec()`; `countByScope`/`hiddenCount`/`enabledCount`
- [x] Add `Tests/Workspace/test_phase37_console_command_bus.cpp` — 24 test cases / 55 assertions:
  - [x] ConsoleCmdScope (1 test): all 6 values
  - [x] ConsoleCmdArgType (1 test): all 6 values
  - [x] ConsoleCmdExecResult (1 test): all 5 values
  - [x] ConsoleCommand (5 tests): stores name/scope/argType, default enabled+visible, setDescription, setEnabled false, setHidden true
  - [x] ConsoleCommandBus (13 tests): default empty, registerCommand, duplicate fails, unregisterCommand, unregister unknown, findCommand nullptr, findCommand found, execute Ok, execute NotFound, execute PermissionDenied, countByScope, hiddenCount, enabledCount
  - [x] Integration (3 tests): multi-scope palette, execute+lastExec tracking, disable+re-enable via findCommand
- [x] Wire `NF_Phase37Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ConsoleCmdScope/ArgType/ExecResult provide typed enums with name helpers ✓
- ConsoleCommand defaults to enabled and visible ✓
- ConsoleCommandBus.execute() returns PermissionDenied for disabled commands, NotFound for unknown ✓
- countByScope/hiddenCount/enabledCount correctly filter command lists ✓
- 24 test cases pass (55 assertions) ✓
- Total test suite: ~2815 tests passing ✓

---

## Phase 38 – Dock Tree Serializer

**Status: Done**

- [x] Use `DockTreeSerializer.h` — text-format dock layout persistence
  - [x] DockSplitOrientation enum (Horizontal/Vertical) with `dockSplitOrientationName()` helper
  - [x] DockNodeKind enum (Split/TabStack) with `dockNodeKindName()` helper
  - [x] DockTreeNode — id + kind + orientation + splitRatio + firstChild + secondChild + panelIds + activeTab; `isValid()`/`isSplit()`/`isTabStack()`; `addPanel`/`removePanel`
  - [x] DockTree — `addNode`/`removeNode`/`findNode`/`findNodeMut`/`setRootId`/`rootId`/`nodeCount`/`nodes`/`clear`; first added node auto-sets rootId
  - [x] DockTreeSerializer — `serialize(DockTree)` → text; `deserialize(text, DockTree&)` → bool; wire format: `root:<id>`, `node:<id>|split|…`, `node:<id>|tabs|…`
- [x] Add `Tests/Workspace/test_phase38_dock_tree_serializer.cpp` — 31 test cases / 78 assertions:
  - [x] DockSplitOrientation (1 test): both values
  - [x] DockNodeKind (1 test): both values
  - [x] DockTreeNode (7 tests): invalid default, valid with id, default TabStack, isSplit, addPanel, removePanel, removePanel unknown
  - [x] DockTree (11 tests): default empty, addNode valid, addNode sets rootId, addNode invalid, addNode duplicate, removeNode, removeNode unknown, findNode nullptr, findNode found, findNodeMut mutates, setRootId, clear
  - [x] DockTreeSerializer (8 tests): serialize root line, serialize TabStack, serialize Split, deserialize empty fails, deserialize no-root fails, round-trip TabStack, round-trip Split
  - [x] Integration (3 tests): full layout round-trip, empty tree serializes but fails deserialize, mutate+re-serialize
- [x] Wire `NF_Phase38Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- DockTreeNode cleanly models Split and TabStack node duality ✓
- DockTree auto-assigns rootId to the first inserted node ✓
- DockTreeSerializer round-trip is lossless for panel ids, active tab, split ratio, children ✓
- deserialize rejects empty input and input without a root: line ✓
- 31 test cases pass (78 assertions) ✓
- Total test suite: ~2846 tests passing ✓

---

## Phase 39 – Content Router and Drop Target Handler

**Status: Done**

- [x] Use `ContentRouter.h` — file-type to tool routing rules
  - [x] ContentRouterPolicy enum (Reject/UseDefault/Prompt) with `contentRouterPolicyName()` helper
  - [x] RouteResult — matched + toolId + ruleName + needsPrompt; `succeeded()`
  - [x] RoutingRule — name + toolId + typeTag + sourceFilter + filterBySource + priority + enabled; `isValid()`; `matches(tag, source)` with wildcard support (Unknown = any type)
  - [x] ContentRouter — MAX_RULES=128; `addRule`/`removeRule`/`enableRule`/`hasRule`/`ruleCount`; `route(tag)`/`route(AssetDescriptor)`/`route(IntakeItem)`; policy handling; `routeCount`/`missCount`/`clearRules`; rules sorted descending by priority
- [x] Use `DropTargetHandler.h` — file drag-and-drop surface
  - [x] DropState enum (Idle/DragOver/DragLeave/Dropped/Rejected) with `dropStateName()` helper
  - [x] DropEffect enum (None/Copy/Move/Link) with `dropEffectName()` helper
  - [x] DropTargetHandler — `onDragEnter`/`onDragOver`/`onDragLeave`/`onDrop`/`reset`; `setDefaultEffect`/`setAcceptUnknown`; `bindPipeline`; enter/leave/drop count tracking; `isDragActive()`; `lastDroppedPaths()`/`hoveredPaths()`
- [x] Add `Tests/Workspace/test_phase39_content_router.cpp` — 42 test cases / 95 assertions:
  - [x] ContentRouterPolicy (1 test): all 3 values
  - [x] RouteResult (1 test): default not matched
  - [x] RoutingRule (7 tests): invalid no name, invalid no toolId, valid, matches type, wildcard, disabled never matches, source filter
  - [x] ContentRouter (16 tests): default Reject policy, addRule, addRule invalid, addRule duplicate, removeRule, removeRule unknown, hasRule, enableRule, route matched, route Reject policy, route UseDefault, route Prompt, route priority ordering, routeCount, missCount, clearRules
  - [x] DropState (1 test): all 5 values
  - [x] DropEffect (1 test): all 4 values
  - [x] DropTargetHandler (12 tests): default Idle, default Copy effect, onDragEnter known ext, onDragEnter rejects unknown, acceptUnknown flag, onDragOver not-rejected, onDragOver rejected, onDragLeave, onDrop without pipeline, lastDroppedPaths, reset, setDefaultEffect
  - [x] Integration (3 tests): multi-type routing pipeline, disable+fallback-to-default, enter→hover→drop sequence
- [x] Wire `NF_Phase39Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- RoutingRule.matches() respects enabled flag, type wildcard, and optional source filter ✓
- ContentRouter sorts rules by priority descending so higher-priority rules win ✓
- ContentRouterPolicy (Reject/UseDefault/Prompt) handled distinctly in route() ✓
- DropTargetHandler rejects unknown extensions by default; acceptUnknown flag overrides ✓
- State machine: Idle→DragOver→Dropped (or Rejected) transitions correct ✓
- 42 test cases pass (95 assertions) ✓
- Total test suite: ~2888 tests passing ✓

---

## Phase 40 – Asset Import Queue

**Status: Done**

- [x] Use `AssetImportQueue.h` — batch import job processing for the intake pipeline
  - [x] ImportJobStatus enum (Queued/Validating/Importing/PostProcess/Done/Failed/Cancelled) with `importJobStatusName()` helper
  - [x] ImportJob — id + intakeItem + status + progress + errorMsg + outputPath + priority; isDone/isFailed/isActive/isFinished helpers
  - [x] AssetImportQueue — MAX_JOBS=256 MAX_PARALLEL=4; `enqueue`/`enqueueFromPipeline`/`cancel`/`startNext`/`advance`/`failJob`/`find`; `clearFinished`; priority-sorted queue; `setOnComplete` callback; stats (totalEnqueued/Completed/Failed/Cancelled)
- [x] Add `Tests/Workspace/test_phase40_asset_import_queue.cpp` — 24 test cases / 75 assertions:
  - [x] ImportJobStatus (1 test): all 7 values
  - [x] ImportJob (5 tests): default state, isDone, isFailed, isActive, isFinished
  - [x] AssetImportQueue (16 tests): default empty, enqueue, find, find unknown, startNext, startNext empty, advance pipeline, totalCompleted, failJob, failJob finished, cancel, cancel unknown, clearFinished, priority ordering, onComplete callback, enqueueFromPipeline
  - [x] Integration (2 tests): full pipeline enqueue+advance+complete, mixed complete/fail/cancel+clearFinished
- [x] Wire `NF_Phase40Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ImportJob.isActive() covers Validating/Importing/PostProcess states ✓
- AssetImportQueue.advance() walks Queued→Validating→Importing→PostProcess→Done ✓
- failJob() and cancel() ignore already-finished jobs ✓
- clearFinished() removes Done/Failed/Cancelled jobs only ✓
- Priority ordering: higher priority enqueued jobs appear first ✓
- onComplete callback fires exactly on Done transition ✓
- 24 test cases pass (75 assertions) ✓
- Total test suite: ~2912 tests passing ✓

---

## Phase 41 – Workspace Layout Manager

**Status: Done**

- [x] Use `WorkspaceLayout.h` — workspace panel layout and layout manager
  - [x] LayoutPanelType enum (Viewport/Inspector/Hierarchy/ContentBrowser/Console/Profiler/Timeline/Custom) with `layoutPanelTypeName()` helper
  - [x] LayoutDockZone enum (Left/Right/Top/Bottom) with `layoutDockZoneName()` helper
  - [x] LayoutPanel — id/title/type/dockZone/width/height/visible/pinned; show/hide/pin/unpin; isVisible/isPinned/hasSize
  - [x] LayoutSplit — firstPanelId/secondPanelId/isHorizontal/ratio; isValid(); flipOrientation()
  - [x] WorkspaceLayout — named container; addPanel/removePanel/findPanel/addSplit; visiblePanelCount/pinnedPanelCount; showAll/hideAll
  - [x] WorkspaceLayoutManager — MAX_LAYOUTS=32; createLayout/removeLayout/findLayout/setActive/activeLayout; hasActive/activeName; removing active clears active name
- [x] Add `Tests/Workspace/test_phase41_workspace_layout.cpp` — 38 test cases / 84 assertions:
  - [x] LayoutPanelType (1 test): all 8 values
  - [x] LayoutDockZone (1 test): all 4 values
  - [x] LayoutPanel (5 tests): default visible/not-pinned, hide+show, pin+unpin, hasSize both dims, hasSize one dim
  - [x] LayoutSplit (4 tests): invalid no ids, invalid ratio 0/1, valid, flipOrientation
  - [x] WorkspaceLayout (12 tests): construct, addPanel, addPanel duplicate, removePanel, removePanel unknown, findPanel, findPanel null, findPanel mutates, addSplit invalid, addSplit valid, visiblePanelCount, pinnedPanelCount, showAll+hideAll
  - [x] WorkspaceLayoutManager (13 tests): default empty, createLayout, duplicate, removeLayout, remove unknown, findLayout, find null, setActive, setActive unknown, activeLayout, activeLayout null, removing active clears name
  - [x] Integration (2 tests): full multi-layout workflow, hide all/show all/manual hide
- [x] Wire `NF_Phase41Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- LayoutPanel visibility and pinning are independent boolean flags ✓
- LayoutSplit.isValid() rejects ratio ≤ 0 or ≥ 1 ✓
- WorkspaceLayout.findPanel() returns a mutable pointer allowing in-place mutation ✓
- WorkspaceLayoutManager.createLayout() returns nullptr on duplicate name ✓
- Removing active layout clears the active name ✓
- Pointers fetched after all creates to avoid vector reallocation ✓
- 38 test cases pass (84 assertions) ✓
- Total test suite: ~2950 tests passing ✓

---

## Phase 42 – Logging Route V1

**Status: Done**

- [x] Use `LoggingRouteV1.h` — structured log routing with sinks, routes, and level filtering
  - [x] `logLevelName()` helper — maps Core LogLevel (Trace/Debug/Info/Warn/Error/Fatal) to string
  - [x] `logLevelAtLeast()` helper — ordered comparison for threshold filtering
  - [x] LogEntry — seq/level/tag/message/source/timestampMs; isValid()/isError()/isWarning()
  - [x] LogSink — id/name/minLevel/tagFilter/callback/enabled; isValid(); accepts() (level+tag+enabled checks)
  - [x] LogRoute — id/name/sourcePattern/sinkIds/passThrough; isValid(); matchesSource() (prefix match; empty = all)
  - [x] LoggingRouteV1 — MAX_BUFFER=4096 MAX_SINKS=32 MAX_ROUTES=64; addSink/removeSink/addRoute/removeRoute; log/trace/debug/info/warn/error/fatal; setSinkEnabled/setMinLevel; buffer/bufferSize/logCount/sinkCount/routeCount; countByLevel; clearBuffer; findSink (const)
- [x] Add `Tests/Workspace/test_phase42_logging_route.cpp` — 40 test cases / 99 assertions:
  - [x] logLevelName (1 test): all 6 levels
  - [x] logLevelAtLeast (1 test): ordering comparisons
  - [x] LogEntry (4 tests): default invalid, valid, isError, isWarning
  - [x] LogSink (7 tests): invalid no id, invalid no name, invalid no callback, valid, accepts minLevel, accepts tagFilter, disabled never accepts
  - [x] LogRoute (5 tests): invalid no id, invalid no name, valid, matchesSource empty, matchesSource prefix
  - [x] LoggingRouteV1 (19 tests): default empty, addSink, addSink invalid, addSink duplicate, removeSink, removeSink unknown, addRoute, addRoute invalid, addRoute duplicate, removeRoute, log buffers, convenience helpers, countByLevel, clearBuffer, sink level filtering, setSinkEnabled, setMinLevel, findSink const
  - [x] Integration (3 tests): multi-sink level filters, tag-filtered sink, buffer accumulation and clearBuffer
- [x] Wire `NF_Phase42Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- logLevelAtLeast() correctly orders Trace < Debug < Info < Warn < Error < Fatal ✓
- LogSink.accepts() checks enabled, minLevel, and tagFilter independently ✓
- LogRoute.matchesSource() with empty pattern matches any source ✓
- LoggingRouteV1 buffers all entries and delivers only to accepting sinks ✓
- setSinkEnabled/setMinLevel mutate sink in-place via private findSink ✓
- clearBuffer empties the ring buffer without resetting the cumulative logCount ✓
- 40 test cases pass (99 assertions) ✓
- Total test suite: ~2985 tests passing ✓

---

## Phase 43 – Notifications (NotificationQueue)

**Status: Done**

- [x] Use `Notifications.h` — lightweight editor notification queue with TTL expiry
  - [x] NotificationType enum (Info/Success/Warning/Error)
  - [x] EditorNotification — type/message/ttl/elapsed; isExpired(); progress() (0..1, capped at 1)
  - [x] NotificationQueue — push(type, message, ttl=3); tick(dt) advances elapsed and removes expired; current(); hasActive(); count(); clear()
- [x] Add `Tests/Workspace/test_phase43_notifications.cpp` — 18 test cases / 45 assertions:
  - [x] NotificationType (1 test): all 4 enum values
  - [x] EditorNotification (6 tests): default not expired, isExpired at ttl, not expired before, progress at 0/0.5/1, capped over-elapsed, zero ttl
  - [x] NotificationQueue (9 tests): default empty, push adds, current returns first, push multiple, tick advances elapsed, tick removes expired, tick removes all, clear, default ttl 3
  - [x] Integration (2 tests): FIFO ordering through ticks, progress tracking and expiry
- [x] Wire `NF_Phase43Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- EditorNotification.progress() is clamped to [0, 1] ✓
- NotificationQueue.tick() removes all entries where elapsed >= ttl ✓
- FIFO ordering: front of queue is always current() ✓
- 18 test cases pass (45 assertions) ✓
- Total test suite: ~3003 tests passing ✓

---

## Phase 44 – NotificationSystem (Advanced Channels)

**Status: Done**

- [x] Use `NotificationSystem.h` — multi-channel notification dispatch
  - [x] NotificationSeverity enum (Info/Success/Warning/Error/Critical/Debug/Trace/System) with `notificationSeverityName()` helper
  - [x] NotificationState enum (Pending/Shown/Dismissed/Expired) with `notificationStateName()` helper
  - [x] Notification — id/title/message/severity/state/durationMs/persistent; show/dismiss/expire; isDismissed/isExpired/isVisible/isError/isCritical
  - [x] NotificationChannel — named container; post/dismiss/find/activeCount/errorCount/clearDismissed (removes Dismissed+Expired)
  - [x] NotificationSystem — MAX_CHANNELS=16; createChannel/removeChannel/findChannel/post(channelName, n); totalActive() sums across all channels
- [x] Add `Tests/Workspace/test_phase44_notification_system.cpp` — 33 test cases / 79 assertions:
  - [x] NotificationSeverity (1 test): all 8 values
  - [x] NotificationState (1 test): all 4 values
  - [x] Notification (7 tests): default Pending, show, dismiss, expire, isError, isCritical, durationMs default
  - [x] NotificationChannel (10 tests): construct, post shows, post duplicate, find, find null, dismiss, dismiss unknown, errorCount, clearDismissed, clearDismissed nothing
  - [x] NotificationSystem (11 tests): default empty, createChannel, duplicate, removeChannel, remove unknown, findChannel, find null, post success, post unknown channel, totalActive sums, totalActive drops on dismiss
  - [x] Integration (3 tests): multi-channel error aggregation, dismiss+clearDismissed, persistent notification
- [x] Wire `NF_Phase44Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- Notification.isError() true for Error and Critical; isCritical() only for Critical ✓
- NotificationChannel.post() auto-calls show() on the notification ✓
- clearDismissed() removes both Dismissed and Expired states ✓
- NotificationSystem.totalActive() aggregates across all channels ✓
- 33 test cases pass (79 assertions) ✓
- Total test suite: ~3036 tests passing ✓

---

## Phase 45 – UndoRedoSystem

**Status: Done**

- [x] Use `UndoRedoSystem.h` — workspace undo/redo with action groups
  - [x] UndoActionType enum (Create/Delete/Move/Resize/Rename/Modify/Group/Ungroup) with `undoActionTypeName()` helper
  - [x] UndoActionState enum (Pending/Applied/Undone/Invalid) with `undoActionStateName()` helper
  - [x] UndoAction — id/description/type/state; apply/undo/invalidate; isApplied/isUndone/isValid/canUndo/canRedo
  - [x] UndoGroup — named batch; addAction/removeAction/find; applyAll/undoAll; actionCount/appliedCount
  - [x] UndoRedoSystem — MAX_GROUPS=64; pushGroup (clears redo stack); undo/redo; canUndo/canRedo/undoDepth/redoDepth; clear
- [x] Add `Tests/Workspace/test_phase45_undo_redo.cpp` — 29 test cases / 88 assertions:
  - [x] UndoActionType (1 test): all 8 values
  - [x] UndoActionState (1 test): all 4 values
  - [x] UndoAction (5 tests): default Pending, apply, undo from Applied, undo from non-Applied, invalidate
  - [x] UndoGroup (9 tests): construct, addAction, duplicate, removeAction, remove unknown, find, find null, applyAll, undoAll, undoAll skips non-Applied
  - [x] UndoRedoSystem (10 tests): default empty, pushGroup, undo, redo, undo empty, redo empty, push clears redo, multiple pushes, undo from top, clear
  - [x] Integration (2 tests): multi-step undo/redo/branch cycle, group applyAll/undoAll through system
- [x] Wire `NF_Phase45Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- UndoAction.undo() only transitions from Applied→Undone (Pending unchanged) ✓
- UndoGroup.undoAll() skips non-Applied actions ✓
- UndoRedoSystem.pushGroup() clears the redo stack ✓
- UndoRedoSystem.undo() pops from top of undo stack, pushes to redo stack ✓
- 29 test cases pass (88 assertions) ✓
- Total test suite: ~3065 tests passing ✓

---

## Phase 46 – SelectionSystem

**Status: Done**

- [x] Use `SelectionSystem.h` — multi-context workspace selection with named sets and history
  - [x] SelectionContextType enum (None/Scene/Asset/UI/Console/Code) with `selectionContextTypeName()` helper
  - [x] SelectionRecord — id/label/context; isValid(); equality operators (by id)
  - [x] SelectionSet — named container; add/remove/contains/find; clear; count/isEmpty/version/items; countByContext
  - [x] SelectionHistory — MAX_HISTORY=32; push (truncates forward on new entry, evicts oldest at cap); back/forward; canBack/canForward; current; hasHistory/depth; clear
  - [x] SelectionSystem — MAX_SETS=16; createSet/removeSet/findSet; setActiveContext/setActiveSet/activeSet; select/deselect/clearActive/isSelected/activeCount; history(); clearAll()
- [x] Add `Tests/Workspace/test_phase46_selection_system.cpp` — 32 test cases / 87 assertions:
  - [x] SelectionContextType (1 test): all 6 values
  - [x] SelectionRecord (3 tests): default invalid, isValid when id set, equality by id
  - [x] SelectionSet (9 tests): default empty, add valid, add invalid rejected, add duplicate rejected, remove existing, remove unknown, contains+find, clear bumps version, items, countByContext
  - [x] SelectionHistory (8 tests): default no history, push one entry, back+forward, back at beginning false, forward at end false, push truncates forward, clear, MAX_HISTORY cap
  - [x] SelectionSystem (11 tests): default empty, createSet, createSet duplicate, createSet empty name, removeSet, removeSet unknown, removeSet clears activeSetName, setActiveSet, setActiveSet unknown, setActiveContext, select, select no active set, deselect, clearActive, MAX_SETS enforced, clearAll
  - [x] Integration (3 tests): multi-context selection (context stamped on record), history navigation after select/deselect, countByContext across contexts
- [x] Wire `NF_Phase46Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- SelectionRecord.isValid() false when id == INVALID_ENTITY ✓
- SelectionSet.add() rejects invalid records and duplicates (by id) ✓
- SelectionSet.countByContext() counts only records with matching context ✓
- SelectionHistory.push() truncates forward history; evicts oldest when at MAX_HISTORY ✓
- SelectionSystem.select() stamps activeContext onto the record before adding ✓
- SelectionSystem.removeSet() clears activeSetName if active set is removed ✓
- 32 test cases pass (87 assertions) ✓
- Total test suite: ~3097 tests passing ✓

---

## Phase 47 – ProjectManager

**Status: Done**

- [x] Use `ProjectManager.h` — session-level project lifecycle coordinator
  - [x] ProjectManagerState enum (Idle/Opening/Open/Saving/Closing/Error) with `projectManagerStateName()` helper
  - [x] RecentProjectEntry — path/displayName/lastOpenedMs; isValid()
  - [x] ProjectManagerConfig — maxRecentProjects=10; autoSaveIntervalSec=300; autoSaveEnabled=true
  - [x] ProjectManager — newProject/openProject (reject if already open or empty path); save (clears dirty, increments saveCount); closeProject (Idle/Error → resets); setError/clearError; markDirty/markClean; tickAutoSave(dt) accumulates and fires callback+save when interval reached; recent list (dedup, front-insert, cap at maxRecentProjects, removeRecent, clearRecent); setConfig; setAutoSaveCallback
- [x] Add `Tests/Workspace/test_phase47_project_manager.cpp` — 36 test cases / 107 assertions:
  - [x] ProjectManagerState (1 test): all 6 values
  - [x] RecentProjectEntry (2 tests): default invalid, isValid when path set
  - [x] ProjectManagerConfig (1 test): defaults
  - [x] ProjectManager (28 tests): default Idle, newProject, empty path rejected, rejected when open, openProject, markDirty, markDirty no-op when not open, save, save fails when not open, closeProject from Open, closeProject fails when Idle, setError, clearError, clearError no-op, closeProject from Error, tickAutoSave no-op not open, no-op not dirty, no-op disabled, triggers after interval, accumulator reset, callback invoked; recent: pushes on open, multiple in order, dedup bumps to front, cap enforced, clearRecent, removeRecent, removeRecent unknown; setConfig
  - [x] Integration (3 tests): full open/dirty/save/close cycle, auto-save fires multiple times, error then reopen
- [x] Wire `NF_Phase47Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ProjectManager.newProject() rejected if already Open ✓
- ProjectManager.save() only succeeds from Open state ✓
- ProjectManager.closeProject() allowed from both Open and Error states ✓
- tickAutoSave() accumulates dt; triggers auto-save and resets accumulator when interval exceeded ✓
- Recent list is front-inserted, deduplicates by path, capped at maxRecentProjects ✓
- 36 test cases pass (107 assertions) ✓
- Total test suite: ~3133 tests passing ✓

---

## Phase 48 – Workspace Activity Bar

**Status: Done**

- [x] Create `WorkspaceActivityBar.h` — activity bar data model
  - [x] ActivityItemKind enum (Tool/Action/Separator) with `activityItemKindName()` helper
  - [x] ActivityBarItem — id + label + iconKey + toolId + commandId + kind + enabled + pinned; `isValid()`; `isTool/isAction/isSeparator()`; `makeTool/makeAction/makeSeparator()` factories; equality by id
  - [x] ActivityBarSection — named, ordered item collection (MAX_ITEMS=32); `addItem`/`removeItem`/`findItem`/`findItemMut`/`contains`/`count`/`clear`; duplicate id rejected; capacity enforced
  - [x] ActivityBarManager — section registry (MAX_SECTIONS=8); `createSection`/`removeSection`/`findSection`/`hasSection`; `addItem`/`removeItem`/`findItem` (search all sections); `setActiveItem` (guards disabled, fires deactivate-then-activate observers on switch, no-ops if same id); `clearActiveItem`; `enableItem`; observer callbacks (MAX_OBSERVERS=16); `clear()`
- [x] Update `WorkspaceRenderer::renderSidebar()` — TOOLS section prepended above LAUNCH TOOL
  - [x] Each registered IHostedTool gets a 30px card: left accent stripe (blue if active), label, `*` marker for active tool
  - [x] Click active tool → `deactivateTool()` (returns to dashboard); click inactive → `activateTool()`
  - [x] Separator drawn between TOOLS and LAUNCH TOOL sections
  - [x] Hint "(no tools registered)" only if both tool and app registries are empty
- [x] Add `Tests/Workspace/test_phase48_activity_bar.cpp` — 48 test cases / 171 assertions:
  - [x] ActivityItemKind (1 test): all 3 name helpers
  - [x] ActivityBarItem (9 tests): default invalid, valid Tool, Tool without toolId, valid Action, Action without commandId, Separator only needs id, equality by id, defaults
  - [x] ActivityBarSection (11 tests): default empty, addItem, duplicate rejected, removeItem, remove unknown, findItem, findItemMut mutates, Separator adds without kind constraints, clear, MAX_ITEMS enforced
  - [x] ActivityBarManager (24 tests): default empty, createSection, duplicate rejected, empty name rejected, removeSection, remove unknown, findSection, addItem, addItem unknown section, removeItem searches all, removeItem unknown, findItem, setActiveItem, setActiveItem unknown, setActiveItem disabled, clearActiveItem, enableItem, enableItem unknown, observer on setActiveItem, observer deactivate+activate on switch, observer on clearActiveItem, clearObservers, MAX_SECTIONS enforced, no-op if same item, clear
  - [x] Integration (4 tests): multi-section navigator, disable+re-enable, multiple observers, sections() view
- [x] Wire `NF_Phase48Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- ActivityItemKind provides three item types with name helpers ✓
- ActivityBarItem.isValid() correctly gates per-kind required fields ✓
- ActivityBarSection maintains order, rejects duplicates, enforces MAX_ITEMS ✓
- ActivityBarManager.setActiveItem() fires deactivate+activate observers on switch, no-ops if same id ✓
- Sidebar TOOLS section shows all registered tools with active highlight and toggle-click behavior ✓
- Tools accessible from sidebar regardless of which view (dashboard / active tool) is shown ✓
- 48 test cases pass (171 assertions) ✓
- Total test suite: ~3213 tests passing ✓

---

## Phase 49 – Workspace Recent Files

**Status: Done**

- [x] Create `WorkspaceRecentFiles.h` — unified recent-files manager
  - [x] RecentFileKind enum (Project/Scene/Asset/Script/Config/Custom) with `recentFileKindName()` helper
  - [x] RecentFileEntry — path + displayName + kind + lastOpenedMs + pinned + accessCount; `isValid()` (non-empty path); equality by path
  - [x] RecentFileList — MRU ring (MAX_ENTRIES=64); `record` (dedup by path → moves to front + bumps accessCount; evicts oldest unpinned at cap; pinned entries survive eviction; rejects all-pinned overflow); `remove`/`pin`/`findByPath`/`contains`/`mostRecent`; `pinned()`/`unpinned()` views; `pinnedCount`/`count`/`empty`; `clearUnpinned`/`clear`; `appendDirect` (for deserialization)
  - [x] RecentFilesManager — one RecentFileList per kind (6 lists); `record`/`remove`/`pin`/`find`/`listForKind`; `globalRecent()` (merges all kinds, sorts by lastOpenedMs desc, capped at MAX_GLOBAL=32); `clearKind`/`clearAll`/`clearAllUnpinned`; observer callbacks on record/remove (MAX_OBSERVERS=16); `serialize()` / `deserialize()` — pipe-delimited wire format with `\P` escape for pipes in paths; `deserialize` clears existing data before loading
- [x] Add `Tests/Workspace/test_phase49_recent_files.cpp` — 45 test cases / 132 assertions:
  - [x] RecentFileKind (1 test): all 6 name helpers
  - [x] RecentFileEntry (3 tests): default invalid, valid with path, equality by path
  - [x] RecentFileList (16 tests): default empty, record adds to front, empty path rejected, re-record moves to front+bumps count, remove, remove unknown, findByPath, contains, pin/unpin, pin unknown, pinned()/unpinned() views, clearUnpinned leaves pinned, clear removes all, MRU order, capacity evicts oldest unpinned, pinned survives eviction
  - [x] RecentFilesManager (21 tests): default empty, record to correct kind, empty path rejected, find, remove, remove unknown, pin/unpin, pin unknown, globalRecent merges+sorts, globalRecent capped at MAX_GLOBAL, clearKind, clearAll, clearAllUnpinned, observer on record, observer on remove, clearObservers, serialize empty, serialize round-trip, serialize escapes pipe, deserialize empty, deserialize clears existing
  - [x] Integration (4 tests): project open workflow, observer tracks all operations, access-count increments on re-record, full serialize/deserialize preserves accessCount and pinned
- [x] Wire `NF_Phase49Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- RecentFileList.record() dedup by path; moves to front; bumps accessCount ✓
- Pinned entries survive capacity eviction; all-pinned-full rejects new entry ✓
- RecentFilesManager.globalRecent() sorts newest-first across all kinds; capped at MAX_GLOBAL ✓
- clearAllUnpinned() removes only unpinned entries across all kinds ✓
- Serialize/deserialize round-trip is lossless (path, displayName, kind, ts, accessCount, pinned) ✓
- Pipe characters in path/displayName escaped as \P ✓
- Observer fires on record (true) and remove (false) with copy of the entry ✓
- 45 test cases pass (132 assertions) ✓
- Total test suite: ~3258 tests passing ✓

---

## Phase 50 – Workspace Quick-Open Palette

**Status: Done**

- [x] Create `WorkspaceQuickOpen.h` — cmd+P style quick-open data model
  - [x] QuickOpenItemKind enum (File/Tool/Command/Symbol/Custom) with `quickOpenItemKindName()` helper
  - [x] QuickOpenItem — id + label + detail + kind + score; `isValid()` (non-empty id+label); equality by id
  - [x] QuickOpenQuery — text + filterKind/filterByKind + maxResults; `matches(item)` (case-insensitive substring, optional kind filter); `score(item)` → Exact(100) > Prefix(60) > Contains(30) > None(-1)
  - [x] QuickOpenProvider — id + name + `populate` callback; `isValid()` (all fields required)
  - [x] QuickOpenSession — id + up to MAX_PROVIDERS=8 providers; `open`/`close`/`query`/`submit`; `addProvider`/`removeProvider`/`hasProvider`; `results()` (scored, sorted desc, capped at maxResults); `submitted()`/`hasSubmit()`/`clearSubmit()`; query rejects when closed; submit requires open + id in results → closes on success
  - [x] QuickOpenManager — session registry (MAX_SESSIONS=8); `createSession`/`removeSession`/`findSession`/`hasSession`; `notifySubmit` — fires observers after caller calls session.submit(); observer callbacks (MAX_OBSERVERS=16); `clear()`
- [x] Add `Tests/Workspace/test_phase50_quick_open.cpp` — 48 test cases / 124 assertions:
  - [x] QuickOpenItemKind (1 test): all 5 name helpers
  - [x] QuickOpenItem (5 tests): default invalid, valid, no label, no id, equality by id
  - [x] QuickOpenQuery (5 tests): empty matches all, case-insensitive, non-match, score Exact>Prefix>Contains, kind filter
  - [x] QuickOpenProvider (4 tests): invalid no id, invalid no name, invalid no populate, valid
  - [x] QuickOpenSession (17 tests): default closed, addProvider, invalid rejected, duplicate rejected, removeProvider, remove unknown, open, close, query when closed=0, query collects all providers, query filters text, query sorts by score, query caps maxResults, submit valid, submit unknown, submit when closed, open clears submission, MAX_PROVIDERS enforced
  - [x] QuickOpenManager (9 tests): default empty, createSession, duplicate rejected, empty id rejected, removeSession, remove unknown, findSession, observer fires on notifySubmit, clearObservers, MAX_SESSIONS enforced, clear
  - [x] Integration (4 tests): full open-query-submit flow, kind filter narrows, multiple providers merged+ranked, empty query returns all
- [x] Wire `NF_Phase50Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- QuickOpenQuery.score() correctly ranks Exact > Prefix > Contains ✓
- QuickOpenSession.query() collects from all providers, scores, sorts descending, caps at maxResults ✓
- QuickOpenSession.submit() requires open + id in result set; closes session on success ✓
- QuickOpenManager.notifySubmit() fires all observers independently of the session ✓
- 48 test cases pass (124 assertions) ✓
- Total test suite: ~3306 tests passing ✓

---

## Phase 51 – Workspace Keymap System

**Status: Done**

- [x] Create `WorkspaceKeymap.h` — layered keyboard-shortcut configuration
  - [x] KeyModifiers — bitmask (Ctrl/Shift/Alt/Meta); `toString()` produces "Ctrl+Shift+" etc.; equality; `none()` guard
  - [x] KeyChord — key string + modifiers; `isValid()` (non-empty key); `toString()` → "Ctrl+S"; equality on both parts
  - [x] KeyAction — id + chord + description + context (tool/panel id or "" for global); `isValid()` (id + chord.isValid()); equality by id
  - [x] KeymapLayer — id + name + priority + enabled flag; `addAction`/`removeAction`/`findAction`/`findByChord` (context-aware: action context="" matches any, query context="" is wildcard); MAX_ACTIONS=128; `setEnabled`/`clear`
  - [x] KeymapManager — ordered layer stack (sorted descending by priority so highest wins first); permanent `"default"` layer (priority=0, not removable); `addLayer`/`removeLayer`/`findLayer`/`hasLayer`/`setLayerEnabled`; `registerAction`/`unregisterAction` (on default layer); `lookup(chord, context)` → first match across enabled layers; `lookupAll(chord, context)` → all matches; `findAction(id)` → any layer; observer callbacks (MAX_OBSERVERS=16); `serialize()` / `deserialize()` — tab-delimited text; `clear()` resets to empty default layer
- [x] Add `Tests/Workspace/test_phase51_keymap.cpp` — 46 test cases / 115 assertions:
  - [x] KeyModifiers (4): default none, single flags, combined, equality
  - [x] KeyChord (4): default invalid, key only, with modifiers, equality
  - [x] KeyAction (4): default invalid, no chord key, valid, equality by id
  - [x] KeymapLayer (11): default empty, addAction, invalid rejected, duplicate rejected, removeAction, remove unknown, findAction, findByChord, context filter, setEnabled, clear
  - [x] KeymapManager (19): default default layer, addLayer, duplicate rejected, empty id rejected, removeLayer, cannot remove default, remove unknown, registerAction, unregisterAction, findAction all layers, lookup highest priority, lookup nullptr no match, disabled layer skipped, lookupAll all matches, observer on register, observer on addLayer, observer on setEnabled, clearObservers, serialize round-trip, deserialize empty, clear resets
  - [x] Integration (2): multi-layer priority resolution with contexts, full serialize/deserialize preserves layers
- [x] Wire `NF_Phase51Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- KeymapLayer.findByChord() correctly applies context filter ✓
- KeymapManager.lookup() resolves highest-priority enabled layer ✓
- Disabled layers are skipped; default layer is always present ✓
- Serialize/deserialize round-trip preserves all action fields ✓
- 46 test cases pass (115 assertions) ✓
- Total test suite: ~3352 tests passing ✓

---

## Phase 52 – Workspace Window State

**Status: Done**

- [x] Create `WorkspaceWindowState.h` — persistent window geometry and monitor-aware restore
  - [x] WindowBounds — x/y/width/height + isMaximized + isMinimized; `isValid()` (w>0, h>0); `contains(px,py)` (center-inclusive, right/bottom exclusive); equality
  - [x] MonitorInfo — id + name + bounds + scaleFactor + isPrimary; `isValid()` (non-empty id + valid bounds)
  - [x] WindowStateEntry — windowId + bounds + monitorId + workspaceId + lastSavedMs; `isValid()` (non-empty id + valid bounds)
  - [x] WindowStateManager — monitor registry (MAX_MONITORS=8): `addMonitor`/`removeMonitor`/`findMonitor`/`primaryMonitor()` (enforces single primary; adding a second primary clears the first); entry store (MAX_ENTRIES=64): `save`/`restore`/`remove`/`has`; `isOnMonitor(entry,monitorId)` (tests center point against work area); `monitorForEntry()` (returns containing monitor or primary as fallback); observer callbacks (MAX_OBSERVERS=16); `serialize()`/`deserialize()` tab-delimited; `clear()`
- [x] Add `Tests/Workspace/test_phase52_window_state.cpp` — 38 test cases / 88 assertions:
  - [x] WindowBounds (6): default invalid, valid, zero dim invalid, contains point, equality, maximized flag
  - [x] MonitorInfo (2): default invalid, valid
  - [x] WindowStateEntry (3): default invalid, valid, no id
  - [x] WindowStateManager (24): default empty, addMonitor, invalid rejected, duplicate rejected, only one primary, removeMonitor, remove unknown, save, invalid save, save updates existing, restore, restore unknown, remove, remove unknown, isOnMonitor true, isOnMonitor false, monitorForEntry fallback to primary, observer on save, observer on remove, clearObservers, serialize empty, serialize round-trip, deserialize empty, deserialize clears existing, clear
  - [x] Integration (2): multi-monitor layout save and restore, orphaned window fallback to primary
- [x] Wire `NF_Phase52Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- addMonitor() with isPrimary=true clears existing primary flag ✓
- isOnMonitor() correctly tests center-point containment ✓
- monitorForEntry() falls back to primary when no monitor contains the window ✓
- save() upserts (creates or updates) without duplicating entries ✓
- serialize/deserialize round-trip is lossless ✓
- 38 test cases pass (88 assertions) ✓
- Total test suite: ~3390 tests passing ✓

---

## Phase 53 – Workspace Project Template

**Status: Done**

- [x] Create `WorkspaceProjectTemplate.h` — project template catalogue and instantiation
  - [x] TemplateCategory — id + label + description; `isValid()` (id + label required)
  - [x] TemplateFileStub — relativePath + contentTemplate (may contain `{{VAR}}`); `isValid()` (path required)
  - [x] TemplateVariable — key + defaultValue + description + required flag; `isValid()` (key required)
  - [x] TemplateDefinition — id + name + categoryId + description + version; `addStub`/`removeStub`/`findStub` (MAX_STUBS=64, unique paths); `addVariable`/`removeVariable`/`findVariable` (MAX_VARS=32, unique keys); `substitute(text, vars)` — replaces all `{{KEY}}` tokens, falls back to `defaultValue`, supports multiple occurrences; `isValid()` (id + name required)
  - [x] TemplateInstance — result of instantiate(): templateId + resolved variable map + resolvedFiles (path→content); `isComplete()` (no missing required vars); `missingRequired()` — list of required key names with empty values
  - [x] TemplateRegistry — category store (MAX_CATEGORIES=32, addCategory/removeCategory/findCategory/hasCategory); template store (MAX_TEMPLATES=256, addTemplate/removeTemplate/findTemplate/hasTemplate/findByCategory); `instantiate(id, vars)` — resolves vars, substitutes all stubs, reports missing required vars; observer callbacks (MAX_OBSERVERS=16); `clear()`
- [x] Add `Tests/Workspace/test_phase53_project_template.cpp` — 43 test cases / 76 assertions:
  - [x] TemplateCategory (2): valid, invalid without id or label
  - [x] TemplateFileStub (2): valid, invalid without path
  - [x] TemplateVariable (2): valid, invalid without key
  - [x] TemplateDefinition (13): valid, invalid, addStub/invalid/duplicate/remove/find, addVariable/invalid/duplicate/remove, substitute replaces, substitute default, substitute multiple occurrences
  - [x] TemplateRegistry (18): empty, addCategory/invalid/duplicate, removeCategory/unknown, addTemplate/invalid/duplicate, removeTemplate/unknown, findByCategory, instantiate resolved files, instantiate default, instantiate missing required, instantiate unknown, observer on add/remove, clearObservers, clear
  - [x] Integration (2): multi-file project instantiation, missing required var detection
- [x] Wire `NF_Phase53Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- substitute() replaces all `{{KEY}}` occurrences including multiples ✓
- substitute() falls back to defaultValue when key absent from supplied vars ✓
- instantiate() reports missing required vars without preventing file generation ✓
- findByCategory() filters correctly; empty category returns empty vector ✓
- 43 test cases pass (76 assertions) ✓
- Total test suite: ~3473 tests passing ✓

---

## Phase 54 – Workspace Breadcrumb Navigation

**Status: Done**

- [x] Create `WorkspaceBreadcrumb.h` — hierarchical navigation breadcrumb trail
  - [x] BreadcrumbItemKind — Root / Category / Item / Leaf; `breadcrumbItemKindName()` helper
  - [x] BreadcrumbItem — id + label + kind + iconKey + contextData; `isValid()` (id + label); equality by id
  - [x] BreadcrumbTrail — ordered stack; `push` (rejects invalid / duplicate id / at MAX_DEPTH=32); `pop`; `current()` (top); `root()` (front); `contains`/`findById`; `truncateTo(id)` (pop all above id); `clear()`; equality
  - [x] BreadcrumbHistory — circular history (MAX_HISTORY=16); `push(trail)` — evicts oldest when full, clears forward history; `back()`/`forward()` — move cursor, return trail pointer; `canBack()`/`canForward()`; `current()`; `clear()`
  - [x] BreadcrumbManager — owns one active BreadcrumbTrail + BreadcrumbHistory; `navigate(item)` — push + record; `popTo(id)` — truncate + record; `pop()` — pop one + record; `back()`/`forward()` — restore from history; `canBack()`/`canForward()`; `reset()`; observer callbacks (MAX_OBSERVERS=16)
- [x] Add `Tests/Workspace/test_phase54_breadcrumb.cpp` — 40 test cases / 107 assertions:
  - [x] BreadcrumbItemKind (1): all 4 name helpers
  - [x] BreadcrumbItem (4): default invalid, valid, no label, equality by id
  - [x] BreadcrumbTrail (11): default empty, push, invalid/duplicate rejected, pop, pop empty, contains/findById, truncateTo, truncateTo unknown, root stays front, clear, equality
  - [x] BreadcrumbHistory (6): default empty, push records, back+forward, back at start, push clears forward, clear
  - [x] BreadcrumbManager (15): navigate, invalid rejected, popTo, popTo unknown, pop, pop empty, back, forward, canBack/canForward, reset, observer on navigate/popTo/back+forward/reset, clearObservers
  - [x] Integration (2): full drill-down + back navigation; popTo mid-trail then continue
- [x] Wire `NF_Phase54Tests` into Tests/CMakeLists.txt

**Success Criteria:**
- BreadcrumbTrail.push() rejects duplicate ids in trail ✓
- BreadcrumbTrail.truncateTo() pops precisely to target item ✓
- BreadcrumbHistory.push() clears forward history ✓
- BreadcrumbManager.navigate() branches correctly after back+navigate ✓
- 40 test cases pass (107 assertions) ✓
- Total test suite: 3473 tests passing ✓
