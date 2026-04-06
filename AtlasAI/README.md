# AtlasAI — Unified AI Broker

> **Canonical Name:** AtlasAI
> **Legacy Names:** ArbiterAI, SwissAgent, Arbiter — all consolidated here.
> **Canonical Location:** `AtlasAI/` (repo root)

## Overview

AtlasAI is the **single AI integration point** for the NovaForge project. It unifies
all AI-driven functionality under one broker that communicates exclusively through the
`.novaforge/pipeline/` directory.

No game code, editor code, or engine code calls any AI system directly. All AI requests
and responses are mediated by AtlasAI through pipeline `ChangeEvent` files.

## Subsystems

| Subsystem | Directory | Description |
|-----------|-----------|-------------|
| **Atlas_Arbiter** | `AtlasAI/Atlas_Arbiter/` | Rule-based decision engine — evaluates declarative rules against JSON context |
| **Atlas_SwissAgent** | `AtlasAI/Atlas_SwissAgent/` | Conversational AI query tool — workspace-aware code generation and analysis |

### Planned Subsystems

| Subsystem | Description | Status |
|-----------|-------------|--------|
| **Atlas_NPC** | In-game NPC behaviour (C++ engine integration) | Planned |
| **Atlas_Pathfinding** | Navigation mesh + flow field AI (C++) | Planned |
| **Atlas_Dialogue** | Dialogue tree + LLM-assisted NPC speech | Planned |

## Architecture

```
┌──────────────────────────────────────────┐
│              AtlasAI Broker              │
│  ┌────────────────┐ ┌─────────────────┐  │
│  │ Atlas_Arbiter   │ │ Atlas_SwissAgent│  │
│  │ (rule engine)   │ │ (query agent)   │  │
│  └────────┬───────┘ └────────┬────────┘  │
│           └────────┬─────────┘           │
│                    │                     │
│          .novaforge/pipeline/            │
│           (ChangeEvent I/O)              │
└──────────────────────────────────────────┘
```

## Pipeline Contract

- **Input:** Reads `pipeline/changes/*.change.json` events
- **Output:** Writes `pipeline/changes/<timestamp>_atlasai_<type>.change.json`
- **Manifest:** Registered in `pipeline/manifest.json`
- **No direct file writes** — all output goes through pipeline events

## Integration Points

| Integration | Method |
|-------------|--------|
| Engine → AtlasAI | `Source/AI/` C++ runtime AI module |
| Editor → AtlasAI | Editor writes pipeline events; AtlasAI responds with change events |
| Build → AtlasAI | Build logs routed to `Logs/build.logger`; AtlasAI watches for issues |
| CI → AtlasAI | ContractScanner findings written as change events for AtlasAI review |

## Naming Canon

All AI features, classes, and modules follow the `Atlas_` prefix or `AtlasAI::` namespace:

- `AtlasAI::Arbiter`
- `AtlasAI::SwissAgent`
- `Atlas_NPCController`
- `Atlas_PathfindingSystem`

See `Docs/Architecture/NAMING_CANON.md` for full naming rules.

## Current Status

- **Phase:** C3 Internal Consolidation — tools consolidated, legacy paths archived
- **S1 Tool Wiring:** ⬜ Queued — full end-to-end pipeline event handling
- **S4 ArbiterAI Integration:** ⬜ Queued — full reasoning broker active

## Legacy Tool Migration

Legacy AI tool directories have been consolidated:

| Legacy Path | Target | Status |
|-------------|--------|--------|
| `Tools/ArbiterAI/` | `AtlasAI/Atlas_Arbiter/` | ✅ Consolidated (legacy path archived) |
| `Tools/SwissAgent/` | `AtlasAI/Atlas_SwissAgent/` | ✅ Consolidated (legacy path archived) |
| `Tools/AtlasAI/` | `AtlasAI/` (repo root) | ✅ Consolidated |

Active development occurs in `AtlasAI/` (repo root). Legacy paths under `Tools/` are
archived references only.
