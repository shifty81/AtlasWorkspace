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

**Status: Partial (stubs done, implementation planned)**

- [x] Create backend selector contract (UIBackendSelector.h)
- [x] Mark GDI as fallback only
- [x] Add D3D11 backend stub
- [x] Add DirectWrite text backend stub (ITextBackend interface)
- [ ] Isolate legacy OpenGL/GLFW paths
- [ ] Formalize backend interface split (layout/input, primitives, text, texture)
- [ ] Implement D3D11 backend
- [ ] Implement DirectWrite text backend

**Success Criteria:**
- Backend selector contract exists
- GDI explicitly fallback-only
- D3D11/DirectWrite path formally targeted
- Legacy compatibility paths isolated

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

**Status: Planned**

- [ ] NovaForge as hosted project with full adapter
- [ ] Project loading contracts
- [ ] Build gating for hosted projects
- [ ] Plugin/project model for future projects

---

## Phase 6 – Build, Patch, and Release Pipeline

**Status: Planned**

- [ ] Stabilize build presets and dependency policy
- [ ] Finalize patch apply/remove workflow
- [ ] Improve repo audit tooling
- [ ] Define packaging and release path
