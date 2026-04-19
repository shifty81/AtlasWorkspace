//! atlas_core — Core types, version string, and shared voxel primitives.
//!
//! This crate is intentionally free of Bevy dependencies so it can be used
//! in server-side and tooling contexts as well as in the game runtime.

use serde::{Deserialize, Serialize};

// ── Version ───────────────────────────────────────────────────────────────────

pub const NF_VERSION_STRING: &str = "0.1.0";
pub const NF_VERSION_MAJOR: u32 = 0;
pub const NF_VERSION_MINOR: u32 = 1;
pub const NF_VERSION_PATCH: u32 = 0;

// ── Voxel material indices (u8 used by bevy_voxel_world) ──────────────────────

/// Raw u8 material index constants consumed by bevy_voxel_world's `WorldVoxel::Solid(idx)`.
pub mod mat {
    pub const STONE:      u8 = 0;
    pub const DIRT:       u8 = 1;
    pub const GRASS:      u8 = 2;
    pub const WATER:      u8 = 3;
    pub const METAL:      u8 = 4;
    pub const GLASS:      u8 = 5;
    pub const ORE_IRON:   u8 = 6;
    pub const ORE_GOLD:   u8 = 7;
    pub const ORE_CRYSTAL:u8 = 8;
}

// ── VoxelType enum (game-logic, not renderer) ─────────────────────────────────

/// High-level voxel type used in game logic.  Serialisation-safe, no Bevy deps.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
#[repr(u8)]
pub enum VoxelType {
    #[default]
    Air = 0,
    Stone,
    Dirt,
    Grass,
    Metal,
    Glass,
    Water,
    OreIron,
    OreGold,
    OreCrystal,
}

impl VoxelType {
    pub fn name(self) -> &'static str {
        match self {
            VoxelType::Air        => "Air",
            VoxelType::Stone      => "Stone",
            VoxelType::Dirt       => "Dirt",
            VoxelType::Grass      => "Grass",
            VoxelType::Metal      => "Metal",
            VoxelType::Glass      => "Glass",
            VoxelType::Water      => "Water",
            VoxelType::OreIron    => "OreIron",
            VoxelType::OreGold    => "OreGold",
            VoxelType::OreCrystal => "OreCrystal",
        }
    }

    pub fn from_name(name: &str) -> Self {
        match name {
            "Stone"      => VoxelType::Stone,
            "Dirt"       => VoxelType::Dirt,
            "Grass"      => VoxelType::Grass,
            "Metal"      => VoxelType::Metal,
            "Glass"      => VoxelType::Glass,
            "Water"      => VoxelType::Water,
            "OreIron"    => VoxelType::OreIron,
            "OreGold"    => VoxelType::OreGold,
            "OreCrystal" => VoxelType::OreCrystal,
            _            => VoxelType::Air,
        }
    }

    /// Convert to the bevy_voxel_world material index.
    pub fn to_mat_index(self) -> u8 {
        match self {
            VoxelType::Air        => mat::STONE, // shouldn't be used for Air
            VoxelType::Stone      => mat::STONE,
            VoxelType::Dirt       => mat::DIRT,
            VoxelType::Grass      => mat::GRASS,
            VoxelType::Water      => mat::WATER,
            VoxelType::Metal      => mat::METAL,
            VoxelType::Glass      => mat::GLASS,
            VoxelType::OreIron    => mat::ORE_IRON,
            VoxelType::OreGold    => mat::ORE_GOLD,
            VoxelType::OreCrystal => mat::ORE_CRYSTAL,
        }
    }
}

// ── Chunk ─────────────────────────────────────────────────────────────────────

/// Side length of a chunk (voxels per axis), matching the C++ CHUNK_SIZE.
pub const CHUNK_SIZE: usize = 16;

/// A 16×16×16 block of voxels. Used for game-logic and serialization;
/// the renderer uses bevy_voxel_world's internal chunk representation.
#[derive(Clone, Serialize, Deserialize)]
pub struct Chunk {
    pub voxels: Box<[[[VoxelType; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE]>,
    pub cx: i32,
    pub cy: i32,
    pub cz: i32,
    #[serde(skip)]
    pub mesh_dirty: bool,
    #[serde(skip)]
    pub collision_dirty: bool,
}

impl Default for Chunk {
    fn default() -> Self {
        Self {
            voxels: Box::new([[[VoxelType::Air; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE]),
            cx: 0,
            cy: 0,
            cz: 0,
            mesh_dirty: true,
            collision_dirty: true,
        }
    }
}

impl Chunk {
    pub fn new(cx: i32, cy: i32, cz: i32) -> Self {
        Self { cx, cy, cz, ..Default::default() }
    }

    #[inline]
    pub fn get(&self, x: usize, y: usize, z: usize) -> VoxelType {
        if x < CHUNK_SIZE && y < CHUNK_SIZE && z < CHUNK_SIZE {
            self.voxels[x][y][z]
        } else {
            VoxelType::Air
        }
    }

    #[inline]
    pub fn set(&mut self, x: usize, y: usize, z: usize, vt: VoxelType) {
        if x < CHUNK_SIZE && y < CHUNK_SIZE && z < CHUNK_SIZE {
            self.voxels[x][y][z] = vt;
            self.mesh_dirty = true;
            self.collision_dirty = true;
        }
    }

    pub fn is_fully_air(&self) -> bool {
        self.voxels.iter().flatten().flatten().all(|&v| v == VoxelType::Air)
    }

    pub fn solid_count(&self) -> usize {
        self.voxels.iter().flatten().flatten().filter(|&&v| v != VoxelType::Air).count()
    }

    pub fn mark_all_clean(&mut self) {
        self.mesh_dirty = false;
        self.collision_dirty = false;
    }
}

// ── Logging macros ────────────────────────────────────────────────────────────

/// Emit an INFO log tagged with a subsystem label, matching the C++ NF_LOG_INFO style.
#[macro_export]
macro_rules! nf_info {
    ($subsystem:expr, $msg:expr) => {
        tracing::info!(target: $subsystem, "{}", $msg)
    };
    ($subsystem:expr, $fmt:literal, $($arg:tt)*) => {
        tracing::info!(target: $subsystem, $fmt, $($arg)*)
    };
}

/// Emit a WARN log tagged with a subsystem label.
#[macro_export]
macro_rules! nf_warn {
    ($subsystem:expr, $msg:expr) => {
        tracing::warn!(target: $subsystem, "{}", $msg)
    };
}

/// Emit an ERROR log tagged with a subsystem label.
#[macro_export]
macro_rules! nf_error {
    ($subsystem:expr, $msg:expr) => {
        tracing::error!(target: $subsystem, "{}", $msg)
    };
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn version_string_is_non_empty() {
        assert!(!NF_VERSION_STRING.is_empty());
    }

    #[test]
    fn version_parts_make_sense() {
        assert!(NF_VERSION_MAJOR >= 0);
        assert!(NF_VERSION_MINOR >= 0);
        assert!(NF_VERSION_PATCH >= 0);
    }

    #[test]
    fn material_ids_are_distinct() {
        let ids = [mat::STONE, mat::DIRT, mat::GRASS, mat::WATER, mat::METAL, mat::GLASS,
                   mat::ORE_IRON, mat::ORE_GOLD, mat::ORE_CRYSTAL];
        for i in 0..ids.len() {
            for j in (i + 1)..ids.len() {
                assert_ne!(ids[i], ids[j], "material IDs must be unique");
            }
        }
    }

    #[test]
    fn stone_material_is_zero() {
        assert_eq!(mat::STONE, 0);
    }
}
