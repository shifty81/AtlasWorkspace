//! atlas_ui — Shared egui widget primitives, theme tokens, and command layer.
//!
//! Mirrors the C++ AtlasUI layer: theme, commands, focus/popup/tooltip services,
//! and widget style tokens. Real egui rendering is handled by the app via
//! bevy_egui; this crate provides the types that all panels share.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

// ── Theme tokens ──────────────────────────────────────────────────────────────

/// RGBA colour (0–255 per channel).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct Color {
    pub r: u8, pub g: u8, pub b: u8, pub a: u8,
}

impl Color {
    pub const fn rgba(r: u8, g: u8, b: u8, a: u8) -> Self { Self { r, g, b, a } }
    pub const fn rgb(r: u8, g: u8, b: u8) -> Self { Self::rgba(r, g, b, 255) }
    pub const WHITE:        Self = Self::rgb(255, 255, 255);
    pub const BLACK:        Self = Self::rgb(0, 0, 0);
    pub const TRANSPARENT:  Self = Self::rgba(0, 0, 0, 0);
}

/// All visual style tokens for the Atlas theme system.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ThemeTokens {
    pub background:        Color,
    pub surface:           Color,
    pub surface_variant:   Color,
    pub primary:           Color,
    pub primary_variant:   Color,
    pub on_primary:        Color,
    pub error:             Color,
    pub warning:           Color,
    pub success:           Color,
    pub text_primary:      Color,
    pub text_secondary:    Color,
    pub text_disabled:     Color,
    pub border:            Color,
    pub separator:         Color,
    pub font_size_sm:      f32,
    pub font_size_md:      f32,
    pub font_size_lg:      f32,
    pub border_radius:     f32,
    pub spacing_xs:        f32,
    pub spacing_sm:        f32,
    pub spacing_md:        f32,
    pub spacing_lg:        f32,
}

impl Default for ThemeTokens {
    fn default() -> Self {
        Self {
            background:       Color::rgb(18, 18, 18),
            surface:          Color::rgb(26, 26, 26),
            surface_variant:  Color::rgb(36, 36, 36),
            primary:          Color::rgb(65, 120, 210),
            primary_variant:  Color::rgb(50, 95, 175),
            on_primary:       Color::WHITE,
            error:            Color::rgb(200, 50, 50),
            warning:          Color::rgb(210, 160, 30),
            success:          Color::rgb(50, 180, 80),
            text_primary:     Color::rgb(220, 220, 220),
            text_secondary:   Color::rgb(160, 160, 160),
            text_disabled:    Color::rgb(90, 90, 90),
            border:           Color::rgb(55, 55, 55),
            separator:        Color::rgb(45, 45, 45),
            font_size_sm:     11.0,
            font_size_md:     13.0,
            font_size_lg:     16.0,
            border_radius:    4.0,
            spacing_xs:       2.0,
            spacing_sm:       4.0,
            spacing_md:       8.0,
            spacing_lg:       16.0,
        }
    }
}

/// Named theme variant.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
pub enum ThemeVariant { #[default] Dark, Light, HighContrast }

/// Active workspace theme — holds the token set and the current variant.
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct Theme {
    pub variant: ThemeVariant,
    pub tokens:  ThemeTokens,
}

impl Theme {
    pub fn dark() -> Self {
        Self { variant: ThemeVariant::Dark, tokens: ThemeTokens::default() }
    }

    pub fn light() -> Self {
        let mut tokens = ThemeTokens::default();
        tokens.background       = Color::rgb(240, 240, 240);
        tokens.surface          = Color::rgb(255, 255, 255);
        tokens.surface_variant  = Color::rgb(225, 225, 225);
        tokens.text_primary     = Color::rgb(20, 20, 20);
        tokens.text_secondary   = Color::rgb(80, 80, 80);
        tokens.border           = Color::rgb(200, 200, 200);
        Self { variant: ThemeVariant::Light, tokens }
    }
}

// ── Command system ────────────────────────────────────────────────────────────

/// Unique command identifier string.
pub type CommandId = String;

/// A registered UI command with display name and keyboard shortcut.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CommandDescriptor {
    pub id:          CommandId,
    pub display_name: String,
    pub shortcut:    String,   // e.g. "Ctrl+Z"
    pub category:   String,
}

/// Central command registry — maps command IDs to descriptors.
#[derive(Debug, Clone, Default)]
pub struct CommandRegistry {
    commands: HashMap<CommandId, CommandDescriptor>,
}

impl CommandRegistry {
    pub fn new() -> Self { Self::default() }

    pub fn register(&mut self, desc: CommandDescriptor) {
        self.commands.insert(desc.id.clone(), desc);
    }

