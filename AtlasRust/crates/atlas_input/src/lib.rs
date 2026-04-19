//! atlas_input — Keyboard, mouse, gamepad, and action-mapping input system.
//!
//! Mirrors the C++ NF::Input system: KeyCode, MouseState, GamepadState,
//! InputState, ActionBinding, and InputSystem.

use serde::{Deserialize, Serialize};
use std::collections::{HashMap, HashSet};

// ── KeyCode ───────────────────────────────────────────────────────────────────

/// All physical key codes — matches the C++ NF::KeyCode enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[repr(u16)]
pub enum KeyCode {
    Unknown = 0,
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Digits
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Navigation
    Left, Right, Up, Down,
    // Special
    Space, Escape, Enter, Tab, Backspace, Delete,
    LShift, RShift, LCtrl, RCtrl, LAlt, RAlt,
    // Mouse (as keys)
    Mouse1, Mouse2, Mouse3, Mouse4, Mouse5,
}

// ── Mouse / Gamepad state ─────────────────────────────────────────────────────

/// Mouse position and delta for one frame.
#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct MouseState {
    pub x:            f32,
    pub y:            f32,
    pub delta_x:      f32,
    pub delta_y:      f32,
    pub scroll_delta: f32,
}

/// Gamepad axis/button state.
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct GamepadState {
    pub connected:      bool,
    pub left_stick_x:   f32,
    pub left_stick_y:   f32,
    pub right_stick_x:  f32,
    pub right_stick_y:  f32,
    pub left_trigger:   f32,
    pub right_trigger:  f32,
    pub buttons:        [bool; 16],
}

// ── Action system ─────────────────────────────────────────────────────────────

/// Which keys are pressed, the mouse state, and gamepad state for one frame.
#[derive(Debug, Clone, Default)]
pub struct InputState {
    pub pressed:     HashSet<KeyCode>,
    pub mouse:       MouseState,
    pub gamepad:     GamepadState,
    pub text_input:  String,
}

/// Binds an action name to a key with optional modifier requirements.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ActionBinding {
    pub action_name: String,
    pub key:         KeyCode,
    pub ctrl:        bool,
    pub shift:       bool,
    pub alt:         bool,
}

/// Per-frame phase of an action event.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ActionPhase {
    Pressed,
    Held,
    Released,
}

/// An action event emitted by the InputSystem each update.
#[derive(Debug, Clone)]
pub struct ActionEvent {
    pub action_name: String,
    pub phase:       ActionPhase,
    pub value:       f32,
}

// ── InputSystem ───────────────────────────────────────────────────────────────

/// Central input manager — collects raw events and derives action events.
pub struct InputSystem {
    state:          InputState,
    prev_pressed:   HashSet<KeyCode>,
    bindings:       Vec<ActionBinding>,
    action_events:  Vec<ActionEvent>,
}

impl InputSystem {
    pub fn new() -> Self {
        Self {
            state:         InputState::default(),
            prev_pressed:  HashSet::new(),
            bindings:      Vec::new(),
            action_events: Vec::new(),
        }
    }

    pub fn init(&self) {
        tracing::info!(target: "Input", "Input system initialized");
    }

    pub fn shutdown(&mut self) {
        self.bindings.clear();
        self.action_events.clear();
        tracing::info!(target: "Input", "Input system shutdown");
    }

    // ── Raw key injection ─────────────────────────────────────────────────────

    pub fn set_key_down(&mut self, key: KeyCode) {
        self.state.pressed.insert(key);
    }

    pub fn set_key_up(&mut self, key: KeyCode) {
        self.state.pressed.remove(&key);
    }

    pub fn is_key_down(&self, key: KeyCode) -> bool {
        self.state.pressed.contains(&key)
    }

    // ── Mouse ─────────────────────────────────────────────────────────────────

    pub fn set_mouse_position(&mut self, x: f32, y: f32) {
        self.state.mouse.delta_x = x - self.state.mouse.x;
        self.state.mouse.delta_y = y - self.state.mouse.y;
        self.state.mouse.x = x;
        self.state.mouse.y = y;
    }

    pub fn set_scroll_delta(&mut self, delta: f32) {
        self.state.mouse.scroll_delta = delta;
    }

    pub fn append_text_input(&mut self, c: char) {
        self.state.text_input.push(c);
    }

    // ── Gamepad ───────────────────────────────────────────────────────────────

    pub fn set_gamepad_connected(&mut self, connected: bool) {
        self.state.gamepad.connected = connected;
    }

    pub fn set_gamepad_axes(&mut self, lx: f32, ly: f32, rx: f32, ry: f32) {
        self.state.gamepad.left_stick_x  = lx;
        self.state.gamepad.left_stick_y  = ly;
        self.state.gamepad.right_stick_x = rx;
        self.state.gamepad.right_stick_y = ry;
    }

    pub fn set_gamepad_triggers(&mut self, left: f32, right: f32) {
        self.state.gamepad.left_trigger  = left;
        self.state.gamepad.right_trigger = right;
    }

    // ── Action bindings ───────────────────────────────────────────────────────

    pub fn bind_action(&mut self, binding: ActionBinding) {
        self.bindings.push(binding);
    }

    pub fn clear_bindings(&mut self) { self.bindings.clear(); }

    // ── Per-frame update ──────────────────────────────────────────────────────

    pub fn update(&mut self) {
        self.state.text_input.clear();
        self.action_events.clear();

        for binding in &self.bindings {
            let is_down  = self.state.pressed.contains(&binding.key);
            let was_down = self.prev_pressed.contains(&binding.key);

            // Check modifier requirements
            if binding.ctrl  && !self.state.pressed.contains(&KeyCode::LCtrl)  && !self.state.pressed.contains(&KeyCode::RCtrl)  { continue; }
            if binding.shift && !self.state.pressed.contains(&KeyCode::LShift) && !self.state.pressed.contains(&KeyCode::RShift) { continue; }
            if binding.alt   && !self.state.pressed.contains(&KeyCode::LAlt)   && !self.state.pressed.contains(&KeyCode::RAlt)   { continue; }

            let phase = match (is_down, was_down) {
                (true,  false) => Some(ActionPhase::Pressed),
                (true,  true)  => Some(ActionPhase::Held),
                (false, true)  => Some(ActionPhase::Released),
                _              => None,
            };

            if let Some(phase) = phase {
                let value = if phase == ActionPhase::Released { 0.0 } else { 1.0 };
                self.action_events.push(ActionEvent {
                    action_name: binding.action_name.clone(),
                    phase,
                    value,
                });
            }
        }

        // Snapshot current key state
        self.prev_pressed = self.state.pressed.clone();
        self.state.mouse.scroll_delta = 0.0;
    }

    pub fn action_events(&self) -> &[ActionEvent] { &self.action_events }

    pub fn is_action_active(&self, action_name: &str) -> bool {
        self.action_events.iter().any(|e| {
            e.action_name == action_name
                && (e.phase == ActionPhase::Pressed || e.phase == ActionPhase::Held)
        })
    }

    pub fn state(&self) -> &InputState { &self.state }
}

impl Default for InputSystem {
    fn default() -> Self { Self::new() }
}
