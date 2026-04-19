//! atlas_animation — Skeletal animation, clips, channels, and state machine.
//!
//! Mirrors the C++ NF::Animation system: Skeleton, AnimationClip, AnimationChannel,
//! AnimationStateMachine, and AnimationSystem.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

// ── Math primitives (re-exported from atlas_core via inline types) ─────────────

/// 3-component float vector.
#[derive(Debug, Clone, Copy, PartialEq, Default, Serialize, Deserialize)]
pub struct Vec3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

impl Vec3 {
    pub const ZERO: Self = Self { x: 0.0, y: 0.0, z: 0.0 };
    pub const ONE:  Self = Self { x: 1.0, y: 1.0, z: 1.0 };

    pub fn new(x: f32, y: f32, z: f32) -> Self { Self { x, y, z } }

    pub fn lerp(a: Self, b: Self, t: f32) -> Self {
        Self {
            x: a.x + (b.x - a.x) * t,
            y: a.y + (b.y - a.y) * t,
            z: a.z + (b.z - a.z) * t,
        }
    }
}

/// Quaternion for rotation.
#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub struct Quat {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}

impl Default for Quat {
    fn default() -> Self { Self { x: 0.0, y: 0.0, z: 0.0, w: 1.0 } }
}

impl Quat {
    pub fn identity() -> Self { Self::default() }

    pub fn slerp(a: Self, b: Self, t: f32) -> Self {
        let mut dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        let b = if dot < 0.0 {
            dot = -dot;
            Quat { x: -b.x, y: -b.y, z: -b.z, w: -b.w }
        } else {
            b
        };

        if dot > 0.9995 {
            // Nearly identical — linear interpolation
            let r = Quat {
                x: a.x + t * (b.x - a.x),
                y: a.y + t * (b.y - a.y),
                z: a.z + t * (b.z - a.z),
                w: a.w + t * (b.w - a.w),
            };
            r.normalized()
        } else {
            let theta_0 = dot.acos();
            let theta = theta_0 * t;
            let sin_theta = theta.sin();
            let sin_theta_0 = theta_0.sin();
            let s1 = theta.cos() - dot * sin_theta / sin_theta_0;
            let s2 = sin_theta / sin_theta_0;
            Quat {
                x: a.x * s1 + b.x * s2,
                y: a.y * s1 + b.y * s2,
                z: a.z * s1 + b.z * s2,
                w: a.w * s1 + b.w * s2,
            }
        }
    }

    fn normalized(self) -> Self {
        let len = (self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w).sqrt();
        if len < 1e-8 { return Self::identity(); }
        Self { x: self.x / len, y: self.y / len, z: self.z / len, w: self.w / len }
    }
}

// ── Transform ─────────────────────────────────────────────────────────────────

/// Position, rotation, scale transform (mirrors C++ NF::Transform).
#[derive(Debug, Clone, Copy, PartialEq, Default, Serialize, Deserialize)]
pub struct Transform {
    pub position: Vec3,
    pub rotation: Quat,
    pub scale:    Vec3,
}

impl Transform {
    pub fn identity() -> Self {
        Self {
            position: Vec3::ZERO,
            rotation: Quat::identity(),
            scale:    Vec3::ONE,
        }
    }
}

// ── Bone ──────────────────────────────────────────────────────────────────────

/// A single bone in a skeleton hierarchy.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Bone {
    pub name:          String,
    /// -1 means root bone.
    pub parent_index:  i32,
    pub local_bind_pose: Transform,
}

// ── Skeleton ──────────────────────────────────────────────────────────────────

/// A named collection of bones with parent-child relationships.
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct Skeleton {
    name:       String,
    bones:      Vec<Bone>,
    bone_names: HashMap<String, i32>,
}

impl Skeleton {
    pub fn new(name: impl Into<String>) -> Self {
        Self { name: name.into(), ..Default::default() }
    }

    /// Add a bone; returns its index.
    pub fn add_bone(&mut self, name: impl Into<String>, parent_index: i32, bind_pose: Transform) -> i32 {
        let index = self.bones.len() as i32;
        let name = name.into();
        self.bone_names.insert(name.clone(), index);
        self.bones.push(Bone { name, parent_index, local_bind_pose: bind_pose });
        index
    }

    pub fn find_bone(&self, name: &str) -> Option<i32> {
        self.bone_names.get(name).copied()
    }

    pub fn bone(&self, index: i32) -> Option<&Bone> {
        self.bones.get(index as usize)
    }

    pub fn bones(&self) -> &[Bone] { &self.bones }
    pub fn bone_count(&self) -> usize { self.bones.len() }
    pub fn name(&self) -> &str { &self.name }
}

// ── Keyframe / Channel ────────────────────────────────────────────────────────

/// A single keyframe pairing a time stamp with a transform value.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TransformKey {
    pub time:  f32,
    pub value: Transform,
}

/// Per-bone keyframe track in an animation clip.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnimationChannel {
    pub bone_index: i32,
    pub keys:       Vec<TransformKey>,
}

impl AnimationChannel {
    /// Sample the channel at `time`, linearly interpolating between keyframes.
    pub fn sample(&self, time: f32) -> Transform {
        if self.keys.is_empty() { return Transform::identity(); }
        if self.keys.len() == 1 || time <= self.keys[0].time {
            return self.keys[0].value;
        }
        if time >= self.keys.last().unwrap().time {
            return self.keys.last().unwrap().value;
        }
        for i in 0..self.keys.len() - 1 {
            let k0 = &self.keys[i];
            let k1 = &self.keys[i + 1];
            if time >= k0.time && time < k1.time {
                let t = (time - k0.time) / (k1.time - k0.time);
                return Transform {
                    position: Vec3::lerp(k0.value.position, k1.value.position, t),
                    rotation: Quat::slerp(k0.value.rotation, k1.value.rotation, t),
                    scale:    Vec3::lerp(k0.value.scale, k1.value.scale, t),
                };
            }
        }
        self.keys.last().unwrap().value
    }
}

