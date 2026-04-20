//! atlas_pipeline — File-based workspace pipeline for inter-tool communication.
//!
//! Mirrors the C++ NF::Pipeline system: ChangeEvent, AssetRecord, Manifest,
//! WatchLog, and PipelineWatcher. All tools communicate via files under the
//! `.atlas/pipeline/` directory tree — no sockets, no RPC.

use serde::{Deserialize, Serialize};
use std::{
    collections::HashMap,
    fs,
    io::Write,
    path::{Path, PathBuf},
    sync::{Arc, Mutex},
    thread,
    time::{Duration, SystemTime, UNIX_EPOCH},
};

// ── ChangeEventType ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
#[serde(rename_all = "PascalCase")]
pub enum ChangeEventType {
    #[default]
    Unknown,
    AssetImported,
    WorldChanged,
    ScriptUpdated,
    AnimationExported,
    ContractIssue,
    ReplayExported,
    AiAnalysis,
}

impl ChangeEventType {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Unknown           => "Unknown",
            Self::AssetImported     => "AssetImported",
            Self::WorldChanged      => "WorldChanged",
            Self::ScriptUpdated     => "ScriptUpdated",
            Self::AnimationExported => "AnimationExported",
            Self::ContractIssue     => "ContractIssue",
            Self::ReplayExported    => "ReplayExported",
            Self::AiAnalysis        => "AIAnalysis",
        }
    }

    pub fn from_str(s: &str) -> Self {
        match s {
            "AssetImported"     => Self::AssetImported,
            "WorldChanged"      => Self::WorldChanged,
            "ScriptUpdated"     => Self::ScriptUpdated,
            "AnimationExported" => Self::AnimationExported,
            "ContractIssue"     => Self::ContractIssue,
            "ReplayExported"    => Self::ReplayExported,
            "AIAnalysis"        => Self::AiAnalysis,
            _                   => Self::Unknown,
        }
    }
}

// ── ChangeEvent ───────────────────────────────────────────────────────────────

/// An event emitted by any tool and persisted as a `.change.json` file.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ChangeEvent {
    pub tool:       String,
    pub event_type: ChangeEventType,
    pub path:       String,
    pub timestamp:  i64,
    pub metadata:   String,
}

impl ChangeEvent {
    pub fn new(tool: impl Into<String>, event_type: ChangeEventType, path: impl Into<String>) -> Self {
        let timestamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|d| d.as_millis() as i64)
            .unwrap_or(0);
        Self {
            tool: tool.into(),
            event_type,
            path: path.into(),
            timestamp,
            metadata: String::new(),
        }
    }

    pub fn to_json_string(&self) -> String {
        serde_json::to_string(self).unwrap_or_default()
    }

    pub fn from_json_str(s: &str) -> Option<Self> {
        serde_json::from_str(s).ok()
    }

    pub fn write_to_dir(&self, changes_dir: &Path) -> std::io::Result<()> {
        fs::create_dir_all(changes_dir)?;
        let filename = format!(
            "{}_{}_{}_{}.change.json",
            self.tool, self.event_type.as_str(), self.timestamp,
            (self.timestamp % 100000) as u32,
        );
        let path = changes_dir.join(filename);
        let mut f = fs::File::create(path)?;
        f.write_all(self.to_json_string().as_bytes())?;
        Ok(())
    }

    pub fn read_from_file(path: &Path) -> Option<Self> {
        let text = fs::read_to_string(path).ok()?;
        Self::from_json_str(&text)
    }
}

// ── AssetRecord ───────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AssetRecord {
    pub guid:        String,
    pub asset_type:  String,
    pub path:        String,
    pub import_date: i64,
    pub checksum:    String,
}

// ── Manifest ──────────────────────────────────────────────────────────────────

/// GUID → asset path registry, persisted as `manifest.json`.
#[derive(Debug, Clone, Default)]
pub struct Manifest {
    pub project_name: String,
    pub modules:      Vec<String>,
    records:          Vec<AssetRecord>,
    guid_index:       HashMap<String, usize>,
    path_index:       HashMap<String, usize>,
}

