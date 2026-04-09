# Naming Canon

## Approved Names

| Name | Usage |
|------|-------|
| **Atlas Workspace** | User-facing product name |
| **AtlasWorkspace.exe** | Primary executable |
| **AtlasUI** | UI framework |
| **AtlasAI** | AI broker system |
| **NovaForge** | Hosted game project |

## Removed Names

These names must not appear in active code, docs, or user-facing strings:

| Old Name | Replacement | Notes |
|----------|-------------|-------|
| Arbiter | AtlasAI | All active paths |
| ArbiterAI | AtlasAI | Tool and module references |
| Atlas Suite | Atlas Workspace | Product naming |
| AtlasToolingSuite | Atlas Workspace | Product naming |
| MasterRepo.exe | AtlasWorkspace.exe | Executable naming |

## Case Variants

| Context | Format |
|---------|--------|
| User-facing text | Atlas Workspace |
| Executable | AtlasWorkspace.exe |
| C++ namespace | NF (existing convention) |
| CMake target | AtlasWorkspace |
| Config keys/macros | ATLAS_AI (not ARBITER) |
| File names/symbols | atlas_ai (where snake_case fits) |

## Rules

- No legacy names in active code paths
- Historical names allowed **only** in `/Archive/` or explicitly labeled historical docs
- User-facing strings must follow this canon
- New code must use canonical names from creation
- Rename drift must be caught and corrected immediately