// ── AnimationClip ─────────────────────────────────────────────────────────────

/// A named, timed collection of bone channels — mirrors C++ NF::AnimationClip.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnimationClip {
    name:     String,
    duration: f32,
    channels: Vec<AnimationChannel>,
}

impl AnimationClip {
    pub fn new(name: impl Into<String>, duration: f32) -> Self {
        Self { name: name.into(), duration, channels: Vec::new() }
    }

    pub fn add_channel(&mut self, bone_index: i32) -> &mut AnimationChannel {
        self.channels.push(AnimationChannel { bone_index, keys: Vec::new() });
        self.channels.last_mut().unwrap()
    }

    /// Write sampled transforms for each channel into `out_pose`.
    pub fn sample(&self, time: f32, out_pose: &mut Vec<Transform>) {
        for ch in &self.channels {
            let idx = ch.bone_index as usize;
            if idx < out_pose.len() {
                out_pose[idx] = ch.sample(time);
            }
        }
    }

    pub fn name(&self) -> &str { &self.name }
    pub fn duration(&self) -> f32 { self.duration }
    pub fn channels(&self) -> &[AnimationChannel] { &self.channels }
    pub fn channel_count(&self) -> usize { self.channels.len() }
}

// ── AnimationStateMachine ─────────────────────────────────────────────────────

/// A state machine node holding a clip reference and transition list.
struct StateNode {
    clip:     Option<usize>,   // index into clip registry
    looping:  bool,
    speed:    f32,
    transitions: Vec<(String, f32)>, // (target_state, blend_duration)
}

/// Simple animation state machine — mirrors C++ NF::AnimationStateMachine.
pub struct AnimationStateMachine {
    states:        HashMap<String, StateNode>,
    clips:         Vec<AnimationClip>,
    current_state: String,
    state_time:    f32,
}

impl AnimationStateMachine {
    pub fn new() -> Self {
        Self {
            states: HashMap::new(),
            clips: Vec::new(),
            current_state: String::new(),
            state_time: 0.0,
        }
    }

    /// Register a clip and return its index.
    pub fn add_clip(&mut self, clip: AnimationClip) -> usize {
        let idx = self.clips.len();
        self.clips.push(clip);
        idx
    }

    pub fn add_state(&mut self, name: impl Into<String>, clip_index: Option<usize>, looping: bool, speed: f32) {
        let name = name.into();
        if self.current_state.is_empty() {
            self.current_state = name.clone();
        }
        self.states.insert(name, StateNode { clip: clip_index, looping, speed, transitions: Vec::new() });
    }

    pub fn add_transition(&mut self, from: &str, to: impl Into<String>, blend_duration: f32) {
        if let Some(node) = self.states.get_mut(from) {
            node.transitions.push((to.into(), blend_duration));
        }
    }

    /// Advance the state machine by `dt` seconds; optionally transition states.
    pub fn update(&mut self, dt: f32) {
        let current = self.current_state.clone();
        if let Some(node) = self.states.get(&current) {
            self.state_time += dt * node.speed;
            if let Some(clip_idx) = node.clip {
                let dur = self.clips[clip_idx].duration();
                if self.state_time >= dur {
                    if node.looping {
                        self.state_time = 0.0;
                    } else {
                        self.state_time = dur;
                    }
                }
            }
        }
    }

    /// Manually transition to another state.
    pub fn transition_to(&mut self, state: impl Into<String>) {
        self.current_state = state.into();
        self.state_time = 0.0;
    }

    pub fn current_state(&self) -> &str { &self.current_state }
    pub fn state_time(&self) -> f32 { self.state_time }

    pub fn sample_current_pose(&self, out_pose: &mut Vec<Transform>) {
        if let Some(node) = self.states.get(&self.current_state) {
            if let Some(clip_idx) = node.clip {
                self.clips[clip_idx].sample(self.state_time, out_pose);
            }
        }
    }
}

impl Default for AnimationStateMachine {
    fn default() -> Self { Self::new() }
}

// ── AnimationSystem ───────────────────────────────────────────────────────────

/// High-level animation system — creates and manages animation instances.
pub struct AnimationSystem {
    machines: Vec<AnimationStateMachine>,
}

impl AnimationSystem {
    pub fn new() -> Self { Self { machines: Vec::new() } }

    pub fn init(&self) {
        tracing::info!(target: "Animation", "Animation system initialized");
    }

    pub fn shutdown(&mut self) {
        self.machines.clear();
        tracing::info!(target: "Animation", "Animation system shutdown");
    }

    pub fn create_machine(&mut self) -> usize {
        let idx = self.machines.len();
        self.machines.push(AnimationStateMachine::new());
        idx
    }

    pub fn machine(&self, idx: usize) -> Option<&AnimationStateMachine> {
        self.machines.get(idx)
    }

    pub fn machine_mut(&mut self, idx: usize) -> Option<&mut AnimationStateMachine> {
        self.machines.get_mut(idx)
    }

    pub fn update_all(&mut self, dt: f32) {
        for m in &mut self.machines {
            m.update(dt);
        }
    }
}

impl Default for AnimationSystem {
    fn default() -> Self { Self::new() }
}
