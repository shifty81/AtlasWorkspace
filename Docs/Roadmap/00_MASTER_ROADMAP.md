# Master Roadmap

> **End Goal:** Atlas Workspace becomes a clean generic host that can load NovaForge
> from `.atlas`, present real editable documents and runtime-backed preview panels,
> run the shared NovaForge PCG pipeline in-editor, and support fast Play-In-Editor
> plus standalone game validation.
>
> Everything below exists to reach that goal. No open-ended brainstorm.

---

## Checkpoint — Foundation Work (Phases 0–71, Complete)

All infrastructure phases are done and checked. They delivered:

- Canon docs, naming scrub, repo hygiene (Phase 0)
- WorkspaceShell, ToolRegistry, PanelRegistry (Phase 1)
- UIBackend contracts, D3D11/DirectWrite stubs, GDI fallback (Phase 2)
- Editor consolidation, 8 hosted tools, shared panels (Phase 3)
- AtlasAI broker flow, Codex mirroring (Phase 4)
- ProjectRegistry, ProjectLoadContract, NovaForge adapter (Phase 5)
- Build presets, patch applier, release manifest (Phase 6)
- ViewportHostContract, TypographySystem (Phase 7)
- WorkspaceBootstrap, FrameController, ConsoleCommandBus, SelectionService (Phase 8)
- AssetCatalog, AssetTransformer, ContentRouter, AssetWatcher (Phase 9)
- WorkspaceProjectFile, ProjectSerializer, SettingsStore (Phase 10)
- CommandRegistry, CommandHistory, ActionMap (Phase 11)
- EventBus, NotificationBus (Phase 12)
- PreferenceRegistry, PreferenceController (Phase 13)
- PluginSystem (Phase 14)
- DiagnosticCollector, TelemetryCollector (Phase 15)
- ScriptEngine, AutomationTask (Phase 16)
- SearchEngine, SearchIndex (Phase 17)
- UndoStack, UndoManager (Phase 18)
- SessionManager (Phase 19)
- ClipboardManager (Phase 20)
- Viewport rendering contracts, viewport manager, end-to-end wiring (Phases 64–66)
- Asset browser, text input, tool render contract, NovaForge panel factories (Phases 67–70)
- EditorApp deprecation, AssetCatalogPopulator, settings/layout persistence, validator fixes (Phase 71)

**What this delivered:** Contracts, shells, registries, descriptors, and test infrastructure.

**What this did NOT deliver:** Real data-backed editor loops. Panels render chrome but
do not bind to editable game state. Viewport shows placeholder grids. NovaForge panels
have titles but no schemas. PCG is not connected. There is no Play-In-Editor mode.

That is what the phases below address.

---

## Phase A — Truth and Cleanup Lock

**Status: In Progress**

**Goal:** Stop repo drift. Make the roadmap, status docs, and codebase honest about
what works vs. what is a stub. Remove forwarding shims and legacy cruft that creates
the illusion of completeness.

### Milestone A.1 — Repo Cleanup
- [x] Archive remaining forwarding shims between `Source/Editor` and `Source/Workspace`
- [x] Remove stale `EditorApp` references from non-test code
- [x] Rename `ArbiterReasoner.cpp` in pipeline to `AtlasAIReasoner.cpp`
- [x] Slim `main.cpp` — extract `LocalProjectAdapter` into dedicated unit
- [x] Remove overclaiming language from roadmap/status docs (presence ≠ completion)

### Milestone A.2 — Build and CI Hygiene
- [x] Verify all test suites build and pass on clean checkout (4125 tests, all green)
- [x] Ensure `validate_project.sh` stays at 79/79 (confirmed)
- [ ] Add CI smoke build for `ATLAS_ENABLE_ONLINE_DEPS=ON`

### Milestone A.3 — Documentation Correction
- [x] Rewrite `Docs/Canon/00_PROJECT_STATUS.md` to reflect honest wiring state
- [x] Mark stale phase-specific roadmap docs (01–07) as historical
- [x] Update `Docs/Canon/09_DEFINITION_OF_DONE.md` to include "panel edits real data" criteria

**Success Criteria:** ✅
- Zero forwarding shims between Editor↔Workspace (37 archived to Deprecated/)
- All naming violations resolved (ArbiterReasoner → AtlasAIReasoner)
- Status docs accurately describe stub vs. wired state
- Build passes on clean checkout (4125 tests green)

---

## Phase B — Real Project Load

**Status: Not Started**

**Goal:** When a user opens `NovaForge.atlas`, the workspace loads real project state—
not just adapter descriptors. Asset registries, gameplay data registries, and document
registries all populate from project files on disk.

