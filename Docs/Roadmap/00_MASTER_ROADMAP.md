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

**Status: In Progress**

- [ ] IHostedTool interface and ToolDescriptor
- [ ] ToolRegistry — tool registration, lookup, lifecycle
- [ ] PanelRegistry — shared panel registration and context binding
- [ ] WorkspaceShell — composition root owning registries, managers, project adapter
- [ ] Wire WorkspaceShell into EditorApp bootstrap
- [ ] Project adapter loading through WorkspaceShell
- [ ] Remove project-specific leakage from workspace core
- [ ] Tests for WorkspaceShell, ToolRegistry, PanelRegistry

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

**Status: Partial (headers split, full consolidation planned)**

- [x] Create EditorToolRegistry and EditorSharedPanels headers
- [x] Host NovaForge gameplay panels through adapter
- [ ] Reduce primary tool roster to ~10
- [ ] Extract shared panels from standalone editors
- [ ] Remove one-off tools from active registry
- [ ] Merge related editors into broader host tools

**Success Criteria:**
- Primary tool roster matches canon
- Shared panel extraction is real
- One-off tools removed from active registry
- NovaForge gameplay panels hosted through adapter

---

## Phase 4 – AtlasAI and Codex Integration

**Status: Planned**

- [ ] Complete AtlasAI naming migration
- [ ] Formalize broker flow
- [ ] Wire build-log routing into AtlasAI
- [ ] Define Codex mirroring, validation, deduplication
- [ ] Define snippet promotion rules

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
