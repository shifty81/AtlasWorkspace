//! atlas_renderer — Mesh, Shader, Material, and forward render pipeline.
//!
//! Pure-Rust data layer mirroring the C++ NF::Renderer system. GPU submission
//! is handled by Bevy's renderer at the app level; this crate provides the
//! shared data types and render-command queue used by both editor and game.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

// ── Math types (local, no Bevy dep) ──────────────────────────────────────────

#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize, PartialEq)]
pub struct Vec2 { pub x: f32, pub y: f32 }
#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize, PartialEq)]
pub struct Vec3 { pub x: f32, pub y: f32, pub z: f32 }
#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq)]
pub struct Vec4 { pub x: f32, pub y: f32, pub z: f32, pub w: f32 }

impl Vec3 {
    pub fn new(x: f32, y: f32, z: f32) -> Self { Self { x, y, z } }
    pub fn min_elem(self, b: Self) -> Self { Self::new(self.x.min(b.x), self.y.min(b.y), self.z.min(b.z)) }
    pub fn max_elem(self, b: Self) -> Self { Self::new(self.x.max(b.x), self.y.max(b.y), self.z.max(b.z)) }
}

impl Default for Vec4 {
    fn default() -> Self { Self { x: 1.0, y: 1.0, z: 1.0, w: 1.0 } }
}

// 4×4 column-major matrix (row-major storage)
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub struct Mat4(pub [[f32; 4]; 4]);

impl Mat4 {
    pub fn identity() -> Self {
        Self([
            [1.0, 0.0, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ])
    }
}

impl Default for Mat4 {
    fn default() -> Self {
        Self([
            [1.0, 0.0, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ])
    }
}

// ── SceneRenderMode ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Serialize, Deserialize)]
pub enum SceneRenderMode {
    #[default]
    Lit,
    Unlit,
    Wireframe,
    Normals,
    Overdraw,
}

impl SceneRenderMode {
    pub fn name(self) -> &'static str {
        match self {
            Self::Lit       => "Lit",
            Self::Unlit     => "Unlit",
            Self::Wireframe => "Wireframe",
            Self::Normals   => "Normals",
            Self::Overdraw  => "Overdraw",
        }
    }
}

// ── Vertex ────────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct Vertex {
    pub position:  Vec3,
    pub normal:    Vec3,
    pub tex_coord: Vec2,
    pub color:     Vec4,
}

// ── Mesh ──────────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct Mesh {
    vertices: Vec<Vertex>,
    indices:  Vec<u32>,
    dirty:    bool,
}

impl Mesh {
    pub fn new(vertices: Vec<Vertex>, indices: Vec<u32>) -> Self {
        Self { vertices, indices, dirty: true }
    }

    pub fn set_vertices(&mut self, v: Vec<Vertex>) { self.vertices = v; self.dirty = true; }
    pub fn set_indices(&mut self, i: Vec<u32>) { self.indices = i; self.dirty = true; }

    pub fn vertices(&self) -> &[Vertex] { &self.vertices }
    pub fn indices(&self) -> &[u32] { &self.indices }
    pub fn vertex_count(&self) -> usize { self.vertices.len() }
    pub fn index_count(&self) -> usize { self.indices.len() }
    pub fn triangle_count(&self) -> usize { self.indices.len() / 3 }
    pub fn is_dirty(&self) -> bool { self.dirty }
    pub fn clear_dirty(&mut self) { self.dirty = false; }

    /// Returns (AABB min, AABB max) of all vertex positions.
    pub fn bounds(&self) -> Option<(Vec3, Vec3)> {
        let mut it = self.vertices.iter();
        let first = it.next()?.position;
        let (lo, hi) = it.fold((first, first), |(lo, hi), v| {
            (lo.min_elem(v.position), hi.max_elem(v.position))
        });
        Some((lo, hi))
    }
}

// ── Shader ────────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ShaderStage { Vertex, Fragment, Geometry, Compute }

