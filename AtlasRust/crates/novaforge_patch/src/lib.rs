//! novaforge_patch — Patch and update system for NovaForge.
//!
//! Handles content patching, versioned file manifests, and delta-update
//! verification for the game client and server.

use serde::{Deserialize, Serialize};
use serde_json as json;
use std::{
    collections::HashMap,
    fs,
    io::{Read, Write},
    path::{Path, PathBuf},
    time::{SystemTime, UNIX_EPOCH},
};

// ── PatchVersion ──────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize)]
pub struct PatchVersion {
    pub major: u16,
    pub minor: u16,
    pub patch: u16,
}

impl PatchVersion {
    pub const fn new(major: u16, minor: u16, patch: u16) -> Self { Self { major, minor, patch } }
    pub const ZERO: Self = Self::new(0, 0, 0);
}

impl std::fmt::Display for PatchVersion {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}.{}.{}", self.major, self.minor, self.patch)
    }
}

// ── PatchEntry ────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum PatchOp {
    /// Replace the file at `path` with the supplied content hash.
    Replace { checksum: String },
    /// Delete the file at `path`.
    Delete,
    /// Create a new file (same as Replace for new paths).
    Create { checksum: String },
}

/// A single file operation inside a patch.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PatchEntry {
    pub path:      String,
    pub operation: PatchOp,
    pub size:      u64,
}

// ── PatchManifest ─────────────────────────────────────────────────────────────

/// Full description of a patch from `from_version` to `to_version`.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PatchManifest {
    pub from_version: PatchVersion,
    pub to_version:   PatchVersion,
    pub description:  String,
    pub created_at:   i64,
    pub entries:      Vec<PatchEntry>,
    pub total_size:   u64,
}

impl PatchManifest {
    pub fn new(from: PatchVersion, to: PatchVersion, description: impl Into<String>) -> Self {
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|d| d.as_secs() as i64)
            .unwrap_or(0);
        Self {
            from_version: from,
            to_version:   to,
            description:  description.into(),
            created_at:   now,
            entries:      Vec::new(),
            total_size:   0,
        }
    }

    pub fn add_entry(&mut self, entry: PatchEntry) {
        self.total_size += entry.size;
        self.entries.push(entry);
    }

    pub fn save(&self, path: &Path) -> std::io::Result<()> {
        if let Some(parent) = path.parent() { fs::create_dir_all(parent)?; }
        let text = serde_json::to_string_pretty(self)?;
        fs::write(path, text)
    }

    pub fn load(path: &Path) -> Option<Self> {
        let text = fs::read_to_string(path).ok()?;
        serde_json::from_str(&text).ok()
    }

    pub fn entry_count(&self) -> usize { self.entries.len() }
}

// ── FileRecord ────────────────────────────────────────────────────────────────

/// A record in the installed-file database.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FileRecord {
    pub path:     String,
    pub checksum: String,
    pub size:     u64,
}

// ── InstalledManifest ─────────────────────────────────────────────────────────

/// Tracks the currently installed version and its file set.
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct InstalledManifest {
    pub version: Option<PatchVersion>,
    pub files:   Vec<FileRecord>,
    index:       HashMap<String, usize>,
}

impl InstalledManifest {
    pub fn new() -> Self { Self::default() }

    pub fn load(path: &Path) -> Option<Self> {
        let text = fs::read_to_string(path).ok()?;
        let mut m: InstalledManifest = serde_json::from_str(&text).ok()?;
        m.rebuild_index();
        Some(m)
    }

    pub fn save(&self, path: &Path) -> std::io::Result<()> {
        if let Some(parent) = path.parent() { fs::create_dir_all(parent)?; }
        let text = serde_json::to_string_pretty(self)?;
        fs::write(path, text)
    }

    pub fn register_file(&mut self, record: FileRecord) {
        if let Some(&idx) = self.index.get(&record.path) {
            self.files[idx] = record;
        } else {
            let idx = self.files.len();
            self.index.insert(record.path.clone(), idx);
            self.files.push(record);
        }
    }