### Milestone B.1 — .atlas Manifest Contract
- [ ] Extend `AtlasProjectFileLoader` to resolve all manifest fields into a typed `ProjectBootstrapResult`
- [ ] Validate content roots, data roots, schema roots, and asset registry locations on load
- [ ] Report missing/invalid paths as structured `ProjectLoadContract` errors

### Milestone B.2 — NovaForge Project Bootstrap
- [ ] Create `NovaForgeProjectBootstrap` service in `NovaForge/Source/EditorAdapter/`
  - Validates NovaForge project structure against `.atlas` manifest
  - Loads asset registry from `Content/` root into `AssetCatalog`
  - Loads gameplay data registries from `Data/` root
  - Populates `ProjectSystemsTool` with real data-backed panel descriptors
- [ ] Wire bootstrap into `NovaForgeAdapter::initialize()` flow

### Milestone B.3 — NovaForge Document Registry
- [ ] Define `NovaForgeDocumentType` enum covering all authoring targets:
  - Item definition, structure archetype, biome definition, planet archetype
  - Faction definition, mission definition, progression rules
  - Character rules, economy rules, crafting definitions, PCG rulesets
- [ ] Create `NovaForgeDocumentRegistry` — maps document type → schema + load/save paths
- [ ] Create `NovaForgeDocument` — base class with dirty tracking, validate, save/apply/revert

### Milestone B.4 — Asset Registry Population
- [ ] Extend `AssetCatalogPopulator` to scan project content root recursively
- [ ] Register discovered assets in `AssetCatalog` with full metadata (type, path, GUID)
- [ ] Wire asset catalog into `ContentBrowserPanel` for live display
- [ ] Wire asset selection into `InspectorPanel` for property display

**Success Criteria:**
- Opening `NovaForge.atlas` populates 3+ registries with real data
- `ContentBrowserPanel` shows actual project assets
- `InspectorPanel` shows selected asset metadata
- Invalid project structure produces actionable error messages

---

## Phase C — Panels Edit Real Data

**Status: Not Started**

**Goal:** Gameplay and data panels stop being empty shells. Each panel binds to a
NovaForge document, loads real schema, and supports edit/save/revert.

### Milestone C.1 — Schema-Backed Panel Framework
- [ ] Create `IDocumentPanel` interface extending `IEditorPanel`:
  - `bindDocument(NovaForgeDocument&)`
  - `isDirty()`, `save()`, `revert()`, `validate()`
  - `onDocumentChanged()` callback
- [ ] Wire dirty tracking through `UndoStack` per-panel
- [ ] Wire save/revert through `CommandRegistry` (Ctrl+S, Ctrl+Z)

### Milestone C.2 — NovaForge Gameplay Panels (Real Content)
- [ ] `EconomyPanel` — currency editor, pricing rules, economy balance preview
- [ ] `InventoryRulesPanel` — slot layout editor, storage rules, stacking config
- [ ] `ShopPanel` — store listing editor, purchase condition rules
- [ ] `MissionRulesPanel` — quest objective editor, chain editor, reward tables
- [ ] `ProgressionPanel` — XP curve editor, level thresholds, skill unlock tree
- [ ] `CharacterRulesPanel` — creation presets, class editor, stat cap tables

### Milestone C.3 — Data Editor Tool (Real Content)
- [ ] Schema-driven property grid for arbitrary NovaForge data tables
- [ ] Load/save JSON or custom NovaForge data formats
- [ ] Validation feedback per field
- [ ] Dirty state indicator in tool tab

### Milestone C.4 — Panel Save/Apply Pipeline
- [ ] Document save writes back to project data directory
- [ ] Undo/redo stack per document
- [ ] Dirty indicator in panel tab/title
- [ ] Validation failure messaging in notification bus

**Success Criteria:**
- All 6 NovaForge panels load and display real data from project files
- Edits in any panel produce dirty state and can be saved/reverted
- Undo/redo works for all panel edits
- Invalid data produces visible validation errors

---

## Phase D — Runtime-Backed Viewport

**Status: Not Started**

**Goal:** Viewport is no longer a placeholder. It renders real NovaForge preview scenes
using the same runtime code the game client uses.

### Milestone D.1 — NovaForge Preview Runtime Bridge
- [ ] Create `NovaForgePreviewRuntime` — lightweight runtime instance for editor preview
  - Instantiates NovaForge world/scene state
  - Provides `IViewportSceneProvider` implementation
  - Supports scene update tick without full game boot
- [ ] Create `NovaForgePreviewWorld` — editable preview scene container
  - Entity instantiation and destruction
  - Transform manipulation
  - Material/mesh assignment