impl Manifest {
    pub fn new() -> Self { Self::default() }

    pub fn load(path: &Path) -> Option<Self> {
        let text = fs::read_to_string(path).ok()?;
        let v: serde_json::Value = serde_json::from_str(&text).ok()?;
        let mut m = Self::new();
        m.project_name = v["projectName"].as_str().unwrap_or_default().to_string();
        if let Some(mods) = v["modules"].as_array() {
            m.modules = mods.iter().filter_map(|x| x.as_str().map(String::from)).collect();
        }
        if let Some(recs) = v["assets"].as_array() {
            for r in recs {
                if let Ok(rec) = serde_json::from_value::<AssetRecord>(r.clone()) {
                    m.push_record(rec);
                }
            }
        }
        Some(m)
    }

    pub fn save(&self, path: &Path) -> std::io::Result<()> {
        if let Some(parent) = path.parent() { fs::create_dir_all(parent)?; }
        let v = serde_json::json!({
            "projectName": self.project_name,
            "modules":     self.modules,
            "assets":      self.records,
        });
        let text = serde_json::to_string_pretty(&v)?;
        fs::write(path, text)
    }

    pub fn register_asset(&mut self, mut record: AssetRecord) -> String {
        if record.guid.is_empty() {
            record.guid = Self::generate_guid();
        }
        // Update existing by path
        if let Some(&idx) = self.path_index.get(&record.path) {
            let guid = record.guid.clone();
            self.records[idx] = record;
            self.guid_index.insert(self.records[idx].guid.clone(), idx);
            return guid;
        }
        self.push_record(record.clone());
        record.guid
    }

    pub fn find_by_guid(&self, guid: &str) -> Option<&AssetRecord> {
        self.guid_index.get(guid).and_then(|&i| self.records.get(i))
    }

    pub fn find_by_path(&self, path: &str) -> Option<&AssetRecord> {
        self.path_index.get(path).and_then(|&i| self.records.get(i))
    }

    pub fn remove_asset(&mut self, guid: &str) -> bool {
        if let Some(&idx) = self.guid_index.get(guid) {
            let rec = self.records.remove(idx);
            self.guid_index.remove(&rec.guid);
            self.path_index.remove(&rec.path);
            self.rebuild_index();
            true
        } else {
            false
        }
    }

    pub fn records(&self) -> &[AssetRecord] { &self.records }
    pub fn record_count(&self) -> usize { self.records.len() }

    fn push_record(&mut self, record: AssetRecord) {
        let idx = self.records.len();
        self.guid_index.insert(record.guid.clone(), idx);
        self.path_index.insert(record.path.clone(), idx);
        self.records.push(record);
    }

    fn rebuild_index(&mut self) {
        self.guid_index.clear();
        self.path_index.clear();
        for (i, r) in self.records.iter().enumerate() {
            self.guid_index.insert(r.guid.clone(), i);
            self.path_index.insert(r.path.clone(), i);
        }
    }

    fn generate_guid() -> String {
        use std::time::{SystemTime, UNIX_EPOCH};
        let ns = SystemTime::now().duration_since(UNIX_EPOCH).map(|d| d.subsec_nanos()).unwrap_or(0);
        format!("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            ns, ns >> 16, ns >> 8 & 0xffff, ns & 0xffff, ns as u64 * 0xdeadbeef)
    }
}

// ── WatchLog ──────────────────────────────────────────────────────────────────

/// Append-only event log shared between all tools.
pub struct WatchLog {
    path:  PathBuf,
    mutex: Mutex<()>,
}

impl WatchLog {
    pub fn new(path: PathBuf) -> Self { Self { path, mutex: Mutex::new(()) } }

    pub fn append(&self, event: &ChangeEvent) {
        self.append_line(&event.to_json_string());
    }

