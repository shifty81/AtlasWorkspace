//! atlas_workspace — AtlasWorkspace host-app logic: project lifecycle, tool
//! registry, and workspace shell.
//!
//! Mirrors the C++ NF::Workspace / ProjectSerializer system.

use serde::{Deserialize, Serialize};
use serde_json as json;
use std::{
    collections::HashMap,
    fs,
    path::{Path, PathBuf},
};

use atlas_pipeline::{ChangeEvent, ChangeEventType, Manifest, PipelineWatcher, WatchLog};

// ── Project file ──────────────────────────────────────────────────────────────

/// Version stamp stored in every project file.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct ProjectFileVersion {
    pub major: u16,
    pub minor: u16,
}

impl ProjectFileVersion {
    pub const fn current() -> Self { Self { major: 1, minor: 0 } }
}

/// A key-value section inside a project file.
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct ProjectSection(pub HashMap<String, String>);

impl ProjectSection {
    pub fn set(&mut self, key: impl Into<String>, value: impl Into<String>) {
        self.0.insert(key.into(), value.into());
    }

    pub fn get(&self, key: &str) -> Option<&str> {
        self.0.get(key).map(String::as_str)
    }
}

/// Serialisable representation of a workspace project.
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct WorkspaceProjectFile {
    pub project_id:   String,
    pub project_name: String,
    pub content_root: String,
    pub version:      Option<ProjectFileVersion>,
    sections:         HashMap<String, ProjectSection>,
}

impl WorkspaceProjectFile {
    pub fn new(id: impl Into<String>, name: impl Into<String>) -> Self {
        Self {
            project_id:   id.into(),
            project_name: name.into(),
            ..Default::default()
        }
    }

    pub fn load(path: &Path) -> Option<Self> {
        let text = fs::read_to_string(path).ok()?;
        serde_json::from_str(&text).ok()
    }

    pub fn save(&self, path: &Path) -> std::io::Result<()> {
        if let Some(parent) = path.parent() { fs::create_dir_all(parent)?; }
        let text = serde_json::to_string_pretty(self)?;
        fs::write(path, text)
    }

    pub fn set_project_id(&mut self, id: impl Into<String>) { self.project_id = id.into(); }
    pub fn set_project_name(&mut self, name: impl Into<String>) { self.project_name = name.into(); }
    pub fn set_content_root(&mut self, root: impl Into<String>) { self.content_root = root.into(); }
    pub fn set_version(&mut self, v: ProjectFileVersion) { self.version = Some(v); }

    pub fn section(&mut self, name: &str) -> &mut ProjectSection {
        self.sections.entry(name.to_string()).or_default()
    }

    pub fn get_section(&self, name: &str) -> Option<&ProjectSection> {
        self.sections.get(name)
    }
}

// ── WorkspaceShellSnapshot ────────────────────────────────────────────────────

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct WorkspaceShellSnapshot {
    pub project_id:         String,
    pub project_name:       String,
    pub content_root:       String,
    pub active_tool_id:     String,
    pub registered_tool_ids: Vec<String>,
    pub visible_panel_ids:  Vec<String>,
    pub file_version:       ProjectFileVersion,
}

impl WorkspaceShellSnapshot {
    pub fn is_valid(&self) -> bool {
        !self.project_id.is_empty() && !self.project_name.is_empty()
    }
}

impl ProjectFileVersion {
    fn default_version() -> Self { Self::current() }
}

impl Default for ProjectFileVersion {
    fn default() -> Self { Self::current() }
}

// ── ProjectSerializer ─────────────────────────────────────────────────────────

pub struct ProjectSerializer;

impl ProjectSerializer {
    pub const CORE_SECTION:   &'static str = "Core";
    pub const TOOLS_SECTION:  &'static str = "Tools";
    pub const PANELS_SECTION: &'static str = "Panels";

    pub fn serialize(snap: &WorkspaceShellSnapshot, file: &mut WorkspaceProjectFile) -> Result<(), String> {
        if !snap.is_valid() {
            return Err("snapshot is not valid (empty projectId or projectName)".into());
        }
        file.set_project_id(&snap.project_id);
        file.set_project_name(&snap.project_name);
        file.set_content_root(&snap.content_root);
        file.set_version(snap.file_version);

        {
            let core = file.section(Self::CORE_SECTION);
            core.set("projectId",   &snap.project_id);
            core.set("projectName", &snap.project_name);
            core.set("contentRoot", &snap.content_root);
            core.set("activeTool",  &snap.active_tool_id);
        }
        {
            let tools = file.section(Self::TOOLS_SECTION);
            tools.set("count", snap.registered_tool_ids.len().to_string());
            for (i, id) in snap.registered_tool_ids.iter().enumerate() {
                tools.set(format!("tool.{i}"), id);
            }
        }
        {
            let panels = file.section(Self::PANELS_SECTION);
            panels.set("count", snap.visible_panel_ids.len().to_string());
            for (i, id) in snap.visible_panel_ids.iter().enumerate() {
                panels.set(format!("panel.{i}"), id);
            }
        }
        Ok(())
    }