### Milestone D.2 — Scene Editor Viewport
- [ ] Wire `SceneEditorTool` to `NovaForgePreviewRuntime` as its scene provider
- [ ] Implement fly-camera controls (WASD + mouse look)
- [ ] Implement gizmo rendering bound to actual selected entities
- [ ] Selection → Inspector binding (selected entity properties editable)
- [ ] Entity hierarchy → Outliner binding (live hierarchy display)

### Milestone D.3 — Asset Preview Viewport
- [ ] Wire `AssetEditorTool` to a per-asset preview scene
- [ ] Preview shows selected asset as NovaForge would render it in-game
- [ ] Editable transform, material, attachment metadata in side inspector
- [ ] Collider/socket/anchor editing for asset placement metadata
- [ ] PCG tag and placement metadata editing

### Milestone D.4 — Material Preview Viewport
- [ ] Wire `MaterialEditorTool` to a test-mesh preview scene
- [ ] Live material parameter editing with viewport refresh
- [ ] Standard preview meshes (sphere, cube, plane)

### Milestone D.5 — D3D11 Backend Activation
- [ ] Activate D3D11 backend as primary (GDI becomes fallback-only in practice)
- [ ] Wire DirectWrite text rendering
- [ ] Verify all panels render correctly through D3D11 path

**Success Criteria:**
- Scene editor viewport shows a real NovaForge world/scene
- Asset editor shows 3D asset previews with editable metadata
- Material editor shows live material preview on test mesh
- Camera controls work (WASD + mouse)
- Selection gizmos bound to actual objects
- D3D11 is the active rendering backend

---

## Phase E — Shared PCG Preview Pipeline

**Status: Not Started**

**Goal:** Editor and game client use the same PCG core. Panels can edit PCG rules and
see regenerated results in the viewport immediately.

### Milestone E.1 — Shared NovaForge PCG Core
- [ ] Factor PCG generation code into a shared library usable by both client and editor
- [ ] Define `PCGRuleSet` — typed rule container loaded from project data
- [ ] Define `PCGGeneratorService` — stateless generator that takes rules + seed → output
- [ ] Define `PCGDeterministicSeedContext` — reproducible seed management

### Milestone E.2 — Editor PCG Preview Service
- [ ] Create `PCGPreviewService` — editor-side wrapper around shared PCG core
  - Accepts rule document + seed
  - Generates preview output (meshes, placements, structure fragments)
  - Caches results for viewport display
- [ ] Wire PCG preview into viewport scene provider
- [ ] Support manual regeneration trigger (button/shortcut)
- [ ] Support automatic regeneration on rule edit

### Milestone E.3 — PCG Rule Editing
- [ ] Wire `ProcGenRuleEditorV1` panel to real `PCGRuleSet` documents
- [ ] Edits in rule panel trigger preview regeneration
- [ ] Save-back translates rule changes to project data
- [ ] Preview deterministic seeds for reproducible testing

### Milestone E.4 — Asset PCG Metadata
- [ ] Asset editor exposes PCG placement tags and generation constraints
- [ ] PCG preview service uses asset metadata for placement/generation
- [ ] Changes to asset PCG tags trigger preview update

**Success Criteria:**
- Editor uses the same PCG generation code as game client
- Rule edits trigger live preview regeneration in viewport
- PCG output renders in viewport as 3D scene
- Deterministic seeds produce identical output
- Asset PCG tags drive placement behavior

---

## Phase F — Play-In-Editor (PIE)

**Status: Not Started**

**Goal:** Test gameplay from inside the workspace. Fast iteration loop for authoring.

### Milestone F.1 — Embedded PIE Runtime
- [ ] Create `PIEService` — manages embedded runtime instance lifecycle
  - `enter()` — duplicate/snapshot current editor world state
  - `exit()` — destroy runtime, restore editor state
  - `pause()` / `resume()` / `step()` — runtime control
  - `reset()` — restart from last snapshot
- [ ] Runtime instance runs in viewport panel (same render surface)
- [ ] Editor panels switch to read-only during PIE

### Milestone F.2 — PIE Input Mode
- [ ] Input routing switches from editor controls to game controls on PIE enter
- [ ] Clean restore to editor input on PIE exit
- [ ] PIE toolbar: Play/Pause/Stop/Step buttons
- [ ] Escape or toolbar Stop exits PIE cleanly

### Milestone F.3 — External Game Launch
- [ ] Launch `NovaForgeServer` or game client as external process from workspace
- [ ] Build → Launch pipeline through `BuildTool`
- [ ] Console output from external process routed to `ConsolePanel`
- [ ] Launch configuration stored in `.atlas` manifest

### Milestone F.4 — PIE Diagnostics
- [ ] Runtime errors during PIE surface in notification bus
- [ ] Performance counters visible during PIE (FPS, entity count, memory)
- [ ] PIE session recording for replay/debugging

