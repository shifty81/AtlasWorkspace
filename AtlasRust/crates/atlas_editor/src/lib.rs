//! atlas_editor — Editor panels, inspector, diagnostics, and tool registry.
//!
//! Mirrors the C++ NF::Editor system. Actual egui rendering is at the app level.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

use atlas_ui::{FocusService, NotificationHost, PanelDescriptor, PanelRegistry, Theme};
use atlas_workspace::{WorkspaceShell, WorkspaceTool};

// ── Editor panel IDs (canonical) ──────────────────────────────────────────────

pub const PANEL_HIERARCHY:        &str = "Hierarchy";
pub const PANEL_INSPECTOR:        &str = "Inspector";
pub const PANEL_CONTENT_BROWSER:  &str = "ContentBrowser";
pub const PANEL_VIEWPORT:         &str = "Viewport";
pub const PANEL_CONSOLE:          &str = "Console";
pub const PANEL_PIPELINE_MONITOR: &str = "PipelineMonitor";
pub const PANEL_GRAPH_EDITOR:     &str = "GraphEditor";
pub const PANEL_IDE:              &str = "IDE";

// ── Inspector property ────────────────────────────────────────────────────────

/// A single displayed property in the inspector panel.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum PropertyValue {
    Bool(bool),
    Int(i64),
    Float(f64),
    String(String),
    Vec3([f32; 3]),
    Color([u8; 4]),
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InspectorProperty {
    pub name:     String,
    pub value:    PropertyValue,
    pub readonly: bool,
    pub category: String,
}

/// Inspector panel state — shows properties of the selected entity/asset.
#[derive(Debug, Clone, Default)]
pub struct InspectorPanel {
    pub selected_id:  Option<u64>,
    pub title:        String,
    pub properties:   Vec<InspectorProperty>,
}

impl InspectorPanel {
    pub fn new() -> Self { Self::default() }

    pub fn select(&mut self, id: u64, title: impl Into<String>) {
        self.selected_id = Some(id);
        self.title = title.into();
        self.properties.clear();
    }

    pub fn deselect(&mut self) {
        self.selected_id = None;
        self.title.clear();
        self.properties.clear();
    }

    pub fn add_property(&mut self, prop: InspectorProperty) {
        self.properties.push(prop);
    }

    pub fn set_bool(&mut self, name: &str, value: bool, category: &str) {
        self.add_property(InspectorProperty {
            name: name.to_string(),
            value: PropertyValue::Bool(value),
            readonly: false,
            category: category.to_string(),
        });
    }

    pub fn set_float(&mut self, name: &str, value: f64, category: &str) {
        self.add_property(InspectorProperty {
            name: name.to_string(),
            value: PropertyValue::Float(value),
            readonly: false,
            category: category.to_string(),
        });
    }

    pub fn set_string(&mut self, name: &str, value: &str, category: &str) {
        self.add_property(InspectorProperty {
            name: name.to_string(),
            value: PropertyValue::String(value.to_string()),
            readonly: false,
            category: category.to_string(),
        });
    }
}

// ── Console panel ─────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum LogLevel { Info, Warn, Error, Debug }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ConsoleEntry {
    pub level:   LogLevel,
    pub message: String,
    pub source:  String,
}

/// In-editor console output panel.
#[derive(Debug, Clone, Default)]
pub struct ConsolePanel {
    entries:     Vec<ConsoleEntry>,
    max_entries: usize,
}

impl ConsolePanel {
    pub fn new(max_entries: usize) -> Self {
        Self { entries: Vec::new(), max_entries: max_entries.max(1) }
    }

    pub fn push(&mut self, level: LogLevel, message: impl Into<String>, source: impl Into<String>) {
        if self.entries.len() >= self.max_entries {
            self.entries.remove(0);
        }
        self.entries.push(ConsoleEntry { level, message: message.into(), source: source.into() });
    }

    pub fn clear(&mut self) { self.entries.clear(); }
    pub fn entries(&self) -> &[ConsoleEntry] { &self.entries }
    pub fn count(&self) -> usize { self.entries.len() }
}

// ── Diagnostics / Profiler ────────────────────────────────────────────────────

/// A single profiler timing sample.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ProfileSample {
    pub name:       String,
    pub duration_ms: f64,
    pub depth:      u32,
}