    pub fn deserialize(file: &WorkspaceProjectFile) -> Result<WorkspaceShellSnapshot, String> {
        let mut snap = WorkspaceShellSnapshot {
            project_id:   file.project_id.clone(),
            project_name: file.project_name.clone(),
            content_root: file.content_root.clone(),
            file_version: file.version.unwrap_or_default(),
            ..Default::default()
        };

        if let Some(core) = file.get_section(Self::CORE_SECTION) {
            snap.active_tool_id = core.get("activeTool").unwrap_or_default().to_string();
        }

        if let Some(tools) = file.get_section(Self::TOOLS_SECTION) {
            let count: usize = tools.get("count").and_then(|s| s.parse().ok()).unwrap_or(0);
            for i in 0..count {
                if let Some(id) = tools.get(&format!("tool.{i}")) {
                    snap.registered_tool_ids.push(id.to_string());
                }
            }
        }

        if let Some(panels) = file.get_section(Self::PANELS_SECTION) {
            let count: usize = panels.get("count").and_then(|s| s.parse().ok()).unwrap_or(0);
            for i in 0..count {
                if let Some(id) = panels.get(&format!("panel.{i}")) {
                    snap.visible_panel_ids.push(id.to_string());
                }
            }
        }

        if !snap.is_valid() {
            return Err("deserialized snapshot is not valid".into());
        }
        Ok(snap)
    }
}

// ── WorkspaceTool ─────────────────────────────────────────────────────────────

/// A tool registered in the workspace shell.
#[derive(Debug, Clone)]
pub struct WorkspaceTool {
    pub id:           String,
    pub display_name: String,
    pub icon:         String,
    pub active:       bool,
}

// ── WorkspaceShell ────────────────────────────────────────────────────────────

/// The top-level workspace host — manages the project, tools, and pipeline.
pub struct WorkspaceShell {
    pub project_file: WorkspaceProjectFile,
    pub manifest:     Manifest,
    tools:            Vec<WorkspaceTool>,
    active_tool:      Option<String>,
    workspace_root:   PathBuf,
}

impl WorkspaceShell {
    pub fn new(root: impl Into<PathBuf>) -> Self {
        Self {
            project_file: WorkspaceProjectFile::default(),
            manifest:     Manifest::new(),
            tools:        Vec::new(),
            active_tool:  None,
            workspace_root: root.into(),
        }
    }

    pub fn init(&self) {
        tracing::info!(target: "Workspace", "AtlasWorkspace shell initialized at {:?}", self.workspace_root);
    }

    pub fn load_project(&mut self, path: &Path) -> bool {
        if let Some(pf) = WorkspaceProjectFile::load(path) {
            self.project_file = pf;
            tracing::info!(target: "Workspace", "Loaded project: {}", self.project_file.project_name);
            true
        } else {
            tracing::warn!(target: "Workspace", "Failed to load project from {:?}", path);
            false
        }
    }

    pub fn save_project(&self, path: &Path) -> bool {
        self.project_file.save(path).is_ok()
    }

    pub fn register_tool(&mut self, tool: WorkspaceTool) {
        tracing::info!(target: "Workspace", "Registered tool: {}", tool.id);
        self.tools.push(tool);
    }

    pub fn activate_tool(&mut self, id: &str) -> bool {
        if self.tools.iter().any(|t| t.id == id) {
            self.active_tool = Some(id.to_string());
            tracing::info!(target: "Workspace", "Activated tool: {}", id);
            true
        } else {
            false
        }
    }

    pub fn active_tool(&self) -> Option<&str> { self.active_tool.as_deref() }
    pub fn tools(&self) -> &[WorkspaceTool] { &self.tools }
    pub fn workspace_root(&self) -> &Path { &self.workspace_root }

    pub fn snapshot(&self) -> WorkspaceShellSnapshot {
        WorkspaceShellSnapshot {
            project_id:          self.project_file.project_id.clone(),
            project_name:        self.project_file.project_name.clone(),
            content_root:        self.project_file.content_root.clone(),
            active_tool_id:      self.active_tool.clone().unwrap_or_default(),
            registered_tool_ids: self.tools.iter().map(|t| t.id.clone()).collect(),
            visible_panel_ids:   Vec::new(),
            file_version:        ProjectFileVersion::current(),
        }
    }
}