#[derive(Debug, Clone, Default)]
pub struct Shader {
    pub name:            String,
    pub vertex_source:   String,
    pub fragment_source: String,
    compiled:            bool,
    float_uniforms:      HashMap<String, f32>,
    vec3_uniforms:       HashMap<String, Vec3>,
    mat4_uniforms:       HashMap<String, Mat4>,
}

impl Shader {
    pub fn new(name: impl Into<String>, vert: impl Into<String>, frag: impl Into<String>) -> Self {
        Self {
            name: name.into(),
            vertex_source: vert.into(),
            fragment_source: frag.into(),
            ..Default::default()
        }
    }

    /// Validate that both source strings are non-empty.
    pub fn compile(&mut self) -> bool {
        self.compiled = !self.vertex_source.is_empty() && !self.fragment_source.is_empty();
        self.compiled
    }

    pub fn is_compiled(&self) -> bool { self.compiled }

    pub fn set_uniform_f32(&mut self, name: impl Into<String>, v: f32) { self.float_uniforms.insert(name.into(), v); }
    pub fn set_uniform_vec3(&mut self, name: impl Into<String>, v: Vec3) { self.vec3_uniforms.insert(name.into(), v); }
    pub fn set_uniform_mat4(&mut self, name: impl Into<String>, v: Mat4) { self.mat4_uniforms.insert(name.into(), v); }

    pub fn uniform_f32(&self, name: &str) -> Option<f32> { self.float_uniforms.get(name).copied() }
    pub fn uniform_vec3(&self, name: &str) -> Option<Vec3> { self.vec3_uniforms.get(name).copied() }
}

// ── Material ──────────────────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct Material {
    pub name:      String,
    pub shader:    Option<usize>,   // index into Renderer::shaders
    pub color:     Vec4,
    pub metallic:  f32,
    pub roughness: f32,
}

impl Material {
    pub fn new(name: impl Into<String>) -> Self {
        Self {
            name:      name.into(),
            shader:    None,
            color:     Vec4::default(),
            metallic:  0.0,
            roughness: 0.5,
        }
    }
}

// ── RenderCommand ─────────────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct RenderCommand {
    pub mesh_index:     usize,
    pub material_index: usize,
    pub transform:      Mat4,
    pub render_mode:    SceneRenderMode,
    pub cast_shadow:    bool,
}

// ── Renderer ─────────────────────────────────────────────────────────────────

/// Forward-renderer data manager — owns meshes, shaders, materials, and the
/// per-frame command queue. Actual GPU submission is the app's responsibility.
pub struct Renderer {
    meshes:    Vec<Mesh>,
    shaders:   Vec<Shader>,
    materials: Vec<Material>,
    commands:  Vec<RenderCommand>,
    render_mode: SceneRenderMode,
    viewport:  (u32, u32),
}

impl Renderer {
    pub fn new(width: u32, height: u32) -> Self {
        Self {
            meshes:    Vec::new(),
            shaders:   Vec::new(),
            materials: Vec::new(),
            commands:  Vec::new(),
            render_mode: SceneRenderMode::Lit,
            viewport:  (width, height),
        }
    }

    pub fn init(&self) {
        tracing::info!(target: "Renderer", "Renderer initialized ({}×{})", self.viewport.0, self.viewport.1);
    }

    pub fn shutdown(&mut self) {
        self.meshes.clear();
        self.shaders.clear();
        self.materials.clear();
        self.commands.clear();
        tracing::info!(target: "Renderer", "Renderer shutdown");
    }

    pub fn add_mesh(&mut self, mesh: Mesh) -> usize { let i = self.meshes.len(); self.meshes.push(mesh); i }
    pub fn add_shader(&mut self, shader: Shader) -> usize { let i = self.shaders.len(); self.shaders.push(shader); i }
    pub fn add_material(&mut self, mat: Material) -> usize { let i = self.materials.len(); self.materials.push(mat); i }

    pub fn submit(&mut self, cmd: RenderCommand) { self.commands.push(cmd); }
    pub fn flush(&mut self) { self.commands.clear(); }

