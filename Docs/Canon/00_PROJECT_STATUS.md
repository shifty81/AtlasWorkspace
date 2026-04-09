# Project Status

Current Phase: **Milestone 0 – Canon Reset and Consolidation**

## Build Status

- **Partial**
- Workspace configures and builds with tests disabled
- Tests require online fetch of Catch2 (now gated behind `ATLAS_ENABLE_ONLINE_DEPS`)
- Full build validation on Windows not yet complete in CI

## Repo Condition

- Structure present and coherent
- Naming inconsistent (Arbiter remnants in active paths)
- Docs inconsistent (historical statements mixed with current canon)
- Editor surface oversized (~312 headers, target ~10 primary tools)
- Backend strategy incomplete (GDI active, D3D11 targeted)

## Active Problems

- Line ending instability (fixed: `.gitattributes` added)
- Legacy naming (Arbiter) in active source and test paths
- Editor sprawl beyond intended tool roster
- Backend mismatch (GDI practical default vs intended GPU-first path)
- Stale documentation reflecting older project states

## Immediate Goals

1. Normalize repo structure and hygiene
2. Unify naming to AtlasAI canon
3. Reset documentation to current direction
4. Reduce editor surface area
5. Define and formalize backend strategy

## Blocked Work

- Major UI work blocked by backend strategy formalization
- Editor expansion blocked by consolidation pass
- AtlasAI integration blocked by naming cleanup completion

## Next Milestone

**Milestone 1: Structural Truth Pass**

Every active tool accounted for, every stale name scrubbed, every doc aligned to canon.
