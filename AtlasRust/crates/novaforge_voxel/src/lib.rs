//! novaforge_voxel — VoxelWorldConfig implementation and procedural terrain generator.
//!
//! The terrain formula mirrors the C++ `WorldGenerator::generateChunk()`:
//!   height = 8 + (2.0 * sin(x * 0.05) * cos(z * 0.05)) as i32
//!
//! Voxel layers (same as C++):
//!   worldY < height - 3  →  Stone
//!   height-3 ≤ worldY < height  →  Dirt
//!   worldY == height  →  Grass
//!   worldY > height  →  Air

use std::sync::Arc;

use bevy::prelude::*;
use bevy_voxel_world::prelude::*;

use atlas_core::mat;

// ── World config ──────────────────────────────────────────────────────────────

/// bevy_voxel_world configuration for the NovaForge game world.
///
/// This struct serves double duty: it holds config values *and* acts as the
/// world-instance identifier (multiple `VoxelWorldPlugin` instances are
/// distinguished by their config struct type).
#[derive(Resource, Clone, Default)]
pub struct NovaForgeWorldConfig;

impl VoxelWorldConfig for NovaForgeWorldConfig {
    /// Material index type — u8 is plenty for our voxel palette.
    type MaterialIndex = u8;
    /// No extra per-chunk components for now.
    type ChunkUserBundle = ();

    /// Render chunks within 12 chunks of the camera (~192 m at 16 m chunks).
    fn spawning_distance(&self) -> u32 {
        12
    }

    /// Always keep 2 chunks loaded around the camera to avoid pop-in near the player.
    fn min_despawn_distance(&self) -> u32 {
        2
    }

    /// Return a delegate that creates the per-chunk terrain lookup closure.
    fn voxel_lookup_delegate(&self) -> VoxelLookupDelegate<Self::MaterialIndex> {
        Box::new(move |_chunk_pos, _lod, _previous| terrain_voxel_fn())
    }

    /// Map material index to [top, sides, bottom] texture atlas slots.
    /// Without a texture atlas configured (`voxel_texture = None`) the renderer
    /// uses its default material, so these values are present for when a texture
    /// is added in a later phase.
    fn texture_index_mapper(&self) -> Arc<dyn Fn(u8) -> [u32; 3] + Send + Sync> {
        Arc::new(|mat_idx| match mat_idx {
            mat::STONE => [0, 0, 0],         // stone: same on all faces
            mat::DIRT  => [1, 1, 1],         // dirt:  same on all faces
            mat::GRASS => [2, 1, 1],         // grass: top=grass, sides/bottom=dirt
            mat::WATER => [3, 3, 3],         // water: same on all faces
            mat::METAL => [4, 4, 4],
            mat::GLASS => [5, 5, 5],
            _          => [0, 0, 0],
        })
    }
}

// ── Terrain generation ────────────────────────────────────────────────────────

/// Build the per-voxel terrain function for one chunk.
///
/// Matches the C++ `WorldGenerator::generateChunk()` formula exactly:
///   height = 8 + int(2.0 * sin(worldX * 0.05) * cos(worldZ * 0.05))
fn terrain_voxel_fn(
) -> Box<dyn FnMut(IVec3, Option<WorldVoxel<u8>>) -> WorldVoxel<u8> + Send + Sync> {
    Box::new(move |pos: IVec3, _previous| {
        let wx = pos.x as f32;
        let wz = pos.z as f32;
        let wy = pos.y;

        // Sine/cosine hill formula from C++ WorldGenerator
        let height = 8 + (2.0_f32 * (wx * 0.05).sin() * (wz * 0.05).cos()) as i32;

        if wy < height - 3 {
            WorldVoxel::Solid(mat::STONE)
        } else if wy < height {
            WorldVoxel::Solid(mat::DIRT)
        } else if wy == height {
            WorldVoxel::Solid(mat::GRASS)
        } else {
            WorldVoxel::Air
        }
    })
}
