//! novaforge_game — Game-layer Bevy plugin.
//!
//! Provides:
//!   - `GamePlugin`  — top-level plugin that wires everything together
//!   - `FlyCamera`   — fly-camera component and controller (WASD + mouse look)
//!   - `PlayerStats` — HP / Energy resource component
//!   - HUD           — egui overlay matching the C++ NovaForgeGame HUD

use std::f32::consts::PI;

use bevy::{
    input::mouse::MouseMotion,
    prelude::*,
    window::{CursorGrabMode, CursorOptions, PrimaryWindow},
};
use bevy_egui::{egui, EguiContexts, EguiPlugin};
use bevy_voxel_world::prelude::*;

use atlas_core::NF_VERSION_STRING;
use novaforge_world::{NovaForgeWorldConfig, WorldPlugin};

// ── Bevy 0.18 compatibility re-exports ──────────────────────────────────────
// In Bevy 0.18, EventReader/EventWriter were renamed to MessageReader/MessageWriter.
// MouseMotion is a Message; use MessageReader<MouseMotion> to read it.
use bevy::ecs::prelude::MessageReader;

// ── Player stats ──────────────────────────────────────────────────────────────

/// HP and energy state for the player, mirroring the C++ game defaults.
#[derive(Component)]
pub struct PlayerStats {
    pub hp: f32,
    pub max_hp: f32,
    pub energy: f32,
    pub max_energy: f32,
}

impl Default for PlayerStats {
    fn default() -> Self {
        // Match C++ defaults: g_hp=50, g_maxHp=100, g_energy=70, g_maxEnergy=100
        Self { hp: 50.0, max_hp: 100.0, energy: 70.0, max_energy: 100.0 }
    }
}

// ── Fly camera ────────────────────────────────────────────────────────────────

/// Marker + state for the free-flying FPS-style camera.
///
/// Click the window to capture the cursor (FPS mode).
/// Press Escape to release the cursor.
/// Controls: WASD to move, Space/LCtrl to ascend/descend, LShift to sprint.
#[derive(Component, Default)]
pub struct FlyCamera {
    /// Accumulated yaw in radians (left/right rotation).
    pub yaw: f32,
    /// Accumulated pitch in radians (up/down rotation), clamped to ±85°.
    pub pitch: f32,
}

// ── Top-level plugin ──────────────────────────────────────────────────────────

/// The top-level game plugin.  Add this to your `App` to get the full game.
///
/// Brings in:
///   - `WorldPlugin`  (bevy_voxel_world streaming terrain)
///   - `EguiPlugin`   (HUD overlay)
///   - Scene setup    (sun, ambient light)
///   - Player setup   (camera, stats)
///   - Camera controller, cursor grab, HUD rendering
pub struct GamePlugin;

impl Plugin for GamePlugin {
    fn build(&self, app: &mut App) {
        app.add_plugins(WorldPlugin)
            .add_plugins(EguiPlugin::default())
            .add_systems(Startup, (setup_scene, setup_player))
            .add_systems(
                Update,
                (
                    fly_camera,
                    cursor_grab,
                    render_hud,
                ),
            );
    }
}

// ── Scene setup ───────────────────────────────────────────────────────────────

fn setup_scene(mut commands: Commands) {
    // Directional sun light angled from the upper-left, matching C++ world background
    commands.spawn((
        DirectionalLight {
            color: Color::srgb(0.98, 0.95, 0.82),
            shadows_enabled: true,
            illuminance: 10_000.0,
            ..default()
        },
        Transform::from_rotation(Quat::from_euler(
            EulerRot::ZYX,
            0.0,
            -PI / 6.0,  // 30° west
            -PI / 4.0,  // 45° elevation
        )),
    ));

    // Global ambient sky light (blueish fill) — Resource in Bevy 0.18
    commands.insert_resource(GlobalAmbientLight {
        color: Color::srgb(0.55, 0.60, 0.75),
        brightness: 250.0,
        ..default()
    });

    tracing::info!(target: "Game", "Scene lighting configured");
}

// ── Player / camera setup ─────────────────────────────────────────────────────

fn setup_player(mut commands: Commands) {
    // Spawn the player entity: Camera3d + FlyCamera controller + PlayerStats +
    // VoxelWorldCamera so bevy_voxel_world knows to stream chunks around this camera.
    commands.spawn((
        Camera3d::default(),
        Transform::from_xyz(8.0, 22.0, 8.0)
            .looking_at(Vec3::new(24.0, 12.0, 24.0), Vec3::Y),
        FlyCamera::default(),
        PlayerStats::default(),
        VoxelWorldCamera::<NovaForgeWorldConfig>::default(),
    ));

    tracing::info!(target: "Game", "Player camera spawned at (8, 22, 8)");
}

// ── Fly camera controller ─────────────────────────────────────────────────────

