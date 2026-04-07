# Atlas Workspace

> **Generic C++ Development Environment** | Powered by the Flagship Atlas Engine

**Atlas Workspace** is a native C++ development environment and the home of the
**Atlas Engine** — the engine that powers all software built in this platform.

The workspace:
- Hosts the Atlas Engine (ECS, Physics, Audio, Animation, Input, Networking, Rendering)
- Loads and runs external projects through adapters and plugins
- Provides editors and tools built on the engine
- Manages assets, builds, logs, and automation
- Brokers AI assistance through AtlasAI (which hooks directly into the engine)

[![Language](https://img.shields.io/badge/language-C%2B%2B20-brightgreen)]()
[![Engine](https://img.shields.io/badge/engine-Atlas%20Engine-blue)]()
[![Build](https://img.shields.io/badge/build-CMake-orange)]()

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                   Atlas Workspace                    │
│                                                      │
│  ┌─────────────────────────────────────────────────┐ │
│  │              Atlas Engine (Flagship)             │ │
│  │  Core · Engine · Physics · Audio · Animation    │ │
│  │  Input · Networking · Renderer                  │ │
│  └─────────────────────────────────────────────────┘ │
│                                                      │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────┐  │
│  │  AtlasUI     │  │  AtlasAI     │  │  Codex    │  │
│  │  GraphVM     │  │  (hooks into │  │  Logger   │  │
│  │  Editor      │  │   Engine)    │  │  Pipeline │  │
│  └──────────────┘  └──────────────┘  └───────────┘  │
│                                                      │
│  ┌─────────────────────────────────────────────────┐ │
│  │         Project / Plugin Layer (SDK)             │ │
│  │   NovaForge · arena2d · atlas-sample · ...      │ │
│  └─────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
```

## Repository Structure

```
/AtlasWorkspace
  /Source
    /Core          — Foundation types, logging, memory
    /Engine        — Atlas Engine: ECS, behavior trees, asset system, scene graph
    /Physics       — Atlas Engine: physics subsystem
    /Audio         — Atlas Engine: audio subsystem
    /Animation     — Atlas Engine: animation subsystem
    /Input         — Atlas Engine: input subsystem
    /Networking    — Atlas Engine: networking subsystem
    /Renderer      — Atlas Engine: rendering subsystem
    /UI            — AtlasUI framework
    /GraphVM       — Visual graph/scripting VM
    /AI            — AtlasAI broker (hooks into Engine)
    /Pipeline      — Build orchestration / workspace broker
    /Editor        — Workspace editor host
  /Tests           — Atlas Engine + workspace tests
  /Docs            — Workspace documentation
  /Schemas         — Workspace schemas (atlas.project.v1, atlas.build.v1)
  /Project         — Generic project samples
  /Atlas           — Atlas tools
  /AtlasAI         — AtlasAI broker
  /Codex           — Logger/diagnostics
  /Scripts         — Build scripts
  /Tools           — Workspace tools
  /NovaForge       — NovaForge game project (uses Atlas Engine via adapter)
```

## Building

```bash
# Build workspace (includes Atlas Engine)
cmake --preset debug
cmake --build --preset debug --parallel

# Run tests
ctest --preset debug
```

## NovaForge

The NovaForge game lives in `/NovaForge`. It uses the Atlas Engine through the
workspace adapter layer — game logic, world generation, and gameplay systems
all sit in `NovaForge/Source/` and link against the workspace-provided engine.

See `/NovaForge/CMakeLists.txt` for the standalone NovaForge build.

## Roadmap

Development follows a **tooling-first** priority: framework and editor
infrastructure ships before game-specific features.

### Phase 1 — AtlasUI Foundation ✅

| Feature | Status |
|---------|--------|
| Backend abstraction (GDI / OpenGL / Null) | ✅ Done |
| Widget kit (Button, Label, TextInput, Dropdown, …) | ✅ 14 widgets |
| Theme & token system | ✅ Done |
| Tabs / chrome | ✅ Done |
| Docking layout | ✅ Done |
| Tooltips | ✅ Done |
| Command & shortcut layer | ✅ Done |
| Menu layer | 🟡 Partial — context menu widget needed |

### Phase 2 — Workspace Shell & Core Surfaces 🟡

| Feature | Status |
|---------|--------|
| Shell frame (EditorApp + PanelHost) | ✅ Done |
| Notification center | ✅ Widget + host done |
| Notification severity / workflow rules | 🔲 Spec'd, not wired |
| AtlasAI panel host | 🔲 Broker exists, UI missing |
| Settings / control panel | 🔲 Not started |
| Project & repo surfaces | 🔲 Not started |

### Phase 3 — Editor Standards & Persistence 🔲

| Feature | Status |
|---------|--------|
| PropertyGrid / TreeView / TableView | 🔲 PropertyRow exists; advanced widgets needed |
| Layout persistence | 🔲 Not started |
| Scroll + virtualization | 🔲 Not started |
| Viewport host contract | 🔲 Not started |
| Graph host contract | 🔲 Not started |

### Phase 4 — Workflow & Debugging Integration 🟡

| Feature | Status |
|---------|--------|
| Logging route (Console → Codex) | ✅ ConsolePanel + PipelineMonitor |
| AtlasAI debug path | 🔲 Not started |
| File intake / drop pipeline | 🔲 Not started |
| Codex / snippet mirroring | 🔲 Not started |

### Phase 5 — Hosted Project Support 🔲

| Feature | Status |
|---------|--------|
| Project templates | 🔲 Not started |
| Import / export flows | 🔲 Not started |
| Build / package workflows | 🔲 Not started |
| Repo tooling polish | 🔲 Not started |

### Phase 6–7 — Game Development (deferred)

Game-specific work (combat, world content, fleet features, missions, etc.) is
intentionally deferred until the tooling foundation is solid.

> Full details: [`Docs/SpecRollup/`](Docs/SpecRollup/)

## Key Principles

- The Atlas Engine is owned by the workspace — all projects use it
- AtlasAI hooks into the engine for context (active entities, logs, git state)
- Projects load through adapters; no game logic lives in the workspace core
- All gameplay systems are project-level (NovaForge, arena2d, etc.)
