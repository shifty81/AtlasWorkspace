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

**Status: Complete**

**Goal:** When a user opens `NovaForge.atlas`, the workspace loads real project state—
not just adapter descriptors. Asset registries, gameplay data registries, and document
registries all populate from project files on disk.

### Milestone B.1 — .atlas Manifest Contract ✅
- [x] Extend `AtlasProjectFileLoader` to resolve all manifest fields into a typed `ProjectBootstrapResult`
- [x] Validate content roots, data roots, schema roots, and asset registry locations on load
- [x] Report missing/invalid paths as structured `ProjectLoadContract` errors

### Milestone B.2 — NovaForge Project Bootstrap ✅
- [x] Create `NovaForgeProjectBootstrap` service in `NovaForge/Source/EditorAdapter/`
  - Validates NovaForge project structure against `.atlas` manifest
  - Loads asset registry from `Content/` root into `AssetCatalog`
  - Loads gameplay data registries from `Data/` root
  - Populates `ProjectSystemsTool` with real data-backed panel descriptors
- [x] Wire bootstrap into `NovaForgeAdapter::initialize()` flow

### Milestone B.3 — NovaForge Document Registry ✅
- [x] Define `NovaForgeDocumentType` enum covering all authoring targets:
  - Item definition, structure archetype, biome definition, planet archetype
  - Faction definition, mission definition, progression rules
  - Character rules, economy rules, crafting definitions, PCG rulesets
- [x] Create `NovaForgeDocumentRegistry` — maps document type → schema + load/save paths
- [x] Create `NovaForgeDocument` — base class with dirty tracking, validate, save/apply/revert

### Milestone B.4 — Asset Registry Population ✅
- [x] Extend `AssetCatalogPopulator` to scan project content root recursively
- [x] Register discovered assets in `AssetCatalog` with full metadata (type, path, GUID)
- [x] Wire asset catalog into `ContentBrowserPanel` for live display
- [x] Wire asset selection into `InspectorPanel` for property display

**Success Criteria:** ✅
- Opening `NovaForge.atlas` populates 3+ registries with real data
- `ContentBrowserPanel` shows actual project assets
- `InspectorPanel` shows selected asset metadata
- Invalid project structure produces actionable error messages
- 40+ tests, all green (test_phase_b.cpp)

---

## Phase C — Panels Edit Real Data

**Status: Complete**

**Goal:** Gameplay and data panels stop being empty shells. Each panel binds to a
NovaForge document, loads real schema, and supports edit/save/revert.

### Milestone C.1 — Schema-Backed Panel Framework ✅
- [x] Create `IDocumentPanel` interface extending `IEditorPanel`:
  - `bindDocument(NovaForgeDocument*)`, `boundDocument()`, `hasDocument()`
  - `isDirty()`, `save()`, `revert()`, `validate()`
  - `onDocumentChanged()` callback, `dirtyTitle()`, `setOnDirtyCallback()`
- [x] `DocumentPanelBase` — common base: per-panel undo stack, dirty tracking,
      save/revert scaffolding, `loadFromDocument` / `applyToDocument` hooks
- [x] Per-panel `UndoStack` integrated via `pushPropertyEdit` helper

### Milestone C.2 — NovaForge Gameplay Panels (Real Content) ✅
- [x] `EconomyPanel` — currency definitions, pricing rules, inflation rate, undo/redo
- [x] `InventoryRulesPanel` — slot layout, storage rules, stacking config, undo/redo
- [x] `ShopPanel` — store listings, purchase conditions, global discount, undo/redo
- [x] `MissionRulesPanel` — objectives (Kill/Collect/Reach/…), chains, rewards, undo/redo
- [x] `ProgressionPanel` — XP curve (level thresholds), skill unlock tree, undo/redo
- [x] `CharacterRulesPanel` — class presets, stat caps, appearance config, undo/redo

### Milestone C.3 — Data Editor Tool (Real Content) ✅
- [x] `DocumentPropertyGrid` — schema-driven property grid (String/Float/Int/Bool/Enum)
- [x] `DocumentPropertyGridBuilder` — fluent builder for schema definition
- [x] Per-field validator callbacks, Enum option validation
- [x] `toFlatMap()` serialization, `resetToDefaults()`, dirty tracking per grid

