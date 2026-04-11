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
