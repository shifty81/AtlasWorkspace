# Legacy Name Scrub List

## Active Renames

| Old Name | New Name | Scope | Status |
|----------|----------|-------|--------|
| Arbiter | AtlasAI | All active code/docs | In Progress |
| ArbiterAI | AtlasAI | Tools, modules | In Progress |
| AtlasArbiter | AtlasAI | Modules, docs | In Progress |
| ARBITER | ATLAS_AI | Config keys, macros | In Progress |
| arbiter | atlas_ai | File names, symbols | In Progress |
| arbiter_reasoner | atlas_ai_reasoner | Pipeline symbols | In Progress |
| Workspace/Arbiter | Workspace/AtlasAI | Directory paths | In Progress |
| Atlas Suite | Atlas Workspace | Product naming | In Progress |
| AtlasToolingSuite | Atlas Workspace | Product naming | In Progress |
| MasterRepo.exe | AtlasWorkspace.exe | Executable naming | Pending |

## Files with Active Arbiter References

### Source Files
| File | References | Status |
|------|-----------|--------|
| Source/Pipeline/src/ArbiterReasoner.cpp | ~15 | Pending |
| Source/Pipeline/src/WorkspaceBroker.cpp | ~10 | Pending |
| Source/Pipeline/src/ToolWiring.cpp | ~5 | Pending |
| Source/Pipeline/include/NF/Pipeline/Pipeline.h | ~5 | Pending |
| Source/Editor/include/NF/Editor/ToolEcosystem.h | ~3 | Pending |

### Test Files
| File | References | Status |
|------|-----------|--------|
| Tests/Pipeline/test_arbiter_reasoner.cpp | ~30 | Pending |
| Tests/Pipeline/test_workspace_broker.cpp | ~20 | Pending |
| Tests/Pipeline/test_tool_wiring.cpp | ~15 | Pending |
| Tests/Pipeline/test_full_suite.cpp | ~10 | Pending |
| Tests/Pipeline/test_pipeline.cpp | ~10 | Pending |
| Tests/Editor/test_s8_editor.cpp | ~5 | Pending |

### Directory Renames
| Old Path | New Path | Status |
|----------|----------|--------|
| Atlas/Workspace/Arbiter/ | Atlas/Workspace/AtlasAI/ | Pending |
| Tools/ArbiterAI/ | Archive (or delete) | Pending |

## Stale Doc References

Search all active docs for:
- ImGui (should say AtlasUI)
- WPF (should say archived/deferred)
- Arbiter (should say AtlasAI)
- MasterRepo (should say Atlas Workspace)
- Atlas Suite (should say Atlas Workspace)

## Rules

- Active docs: correct to canon
- Historical docs: prefix with "Historical:" or move to Docs/Archive
- Archive content: leave as-is (do not rename inside archive)