fn fly_camera(
    time: Res<Time>,
    keys: Res<ButtonInput<KeyCode>>,
    mut mouse_motion: MessageReader<MouseMotion>,
    cursor: Query<&CursorOptions, With<PrimaryWindow>>,
    mut query: Query<(&mut Transform, &mut FlyCamera)>,
) {
    let Ok((mut transform, mut cam)) = query.single_mut() else { return };
    let dt = time.delta_secs();

    // Only look around when the cursor is captured (FPS mode)
    let grabbed = cursor
        .single()
        .ok()
        .map(|c| c.grab_mode != CursorGrabMode::None)
        .unwrap_or(false);

    if grabbed {
        for ev in mouse_motion.read() {
            cam.yaw   -= ev.delta.x * 0.002;
            cam.pitch -= ev.delta.y * 0.002;
            // Clamp pitch to ±85°
            cam.pitch = cam.pitch.clamp(-85.0_f32.to_radians(), 85.0_f32.to_radians());
        }
    } else {
        // Drain events even when not grabbed to avoid accumulation
        mouse_motion.clear();
    }

    // Apply yaw and pitch as YXZ Euler (yaw around world-Y, pitch around local-X)
    transform.rotation = Quat::from_euler(EulerRot::YXZ, cam.yaw, cam.pitch, 0.0);

    // Sprint multiplier
    let speed = (if keys.pressed(KeyCode::ShiftLeft) { 30.0 } else { 10.0 }) * dt;

    let forward = *transform.forward();
    let right   = *transform.right();

    if keys.pressed(KeyCode::KeyW) { transform.translation += forward * speed; }
    if keys.pressed(KeyCode::KeyS) { transform.translation -= forward * speed; }
    if keys.pressed(KeyCode::KeyA) { transform.translation -= right   * speed; }
    if keys.pressed(KeyCode::KeyD) { transform.translation += right   * speed; }
    if keys.pressed(KeyCode::Space)        { transform.translation.y += speed; }
    if keys.pressed(KeyCode::ControlLeft)  { transform.translation.y -= speed; }
}

// ── Cursor grab ───────────────────────────────────────────────────────────────

fn cursor_grab(
    mouse: Res<ButtonInput<MouseButton>>,
    keys: Res<ButtonInput<KeyCode>>,
    mut cursor: Query<&mut CursorOptions, With<PrimaryWindow>>,
) {
    let Ok(mut opts) = cursor.single_mut() else { return };

    if mouse.just_pressed(MouseButton::Left) && opts.grab_mode == CursorGrabMode::None {
        opts.grab_mode = CursorGrabMode::Locked;
        opts.visible   = false;
    }
    if keys.just_pressed(KeyCode::Escape) {
        opts.grab_mode = CursorGrabMode::None;
        opts.visible   = true;
    }
}

// ── HUD ───────────────────────────────────────────────────────────────────────

fn render_hud(
    mut contexts: EguiContexts,
    player_q: Query<&PlayerStats, With<FlyCamera>>,
    windows: Query<&Window, With<PrimaryWindow>>,
) {
    let Ok(stats) = player_q.single() else { return };
    let Ok(window) = windows.single() else { return };
    let Ok(ctx) = contexts.ctx_mut() else { return };

    let w = window.width();
    let h = window.height();

    // ── HP bar (bottom-left, matching C++ position fh-60) ─────────────────────
    egui::Area::new("nf_hp".into())
        .fixed_pos(egui::pos2(20.0, h - 62.0))
        .show(ctx, |ui| {
            ui.add(
                egui::ProgressBar::new(stats.hp / stats.max_hp)
                    .text(format!("HP  {:.0}/{:.0}", stats.hp, stats.max_hp))
                    .fill(egui::Color32::from_rgb(180, 30, 30))
                    .desired_width(200.0),
            );
        });

    // ── Energy bar (bottom-left, matching C++ position fh-38) ─────────────────
    egui::Area::new("nf_en".into())
        .fixed_pos(egui::pos2(20.0, h - 40.0))
        .show(ctx, |ui| {
            ui.add(
                egui::ProgressBar::new(stats.energy / stats.max_energy)
                    .text(format!("EN  {:.0}/{:.0}", stats.energy, stats.max_energy))
                    .fill(egui::Color32::from_rgb(30, 80, 200))
                    .desired_width(200.0),
            );
        });

    // ── Crosshair (screen center) ─────────────────────────────────────────────
    egui::Area::new("nf_crosshair".into())
        .fixed_pos(egui::pos2(w / 2.0 - 8.0, h / 2.0 - 10.0))
        .show(ctx, |ui| {
            ui.label(
                egui::RichText::new("+")
                    .size(22.0)
                    .color(egui::Color32::WHITE)
                    .strong(),
            );
        });

    // ── Minimap placeholder (top-right, matching C++ mmSize=120) ─────────────
    egui::Area::new("nf_minimap".into())
        .fixed_pos(egui::pos2(w - 140.0, 12.0))
        .show(ctx, |ui| {
            egui::Frame::new()
                .fill(egui::Color32::from_rgb(26, 26, 26))
                .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(85, 85, 85)))
                .inner_margin(egui::Margin::same(4))
                .show(ui, |ui| {
                    ui.set_min_size(egui::vec2(120.0, 120.0));
                    ui.colored_label(egui::Color32::GRAY, "Map");
                });
        });

    // ── Version watermark (bottom-right, matching C++ style) ─────────────────
    let version = format!("NovaForge v{NF_VERSION_STRING}");
    egui::Area::new("nf_version".into())
        .fixed_pos(egui::pos2(w - 170.0, h - 22.0))
        .show(ctx, |ui| {
            ui.colored_label(egui::Color32::DARK_GRAY, &version);
        });

    // ── Controls hint (shown only when cursor is NOT grabbed) ─────────────────
    egui::Area::new("nf_hint".into())
        .fixed_pos(egui::pos2(w / 2.0 - 120.0, h - 30.0))
        .show(ctx, |ui| {
            ui.colored_label(
                egui::Color32::from_rgba_premultiplied(200, 200, 200, 180),
                "Click to capture cursor  |  Escape to release  |  WASD+Mouse to fly",
            );
        });
}