    pub fn remove_file(&mut self, path: &str) {
        if let Some(&idx) = self.index.get(path) {
            self.files.remove(idx);
            self.rebuild_index();
        }
    }

    pub fn find_file(&self, path: &str) -> Option<&FileRecord> {
        self.index.get(path).and_then(|&i| self.files.get(i))
    }

    fn rebuild_index(&mut self) {
        self.index.clear();
        for (i, f) in self.files.iter().enumerate() {
            self.index.insert(f.path.clone(), i);
        }
    }
}

// ── PatchVerifier ─────────────────────────────────────────────────────────────

/// Verifies patch integrity — compares on-disk files against the patch manifest.
pub struct PatchVerifier {
    pub install_root: PathBuf,
}

impl PatchVerifier {
    pub fn new(install_root: impl Into<PathBuf>) -> Self {
        Self { install_root: install_root.into() }
    }

    /// Simple byte-length checksum for verification (a real impl would use SHA-256).
    fn checksum_file(path: &Path) -> Option<String> {
        let data = fs::read(path).ok()?;
        // Cheap checksum: XOR all bytes, format as hex
        let hash: u64 = data.iter().enumerate().fold(0u64, |acc, (i, &b)| {
            acc ^ ((b as u64) << (i % 56))
        });
        Some(format!("{hash:016x}"))
    }

    /// Returns a list of (path, issue) pairs for any integrity failures.
    pub fn verify(&self, manifest: &PatchManifest) -> Vec<(String, String)> {
        let mut issues = Vec::new();
        for entry in &manifest.entries {
            let abs_path = self.install_root.join(&entry.path);
            match &entry.operation {
                PatchOp::Delete => {
                    if abs_path.exists() {
                        issues.push((entry.path.clone(), "file should be deleted but exists".into()));
                    }
                }
                PatchOp::Replace { checksum } | PatchOp::Create { checksum } => {
                    if !abs_path.exists() {
                        issues.push((entry.path.clone(), "expected file missing".into()));
                        continue;
                    }
                    if !checksum.is_empty() {
                        if let Some(actual) = Self::checksum_file(&abs_path) {
                            if &actual != checksum {
                                issues.push((entry.path.clone(), format!("checksum mismatch: expected {checksum}, got {actual}")));
                            }
                        }
                    }
                }
            }
        }
        issues
    }
}

// ── PatchApplicator ───────────────────────────────────────────────────────────

/// Applies a patch manifest to an installation directory.
pub struct PatchApplicator {
    pub install_root: PathBuf,
    pub patch_source: PathBuf,
}

impl PatchApplicator {
    pub fn new(install_root: impl Into<PathBuf>, patch_source: impl Into<PathBuf>) -> Self {
        Self { install_root: install_root.into(), patch_source: patch_source.into() }
    }

    /// Apply a patch manifest. Returns Ok(applied_count) or Err(path).
    pub fn apply(&self, manifest: &PatchManifest) -> Result<usize, String> {
        let mut applied = 0;
        for entry in &manifest.entries {
            let dest = self.install_root.join(&entry.path);
            match &entry.operation {
                PatchOp::Delete => {
                    if dest.exists() {
                        fs::remove_file(&dest).map_err(|e| format!("{}: {e}", entry.path))?;
                    }
                }
                PatchOp::Replace { .. } | PatchOp::Create { .. } => {
                    let src = self.patch_source.join(&entry.path);
                    if let Some(parent) = dest.parent() {
                        fs::create_dir_all(parent).map_err(|e| format!("{}: {e}", entry.path))?;
                    }
                    if src.exists() {
                        fs::copy(&src, &dest).map_err(|e| format!("{}: {e}", entry.path))?;
                    }
                }
            }
            applied += 1;
        }
        tracing::info!(target: "Patch", "Applied {} entries (v{} → v{})", applied, manifest.from_version, manifest.to_version);
        Ok(applied)
    }
}
