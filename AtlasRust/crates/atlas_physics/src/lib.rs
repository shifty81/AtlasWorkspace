//! atlas_physics — AABB, Ray, RigidBody, collision detection, and PhysicsWorld.
//!
//! Mirrors the C++ NF::Physics system. Pure-Rust integration layer; heavier
//! physics simulation (bevy_rapier3d) is wired in at the app level.

use serde::{Deserialize, Serialize};

// ── Vec3 (local alias, no Bevy dep) ──────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Default, Serialize, Deserialize)]
pub struct Vec3 { pub x: f32, pub y: f32, pub z: f32 }

impl Vec3 {
    pub fn new(x: f32, y: f32, z: f32) -> Self { Self { x, y, z } }
    pub fn length_sq(self) -> f32 { self.x * self.x + self.y * self.y + self.z * self.z }
    pub fn length(self) -> f32 { self.length_sq().sqrt() }
    pub fn dot(self, b: Self) -> f32 { self.x * b.x + self.y * b.y + self.z * b.z }
}

impl std::ops::Add for Vec3 {
    type Output = Self;
    fn add(self, b: Self) -> Self { Self::new(self.x + b.x, self.y + b.y, self.z + b.z) }
}
impl std::ops::Sub for Vec3 {
    type Output = Self;
    fn sub(self, b: Self) -> Self { Self::new(self.x - b.x, self.y - b.y, self.z - b.z) }
}
impl std::ops::Mul<f32> for Vec3 {
    type Output = Self;
    fn mul(self, s: f32) -> Self { Self::new(self.x * s, self.y * s, self.z * s) }
}

// ── AABB ──────────────────────────────────────────────────────────────────────

/// Axis-aligned bounding box.
#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct Aabb {
    pub min: Vec3,
    pub max: Vec3,
}

impl Aabb {
    pub fn new(min: Vec3, max: Vec3) -> Self { Self { min, max } }

    pub fn intersects(self, other: Self) -> bool {
        self.min.x <= other.max.x && self.max.x >= other.min.x &&
        self.min.y <= other.max.y && self.max.y >= other.min.y &&
        self.min.z <= other.max.z && self.max.z >= other.min.z
    }

    pub fn contains(self, p: Vec3) -> bool {
        p.x >= self.min.x && p.x <= self.max.x &&
        p.y >= self.min.y && p.y <= self.max.y &&
        p.z >= self.min.z && p.z <= self.max.z
    }

    pub fn center(self) -> Vec3 {
        Vec3::new(
            (self.min.x + self.max.x) * 0.5,
            (self.min.y + self.max.y) * 0.5,
            (self.min.z + self.max.z) * 0.5,
        )
    }

    pub fn extents(self) -> Vec3 {
        Vec3::new(
            (self.max.x - self.min.x) * 0.5,
            (self.max.y - self.min.y) * 0.5,
            (self.max.z - self.min.z) * 0.5,
        )
    }

    pub fn merged(self, other: Self) -> Self {
        Self {
            min: Vec3::new(self.min.x.min(other.min.x), self.min.y.min(other.min.y), self.min.z.min(other.min.z)),
            max: Vec3::new(self.max.x.max(other.max.x), self.max.y.max(other.max.y), self.max.z.max(other.max.z)),
        }
    }
}

// ── Ray ───────────────────────────────────────────────────────────────────────

/// A ray with an origin and a (unit) direction.
#[derive(Debug, Clone, Copy)]
pub struct Ray {
    pub origin:    Vec3,
    pub direction: Vec3,
}

/// Result of a ray-AABB intersection test.
#[derive(Debug, Clone, Copy)]
pub struct RayHit {
    pub distance: f32,
    pub point:    Vec3,
    pub normal:   Vec3,
}