    pub fn set_render_mode(&mut self, mode: SceneRenderMode) { self.render_mode = mode; }
    pub fn render_mode(&self) -> SceneRenderMode { self.render_mode }
    pub fn set_viewport(&mut self, w: u32, h: u32) { self.viewport = (w, h); }
    pub fn viewport(&self) -> (u32, u32) { self.viewport }

    pub fn mesh(&self, i: usize) -> Option<&Mesh> { self.meshes.get(i) }
    pub fn shader(&self, i: usize) -> Option<&Shader> { self.shaders.get(i) }
    pub fn material(&self, i: usize) -> Option<&Material> { self.materials.get(i) }
    pub fn commands(&self) -> &[RenderCommand] { &self.commands }

    pub fn mesh_count(&self) -> usize { self.meshes.len() }
    pub fn shader_count(&self) -> usize { self.shaders.len() }
    pub fn material_count(&self) -> usize { self.materials.len() }
}

impl Default for Renderer {
    fn default() -> Self { Self::new(1280, 720) }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn sample_mesh() -> Mesh {
        let verts = vec![
            Vertex { position: Vec3::new(0.0, 0.0, 0.0), ..Default::default() },
            Vertex { position: Vec3::new(1.0, 0.0, 0.0), ..Default::default() },
            Vertex { position: Vec3::new(0.5, 1.0, 0.0), ..Default::default() },
        ];
        let indices = vec![0, 1, 2];
        Mesh::new(verts, indices)
    }

    #[test]
    fn mesh_triangle_count() {
        let mesh = sample_mesh();
        assert_eq!(mesh.triangle_count(), 1);
        assert_eq!(mesh.vertex_count(), 3);
    }

    #[test]
    fn mesh_bounds() {
        let mesh = sample_mesh();
        let (min, max) = mesh.bounds().unwrap();
        assert_eq!(min.x, 0.0);
        assert_eq!(max.x, 1.0);
        assert_eq!(max.y, 1.0);
    }

    #[test]
    fn mesh_dirty_flag() {
        let mut mesh = sample_mesh();
        assert!(mesh.is_dirty(), "new mesh should be dirty");
        mesh.clear_dirty();
        assert!(!mesh.is_dirty());
        mesh.set_vertices(vec![]);
        assert!(mesh.is_dirty(), "set_vertices must mark dirty");
    }

    #[test]
    fn shader_compile() {
        let mut shader = Shader::new("Test", "void main() {}", "void main() {}");
        assert!(shader.compile());
        assert!(shader.is_compiled());
    }

    #[test]
    fn shader_uniforms() {
        let mut shader = Shader::new("Test", "", "");
        shader.set_uniform_f32("time", 1.5);
        shader.set_uniform_vec3("lightDir", Vec3::new(0.0, 1.0, 0.0));
        assert!((shader.uniform_f32("time").unwrap() - 1.5).abs() < 1e-5);
        let ld = shader.uniform_vec3("lightDir").unwrap();
        assert_eq!(ld.y, 1.0);
    }

    #[test]
    fn renderer_add_and_query() {
        let mut renderer = Renderer::new(1280, 720);
        renderer.init();
        let mesh_idx = renderer.add_mesh(sample_mesh());
        let shader = Shader::new("Default", "", "");
        let shader_idx = renderer.add_shader(shader);
        let mat = Material::new("Ground");
        let mat_idx = renderer.add_material(mat);
        assert_eq!(mesh_idx, 0);
        assert_eq!(shader_idx, 0);
        assert_eq!(mat_idx, 0);
    }

    #[test]
    fn renderer_submit_and_flush() {
        let mut renderer = Renderer::new(1280, 720);
        renderer.init();
        let _m = renderer.add_mesh(sample_mesh());
        let cmd = RenderCommand {
            mesh_index:     0,
            material_index: 0,
            transform:      Mat4::identity(),
            render_mode:    SceneRenderMode::Lit,
            cast_shadow:    false,
        };
        renderer.submit(cmd);
        assert_eq!(renderer.commands.len(), 1);
        renderer.flush();
        assert_eq!(renderer.commands.len(), 0);
    }
}
