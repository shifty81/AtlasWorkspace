# Project Status

Current Phase: **Phase 12 Complete — Event Bus and Workspace Notifications**

## Build Status

- **Passing**
- All 21 test suites build and pass
- Tests require online fetch of Catch2 (gated behind `ATLAS_ENABLE_ONLINE_DEPS`)

## Repo Condition

- Structure coherent and consistent
- Naming unified to AtlasAI canon (zero Arbiter remnants in active paths)
- Docs aligned with current canon
- Editor surface rationalized (~10 primary tool implementations)
- Backend strategy formalized (Phase 2 complete)
- Workspace integration surfaces formally contracted (Phase 7)
- Workspace-level event bus and notification system operational (Phase 12)

## Phase Summary

| Phase | Title | Status |
|-------|-------|--------|
| 0 | Canon Reset and Consolidation | ✅ Done |
| 1 | Workspace Core Stabilization | ✅ Done |
| 2 | AtlasUI Backend Strategy | ✅ Done |
| 3 | Editor Consolidation | ✅ Done |
| 4 | AtlasAI and Codex Integration | ✅ Done |
| 5 | Hosted Project Support | ✅ Done |
| 6 | Build, Patch, and Release Pipeline | ✅ Done |
| 7 | Workspace Integration Surfaces | ✅ Done |
| 8 | Runtime Wiring and First Real Tool Loop | ✅ Done |
| 9 | Asset Pipeline and Content Routing | ✅ Done |
| 10 | Project Persistence and Serialization | ✅ Done |
| 11 | Command Bus and Action System | ✅ Done |
| 12 | Event Bus and Workspace Notifications | ✅ Done |

## Active Problems

None critical. Minor open items:
- GDI backend is still the active rendering path; D3D11 targeted but not yet executing
- AtlasAI panel host tests require Editor-heavy deps (covered in Phase4 tests)

## Next Milestone

**Phase 13: Workspace Preferences and Configuration**

Implement the workspace-level preferences system:
- PreferenceCategory — typed grouping (General/Appearance/Keybindings/Editor/Build/AI/Plugin)
- PreferenceEntry — typed entry (string/bool/int/float) with default, min/max, description
- PreferenceStore — load/save/reset preferences with observer notifications via EventBus
- PreferenceSerializer — serialize/deserialize preferences to/from WorkspaceProjectFile sections