/// Test a ray against an AABB using the slab method.
pub fn ray_intersect_aabb(ray: Ray, aabb: Aabb) -> Option<RayHit> {
    let mut tmin = f32::NEG_INFINITY;
    let mut tmax = f32::INFINITY;
    let mut hit_normal = Vec3::default();

    let axes = [
        (ray.origin.x, ray.direction.x, aabb.min.x, aabb.max.x, Vec3::new(-1.0, 0.0, 0.0), Vec3::new(1.0, 0.0, 0.0)),
        (ray.origin.y, ray.direction.y, aabb.min.y, aabb.max.y, Vec3::new(0.0, -1.0, 0.0), Vec3::new(0.0, 1.0, 0.0)),
        (ray.origin.z, ray.direction.z, aabb.min.z, aabb.max.z, Vec3::new(0.0, 0.0, -1.0), Vec3::new(0.0, 0.0, 1.0)),
    ];

    for (orig, dir, bmin, bmax, n_low, n_high) in axes {
        if dir.abs() < 1e-8 {
            if orig < bmin || orig > bmax { return None; }
            continue;
        }
        let mut t1 = (bmin - orig) / dir;
        let mut t2 = (bmax - orig) / dir;
        let mut n1 = n_low;
        if t1 > t2 { std::mem::swap(&mut t1, &mut t2); n1 = n_high; }
        if t1 > tmin { tmin = t1; hit_normal = n1; }
        if t2 < tmax { tmax = t2; }
        if tmin > tmax { return None; }
    }

    if tmax < 0.0 { return None; }
    let t = if tmin >= 0.0 { tmin } else { tmax };
    let point = ray.origin + ray.direction * t;
    Some(RayHit { distance: t, point, normal: hit_normal })
}

// ── Sphere ────────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct Sphere { pub center: Vec3, pub radius: f32 }

impl Sphere {
    pub fn new(center: Vec3, radius: f32) -> Self { Self { center, radius } }
    pub fn contains(self, p: Vec3) -> bool { (p - self.center).length_sq() <= self.radius * self.radius }
    pub fn intersects(self, other: Self) -> bool { (self.center - other.center).length() <= self.radius + other.radius }
}

// ── RigidBody ─────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
pub enum BodyType { #[default] Dynamic, Static, Kinematic }

/// A simple Euler-integrated rigid body.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RigidBody {
    pub body_type:   BodyType,
    pub mass:        f32,
    pub restitution: f32,
    pub friction:    f32,
    pub velocity:    Vec3,
    pub acceleration: Vec3,
    pub force:       Vec3,
    pub use_gravity: bool,
}

impl Default for RigidBody {
    fn default() -> Self {
        Self {
            body_type:    BodyType::Dynamic,
            mass:         1.0,
            restitution:  0.3,
            friction:     0.5,
            velocity:     Vec3::default(),
            acceleration: Vec3::default(),
            force:        Vec3::default(),
            use_gravity:  true,
        }
    }
}

impl RigidBody {
    pub fn inverse_mass(&self) -> f32 {
        if self.body_type == BodyType::Static || self.mass <= 0.0 { 0.0 } else { 1.0 / self.mass }
    }

    pub fn apply_force(&mut self, f: Vec3) {
        self.force = self.force + f;
    }

    pub fn integrate(&mut self, dt: f32) {
        if self.body_type == BodyType::Static { return; }
        self.acceleration = self.force * self.inverse_mass();
        self.velocity = self.velocity + self.acceleration * dt;
        self.force = Vec3::default();
    }
}

// ── CollisionShape ────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum CollisionShape {
    Box(Aabb),
    Sphere(Sphere),
}

impl CollisionShape {
    pub fn intersects(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::Box(a), Self::Box(b)) => a.intersects(*b),
            (Self::Sphere(a), Self::Sphere(b)) => a.intersects(*b),
            (Self::Box(a), Self::Sphere(b)) | (Self::Sphere(b), Self::Box(a)) => {
                // AABB-sphere overlap: clamp sphere centre to AABB then check distance
                let cx = b.center.x.clamp(a.min.x, a.max.x);
                let cy = b.center.y.clamp(a.min.y, a.max.y);
                let cz = b.center.z.clamp(a.min.z, a.max.z);
                let closest = Vec3::new(cx, cy, cz);
                (b.center - closest).length_sq() <= b.radius * b.radius
            }
        }
    }
}

// ── PhysicsBody (scene object) ────────────────────────────────────────────────

pub struct PhysicsBody {
    pub rigid_body: RigidBody,
    pub shape:      CollisionShape,
    pub position:   Vec3,
}

// ── PhysicsWorld ─────────────────────────────────────────────────────────────

/// Simple integrated physics world — step, gravity, and collision detection.
pub struct PhysicsWorld {
    bodies:  Vec<PhysicsBody>,
    gravity: Vec3,
}

impl PhysicsWorld {
    pub fn new() -> Self {
        Self {
            bodies:  Vec::new(),
            gravity: Vec3::new(0.0, -9.81, 0.0),
        }
    }

    pub fn init(&self) {
        tracing::info!(target: "Physics", "Physics world initialized");
    }

    pub fn add_body(&mut self, rigid_body: RigidBody, shape: CollisionShape, position: Vec3) -> usize {
        let idx = self.bodies.len();
        self.bodies.push(PhysicsBody { rigid_body, shape, position });
        idx
    }

