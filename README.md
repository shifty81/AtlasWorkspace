# Atlas Workspace

> **Generic C++ Development Environment** | Platform-first design

**Atlas Workspace** is a native C++ development environment that:

- Loads external projects
- Provides editors and tools
- Manages assets, builds, logs, and automation
- Brokers AI assistance through AtlasAI

[![Language](https://img.shields.io/badge/language-C%2B%2B20-brightgreen)]()
[![Build](https://img.shields.io/badge/build-CMake-blue)]()

## Architecture

Atlas Workspace is divided into four layers:

1. **Workspace Core** — windowing, UI framework, docking, command system, panels, project loader, plugin system, asset indexing, build orchestration, AtlasAI broker, Git integration
2. **Workspace SDK** — extension contracts for plugins, editors, asset types, document types, build providers
3. **Project Adapter** — bridge between Workspace and a specific project (e.g. NovaForge)
4. **Project Content** — the loaded repo, read through the adapter

## Repository Structure

```
/AtlasWorkspace
  /Source          — Workspace platform modules (Core, UI, Editor, GraphVM, Renderer, AI, Pipeline)
  /Tests           — Workspace platform tests
  /Docs            — Workspace documentation
  /Project         — Generic project samples
  /Schemas         — Workspace schemas (atlas.project.v1, atlas.build.v1)
  /Atlas           — Atlas tools and configuration
  /AtlasAI         — AtlasAI broker
  /Codex           — Logger/diagnostics/knowledge system
  /Scripts         — Build scripts
  /Tools           — Workspace tools
  /ThirdParty      — Third-party dependencies
  /NovaForge       — NovaForge game project (adapter + game source)
```

## Building

```bash
# Configure and build workspace
cmake --preset debug
cmake --build --preset debug --parallel

# Run workspace tests
ctest --preset debug
```

## NovaForge Project

The NovaForge game project lives in `/NovaForge`. It is a project adapter that
extends Atlas Workspace with game-specific editors, systems, and content.

See `/NovaForge/CMakeLists.txt` for the NovaForge build configuration.

## Key Principles

- Workspace must never depend on a project
- Projects depend on Workspace via SDK
- All extensions go through plugins/adapters
- No gameplay logic in core
- All systems must be reusable across unknown project types