/// Collects per-frame profiling samples.
#[derive(Debug, Clone, Default)]
pub struct DiagnosticsProfiler {
    pub samples:     Vec<ProfileSample>,
    pub frame_time:  f64,
    pub fps:         f64,
}

impl DiagnosticsProfiler {
    pub fn begin_frame(&mut self, dt_secs: f64) {
        self.samples.clear();
        self.frame_time = dt_secs * 1000.0;
        self.fps = if dt_secs > 0.0 { 1.0 / dt_secs } else { 0.0 };
    }

    pub fn push_sample(&mut self, name: impl Into<String>, duration_ms: f64, depth: u32) {
        self.samples.push(ProfileSample { name: name.into(), duration_ms, depth });
    }
}

// ── Undo / Redo ───────────────────────────────────────────────────────────────

/// Reversible editor operation.
pub trait UndoableCommand: Send + Sync {
    fn execute(&mut self);
    fn undo(&mut self);
    fn description(&self) -> &str;
}

/// Undo/redo stack.
pub struct UndoStack {
    undo: Vec<Box<dyn UndoableCommand>>,
    redo: Vec<Box<dyn UndoableCommand>>,
    max_depth: usize,
}

impl UndoStack {
    pub fn new(max_depth: usize) -> Self {
        Self { undo: Vec::new(), redo: Vec::new(), max_depth }
    }

    pub fn execute(&mut self, mut cmd: Box<dyn UndoableCommand>) {
        cmd.execute();
        self.undo.push(cmd);
        self.redo.clear();
        while self.undo.len() > self.max_depth { self.undo.remove(0); }
    }

    pub fn undo(&mut self) -> bool {
        if let Some(mut cmd) = self.undo.pop() {
            cmd.undo();
            self.redo.push(cmd);
            true
        } else { false }
    }

    pub fn redo(&mut self) -> bool {
        if let Some(mut cmd) = self.redo.pop() {
            cmd.execute();
            self.undo.push(cmd);
            true
        } else { false }
    }

    pub fn can_undo(&self) -> bool { !self.undo.is_empty() }
    pub fn can_redo(&self) -> bool { !self.redo.is_empty() }
    pub fn undo_description(&self) -> Option<&str> { self.undo.last().map(|c| c.description()) }
    pub fn redo_description(&self) -> Option<&str> { self.redo.last().map(|c| c.description()) }
    pub fn clear(&mut self) { self.undo.clear(); self.redo.clear(); }
}

impl Default for UndoStack {
    fn default() -> Self { Self::new(100) }
}

// ── Editor context ────────────────────────────────────────────────────────────

/// Top-level editor state — owns all panels and services.
pub struct EditorContext {
    pub panels:       PanelRegistry,
    pub inspector:    InspectorPanel,
    pub console:      ConsolePanel,
    pub diagnostics:  DiagnosticsProfiler,
    pub theme:        Theme,
    pub focus:        FocusService,
    pub notifications: NotificationHost,
    pub undo:         UndoStack,
}

impl EditorContext {
    pub fn new() -> Self {
        let mut panels = PanelRegistry::default();
        for (id, title) in [
            (PANEL_HIERARCHY,        "Hierarchy"),
            (PANEL_INSPECTOR,        "Inspector"),
            (PANEL_CONTENT_BROWSER,  "Content Browser"),
            (PANEL_VIEWPORT,         "Viewport"),
            (PANEL_CONSOLE,          "Console"),
            (PANEL_PIPELINE_MONITOR, "Pipeline Monitor"),
            (PANEL_GRAPH_EDITOR,     "Graph Editor"),
            (PANEL_IDE,              "IDE"),
        ] {
            panels.register(PanelDescriptor {
                id: id.to_string(), title: title.to_string(), visible: true, closable: true,
            });
        }

        Self {
            panels,
            inspector:     InspectorPanel::new(),
            console:       ConsolePanel::new(1000),
            diagnostics:   DiagnosticsProfiler::default(),
            theme:         Theme::dark(),
            focus:         FocusService::default(),
            notifications: NotificationHost::default(),
            undo:          UndoStack::default(),
        }
    }

    pub fn init(&self) {
        tracing::info!(target: "Editor", "Editor context initialized ({} panels)", self.panels.panels().len());
    }
}

impl Default for EditorContext {
    fn default() -> Self { Self::new() }
}
