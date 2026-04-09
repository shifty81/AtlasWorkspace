# Repo Cleanup Manifest

## Legend

| Action | Meaning |
|--------|---------|
| **edit** | Modify in place |
| **rename** | Move or rename path |
| **archive** | Move to Archive/ or Docs/Archive/ |
| **delete** | Remove from repo |
| **keep** | Keep as-is, verify only |
| **add** | Create new file |

---

## Pass 1: Repo Hygiene (Done)

| Action | Path | Status |
|--------|------|--------|
| add | .gitattributes | ✅ Done |
| edit | .gitignore | ✅ Done |
| edit | CMakeLists.txt (ATLAS_ENABLE_ONLINE_DEPS) | ✅ Done |
| edit | Tests/CMakeLists.txt (guard Catch2 fetch) | ✅ Done |

## Pass 2: Documentation Reset (Done)

| Action | Path | Status |
|--------|------|--------|
| edit | README.md | ✅ Done |
| add | Docs/Canon/* | ✅ Done |
| add | Docs/Roadmap/* | ✅ Done |
| add | Docs/Inventory/* | ✅ Done |

## Pass 3: Active-Path Naming

| Action | Path | Status |
|--------|------|--------|
| rename | Atlas/Workspace/Arbiter/ → Atlas/Workspace/AtlasAI/ | Pending |
| edit | Source/Pipeline/src/WorkspaceBroker.cpp | Pending |
| edit | Source/Pipeline/src/ArbiterReasoner.cpp | Pending |
| edit | Source/Pipeline/src/ToolWiring.cpp | Pending |
| edit | Source/Pipeline/include/NF/Pipeline/Pipeline.h | Pending |
| edit | Source/Editor/include/NF/Editor/ToolEcosystem.h | Pending |
| edit | Source/Programs/AtlasWorkspace/main.cpp | Pending |
| edit | Tests/Pipeline/test_arbiter_reasoner.cpp | Pending |
| edit | Tests/Pipeline/test_workspace_broker.cpp | Pending |
| edit | Tests/Pipeline/test_tool_wiring.cpp | Pending |
| edit | Tests/Pipeline/test_full_suite.cpp | Pending |
| edit | Tests/Pipeline/test_pipeline.cpp | Pending |
| edit | Tests/Editor/test_s8_editor.cpp | Pending |

## Pass 4: Workspace Entrypoint

| Action | Path | Status |
|--------|------|--------|
| edit | Source/Programs/AtlasWorkspace/main.cpp | Pending |
| edit | Source/Programs/AtlasWorkspace/CMakeLists.txt | Pending |

## Pass 5: UI Backend

| Action | Path | Status |
|--------|------|--------|
| edit | Source/UI/include/NF/UI/GDIBackend.h (mark fallback) | Pending |
| edit | Source/UI/include/NF/UI/OpenGLBackend.h (mark compat) | Pending |
| add | Source/UI/include/NF/UI/UIBackendSelector.h | Pending |
| add | Source/UI/src/UIBackendSelector.cpp | Pending |
| add | Source/UI/include/NF/UI/D3D11Backend.h | Pending |
| add | Source/UI/src/D3D11Backend.cpp | Pending |
| add | Source/UI/include/NF/UI/DirectWriteTextBackend.h | Pending |
| add | Source/UI/src/DirectWriteTextBackend.cpp | Pending |

## Pass 6: Editor Consolidation

| Action | Path | Status |
|--------|------|--------|
| edit | Source/Editor/include/NF/Editor/Editor.h (reduce umbrella) | Pending |
| edit | Source/Editor/src/Editor.cpp (composition root) | Pending |
| add | Source/Editor/include/NF/Editor/EditorSharedPanels.h | Pending |
| add | Source/Editor/include/NF/Editor/EditorToolRegistry.h | Pending |

## Pass 7: Archive Legacy

| Action | Path | Status |
|--------|------|--------|
| archive | Tools/ArbiterAI/ | Pending |
| archive | Tools/SwissAgent/ | Pending |
| archive | AtlasAI/Atlas_Arbiter/ | Pending |
| archive | AtlasAI/Atlas_SwissAgent/ | Pending |
| archive | build_verify/ | Pending |

## Pass 8: NovaForge Adapter

| Action | Path | Status |
|--------|------|--------|
| add | Source/Editor/include/NF/Editor/IGameProjectAdapter.h | Pending |
| add | Source/Editor/include/NF/Editor/GameplaySystemPanelDescriptor.h | Pending |
| add | Source/Editor/include/NF/Editor/ProjectSystemsTool.h | Pending |
| add | NovaForge/Source/EditorAdapter/ (directory) | Pending |

## Pass 9: Doc Cleanup

| Action | Path | Status |
|--------|------|--------|
| archive | Docs/SpecPack/ (if conflicts with SpecRollup) | Pending |
| edit | Docs/SpecRollup/* (align to canon) | Pending |
| edit | AtlasAI/README.md | Pending |
| edit | AtlasAI/ATLAS_AI_OVERVIEW.md | Pending |
| edit | Atlas/Workspace/README.md | Pending |
