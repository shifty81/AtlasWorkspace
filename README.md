# AtlasWorkspace

**AtlasWorkspace** is a Rust workspace platform for building games, tools, and development pipelines. It provides a unified host for editors, build systems, AtlasAI workflows, and project orchestration.

> **Experimental branch:** This branch is a full Rust conversion. The C++ source in `Source/` is retained as a reference. All active development now lives in the Cargo workspace rooted at this directory.

---

## Quick Start

```bash
# Type-check everything (fast, no codegen)
cargo check

# Build all crates and binaries
cargo build

# Run the AtlasWorkspace IDE host
cargo run --bin atlas_workspace

# Run the NovaForge voxel editor
cargo run --bin novaforge_editor

# Run the NovaForge dedicated server
cargo run --bin novaforge_server

# Run all tests
cargo test

# Lint (warnings as errors)
cargo clippy -- -D warnings

# Format check
cargo fmt --check
```

Or use the `Makefile` shortcuts:

```bash
make check        # fast type-check
make build        # debug build
make build-release # optimised release
make test         # all tests
make lint         # clippy strict
make fmt          # format check
make run-workspace # launch workspace IDE
make run-server    # launch headless server
```

---

## Workspace Layout

```
Cargo.toml              ← Rust workspace root  ← START HERE
rust-toolchain.toml     ← pinned stable toolchain
.cargo/config.toml      ← cargo aliases, build profiles, clippy settings

AtlasRust/
  crates/
    atlas_core          ← shared primitives, versioning
    atlas_engine        ← ECS, event bus, logger, tick loop
    atlas_animation     ← skeleton, clips, state machine
    atlas_audio         ← audio device, mixer, spatial audio
    atlas_editor        ← panels, inspector, console, undo stack
    atlas_input         ← keyboard, mouse, gamepad, action bindings
    atlas_networking    ← packets, connections, network manager
    atlas_physics       ← AABB, rigid bodies, physics world
    atlas_pipeline      ← file-based IPC: change events, manifest, watcher
    atlas_renderer      ← mesh, shader, material, render commands
    atlas_ui            ← theme tokens, widgets, notification host
    atlas_workspace     ← workspace shell, project serializer
    novaforge_editor_adapter ← editor ↔ game bridge
    novaforge_patch     ← patch manifests, verifier, applicator
    novaforge_voxel     ← voxel world config and terrain (Bevy)
    novaforge_world     ← Bevy world plugin
    novaforge_game      ← game logic, fly-cam, HUD (Bevy)
  apps/
    atlas_workspace_app  → binary: atlas_workspace
    novaforge_editor_app → binary: novaforge_editor
    novaforge_server_app → binary: novaforge_server
    novaforge_game_app   → binary: novaforge_game

Source/   ← C++ reference implementation (archived, not built by default)
Docs/     ← Architecture, roadmap, and canon documentation
```

---

## Crate Overview

| Crate | Purpose |
|-------|---------|
| `atlas_core` | Version constants, shared primitive types |
| `atlas_engine` | Engine config, ECS entity/component store, event bus, tick loop |
| `atlas_animation` | Skeleton, keyframe channels, animation clip, state machine |
| `atlas_audio` | Audio clips, playback sources, multi-channel mixer, listener |
| `atlas_input` | Key codes, action bindings, per-frame event dispatch |
| `atlas_physics` | AABB, ray-cast, sphere, rigid-body integration, physics world |
| `atlas_networking` | Packet types, connection state, reliable queue, network manager |
| `atlas_pipeline` | File-based IPC: change events, asset manifest, pipeline watcher |
| `atlas_renderer` | Mesh, shader, material, render-command queue (GPU at app level) |
| `atlas_ui` | Theme tokens, colour system, command registry, panel registry |
| `atlas_workspace` | Project file, workspace shell, tool registry, snapshot/restore |
| `atlas_editor` | Editor panels, inspector, console, diagnostics, undo stack |
| `novaforge_editor_adapter` | Editor ↔ game bridge, launch contract, world sync |
| `novaforge_patch` | Patch manifests, integrity verification, applicator |
| `novaforge_voxel` | Voxel world config and procedural terrain (requires Bevy) |
| `novaforge_world` | Bevy WorldPlugin for NovaForge (requires Bevy) |
| `novaforge_game` | Game logic, fly-camera, player stats, egui HUD (requires Bevy) |

---

## Binaries

| Binary | Crate | Description |
|--------|-------|-------------|
| `atlas_workspace` | `atlas_workspace_app` | AtlasWorkspace IDE host |
| `novaforge_editor` | `novaforge_editor_app` | NovaForge voxel world editor |
| `novaforge_server` | `novaforge_server_app` | NovaForge headless dedicated server |
| `novaforge_game` | `novaforge_game_app` | NovaForge standalone game client (Bevy) |

---

## Build Profiles

| Profile | Command | Use |
|---------|---------|-----|
| `dev` (default) | `cargo build` | Fast iteration |
| `release` | `cargo build --release` | Shipping builds (LTO, strip) |
| `release-debug` | `cargo build --profile release-debug` | Profiling with symbols |
| `ci` | `cargo build --profile ci` | Faster CI (opt-level 1) |

---

## Docker

```bash
# Build the dedicated server image
docker build --target server -t novaforge-server .

# Run it
docker run --rm -p 7777:7777/udp novaforge-server

# Build the workspace IDE image
docker build --target workspace -t atlas-workspace .
```

---

## Cargo Aliases

Defined in `.cargo/config.toml`:

| Alias | Expands to |
|-------|-----------|
| `cargo ws` | `cargo build --workspace` |
| `cargo wsr` | `cargo build --workspace --release` |
| `cargo wsc` | `cargo check --workspace` |
| `cargo editor` | `cargo run --bin novaforge_editor` |
| `cargo server` | `cargo run --bin novaforge_server` |
| `cargo game` | `cargo run --bin novaforge_game` |
| `cargo workspace` | `cargo run --bin atlas_workspace` |
| `cargo t` | `cargo test --workspace` |
| `cargo lint` | `cargo clippy --workspace -- -D warnings` |
| `cargo doc` | `cargo doc --workspace --no-deps --open` |

---

## Current Status

Active phase: **Experimental — Full Rust Conversion**

- [x] Cargo workspace promoted to repo root
- [x] All engine subsystems implemented in Rust
- [x] All app binaries implemented
- [x] Rust-native Makefile, Dockerfile, toolchain, and cargo config
- [ ] Bevy integration crates compile (requires rustc ≥ 1.95)
- [ ] egui window loop wired for editor and workspace apps
- [ ] Full test suite

---

## Documentation

- [Architecture docs](Docs/Canon/)
- [Roadmap](Docs/Roadmap/)
- [C++ reference source](Source/)

