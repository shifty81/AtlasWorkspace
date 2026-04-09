# Master Roadmap

This is the execution ladder. Every line is tied to a real milestone. No brainstorm clutter.

---

## Phase 0 – Canon Reset and Consolidation

**Status: In Progress**

- [x] Add `.gitattributes` for line-ending normalization
- [x] Tighten `.gitignore`
- [x] Gate online test dependencies (`ATLAS_ENABLE_ONLINE_DEPS`)
- [x] Rewrite README with current canon
- [x] Create Canon docs
- [x] Create Roadmap docs
- [x] Create Inventory docs
- [ ] Active-path Arbiter → AtlasAI rename
- [ ] Workspace bootstrap naming cleanup
- [ ] GDI/OpenGL fallback marking
- [ ] Editor umbrella include reduction
- [ ] Editor inventory and consolidation plan
- [ ] Archive stale tools and docs

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

**Status: Planned**

- [ ] Clean AtlasWorkspace.exe host bootstrap
- [ ] Rationalize app/tool registry
- [ ] Wire project adapter contract
- [ ] Enforce generic workspace core
- [ ] Remove project-specific leakage

**Success Criteria:**
- Host bootstrap is clean and deterministic
- Tool registry is rationalized
- Project adapter contract exists
- No game-specific logic in workspace core

---

## Phase 2 – AtlasUI Backend Strategy

**Status: Planned**

- [ ] Create backend selector contract
- [ ] Mark GDI as fallback only
- [ ] Add D3D11 backend stub
- [ ] Add DirectWrite text backend stub
- [ ] Isolate legacy OpenGL/GLFW paths
- [ ] Formalize backend interface split

**Success Criteria:**
- Backend selector contract exists
- GDI explicitly fallback-only
- D3D11/DirectWrite path formally targeted
- Legacy compatibility paths isolated

---

## Phase 3 – Editor Consolidation

**Status: Planned**

- [ ] Reduce primary tool roster to ~10
- [ ] Extract shared panels from standalone editors
- [ ] Remove one-off tools from active registry
- [ ] Host NovaForge gameplay panels through adapter
- [ ] Create EditorToolRegistry and EditorSharedPanels headers

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