**Success Criteria:**
- PIE enters and exits cleanly without corrupting editor state
- Gameplay is testable inside the viewport
- Input routing switches correctly between editor and game
- External game launch works as full validation path
- Runtime errors visible in editor during PIE

---

## Phase G — Full Tool Wiring

**Status: Not Started**

**Goal:** All core hosted tools become functional for real development work.

### Milestone G.1 — Scene Editor
- [ ] World/level document model with entity hierarchy
- [ ] Entity create/delete/duplicate
- [ ] Transform editing via gizmo and inspector
- [ ] Component add/remove/edit
- [ ] Scene save/load

### Milestone G.2 — Asset Editor
- [ ] Asset document model with metadata editing
- [ ] 3D preview with editable transform/material/attachment
- [ ] LOD/variant metadata editing
- [ ] Import/reimport pipeline integration
- [ ] Asset dependency viewer

### Milestone G.3 — Material Editor
- [ ] Material graph document model
- [ ] Node-based shader graph editing
- [ ] Live preview on test meshes
- [ ] Parameter editing with immediate viewport refresh
- [ ] Save/apply/revert

### Milestone G.4 — Animation Editor
- [ ] Animation clip document model
- [ ] Timeline editing (keyframes, curves)
- [ ] Skeleton/rig preview in viewport
- [ ] Clip playback controls
- [ ] Blend tree editing

### Milestone G.5 — Visual Logic Editor
- [ ] Graph document model
- [ ] Node palette and graph canvas
- [ ] Compile/validate with error reporting
- [ ] Test-run preview in viewport
- [ ] Runtime hook point binding

### Milestone G.6 — Build Tool
- [ ] Real build task graph execution
- [ ] Build output stream to console panel
- [ ] Error/warning parsing with file links
- [ ] Launch targets from `.atlas` build profiles
- [ ] AtlasAI build error analysis handoff

### Milestone G.7 — AtlasAI Tool
- [ ] Context-aware request input (selected object, active document, current errors)
- [ ] Diff proposal display
- [ ] Apply/reject workflow
- [ ] Codex snippet management

### Milestone G.8 — Data Editor
- [ ] Generic schema-driven property grid
- [ ] Table/list/tree views for structured data
- [ ] JSON/custom format load/save
- [ ] Validation per field with error display

**Success Criteria:**
- All 8 core tools operate on real documents
- Each tool supports create/edit/save/undo/redo
- Tools interact correctly with shared panels (inspector, outliner, console)
- Selection in one tool updates shared panels

---

## Phase H — UX Completion

**Status: Not Started**

**Goal:** Workspace becomes stable and usable for daily development.

### Milestone H.1 — Preferences UI
- [ ] Preferences window with category navigation
- [ ] All 13+ registered preferences editable via UI
- [ ] Project vs. user preference scoping visible
- [ ] Import/export/reset settings

### Milestone H.2 — Keybind UI
- [ ] Keybinding editor panel
- [ ] Per-tool and global shortcut display
- [ ] Rebind via capture (press desired key combo)
- [ ] Conflict detection and resolution
- [ ] Reset to defaults

### Milestone H.3 — Layout Persistence
- [ ] Panel layout saves to disk on close
- [ ] Layout restores on next open
- [ ] Named layout presets (save/load/rename/delete)
- [ ] Built-in layout presets (Default, Compact, Wide)

### Milestone H.4 — Command Palette
- [ ] Ctrl+P opens searchable command list
- [ ] Commands filtered by current context (tool, panel, scope)
- [ ] Recent commands section
- [ ] Fuzzy search matching

### Milestone H.5 — Notification Center
- [ ] Notification panel with history
- [ ] Toast popups for important events
- [ ] Severity filtering (info/warning/error/critical)
- [ ] Click-to-navigate for actionable notifications

### Milestone H.6 — Project Open Flow
- [ ] Recent projects list on startup
- [ ] File → Open Project flow with `.atlas` file picker
- [ ] Project validation on open with error summary
- [ ] New project wizard (create `.atlas` from template)

**Success Criteria:**
- Preferences editable and persisted
- Keybinds customizable and conflict-free
- Panel layout survives restart
- Command palette provides fast access to all commands
- Notifications visible and actionable
- Project open flow is clean and error-tolerant

---

## Version Target

When Phases A–H are complete, the repository reaches **Atlas Workspace v1.0** — a
functional development workspace that loads NovaForge projects, presents real editable
content, renders runtime-backed viewport previews, runs shared PCG generation, and
supports Play-In-Editor for rapid iteration.

All further work (multiplayer preview, advanced VFX, cloud sync, marketplace) builds
on top of v1.0 and is not part of this roadmap.