### Milestone C.4 — Panel Save/Apply Pipeline ✅
- [x] `DocumentSavePipeline` — validates, writes JSON to project data directory
- [x] Per-panel undo/redo stack (cleared on successful save)
- [x] Dirty indicator via `dirtyTitle()` ("*" suffix when dirty)
- [x] Validation failure messaging via `NotificationCallback` sink
- [x] `revert()` — reloads from document and clears dirty state

**Success Criteria:** ✅
- All 6 NovaForge panels bind documents, load real data schemas, and support edit/save/revert
- Edits in any panel produce dirty state and can be saved/reverted
- Undo/redo works for all panel edits (per-panel UndoStack)
- Invalid data produces visible validation errors (ValidationFailed result + notifications)
- 58 new tests, 104 assertions, all green (test_phase_c.cpp)

---

## Phase D — Runtime-Backed Viewport

**Status: In Progress**

**Goal:** Viewport is no longer a placeholder. It renders real NovaForge preview scenes
using the same runtime code the game client uses.

### Milestone D.1 — NovaForge Preview Runtime Bridge ✅
- [x] Create `NovaForgePreviewRuntime` — lightweight runtime instance for editor preview
  - Instantiates NovaForge world/scene state
  - Provides `IViewportSceneProvider` implementation
  - Supports scene update tick without full game boot
- [x] Create `NovaForgePreviewWorld` — editable preview scene container
  - Entity instantiation and destruction
  - Transform manipulation
  - Material/mesh assignment

### Milestone D.2 — Scene Editor Viewport ✅
- [x] Wire `SceneEditorTool` to `NovaForgePreviewRuntime` as its scene provider (`attachSceneProvider()`)
- [x] Implement fly-camera controls (WASD + mouse look via `processCameraInput()`)
- [x] Implement gizmo state bound to selected entity (`gizmoState()` derives from world selection)
- [x] Selection → Inspector binding (`selectedEntityProperties()` returns flat property map)
- [x] Entity hierarchy → Outliner binding (`hierarchyOrder()` returns parent-before-child order)

### Milestone D.3 — Asset Preview Viewport ✅
- [x] Wire `AssetEditorTool` to a per-asset preview scene (`attachAssetPreviewProvider()`)
- [x] Preview shows selected asset as a `NovaForgeAssetPreview` scene provider
- [x] Editable transform, material, attachment metadata (setTransform/setMeshTag/setMaterialTag/setAttachmentTag)
- [x] Collider/socket/anchor editing for asset placement metadata
  - `ColliderDescriptor` (shape/extents/radius/isTrigger/tag), `setCollider*()` methods
  - `SocketDescriptor` — addSocket/removeSocket/setSocketTransform
  - `AnchorDescriptor` — addAnchor/removeAnchor/setAnchorTransform
- [x] PCG tag and placement metadata editing
  - `AssetPCGMetadata` (placementTag, generationTags, scale range, density, exclusionGroup)
  - `setPlacementTag`, `addGenerationTag`, `setPCGScaleRange`, `setPCGDensity`, etc.

### Milestone D.4 — Material Preview Viewport ✅
- [x] Create `NovaForgeMaterialPreview` — `IViewportSceneProvider` for `MaterialEditorTool`
  - `bindMaterial()` / `clearMaterial()` / `hasMaterial()`
  - Standard preview meshes: Sphere, Cube, Plane (`setPreviewMesh()`)
  - Shader tag (`setShaderTag()`)
  - Material parameters (`setParameter(name, value, type)`, `removeParameter()`, `resetParameterToDefault()`)
  - `apply()` / `revert()` baseline management
  - `properties()` — flat map for inspector display
- [x] Wire `MaterialEditorTool` to material preview via `attachMaterialPreviewProvider()`
  - `provideScene()` delegates to `NovaForgeMaterialPreview` when attached
  - Stub state (hasContent=false) when no provider or no material bound

