//! novaforge_world — Bevy plugin that integrates bevy_voxel_world for NovaForge.
//!
//! Add `WorldPlugin` to your `App` to enable infinite streaming voxel terrain.
//! The camera must have a `VoxelWorldCamera::<NovaForgeWorldConfig>` component
//! so that bevy_voxel_world knows which camera drives chunk spawning.

use bevy::prelude::*;
use bevy_voxel_world::prelude::*;

pub use novaforge_voxel::NovaForgeWorldConfig;

// ── Plugin ────────────────────────────────────────────────────────────────────

/// Adds the bevy_voxel_world streaming voxel world to the application.
///
/// What this sets up:
///   - Registers `VoxelWorldPlugin::<NovaForgeWorldConfig>` which handles:
///       • Procedural terrain generation (sine/cosine hills, stone/dirt/grass layers)
///       • Multithreaded chunk meshing
///       • Dynamic chunk spawn/despawn around the camera
pub struct WorldPlugin;

impl Plugin for WorldPlugin {
    fn build(&self, app: &mut App) {
        app.add_plugins(VoxelWorldPlugin::with_config(NovaForgeWorldConfig));

        tracing::info!(target: "World", "NovaForge voxel world plugin registered");
    }
}