    pub fn set_gravity(&mut self, g: Vec3) { self.gravity = g; }
    pub fn gravity(&self) -> Vec3 { self.gravity }

    pub fn body_position(&self, idx: usize) -> Vec3 {
        self.bodies[idx].position
    }

    pub fn step(&mut self, dt: f32) {
        for body in &mut self.bodies {
            if body.rigid_body.use_gravity && body.rigid_body.body_type == BodyType::Dynamic {
                body.rigid_body.apply_force(self.gravity * body.rigid_body.mass);
            }
            body.rigid_body.integrate(dt);
            if body.rigid_body.body_type != BodyType::Static {
                body.position = body.position + body.rigid_body.velocity * dt;
            }
        }
    }

    pub fn body(&self, idx: usize) -> Option<&PhysicsBody> { self.bodies.get(idx) }
    pub fn body_mut(&mut self, idx: usize) -> Option<&mut PhysicsBody> { self.bodies.get_mut(idx) }
    pub fn body_count(&self) -> usize { self.bodies.len() }

    /// Returns indices of all overlapping body pairs.
    pub fn detect_collisions(&self) -> Vec<(usize, usize)> {
        let mut pairs = Vec::new();
        for i in 0..self.bodies.len() {
            for j in i + 1..self.bodies.len() {
                if self.bodies[i].shape.intersects(&self.bodies[j].shape) {
                    pairs.push((i, j));
                }
            }
        }
        pairs
    }

    /// Cast a ray and return (body_index, RayHit) for the first hit.
    pub fn raycast(&self, ray: Ray) -> Option<(usize, RayHit)> {
        let mut best: Option<(usize, RayHit)> = None;
        for (i, body) in self.bodies.iter().enumerate() {
            let hit = match &body.shape {
                CollisionShape::Box(aabb) => {
                    let world_aabb = Aabb {
                        min: body.position + aabb.min,
                        max: body.position + aabb.max,
                    };
                    ray_intersect_aabb(ray, world_aabb)
                }
                CollisionShape::Sphere(s) => {
                    let world_sphere = Sphere::new(body.position + s.center, s.radius);
                    // Ray-sphere intersection
                    let oc = ray.origin - world_sphere.center;
                    let a  = ray.direction.dot(ray.direction);
                    let b  = 2.0 * oc.dot(ray.direction);
                    let c  = oc.dot(oc) - world_sphere.radius * world_sphere.radius;
                    let disc = b * b - 4.0 * a * c;
                    if disc < 0.0 { None } else {
                        let t = (-b - disc.sqrt()) / (2.0 * a);
                        if t < 0.0 { None } else {
                            let point = ray.origin + ray.direction * t;
                            let normal = (point - world_sphere.center) * (1.0 / world_sphere.radius);
                            Some(RayHit { distance: t, point, normal })
                        }
                    }
                }
            };
            if let Some(h) = hit {
                if best.map_or(true, |(_, bh)| h.distance < bh.distance) {
                    best = Some((i, h));
                }
            }
        }
        best
    }
}

impl Default for PhysicsWorld {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    // ── Vec3 ──────────────────────────────────────────────────────────────────

    #[test]
    fn vec3_dot_product() {
        let a = Vec3::new(1.0, 0.0, 0.0);
        let b = Vec3::new(0.0, 1.0, 0.0);
        assert_eq!(a.dot(b), 0.0, "orthogonal vectors have zero dot product");

        let c = Vec3::new(2.0, 3.0, 4.0);
        let d = Vec3::new(1.0, 1.0, 1.0);
        assert_eq!(c.dot(d), 9.0);
    }

    #[test]
    fn vec3_length() {
        let v = Vec3::new(3.0, 4.0, 0.0);
        assert!((v.length() - 5.0).abs() < 1e-5);
    }

    #[test]
    fn vec3_add_sub_mul() {
        let a = Vec3::new(1.0, 2.0, 3.0);
        let b = Vec3::new(4.0, 5.0, 6.0);
        let sum = a + b;
        assert_eq!(sum.x, 5.0);
        assert_eq!(sum.y, 7.0);
        assert_eq!(sum.z, 9.0);

        let diff = b - a;
        assert_eq!(diff.x, 3.0);

        let scaled = a * 2.0;
        assert_eq!(scaled.z, 6.0);
    }

    // ── Aabb ─────────────────────────────────────────────────────────────────

    #[test]
    fn aabb_contains_center() {
        let aabb = Aabb::new(Vec3::new(-1.0, -1.0, -1.0), Vec3::new(1.0, 1.0, 1.0));
        assert!(aabb.contains(Vec3::new(0.0, 0.0, 0.0)), "AABB must contain its center");
    }

