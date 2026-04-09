# Editor Consolidation

**Phase 3 of the Master Roadmap**

## Purpose

Collapse editor sprawl into a focused set of primary tools. Convert one-off editors into shared panels, services, or project adapter modules.

## Tasks

### Umbrella Include Reduction
- [ ] Break Editor.h into focused module headers
- [ ] Create EditorSharedPanels.h for shared panel includes
- [ ] Create EditorToolRegistry.h for tool registration
- [ ] Keep Editor.h minimal (core editor types only)

### Editor.cpp Composition Root
- [ ] Make Editor.cpp the real composition root OR
- [ ] Move composition to actual module registries
- [ ] Eliminate empty-shell architecture

### Primary Tool Roster
- [ ] Keep ~10 primary tools (see Editor Strategy canon)
- [ ] Remove non-core editors from active registry
- [ ] Do not delete files yet — mark excluded from build first

### Shared Panel Extraction
- [ ] ContentBrowserPanel → shared panel
- [ ] ComponentInspectorV1 → shared panel
- [ ] DiagnosticPanelV1 → shared panel
- [ ] MemoryProfilerPanel → shared panel
- [ ] PipelineMonitorPanel → shared panel
- [ ] NotificationCenterEditor → NotificationCenterPanel

### Shared Service Extraction
- [ ] GraphHostContract → shared service contract
- [ ] FileIntakePipeline → shared service
- [ ] CodexSnippetMirror → shared service
- [ ] EditorEventBus → shared service
- [ ] ConsoleCommandBus → shared service

### Archive Candidates
See `Docs/Inventory/EDITOR_TOOL_INVENTORY.md` for full list.

Top archive targets:
- ArcadeGameEditor, DailyQuestEditor, GameEconomyEditor
- InventoryEditor, ItemShopEditor, LeaderboardEditor
- LobbyEditor, LiveOpsEditor, MatchReplayEditor
- MatchmakingEditor, MiniGameEditor, CloudStorageEditor
- BenchmarkSuiteEditor, LoadTestEditor, AccessibilityEditor
- All narrow AI editors (merge into AtlasAI tool)

### NovaForge Adapter Panels
- [ ] GameEconomyEditor → NovaForgeEconomyPanel
- [ ] InventoryEditor → NovaForgeInventoryRulesPanel
- [ ] ItemShopEditor → NovaForgeShopPanel
- [ ] DailyQuestEditor → NovaForgeMissionRulesPanel
- [ ] AchievementEditor → NovaForgeProgressionPanel
- [ ] CharacterCreatorEditor → NovaForgeCharacterRulesPanel