    pub fn find(&self, id: &str) -> Option<&CommandDescriptor> {
        self.commands.get(id)
    }

    pub fn all(&self) -> impl Iterator<Item = &CommandDescriptor> {
        self.commands.values()
    }

    pub fn count(&self) -> usize { self.commands.len() }
}

// ── Focus service ─────────────────────────────────────────────────────────────

/// Tracks which panel or widget currently holds keyboard focus.
#[derive(Debug, Clone, Default)]
pub struct FocusService {
    focused_panel: Option<String>,
}

impl FocusService {
    pub fn request_focus(&mut self, panel_id: impl Into<String>) {
        self.focused_panel = Some(panel_id.into());
    }

    pub fn release_focus(&mut self) { self.focused_panel = None; }

    pub fn focused_panel(&self) -> Option<&str> {
        self.focused_panel.as_deref()
    }

    pub fn has_focus(&self, panel_id: &str) -> bool {
        self.focused_panel.as_deref() == Some(panel_id)
    }
}

// ── Notification system ───────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum NotificationLevel { Info, Warning, Error, Success }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Notification {
    pub id:      u64,
    pub level:   NotificationLevel,
    pub message: String,
    pub timeout: f32,    // seconds until auto-dismiss, 0 = persistent
}

/// Notification host — collects and expires notifications.
#[derive(Debug, Clone, Default)]
pub struct NotificationHost {
    notifications: Vec<Notification>,
    next_id:       u64,
}

impl NotificationHost {
    pub fn push(&mut self, level: NotificationLevel, message: impl Into<String>, timeout: f32) -> u64 {
        let id = self.next_id;
        self.next_id += 1;
        self.notifications.push(Notification { id, level, message: message.into(), timeout });
        id
    }

    pub fn dismiss(&mut self, id: u64) {
        self.notifications.retain(|n| n.id != id);
    }

    /// Tick down timeouts and remove expired notifications.
    pub fn tick(&mut self, dt: f32) {
        for n in &mut self.notifications {
            if n.timeout > 0.0 { n.timeout -= dt; }
        }
        self.notifications.retain(|n| n.timeout > 0.0 || n.timeout == 0.0 && {
            // Persistent (timeout == 0) → keep
            true
        });
        // Actually remove only expired ones (timeout went negative)
        self.notifications.retain(|n| n.timeout >= 0.0);
    }

    pub fn notifications(&self) -> &[Notification] { &self.notifications }
    pub fn count(&self) -> usize { self.notifications.len() }
}

// ── Tooltip service ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Default)]
pub struct TooltipService {
    active: Option<String>,
    delay:  f32,
    timer:  f32,
}

impl TooltipService {
    pub fn hover(&mut self, text: impl Into<String>, delay: f32) {
        let t = text.into();
        if self.active.as_deref() != Some(&t) {
            self.active = Some(t);
            self.delay = delay;
            self.timer = 0.0;
        }
    }

    pub fn clear(&mut self) { self.active = None; self.timer = 0.0; }

    pub fn tick(&mut self, dt: f32) {
        if self.active.is_some() { self.timer += dt; }
    }

    pub fn visible_text(&self) -> Option<&str> {
        if self.timer >= self.delay { self.active.as_deref() } else { None }
    }
}

// ── Panel registry ────────────────────────────────────────────────────────────

/// Describes a dockable panel in the editor.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PanelDescriptor {
    pub id:       String,
    pub title:    String,
    pub visible:  bool,
    pub closable: bool,
}

/// Registry of all panels known to the workspace.
#[derive(Debug, Clone, Default)]
pub struct PanelRegistry {
    panels: Vec<PanelDescriptor>,
}

impl PanelRegistry {
    pub fn register(&mut self, desc: PanelDescriptor) {
        self.panels.push(desc);
    }

    pub fn find(&self, id: &str) -> Option<&PanelDescriptor> {
        self.panels.iter().find(|p| p.id == id)
    }

    pub fn find_mut(&mut self, id: &str) -> Option<&mut PanelDescriptor> {
        self.panels.iter_mut().find(|p| p.id == id)
    }

    pub fn set_visible(&mut self, id: &str, visible: bool) {
        if let Some(p) = self.find_mut(id) { p.visible = visible; }
    }

    pub fn panels(&self) -> &[PanelDescriptor] { &self.panels }
    pub fn visible_panels(&self) -> impl Iterator<Item = &PanelDescriptor> {
        self.panels.iter().filter(|p| p.visible)
    }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    // ── Color ────────────────────────────────────────────────────────────────

    #[test]
    fn color_from_rgb() {
        let c = Color::rgb(255, 128, 0);
        assert_eq!(c.r, 255);
        assert_eq!(c.g, 128);
        assert_eq!(c.b, 0);
        assert_eq!(c.a, 255);
    }