    #[test]
    fn aabb_does_not_contain_outside_point() {
        let aabb = Aabb::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(1.0, 1.0, 1.0));
        assert!(!aabb.contains(Vec3::new(2.0, 0.5, 0.5)));
    }

    #[test]
    fn aabb_intersects_overlapping_boxes() {
        let a = Aabb::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(2.0, 2.0, 2.0));
        let b = Aabb::new(Vec3::new(1.0, 1.0, 1.0), Vec3::new(3.0, 3.0, 3.0));
        assert!(a.intersects(b), "overlapping boxes must intersect");
    }

    #[test]
    fn aabb_does_not_intersect_separated_boxes() {
        let a = Aabb::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(1.0, 1.0, 1.0));
        let b = Aabb::new(Vec3::new(2.0, 2.0, 2.0), Vec3::new(3.0, 3.0, 3.0));
        assert!(!a.intersects(b), "separated boxes must not intersect");
    }

    #[test]
    fn aabb_center() {
        let aabb = Aabb::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(4.0, 4.0, 4.0));
        let c = aabb.center();
        assert!((c.x - 2.0).abs() < 1e-5);
        assert!((c.y - 2.0).abs() < 1e-5);
    }

    // ── Ray ──────────────────────────────────────────────────────────────────

    #[test]
    fn ray_hits_unit_aabb() {
        let ray = Ray {
            origin: Vec3::new(-5.0, 0.5, 0.5),
            direction: Vec3::new(1.0, 0.0, 0.0),
        };
        let aabb = Aabb::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(1.0, 1.0, 1.0));
        let hit = ray_intersect_aabb(ray, aabb);
        assert!(hit.is_some(), "ray aligned with +X must hit the box");
        let h = hit.unwrap();
        assert!((h.distance - 5.0).abs() < 1e-3, "hit distance should be 5 units");
    }

    #[test]
    fn ray_misses_aabb() {
        let ray = Ray {
            origin: Vec3::new(-5.0, 5.0, 0.5),
            direction: Vec3::new(1.0, 0.0, 0.0),
        };
        let aabb = Aabb::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(1.0, 1.0, 1.0));
        assert!(ray_intersect_aabb(ray, aabb).is_none(), "ray above box must miss");
    }

    // ── RigidBody / PhysicsWorld ───────────────────────────────────────────

    #[test]
    fn physics_world_step_moves_dynamic_body() {
        let mut world = PhysicsWorld::new();
        world.init();
        let body = RigidBody {
            body_type:   BodyType::Dynamic,
            mass:        1.0,
            use_gravity: true,
            ..Default::default()
        };
        let shape = CollisionShape::Box(Aabb::new(
            Vec3::new(-0.5, -0.5, -0.5),
            Vec3::new(0.5, 0.5, 0.5),
        ));
        let start = Vec3::new(0.0, 10.0, 0.0);
        let idx = world.add_body(body, shape, start);
        let y_before = world.body_position(idx).y;
        world.step(1.0);
        let y_after = world.body_position(idx).y;
        assert!(y_after < y_before, "dynamic body should fall under gravity");
    }

    #[test]
    fn physics_world_static_body_does_not_move() {
        let mut world = PhysicsWorld::new();
        let body = RigidBody {
            body_type: BodyType::Static,
            velocity:  Vec3::new(1.0, 1.0, 1.0),
            mass:      1.0,
            ..Default::default()
        };
        let shape = CollisionShape::Box(Aabb::new(
            Vec3::new(-0.5, -0.5, -0.5),
            Vec3::new(0.5, 0.5, 0.5),
        ));
        let start = Vec3::new(0.0, 0.0, 0.0);
        let idx = world.add_body(body, shape, start);
        world.step(1.0);
        assert_eq!(world.body_position(idx).y, 0.0, "static body must not move");
    }

    #[test]
    fn physics_world_raycast_hits_body() {
        let mut world = PhysicsWorld::new();
        let body = RigidBody::default();
        let shape = CollisionShape::Box(Aabb::new(
            Vec3::new(-1.0, -1.0, -1.0),
            Vec3::new(1.0, 1.0, 1.0),
        ));
        world.add_body(body, shape, Vec3::new(0.0, 0.0, 0.0));
        let ray = Ray {
            origin:    Vec3::new(-10.0, 0.0, 0.0),
            direction: Vec3::new(1.0, 0.0, 0.0),
        };
        assert!(world.raycast(ray).is_some(), "raycast should hit body");
    }
}
