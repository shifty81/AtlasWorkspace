# Editor Consolidation

**Phase 3 of the Master Roadmap — IN PROGRESS**

> ⚠️ STOP: No new V1 stub headers. No new S-story test expansions.
> Stories S4–S189 are archived. Phase 3 is the active track.

## Purpose

Collapse editor sprawl into a focused set of primary tools. Implement real
NF::IHostedTool instances. Convert one-off editors into shared panels, services,
or project adapter modules.

## Status

### Umbrella Include Reduction
- [x] Break Editor.h into focused module headers
- [x] Create EditorSharedPanels.h for shared panel includes
- [x] Create EditorToolRegistry.h for tool registration
- [x] Keep Editor.h minimal (core editor types only)

### Legacy Archival
- [x] Move S-story stub tests (test_s4–test_s189) to Tests/Editor/Legacy/
- [x] Move non-core V1 stub headers to Source/Editor/include/NF/Editor/Legacy/
- [x] Remove archived tests from CMakeLists build

### Primary Tool Implementation (NF::IHostedTool)
Each tool below must be a full IHostedTool implementation, not a stub.
It must implement initialize/shutdown/activate/suspend/update, declare
its shared panels, and register with WorkspaceShell via ToolRegistry.

| Tool | ToolId | Status |
|------|--------|--------|
| SceneEditorTool | workspace.scene_editor | [x] Done |
| AssetEditorTool | workspace.asset_editor | [ ] |
| MaterialEditorTool | workspace.material_editor | [ ] |
| AnimationEditorTool | workspace.animation_editor | [ ] |
| DataEditorTool | workspace.data_editor | [ ] |
| VisualLogicEditorTool | workspace.visual_logic_editor | [ ] |
| BuildTool | workspace.build_tool | [ ] |
| AtlasAITool | workspace.atlasai | [ ] |
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
- [ ] GraphHostContract → shared service contract
- [ ] FileIntakePipeline → shared service
- [ ] CodexSnippetMirror → shared service
- [ ] EditorEventBus → shared service
- [ ] ConsoleCommandBus → shared service

### NovaForge Adapter Panels (already done)
- [x] GameEconomyEditor → NovaForgeEconomyPanel
- [x] InventoryEditor → NovaForgeInventoryRulesPanel
- [x] ItemShopEditor → NovaForgeShopPanel
- [x] DailyQuestEditor → NovaForgeMissionRulesPanel
- [x] AchievementEditor → NovaForgeProgressionPanel
- [x] CharacterCreatorEditor → NovaForgeCharacterRulesPanel
