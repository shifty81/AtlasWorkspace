# NovaForge — Atlas Engine Suite
> **Active Development Repo** | All projects converging here

**tempnovaforge** is the single canonical repository for the entire NovaForge game + Atlas engine suite.

[![Status](https://img.shields.io/badge/status-active%20convergence-orange)]()
[![Engine](https://img.shields.io/badge/engine-AtlasForge-blue)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B%2FPython-brightgreen)]()

## What is this?

This repo is the merger target for 12+ repos being consolidated into one coherent project:
- **Atlas Engine** (C++ game engine: rendering, physics, audio, input, networking, animation)
- **NovaForge Game** (voxel space game built on Atlas Engine)
- **AtlasAI** (unified AI tooling: Arbiter + SwissAgent + NPC runtime)
- **Atlas Editor** (level/world editor, formerly MasterRepoV001)
- **Atlas Tools** (BlenderGenerator, ContractScanner, ReplayMinimizer)

## Quick Navigation

| Area | Location |
|------|----------|
| Engine source | `Source/` |
| Game source | `Source/Game/` |
| Editor | `Source/Editor/` |
| AI Tools | `Tools/AtlasAI/` |
| Documentation | `Docs/` |
| Roadmap | `Docs/roadmap.md` |
| Architecture | `Docs/Architecture/` |
| Archives | `Archive/` |

## Roadmap

See [`Docs/roadmap.md`](Docs/roadmap.md) for the full convergence plan.

---

## 🏗️ System Architecture Blueprint

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          NOVAFORGE MONOREPO                             │
│                                                                         │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │                        WORKSPACE                                  │  │
│  │          Central environment · pipeline routing · state          │  │
│  │                                                                   │  │
│  │   Active Project Context · Tool Registry · Directory Rules       │  │
│  │   Cross-tool Events · Session State · AtlasAI Context Feed       │  │
│  └────────────────────────────┬─────────────────────────────────────┘  │
│                               │ unified context pipeline                │
│              ┌────────────────┼────────────────┐                        │
│              ▼                ▼                ▼                        │
│  ┌───────────────┐  ┌─────────────────┐  ┌───────────────────────┐    │
│  │  ATLAS EDITOR │  │   ATLAS ENGINE  │  │    ATLAS AI BROKER    │    │
│  │    SUITE      │  │    RUNTIME      │  │       (AtlasAI)       │    │
│  │               │  │                 │  │                       │    │
│  │ World Editor  │  │ ECS Core        │  │  Single broker API    │    │
│  │ Tile Editor   │  │ Renderer        │  │  Atlas_Arbiter        │    │
│  │ Visual Script │  │ Physics         │  │  Atlas_SwissAgent     │    │
│  │ Shader Editor │  │ Audio           │  │  Atlas_Memory         │    │
│  │ Content Brow. │  │ Animation       │  │  Atlas_LLM            │    │
│  │ Scene Outliner│  │ Networking      │  │  Atlas_AudioAI        │    │
│  │ Inspector     │  │ GraphVM         │  │  Atlas_Vision         │    │
│  │ Terrain Editor│  │ World Gen       │  │  Atlas_Bridge (C++↔Py)│    │
│  │ Debug Tools   │  │ Voxel Runtime   │  │                       │    │
│  │ Build Pipeline│  │ Scripting       │  │  ← ALL AI goes here   │    │
│  └───────┬───────┘  └────────┬────────┘  └──────────┬────────────┘    │
│          │                   │                       │                  │
│          └───────────────────┴───────────────────────┘                 │
│                              │                                          │
│              ┌───────────────┼───────────────┐                         │
│              ▼               ▼               ▼                         │
│  ┌────────────────┐  ┌──────────────┐  ┌────────────────────────┐     │
│  │  NOVAFORGE     │  │    TOOLS     │  │      SERVICES          │     │
│  │  GAME LAYER    │  │    SUITE     │  │                        │     │
│  │                │  │              │  │  Atlas_Server          │     │
│  │ Ships/Fleets   │  │ BlenderGen   │  │  Atlas_Client          │     │
│  │ Factions       │  │ RepoTools    │  │  Dedicated Server      │     │
│  │ Economy        │  │ BuildTools   │  │  REST / WebSocket      │     │
│  │ Expeditions    │  │ DevTools     │  │                        │     │
│  │ R.I.G. System  │  │ Installer    │  │                        │     │
│  └────────────────┘  └──────────────┘  └────────────────────────┘     │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 🖥️ Editor Suite — Blueprint & Roadmap

> The full Atlas Editor Suite is the primary development environment for everything in this repo.
> The Workspace is the root shell — all editors dock into it and cross-talk through it.

```
Editor Suite Progress:  ██░░░░░░░░░░░░░░░░░░░░░░░░░░░░  6%
```

### Workspace (Root Shell)

| System | Description | Status |
|--------|-------------|--------|
| Win32 Window + OpenGL | Native window, DPI scaling, GL context | ✅ Done (MasterRepo) |
| Docking Layout | 7-panel docking, live resize | ✅ Done (MasterRepo) |
| Toolbar Panel | Play/Stop/Launch controls | ✅ Done (MasterRepo) |
| Menu Bar | File/Edit/View/Tools menus | ✅ Done (MasterRepo) |
| Status Bar | Real-time editor state display | ✅ Done (MasterRepo) |
| Project Path Service | Resolves all paths from manifest | ✅ Done (MasterRepo) |
| Editor Command Registry | Centralized command/hotkey dispatch | ✅ Done (MasterRepo) |
| Tool Registry | Tracks open tools and pipeline state | ⬜ To Build |
| Directory Pipeline Router | Cross-tool file routing rules | ⬜ To Build |
| AtlasAI Context Feed | Feeds workspace state to AtlasAI broker | ⬜ To Build |
| Session Persistence | Save/restore editor layout and state | ⬜ To Build |
| Notification System | Toast/event notifications across panels | ⬜ To Build |
| Plugin Bus | atlas-plugin-bus — dynamic tool registration | ⬜ To Build |

### World Editor

| System | Description | Status |
|--------|-------------|--------|
| Viewport (3D) | 3D grid, yaw/pitch/zoom camera | ✅ Done (MasterRepo) |
| Scene Outliner | ECS entity list for active level | ✅ Done (MasterRepo) |
| Inspector Panel | Component properties for selected entity | ✅ Done (MasterRepo) |
| Content Browser | Navigate Content/ directory tree | ✅ Done (MasterRepo) |
| Voxel Inspector | Chunk map stats, voxel overlay toggle | ✅ Done (MasterRepo) |
| HUD Panel | R.I.G. health/energy/inventory (in-editor) | ✅ Done (MasterRepo) |
| Voxel Pick Service | Camera ray → chunk traversal → voxel hit | ✅ Done (MasterRepo) |
| World File Service | Load/save/reload/save-as world files | ✅ Done (MasterRepo) |
| Selection Service | Multi-object selection, sync across panels | ✅ Done (MasterRepo) |
| Property Inspector System | Dirty tracking, real type widgets | ✅ Done (MasterRepo) |
| Viewport Highlight Sync | Selected chunk/voxel/object overlays | ✅ Done (MasterRepo) |
| Undo/Redo Transaction Model | Full edit history | ⬜ To Build |
| Voxel Palette / Brush Panel | Paint voxels, select materials | ⬜ To Build |
| DevWorld Settings Panel | Live sandbox configuration | ⬜ To Build |
| World Gen Preview | See world gen output in-editor | ⬜ To Build |
| Level Streaming Editor | Configure streaming zones | ⬜ To Build |

### Tile Editor

| System | Description | Status |
|--------|-------------|--------|
| Tile Canvas | 2D tile painting surface | ⬜ Merge from AtlasForge |
| Tileset Manager | Load/manage/edit tilesets | ⬜ Merge from AtlasForge |
| Layer System | Multi-layer tile stacking | ⬜ Merge from AtlasForge |
| Collision Shape Editor | Per-tile collision zones | ⬜ To Build |
| Atlas Exporter | Export tilesets to Content/ pipeline | ⬜ To Build |
| Workspace Integration | Notifies Workspace on export | ⬜ To Build |

### Visual Script Editor (GraphVM)

| System | Description | Status |
|--------|-------------|--------|
| Graph VM Core | Deterministic bytecode VM | ✅ Done (MasterRepo) |
| 14 Graph Types | Logic, event, behavior, etc. | ✅ Done (MasterRepo) |
| Graph Canvas UI | Node drag/connect/delete | ⬜ To Build |
| Node Library Panel | Searchable node palette | ⬜ To Build |
| Execution Debugger | Step through graph live | ⬜ To Build |
| AtlasAI Node | Feed/query AtlasAI from a graph node | ⬜ To Build |

### Shader Editor

| System | Description | Status |
|--------|-------------|--------|
| GLSL Editor | Syntax-highlighted GLSL edit panel | ⬜ To Build |
| Live Preview | Real-time shader preview viewport | ⬜ To Build |
| Uniform Inspector | Tweak shader uniforms live | ⬜ To Build |
| Shader Hot-Reload | Recompile and apply without restart | ⬜ To Build |

### Terrain Editor

| System | Description | Status |
|--------|-------------|--------|
| Height Brush | Sculpt terrain height | ⬜ To Build |
| Paint Brush | Surface texture painting | ⬜ To Build |
| Erosion Tools | Procedural erosion simulation | ⬜ To Build |
| Foliage Scatter | Place vegetation instances | ⬜ To Build |

### Content Browser (Extended)

| System | Description | Status |
|--------|-------------|--------|
| Basic Directory Nav | Navigate Content/ tree | ✅ Done (MasterRepo) |
| Asset Thumbnails | Preview images for assets | ⬜ To Build |
| Import Pipeline | Drag-drop assets → import to Content/ | ⬜ To Build |
| Blender Pipeline Integration | BlenderGen output → Content/Ships/ etc. | ⬜ To Build |
| Asset Database | Indexed, searchable asset registry | ⬜ To Build |
| Schema Validation | Validate JSON data files against Schemas/ | ⬜ To Build |

### Debug & Diagnostics Tools

| System | Description | Status |
|--------|-------------|--------|
| Contract Scanner Panel | C++ code issue display, jump-to-line | ⬜ To Build (ContractScanner tool exists) |
| Balance Checker Panel | AtlasAI rule-fire display | ⬜ To Build (AtlasAI tool exists) |
| Performance Profiler | Frame time, draw call, memory overlays | ⬜ To Build |
| Network Monitor | Live replication/session stats | ⬜ To Build |
| AI Debug Panel | AtlasAI decision log, memory state | ⬜ To Build |
| Log Viewer | Filtered, searchable runtime logs | ⬜ To Build |

### Build Pipeline Panel

| System | Description | Status |
|--------|-------------|--------|
| Build Target Selector | Choose Editor/Game/Server/Tests | ⬜ To Build |
| Inline Build Output | CMake/MSBuild output in panel | ⬜ To Build |
| Packaging Wizard | Bundle + installer generation | ⬜ To Build |

---

## 🤖 AtlasAI — Unified AI Broker Blueprint

> **One broker. Everything talks to AtlasAI through the Workspace.**
> No standalone AI agents. SwissAgent, Arbiter, ArbiterAI all absorbed as subsystems.

```
AtlasAI Progress:  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  0%  (pending merge phases 7-9)
```

```
AtlasAI/
├── Atlas_Broker/          ← THE core broker process (single API entry point)
│   ├── BrokerCore         ← C++ engine-facing API
│   ├── BrokerService      ← Python runtime (LLM/tool routing)
│   ├── API/               ← Unified interface all systems call
│   └── Router/            ← Routes to correct subsystem
│
├── Atlas_Arbiter/         ← Decision-making, task planning (from Arbiter + ArbiterAI)
│   ├── AIEngine/
│   ├── Memory/
│   ├── HostApp/
│   └── VisualStudioExtension/
│
├── Atlas_SwissAgent/      ← Tool execution, plugins, audio, vision (from SwissAgent)
│   ├── core/
│   ├── llm/
│   ├── audio_pipeline/
│   ├── plugins/
│   ├── stable_diffusion/
│   └── native/
│
├── Atlas_Memory/          ← Unified memory (SessionMemory, ProjectMemory, LongTermMemory)
├── Atlas_Bridge/          ← C++ ↔ Python interop layer
└── Atlas_Shared/          ← Types, interfaces, schemas across all subsystems
```

| Subsystem | Source Repo | Status |
|-----------|-------------|--------|
| Atlas_Broker core | New — built here | ⬜ To Build |
| Atlas_Broker API | New — built here | ⬜ To Build |
| Atlas_Arbiter / AIEngine | Arbiter + ArbiterAI | ⬜ Pending merge |
| Atlas_Arbiter / Memory | Arbiter + ArbiterAI | ⬜ Pending merge |
| Atlas_Arbiter / HostApp | Arbiter | ⬜ Pending merge |
| Atlas_Arbiter / VS Extension | Arbiter | ⬜ Pending merge |
| Atlas_SwissAgent / LLM | SwissAgent | ⬜ Pending merge |
| Atlas_SwissAgent / AudioAI | SwissAgent | ⬜ Pending merge |
| Atlas_SwissAgent / Plugins | SwissAgent | ⬜ Pending merge |
| Atlas_SwissAgent / Vision | SwissAgent (stable_diffusion) | ⬜ Pending merge |
| Atlas_Memory unified layer | New — built here | ⬜ To Build |
| Atlas_Bridge (C++↔Python) | New — built here | ⬜ To Build |
| Workspace ↔ AtlasAI pipeline | New — built here | ⬜ To Build |
| Editor AI Debug Panel | New — built here | ⬜ To Build |

---

## 🎮 NovaForge Game — Blueprint & Roadmap

> Built on MasterRepo v001 baseline. Phases 0–5 complete. Phase 6 (Multiplayer) next.

```
Game Progress:  ██████████████████░░░░░░░░░░░░  58%   (Phases 0-5 done, 6-9 remaining)
```

| Phase | Goal | Status |
|-------|------|--------|
| **Phase 0** | Bootstrap — scaffold, build system, project manifest | ✅ Done |
| **Phase 1** | Dev World — sandbox, spawn, save/load, debug overlay | ✅ Done |
| **Phase 2** | Voxel Runtime — chunk data, edit API, serialization | ✅ Done |
| **Phase 3** | First Interaction Loop — R.I.G., mining, inventory, HUD | ✅ Done |
| **Phase 4** | Voxel Mesh Rendering — mesher, GPU cache, Phong shader | ✅ Done |
| **Phase 5** | Movement & FPS Camera — WASD, collision, FPS camera | ✅ Done |
| **Phase 6** | Multiplayer Foundation — server authority, replication, sessions | 🔜 Next |
| **Phase 7** | Galaxy World Gen — cube-sphere, procedural star systems | ⬜ |
| **Phase 8** | Ships & Fleets — modular construction, AI fleet captains | ⬜ |
| **Phase 9** | Economy & Factions — NPC economy, faction standings | ⬜ |
| **Phase 10** | Expeditions — mission system, exploration, salvage | ⬜ |
| **Phase 11** | R.I.G. EVA & Interiors — FPS interiors, oxygen, boarding | ⬜ |
| **Phase 12** | Polish, CI, Modding — packaging, installer, mod support | ⬜ |

---

## ⚙️ Engine Modules — Status

```
Engine Progress:  ██████████████████████░░░░░░░░  73%
```

| Module | Description | Status |
|--------|-------------|--------|
| `NF::Core` | Math, memory, logging, events, reflection, serialization | ✅ Done |
| `NF::Engine` | ECS, world/level, behavior trees, asset system | ✅ Done |
| `NF::Renderer` | OpenGL RHI, forward pipeline, mesh, materials | ✅ Done |
| `NF::Physics` | Rigid bodies, collision, character controller | ✅ Done |
| `NF::Audio` | Device, spatial audio, mixer, cues | ✅ Done |
| `NF::Animation` | Skeleton, blend tree, state machine, IK | ✅ Done |
| `NF::Input` | Keyboard, mouse, gamepad, action mappings | ✅ Done |
| `NF::Networking` | Sockets, replication, sessions, lockstep/rollback | ✅ Done |
| `NF::GraphVM` | Deterministic bytecode VM, 14 graph types | ✅ Done |
| `NF::UI` | Custom 2D renderer — quad batching + stb_easy_font (no ImGui — Custom UI only) | ✅ Done |
| `NF::World` | World gen, cube-sphere, voxel, terrain, galaxy, streaming | ⬜ In progress |
| `NF::AI` | Behavior graphs, NPC logic, faction AI | ⬜ Pending AtlasAI merge |

### Module Dependency Graph

```
NF::Core   ──────────────────────────────────────────────────── all modules depend on this
NF::Engine      → NF::Core
NF::Renderer    → NF::Core, NF::Engine
NF::Physics     → NF::Core, NF::Engine
NF::Audio       → NF::Core
NF::Animation   → NF::Core, NF::Engine
NF::Input       → NF::Core
NF::Networking  → NF::Core
NF::GraphVM     → NF::Core
NF::AI          → NF::Core, NF::Engine, NF::GraphVM, AtlasAI::Broker
NF::World       → NF::Core, NF::Engine
NF::UI          → NF::Core, NF::Renderer
NF::Game        → NF::Core, NF::Engine, NF::UI, NF::Renderer, NF::World
NF::Editor      → NF::Core, NF::Engine, NF::Renderer, NF::Game, NF::UI, Workspace

NovaForgeEditor  → NF::Editor + all modules
NovaForgeGame    → NF::Game + NF::Renderer + NF::UI + supporting modules
NovaForgeServer  → NF::Game + NF::Networking + NF::AI + NF::World
```

---

## 🛠️ Tools Suite — Blueprint

```
Tools Progress:  █░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  5%
```

| Tool | Description | Source | Status |
|------|-------------|--------|--------|
| `Atlas_BlenderGen` | Blender addon — ship/station/asteroid procedural gen | Blender-Generator-for-AtlasForge | ⬜ Pending merge |
| `Atlas_RepoTools` | Repo management, rollup, index tools | Atlas Workspace | ⬜ Pending merge |
| `Atlas_BuildTools` | CMake helpers, build scripts, Makefile | Multiple repos | 🔄 Partial |
| `Atlas_DevTools` | dev_mode, stage_manager, workspace tools | AtlasAI, Atlas Workspace | ⬜ Pending merge |
| `ContractScanner` | C++ static analysis — null checks, bad patterns | tempnovaforge (exists) | ✅ Scaffolded |
| `AtlasAI (tool)` | Rule-based balance checker, CI gate | tempnovaforge (exists) | ✅ Scaffolded |
| `Atlas_Installer` | Packaging + installer generation | NovaForge-Project | ⬜ Pending merge |

---

## 📁 Repository Structure

```
tempnovaforge/
├── README.md                    ← You are here
├── ROADMAP.md                   ← Full detailed roadmap
├── ARCHITECTURE.md              ← Deep system architecture
├── CONTRIBUTING.md
├── LICENSE                      ← GPL-3.0
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json
├── NovaForge.sln
├── Directory.Build.props
├── Dockerfile
├── build.cmd / Makefile
│
├── Source/
│   ├── Core/
│   ├── Engine/
│   ├── Renderer/
│   ├── Physics/
│   ├── Audio/
│   ├── Animation/
│   ├── Input/
│   ├── Networking/
│   ├── GraphVM/
│   ├── AI/
│   ├── World/
│   ├── UI/
│   ├── Game/
│   ├── Editor/
│   └── Programs/
│       ├── NovaForgeEditor/
│       ├── NovaForgeGame/
│       └── NovaForgeServer/
│
├── AtlasAI/                     ← Unified AI broker (all AI systems)
│   ├── Atlas_Broker/
│   ├── Atlas_Arbiter/
│   ├── Atlas_SwissAgent/
│   ├── Atlas_Memory/
│   ├── Atlas_Bridge/
│   └── Atlas_Shared/
│
├── Tools/                       ← All development tools
│   ├── Atlas_BlenderGen/
│   ├── Atlas_RepoTools/
│   ├── Atlas_BuildTools/
│   ├── Atlas_DevTools/
│   ├── ContractScanner/
│   └── ArbiterAI/
│
├── Services/
│   ├── Atlas_Server/
│   └── Atlas_Client/
│
├── Shared/                      ← Shared types, interfaces, common headers
├── Config/
├── Content/
├── Data/
├── Schemas/
├── Scripts/
├── Tests/
├── ThirdParty/
├── Docs/
│   ├── CONSOLIDATION_PLAN.md
│   ├── Engine/
│   ├── Editor/
│   ├── NovaForge_Game/
│   ├── AtlasAI/
│   └── Tools/
│
└── Archive/                     ← Completed repo dumps (pull locally then delete source)
    ├── _MasterRepo/
    ├── _MasterRepoRefactor/
    ├── _AtlasToolingSuite/
    ├── _Nova-Forge-Expeditions/
    ├── _Atlas-NovaForge/
    ├── _AtlasForge/
    ├── _NovaForge-Project/
    ├── _SwissAgent/
    ├── _ArbiterAI/
    ├── _Arbiter/
    ├── _AtlasForge-EveOffline/
    └── _Blender-Generator-for-AtlasForge/
```

---

## 🚀 Build Requirements

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| CMake | 3.22 | CMakePresets v6 |
| Visual Studio | 2022 (17.x) | Primary — C++ Desktop workload |
| MSVC | 19.30+ | C++20 |
| GCC | 13+ | Linux/CI |
| Clang | 15+ | Optional |
| OpenGL | 3.3 core | System driver |
| Python | 3.11+ | AtlasAI runtime |

---

## 🚀 Quick Start — Windows (Primary)

```bat
git clone https://github.com/shifty81/tempnovaforge.git NovaForge
cd NovaForge
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug
Builds\windows-x64-debug\bin\Debug\NovaForgeEditor.exe
```

Or open Visual Studio → **File → Open → CMake…** → select `CMakeLists.txt` → pick `windows-x64-debug` preset → F5.

---

## 🚀 Quick Start — Linux / macOS

```bash
git clone https://github.com/shifty81/tempnovaforge.git NovaForge
cd NovaForge
cmake --preset debug
cmake --build --preset debug --parallel
./Builds/debug/bin/NovaForgeEditor
```

---

## 📐 Design Principles

- **Workspace is the glue.** Every tool and editor routes through the Workspace. Nothing moves files or fires events without Workspace knowing.
- **AtlasAI is the single broker.** No standalone AI agents. One API, one runtime, all systems route through it.
- **Voxel layer is authoritative.** Structure, mining, repair, damage, destruction — all voxel operations.
- **R.I.G. first.** Player suit is the primary gameplay object. Mechanics build outward from R.I.G. state.
- **No ImGui.** Editor uses a fully custom UI renderer (Custom UI — UIRenderer + GDI/GPU backend).
- **Determinism first.** All simulation must be bit-exact reproducible.
- **Phases over features.** Each phase has a tight locked deliverable.
- **Editor does not ship.** `NovaForgeGame` is the shippable artifact.
- **Everything is data.** Game content lives in JSON, fully moddable.
- **One repo, full context.** Engine, editor, game, server, AI, tools — all context-aware.

---

## 🌌 Game Vision

Nova Forge is a **PvE space simulation** — command ships, build fleets, explore procedurally generated star systems, forge your legend.

| Pillar | Description |
|--------|-------------|
| **Command** | Pilot ships from frigate to titan |
| **Build** | Modular ships, stations, habitats with snap-grid construction |
| **Lead** | AI fleet captains with personalities, morale, and grudges. **Fleet command is a secondary system unlocked after growing your crew beyond 5 members. First-person gameplay is the primary loop.** |
| **Discover** | Scan anomalies, salvage wrecks, decode ancient tech |
| **Forge** | Manufacture, refine, research — drive a living NPC economy |
| **Survive** | FPS interiors, EVA, oxygen management, planetary exploration |

| Faction | Style | Specialty |
|---------|-------|-----------|
| **Solari** | Golden / elegant | Armor tanking, energy weapons |
| **Veyren** | Angular / utilitarian | Shield tanking, hybrid turrets |
| **Aurelian** | Sleek / organic | Speed, drones, electronic warfare |
| **Keldari** | Rugged / industrial | Missiles, shields, ECM |

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [`Docs/CONSOLIDATION_PLAN.md`](Docs/CONSOLIDATION_PLAN.md) | Full repo merge plan, per-repo action items |
| [`Docs/ARCHITECTURE.md`](Docs/ARCHITECTURE.md) | Deep system architecture |
| [`Docs/Editor/EDITOR_ROADMAP.md`](Docs/Editor/EDITOR_ROADMAP.md) | Full editor suite roadmap |
| [`Docs/AtlasAI/ATLASAI_ROADMAP.md`](Docs/AtlasAI/ATLASAI_ROADMAP.md) | AtlasAI broker build plan |
| [`Docs/Game/ROADMAP.md`](Docs/Game/ROADMAP.md) | Phase-by-phase game delivery plan |
| [`Docs/Game/TASKS.md`](Docs/Game/TASKS.md) | Checked task list per phase |
| [`Docs/Game/PROJECT_RULES.md`](Docs/Game/PROJECT_RULES.md) | Hard boundaries and coding rules |

---

## 📦 Consolidating From

| Repo | Role | Archive Status |
|------|------|----------------|
| [MasterRepo](https://github.com/shifty81/MasterRepo) | v001 structural baseline — seed | 🔄 In Progress |
| [MasterRepoRefactor](https://github.com/shifty81/MasterRepoRefactor) | Refactored Atlas+NovaForge structure | ⬜ Queued |
| [AtlasToolingSuite](https://github.com/shifty81/AtlasToolingSuite) | Atlas Workspace tooling source | ⬜ Queued |
| [Nova-Forge-Expeditions](https://github.com/shifty81/Nova-Forge-Expeditions) | Richest game codebase | ⬜ Queued |
| [Atlas-NovaForge](https://github.com/shifty81/Atlas-NovaForge) | Merged engine+game attempt | ⬜ Queued |
| [AtlasForge](https://github.com/shifty81/AtlasForge) | Original engine | ⬜ Queued |
| [NovaForge-Project](https://github.com/shifty81/NovaForge-Project) | Game project structure + rules | ⬜ Queued |
| [SwissAgent](https://github.com/shifty81/SwissAgent) | AI agent → AtlasAI | ⬜ Queued |
| [ArbiterAI](https://github.com/shifty81/ArbiterAI) | AI agent → AtlasAI | ⬜ Queued |
| [Arbiter](https://github.com/shifty81/Arbiter) | AI agent → AtlasAI | ⬜ Queued |
| [AtlasForge-EveOffline](https://github.com/shifty81/AtlasForge-EveOffline) | Networking prototype | ⬜ Queued |
| [Blender-Generator-for-AtlasForge](https://github.com/shifty81/Blender-Generator-for-AtlasForge) | Blender ship/station gen | ⬜ Queued |

---

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE).