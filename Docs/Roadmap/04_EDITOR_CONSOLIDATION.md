# Editor Consolidation

**Phase 3 of the Master Roadmap — IN PROGRESS**

> ⚠️ STOP: No new V1 stub headers. No new S-story test expansions.
> Stories S4–S189 are archived. Phase 3 is the active track.

## Purpose

Collapse editor sprawl into a focused set of primary tools. Implement real
NF::IHostedTool instances. Convert one-off editors into shared panels, services,
or project adapter modules. Separate workspace (OS layer) from editor tools.

## Status

### Workspace Architecture Separation
- [x] Create `Source/Workspace/` module (NF::Workspace) as the OS-like host layer
- [x] Move 36 workspace-core headers (shell, registries, services, AI host) to `NF/Workspace/`
- [x] Create forwarding headers in `NF/Editor/` for backward compatibility
- [x] Decouple WorkspaceShell from hardcoded tool includes (factory-based registration)
- [x] Create CoreToolRoster.h for primary tool registration

### One-Off Tool Removal
- [x] Archive 29 project-specific game editors to Legacy/ (AbilitySystem, Costume, Arcade, etc.)
- [x] Group 143 one-off editors into tool-specific sub-directories:
  - Scene/ (28 headers): terrain, foliage, nav mesh, water, weather, cameras, world tools
  - Asset/ (30 headers): textures, sprites, meshes, audio, particles, fonts, prefabs
  - Material/ (12 headers): shader graph, post-process, gradients, curves, lightmaps
  - Animation/ (13 headers): anim blueprints, timelines, cinematics, cloth/rope sim
  - Data/ (18 headers): data tables, localization, dialogue, input bindings, UI design
  - Logic/ (9 headers): logic graph, state graph, AI behavior, event bus
  - Build/ (7 headers): deployment, packaging, benchmarks, stress testing
  - AI/ (10 headers): pathfinding, perception, telemetry, diagnostics, profiling
  - Infra/ (11 headers): cloud storage, network topology, accessibility, display
  - ProjectSystems/ (5 headers): sandbox, playmode, replay, playtest

### Umbrella Include Reduction
- [x] Break Editor.h into focused module headers
- [x] Create EditorSharedPanels.h for shared panel includes
- [x] Create EditorToolRegistry.h for tool registration
- [x] Keep Editor.h minimal (core editor types only)
- [x] Update Editor.h to include CoreToolRoster.h

### Legacy Archival
- [x] Move S-story stub tests (test_s4–test_s189) to Tests/Editor/Legacy/
- [x] Move non-core V1 stub headers to Source/Editor/include/NF/Editor/Legacy/
- [x] Archive project-specific game editors to Source/Editor/include/NF/Editor/Legacy/
- [x] Remove archived tests from CMakeLists build

### Primary Tool Implementation (NF::IHostedTool)
Each tool below must be a full IHostedTool implementation, not a stub.
It must implement initialize/shutdown/activate/suspend/update, declare
its shared panels, and register with WorkspaceShell via ToolRegistry.

| Tool | ToolId | Status |
|------|--------|--------|
| SceneEditorTool | workspace.scene_editor | [x] Done |
| AssetEditorTool | workspace.asset_editor | [x] Done |
| MaterialEditorTool | workspace.material_editor | [x] Done |
| AnimationEditorTool | workspace.animation_editor | [x] Done |
| DataEditorTool | workspace.data_editor | [x] Done |
| VisualLogicEditorTool | workspace.visual_logic_editor | [x] Done |
| BuildTool | workspace.build_tool | [x] Done |
| AtlasAITool | workspace.atlasai | [x] Done |
| ProjectSystemsTool | workspace.project_systems | [x] Exists (adapter host) |

### Shared Panel Extraction
Convert these from standalone headers to registered shared panels in PanelRegistry:
- [ ] ContentBrowserPanel → `panel.content_browser`
- [ ] ComponentInspectorV1 → `panel.component_inspector`
- [ ] DiagnosticPanelV1 → `panel.diagnostics`
- [ ] MemoryProfilerPanel → `panel.memory_profiler`
- [ ] PipelineMonitorPanel → `panel.pipeline_monitor`
- [ ] NotificationCenterEditor → `panel.notification_center`

### Shared Service Extraction
- [x] FileIntakePipeline → moved to NF::Workspace
- [x] CodexSnippetMirror → moved to NF::Workspace
- [x] EditorEventBus → moved to NF::Workspace
- [x] ConsoleCommandBus → moved to NF::Workspace
- [ ] GraphHostContract → shared service contract

### NovaForge Adapter Panels (already done)
- [x] GameEconomyEditor → NovaForgeEconomyPanel
- [x] InventoryEditor → NovaForgeInventoryRulesPanel
- [x] ItemShopEditor → NovaForgeShopPanel
- [x] DailyQuestEditor → NovaForgeMissionRulesPanel
- [x] AchievementEditor → NovaForgeProgressionPanel
- [x] CharacterCreatorEditor → NovaForgeCharacterRulesPanel

## Architecture

```
Source/Workspace/     ← "OS" layer (NF::Workspace)
  include/NF/Workspace/
    Shell:      WorkspaceShell, ToolRegistry, PanelRegistry, layout, commands
    Services:   EventBus, CommandBus, Selection, Undo, FileIntake, Notifications
    AI:         AtlasAIPanelHost, AIPanelSession, AIActionSurface, CodexSnippetMirror

Source/Editor/        ← Tool implementations (NF::Editor)
  include/NF/Editor/
    Core:           EditorApp, EditorPanel, EditorTheme, EditorCamera, etc.
    CoreToolRoster: registerCoreTools() — wires primary tools into shell
    Scene/          scene editing sub-panels (terrain, foliage, cameras, etc.)
    Asset/          asset management sub-panels (texture, mesh, audio, etc.)
    Material/       material/shader sub-panels
    Animation/      animation sub-panels
    Data/           data table/config sub-panels
    Logic/          visual logic sub-panels
    Build/          build/deploy sub-panels
    AI/             AI diagnostics sub-panels
    Infra/          workspace infrastructure sub-panels
    ProjectSystems/ adapter-hosted sub-panels
    Legacy/         archived V1 stubs and project-specific editors
```