    #[test]
    fn color_from_rgba() {
        let c = Color::rgba(10, 20, 30, 128);
        assert_eq!(c.a, 128);
    }

    // ── Theme ────────────────────────────────────────────────────────────────

    #[test]
    fn dark_theme_background_is_dark() {
        let dark = Theme::dark();
        // Background colour for dark theme should have low luminance
        let bg = dark.tokens.background;
        assert!(bg.r < 80 && bg.g < 80 && bg.b < 80, "dark theme background must be dark");
    }

    #[test]
    fn light_theme_background_is_light() {
        let light = Theme::light();
        let bg = light.tokens.background;
        assert!(bg.r > 180 && bg.g > 180, "light theme background must be light");
    }

    // ── CommandRegistry ───────────────────────────────────────────────────────

    #[test]
    fn command_registry_register_and_find() {
        let mut reg = CommandRegistry::new();
        reg.register(CommandDescriptor {
            id:           "file.save".into(),
            display_name: "Save".into(),
            shortcut:     "Ctrl+S".into(),
            category:     "File".into(),
        });
        let cmd = reg.find("file.save").expect("must find registered command");
        assert_eq!(cmd.display_name, "Save");
        assert!(reg.find("nonexistent").is_none());
    }

    #[test]
    fn command_registry_count() {
        let mut reg = CommandRegistry::new();
        assert_eq!(reg.count(), 0);
        for i in 0..5 {
            reg.register(CommandDescriptor {
                id:           format!("cmd.{i}"),
                display_name: format!("Cmd {i}"),
                shortcut:     String::new(),
                category:     "Test".into(),
            });
        }
        assert_eq!(reg.count(), 5);
    }

    // ── FocusService ─────────────────────────────────────────────────────────

    #[test]
    fn focus_service_request_and_release() {
        let mut svc = FocusService::default();
        svc.request_focus("Inspector");
        assert_eq!(svc.focused_panel(), Some("Inspector"));
        assert!(svc.has_focus("Inspector"));
        assert!(!svc.has_focus("Console"));
        svc.release_focus();
        assert!(svc.focused_panel().is_none());
    }

    #[test]
    fn focus_service_steal_focus() {
        let mut svc = FocusService::default();
        svc.request_focus("PanelA");
        svc.request_focus("PanelB");
        assert_eq!(svc.focused_panel(), Some("PanelB"), "latest request wins");
    }

    // ── NotificationHost ──────────────────────────────────────────────────────

    #[test]
    fn notification_host_push_and_expire() {
        let mut host = NotificationHost::default();
        host.push(NotificationLevel::Info, "Hello", 2.0);
        assert_eq!(host.count(), 1);
        host.tick(3.0);   // advance past the timeout
        assert_eq!(host.count(), 0, "notification must expire after timeout");
    }

    #[test]
    fn notification_host_dismiss() {
        let mut host = NotificationHost::default();
        let id = host.push(NotificationLevel::Warning, "Warn", 60.0);
        assert_eq!(host.count(), 1);
        host.dismiss(id);
        assert_eq!(host.count(), 0);
    }

    // ── TooltipService ────────────────────────────────────────────────────────

    #[test]
    fn tooltip_appears_after_delay() {
        let mut svc = TooltipService::default();
        svc.hover("Hello tooltip", 0.5);
        svc.tick(0.3);
        assert!(svc.visible_text().is_none(), "tooltip must not show before delay");
        svc.tick(0.3);
        assert_eq!(svc.visible_text(), Some("Hello tooltip"));
    }

    #[test]
    fn tooltip_clears() {
        let mut svc = TooltipService::default();
        svc.hover("Test", 0.0);
        svc.tick(0.1);
        svc.clear();
        assert!(svc.visible_text().is_none());
    }

    // ── PanelRegistry ─────────────────────────────────────────────────────────

    #[test]
    fn panel_registry_register_and_find() {
        let mut reg = PanelRegistry::default();
        reg.register(PanelDescriptor {
            id:       "Inspector".into(),
            title:    "Inspector".into(),
            visible:  true,
            closable: true,
        });
        assert!(reg.find("Inspector").is_some());
        assert!(reg.find("Missing").is_none());
    }

    #[test]
    fn panel_registry_set_visible() {
        let mut reg = PanelRegistry::default();
        reg.register(PanelDescriptor {
            id: "Console".into(), title: "Console".into(), visible: true, closable: true,
        });
        reg.set_visible("Console", false);
        let count: usize = reg.visible_panels().count();
        assert_eq!(count, 0, "hidden panel must not appear in visible_panels");
    }
}