    pub fn append_line(&self, line: &str) {
        let _guard = self.mutex.lock().unwrap_or_else(|p| p.into_inner());
        if let Ok(mut f) = fs::OpenOptions::new().create(true).append(true).open(&self.path) {
            let _ = writeln!(f, "{}", line);
        }
    }

    pub fn path(&self) -> &Path { &self.path }
}

// ── PipelineWatcher ───────────────────────────────────────────────────────────

type EventCallback = Arc<dyn Fn(ChangeEvent) + Send + Sync>;

/// Watches a directory for new `.change.json` files and dispatches events.
pub struct PipelineWatcher {
    watch_dir:  PathBuf,
    callbacks:  Arc<Mutex<Vec<EventCallback>>>,
    running:    Arc<std::sync::atomic::AtomicBool>,
    seen_files: Mutex<std::collections::HashSet<PathBuf>>,
}

impl PipelineWatcher {
    pub fn new(watch_dir: PathBuf) -> Self {
        Self {
            watch_dir,
            callbacks:  Arc::new(Mutex::new(Vec::new())),
            running:    Arc::new(std::sync::atomic::AtomicBool::new(false)),
            seen_files: Mutex::new(std::collections::HashSet::new()),
        }
    }

    pub fn subscribe<F>(&self, cb: F)
    where F: Fn(ChangeEvent) + Send + Sync + 'static
    {
        if let Ok(mut cbs) = self.callbacks.lock() {
            cbs.push(Arc::new(cb));
        }
    }

    pub fn start(&self) -> bool {
        use std::sync::atomic::Ordering;
        if self.running.swap(true, Ordering::SeqCst) { return false; }
        let running   = Arc::clone(&self.running);
        let callbacks = Arc::clone(&self.callbacks);
        let watch_dir = self.watch_dir.clone();
        let mut seen: std::collections::HashSet<PathBuf> = std::collections::HashSet::new();

        thread::spawn(move || {
            while running.load(Ordering::SeqCst) {
                if let Ok(entries) = fs::read_dir(&watch_dir) {
                    for entry in entries.flatten() {
                        let p = entry.path();
                        if p.extension().and_then(|e| e.to_str()) == Some("json")
                            && p.to_str().map_or(false, |s| s.ends_with(".change.json"))
                            && !seen.contains(&p)
                        {
                            seen.insert(p.clone());
                            if let Some(evt) = ChangeEvent::read_from_file(&p) {
                                if let Ok(cbs) = callbacks.lock() {
                                    for cb in cbs.iter() { cb(evt.clone()); }
                                }
                            }
                        }
                    }
                }
                thread::sleep(Duration::from_millis(100));
            }
        });
        true
    }

    pub fn stop(&self) {
        self.running.store(false, std::sync::atomic::Ordering::SeqCst);
    }

    /// Single-threaded poll — call from the main loop in tests or headless mode.
    pub fn poll(&self) {
        if let (Ok(mut seen), Ok(entries)) = (
            self.seen_files.lock(),
            fs::read_dir(&self.watch_dir),
        ) {
            for entry in entries.flatten() {
                let p = entry.path();
                if p.extension().and_then(|e| e.to_str()) == Some("json")
                    && p.to_str().map_or(false, |s| s.ends_with(".change.json"))
                    && !seen.contains(&p)
                {
                    seen.insert(p.clone());
                    if let Some(evt) = ChangeEvent::read_from_file(&p) {
                        if let Ok(cbs) = self.callbacks.lock() {
                            for cb in cbs.iter() { cb(evt.clone()); }
                        }
                    }
                }
            }
        }
    }

    pub fn watch_dir(&self) -> &Path { &self.watch_dir }
}

impl Drop for PipelineWatcher {
    fn drop(&mut self) { self.stop(); }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    #[test]
    fn change_event_type_round_trip() {
        // ChangeEventType uses domain-specific variants; test as_str / from_str symmetry
        for variant in [
            ChangeEventType::AssetImported,
            ChangeEventType::WorldChanged,
            ChangeEventType::ScriptUpdated,
        ] {
            let s = variant.as_str();
            let back = ChangeEventType::from_str(s);
            assert_eq!(back, variant, "round-trip failed for {s}");
        }
    }

