# Editor Strategy

## Goals

- Reduce editor count to a focused primary tool roster
- Maximize reuse through shared panels and services
- Enforce shared panel residency across tools
- Stop uncontrolled editor expansion

## Definitions

| Term | Meaning |
|------|---------|
| **Tool** | Top-level editor application within the workspace |
| **Panel** | Reusable UI component shared across tools |
| **Service** | Non-visual backend system shared across tools |
| **Plugin** | Project-specific extension loaded through adapter |
| **Archive** | Deferred or retired concept not in active build |

## Primary Tool Roster (~10 tools)

| Tool | Purpose |
|------|---------|
| Workspace Browser | Project and workspace navigation |
| Scene Editor | Scene/world editing and viewport |
| Asset Editor | Asset management and editing |
| Material Editor | Material/shader authoring |
| Visual Logic Editor | Blueprint/graph-based logic |
| UI Editor | UI layout and design |
| Animation Editor | Animation timeline and state |
| Data Editor | Data tables and configuration |
| Build Tool | Build pipeline and packaging |
| AtlasAI Tool | AI broker, diagnostics, Codex |
| Project Systems Tool | Hosted-project gameplay panels (via adapter) |

## Shared Panels

These are reusable, owned by workspace core, not duplicated per tool:

- Inspector / Properties
- Outliner / Hierarchy
- Content Browser
- Console / Log
- Command Palette / Search
- Notification Center
- Asset Preview
- Viewport Overlays

## Shared Services

- Command Bus
- Selection Model
- Undo/Redo
- Event Bus
- Asset Resolution
- Layout Save/Load
- Validation Framework
- AtlasAI Broker Bridge

## Rules

- No new one-off standalone editors without justification
- Panels are shared once, reused everywhere
- Services are centralized
- Project-specific authoring surfaces are hosted through adapters, not workspace core
- Everything beyond the primary roster must be classified as panel, service, plugin, or archive