### Milestone D.5 — D3D11 Backend Activation
- [ ] Activate D3D11 backend as primary (GDI becomes fallback-only in practice)
- [ ] Wire DirectWrite text rendering
- [ ] Verify all panels render correctly through D3D11 path

**Success Criteria (D.1–D.4):** ✅
- `NovaForgePreviewRuntime` implements `IViewportSceneProvider`; `provideScene()` reflects world state
- `SceneEditorTool.provideScene()` delegates to attached runtime when wired
- Fly-camera state updates on WASD/mouse input
- Gizmo state reflects selected entity position
- `selectedEntityProperties()` returns name/position/mesh/material for selected entity
- `hierarchyOrder()` returns parents before children
- `AssetEditorTool.provideScene()` delegates to `NovaForgeAssetPreview` when wired
- Asset preview collider/socket/anchor/PCG tag editing with apply()/revert() round-trips
- `MaterialEditorTool.provideScene()` delegates to `NovaForgeMaterialPreview` when wired
- Material parameter CRUD, preview mesh selection, shader tag — all with dirty/apply/revert
- 161 new tests (84 original + 79 D completion), all green

---

## Phase E — Shared PCG Preview Pipeline

**Status: In Progress**

**Goal:** Editor and game client use the same PCG core. Panels can edit PCG rules and
see regenerated results in the viewport immediately.

### Milestone E.1 — Shared NovaForge PCG Core ✅
- [x] Factor PCG generation code into a shared library usable by both client and editor
- [x] Define `PCGRuleSet` — typed rule container loaded from project data
  - addRule/setValue/removeRule/findRule/getValue/hasRule
  - resetToDefaults/resetRule, rulesInCategory, dirty tracking
- [x] Define `PCGGeneratorService` — stateless generator: rules + seed → placements
  - Deterministic: same inputs always produce same output
  - validate() warns on missing recommended rules
- [x] Define `PCGDeterministicSeedContext` — reproducible seed management
  - Universe seed + domain derivation (FNV-1a + xorshift mixing)
  - childContext, pinDomainSeed, registerDomain

### Milestone E.2 — Editor PCG Preview Service ✅
- [x] Create `PCGPreviewService` — editor-side wrapper around shared PCG core
  - Accepts rule document (`bindRuleSet()`) + seed (`setSeed()`)
  - Generates preview output (placements with assetTag/position/scale/yaw)
  - Caches results (`hasResult()`, `lastResult()`)
- [x] Wire PCG preview into viewport via `populatePreviewWorld(NovaForgePreviewWorld&)`
- [x] Support manual regeneration trigger (`forceRegenerate()`)
- [x] Support automatic regeneration on rule edit (`autoRegenerate` flag)

### Milestone E.3 — PCG Rule Editing ✅
- [x] `PCGPreviewService::setRuleValue()` — edits ruleset + triggers regen if auto=true
- [x] `PCGPreviewService::resetRules()` — reverts to defaults + triggers regen
- [x] Change callback (`setOnRegenerateCallback()`) — invoked after each successful regen
- [x] Domain override (`setDomainOverride()`) — controls which seed stream is used
- [ ] Wire `ProcGenRuleEditorV1` panel to real `PCGRuleSet` documents (panel-level wiring)
- [ ] Save-back translates rule changes to project data

### Milestone E.4 — Asset PCG Metadata ✅
- [x] Asset editor (`NovaForgeAssetPreview`) exposes PCG placement tags and generation constraints
- [x] `PCGPreviewService::populatePreviewWorld()` uses asset/placement tags for entity mesh tags
- [ ] Changes to asset PCG tags auto-trigger preview update (event-driven wiring)

**Success Criteria (E.1–E.4):** ✅
- Same `PCGGeneratorService` usable in editor and game runtime (no engine dependency)
- Same inputs + same seed always produce identical `PCGGenerationResult`
- Rule edits via `PCGPreviewService` trigger live preview regeneration when autoRegenerate=true
- PCG output populates `NovaForgePreviewWorld` with positioned, tagged entities
- Deterministic seeds produce identical placement positions
- 82 new tests, 175 assertions, all green (test_phase_e.cpp)

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