    #[test]
    fn change_event_unknown_type_is_unknown() {
        assert!(matches!(ChangeEventType::from_str("gibberish"), ChangeEventType::Unknown));
    }

    #[test]
    fn change_event_json_round_trip() {
        let ev = ChangeEvent::new("WorldEditor", ChangeEventType::WorldChanged, "Content/Map.atlas");
        let json = ev.to_json_string();
        let back = ChangeEvent::from_json_str(&json).expect("must deserialize");
        assert_eq!(back.tool, "WorldEditor");
        assert_eq!(back.path, "Content/Map.atlas");
        assert!(matches!(back.event_type, ChangeEventType::WorldChanged));
    }

    #[test]
    fn manifest_register_and_find_asset() {
        let mut manifest = Manifest::new();
        let record = AssetRecord {
            guid:        String::new(),
            path:        "Textures/grass.png".into(),
            asset_type:  "Texture2D".into(),
            import_date: 0,
            checksum:    String::new(),
        };
        let guid = manifest.register_asset(record);
        assert!(!guid.is_empty(), "registered asset must get a GUID");
        let found = manifest.find_by_guid(&guid);
        assert!(found.is_some(), "must be findable by GUID");
        assert_eq!(found.unwrap().path, "Textures/grass.png");
    }

    #[test]
    fn manifest_find_by_path() {
        let mut manifest = Manifest::new();
        let record = AssetRecord {
            guid: String::new(), path: "Audio/click.wav".into(),
            asset_type: "AudioClip".into(), import_date: 0, checksum: String::new(),
        };
        manifest.register_asset(record);
        assert!(manifest.find_by_path("Audio/click.wav").is_some());
        assert!(manifest.find_by_path("NonExistent").is_none());
    }

    #[test]
    fn manifest_remove_asset() {
        let mut manifest = Manifest::new();
        let record = AssetRecord {
            guid: String::new(), path: "Mesh/cube.obj".into(),
            asset_type: "Mesh".into(), import_date: 0, checksum: String::new(),
        };
        let guid = manifest.register_asset(record);
        assert_eq!(manifest.record_count(), 1);
        assert!(manifest.remove_asset(&guid));
        assert_eq!(manifest.record_count(), 0);
    }

    #[test]
    fn watch_log_append_and_file_created() {
        let dir = std::env::temp_dir();
        let path = dir.join(format!("atlas_test_watchlog_{}.log", std::process::id()));
        let log = WatchLog::new(path.clone());
        let ev = ChangeEvent::new("Test", ChangeEventType::AssetImported, "asset.png");
        log.append(&ev);
        assert!(path.exists(), "WatchLog must create the file on first append");
        let content = std::fs::read_to_string(&path).unwrap();
        assert!(content.contains("asset.png"));
        std::fs::remove_file(&path).ok();
    }

    #[test]
    fn pipeline_watcher_poll_mode() {
        let dir = std::env::temp_dir().join(format!("atlas_watcher_test_{}", std::process::id()));
        std::fs::create_dir_all(&dir).ok();
        let watcher = PipelineWatcher::new(dir.clone());
        let received = std::sync::Arc::new(std::sync::Mutex::new(Vec::<ChangeEvent>::new()));
        let r2 = received.clone();
        watcher.subscribe(move |ev| { r2.lock().unwrap().push(ev); });
        let ev = ChangeEvent::new("Test", ChangeEventType::AssetImported, "poll_test.txt");
        ev.write_to_dir(&dir).expect("must write event file");
        watcher.poll();
        let events = received.lock().unwrap();
        assert!(!events.is_empty(), "poll must deliver the event written to the watch dir");
        std::fs::remove_dir_all(&dir).ok();
    }
}
