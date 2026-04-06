#pragma once
// NF::Editor — Editor app, docking panels, viewport, toolbar. Does not ship.
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/Game/Game.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

// ── Dock layout ──────────────────────────────────────────────────

enum class DockSlot : uint8_t {
    Left,
    Right,
    Top,
    Bottom,
    Center
};

struct DockPanel {
    std::string name;
    DockSlot slot = DockSlot::Center;
    Rect bounds;
    bool visible = true;
};

// ── Selection Service ────────────────────────────────────────────

class SelectionService {
public:
    void select(EntityID id) {
        m_selection.insert(id);
        m_primary = id;
        ++m_version;
    }

    void deselect(EntityID id) {
        m_selection.erase(id);
        if (m_primary == id) {
            m_primary = m_selection.empty() ? INVALID_ENTITY : *m_selection.begin();
        }
        ++m_version;
    }

    void toggleSelect(EntityID id) {
        if (isSelected(id)) deselect(id);
        else select(id);
    }

    void clearSelection() {
        m_selection.clear();
        m_primary = INVALID_ENTITY;
        ++m_version;
    }

    void selectExclusive(EntityID id) {
        m_selection.clear();
        m_selection.insert(id);
        m_primary = id;
        ++m_version;
    }

    [[nodiscard]] bool isSelected(EntityID id) const {
        return m_selection.count(id) > 0;
    }

    [[nodiscard]] EntityID primarySelection() const { return m_primary; }
    [[nodiscard]] const std::set<EntityID>& selection() const { return m_selection; }
    [[nodiscard]] size_t selectionCount() const { return m_selection.size(); }
    [[nodiscard]] bool hasSelection() const { return !m_selection.empty(); }
    [[nodiscard]] uint32_t version() const { return m_version; }

private:
    std::set<EntityID> m_selection;
    EntityID m_primary = INVALID_ENTITY;
    uint32_t m_version = 0;
};

// ── Content Browser ──────────────────────────────────────────────

enum class ContentEntryType : uint8_t {
    Directory,
    Scene,
    Mesh,
    Material,
    Texture,
    Audio,
    Script,
    Data,
    Unknown
};

struct ContentEntry {
    std::string name;
    std::string path;
    ContentEntryType type = ContentEntryType::Unknown;
    size_t sizeBytes = 0;
    bool isDirectory = false;
};

class ContentBrowser {
public:
    void setRootPath(const std::string& root) {
        m_rootPath = root;
        m_currentPath = root;
    }

    bool navigateTo(const std::string& path) {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            NF_LOG_WARN("ContentBrowser", "Invalid path: " + path);
            return false;
        }
        m_currentPath = path;
        refresh();
        return true;
    }

    bool navigateUp() {
        if (m_currentPath == m_rootPath) return false;
        auto parent = std::filesystem::path(m_currentPath).parent_path().string();
        if (parent.size() < m_rootPath.size()) return false;
        return navigateTo(parent);
    }

    void refresh() {
        m_entries.clear();
        if (!std::filesystem::exists(m_currentPath)) return;

        for (auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
            ContentEntry ce;
            ce.name = entry.path().filename().string();
            ce.path = entry.path().string();
            ce.isDirectory = entry.is_directory();

            if (ce.isDirectory) {
                ce.type = ContentEntryType::Directory;
            } else {
                auto ext = entry.path().extension().string();
                ce.type = classifyExtension(ext);
                if (entry.is_regular_file()) {
                    ce.sizeBytes = static_cast<size_t>(entry.file_size());
                }
            }

            // Skip hidden files
            if (!ce.name.empty() && ce.name[0] != '.') {
                m_entries.push_back(std::move(ce));
            }
        }

        // Sort: directories first, then alphabetical
        std::sort(m_entries.begin(), m_entries.end(), [](const ContentEntry& a, const ContentEntry& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory;
            return a.name < b.name;
        });
    }

    [[nodiscard]] const std::string& rootPath() const { return m_rootPath; }
    [[nodiscard]] const std::string& currentPath() const { return m_currentPath; }
    [[nodiscard]] const std::vector<ContentEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    [[nodiscard]] bool isAtRoot() const { return m_currentPath == m_rootPath; }

    static ContentEntryType classifyExtension(const std::string& ext) {
        if (ext == ".json") return ContentEntryType::Data;
        if (ext == ".scene" || ext == ".world") return ContentEntryType::Scene;
        if (ext == ".mesh" || ext == ".obj" || ext == ".fbx") return ContentEntryType::Mesh;
        if (ext == ".mat" || ext == ".material") return ContentEntryType::Material;
        if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".bmp") return ContentEntryType::Texture;
        if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") return ContentEntryType::Audio;
        if (ext == ".lua" || ext == ".graph") return ContentEntryType::Script;
        return ContentEntryType::Unknown;
    }

private:
    std::string m_rootPath;
    std::string m_currentPath;
    std::vector<ContentEntry> m_entries;
};

// ── Project Path Service ─────────────────────────────────────────

class ProjectPathService {
public:
    static constexpr const char* kProjectFileName = "novaforge.project.json";

    void init(const std::string& executablePath) {
        m_executablePath = executablePath;

        // Derive project root from executable path
        auto exePath = std::filesystem::path(executablePath);
        // Walk up from bin/ or Builds/ to find project root
        constexpr int kMaxProjectRootSearchDepth = 5;
        auto dir = exePath.parent_path();
        for (int i = 0; i < kMaxProjectRootSearchDepth; ++i) {
            if (std::filesystem::exists(dir / "Config" / kProjectFileName)) {
                m_projectRoot = dir.string();
                break;
            }
            dir = dir.parent_path();
        }

        if (m_projectRoot.empty()) {
            m_projectRoot = std::filesystem::current_path().string();
        }

        m_contentPath = (std::filesystem::path(m_projectRoot) / "Content").string();
        m_dataPath = (std::filesystem::path(m_projectRoot) / "Data").string();
        m_configPath = (std::filesystem::path(m_projectRoot) / "Config").string();

        NF_LOG_INFO("Editor", "Project root: " + m_projectRoot);
        NF_LOG_INFO("Editor", "Content path: " + m_contentPath);
    }

    [[nodiscard]] const std::string& executablePath() const { return m_executablePath; }
    [[nodiscard]] const std::string& projectRoot() const { return m_projectRoot; }
    [[nodiscard]] const std::string& contentPath() const { return m_contentPath; }
    [[nodiscard]] const std::string& dataPath() const { return m_dataPath; }
    [[nodiscard]] const std::string& configPath() const { return m_configPath; }

    [[nodiscard]] std::string resolvePath(const std::string& relativePath) const {
        return (std::filesystem::path(m_projectRoot) / relativePath).string();
    }

private:
    std::string m_executablePath;
    std::string m_projectRoot;
    std::string m_contentPath;
    std::string m_dataPath;
    std::string m_configPath;
};

// ── Launch Service ───────────────────────────────────────────────

struct LaunchResult {
    bool success = false;
    std::string executablePath;
    std::string errorMessage;
};

class LaunchService {
public:
    void setGameExecutableName(const std::string& name) { m_gameExeName = name; }
    void setBuildDirectory(const std::string& dir) { m_buildDir = dir; }

    [[nodiscard]] std::string resolveGamePath() const {
        // Search order: explicit build dir, then common locations
        std::vector<std::string> searchPaths;

        if (!m_buildDir.empty()) {
            searchPaths.push_back(m_buildDir + "/bin/" + m_gameExeName);
            searchPaths.push_back(m_buildDir + "/" + m_gameExeName);
        }

        searchPaths.push_back("./Builds/debug/bin/" + m_gameExeName);
        searchPaths.push_back("./Builds/release/bin/" + m_gameExeName);
        searchPaths.push_back("./bin/" + m_gameExeName);
        searchPaths.push_back("./" + m_gameExeName);

        for (auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                return std::filesystem::canonical(path).string();
            }
        }

        return "";
    }

    [[nodiscard]] LaunchResult validateLaunch() const {
        LaunchResult result;
        result.executablePath = resolveGamePath();

        if (result.executablePath.empty()) {
            result.success = false;
            result.errorMessage = "Game executable '" + m_gameExeName + "' not found in search paths";
            return result;
        }

        if (!std::filesystem::exists(result.executablePath)) {
            result.success = false;
            result.errorMessage = "Game executable does not exist: " + result.executablePath;
            return result;
        }

        result.success = true;
        return result;
    }

private:
    std::string m_gameExeName = "NovaForgeGame";
    std::string m_buildDir;
};

// ── Editor Command State ─────────────────────────────────────────

struct CommandInfo {
    std::string name;
    std::string displayName;
    std::string hotkey;
    std::function<void()> handler;
    std::function<bool()> enabledCheck;  // returns true if command is available
    bool enabled = true;
};

class EditorCommandRegistry {
public:
    void registerCommand(const std::string& name, std::function<void()> handler,
                         const std::string& displayName = "",
                         const std::string& hotkey = "") {
        CommandInfo info;
        info.name = name;
        info.displayName = displayName.empty() ? name : displayName;
        info.hotkey = hotkey;
        info.handler = std::move(handler);
        info.enabled = true;
        m_commands[name] = std::move(info);
    }

    void setEnabledCheck(const std::string& name, std::function<bool()> check) {
        auto it = m_commands.find(name);
        if (it != m_commands.end()) {
            it->second.enabledCheck = std::move(check);
        }
    }

    bool executeCommand(const std::string& name) {
        auto it = m_commands.find(name);
        if (it == m_commands.end()) {
            NF_LOG_WARN("Editor", "Unknown command: " + name);
            return false;
        }
        if (it->second.enabledCheck && !it->second.enabledCheck()) {
            NF_LOG_WARN("Editor", "Command disabled: " + name);
            return false;
        }
        it->second.handler();
        return true;
    }

    [[nodiscard]] bool isCommandEnabled(const std::string& name) const {
        auto it = m_commands.find(name);
        if (it == m_commands.end()) return false;
        if (it->second.enabledCheck) return it->second.enabledCheck();
        return it->second.enabled;
    }

    [[nodiscard]] const CommandInfo* findCommand(const std::string& name) const {
        auto it = m_commands.find(name);
        return (it != m_commands.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] std::vector<std::string> allCommandNames() const {
        std::vector<std::string> names;
        names.reserve(m_commands.size());
        for (auto& [k, _] : m_commands) names.push_back(k);
        std::sort(names.begin(), names.end());
        return names;
    }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }

private:
    std::unordered_map<std::string, CommandInfo> m_commands;
};

// ── Recent Files ─────────────────────────────────────────────────

class RecentFilesList {
public:
    void addFile(const std::string& path) {
        // Remove if already present
        m_files.erase(std::remove(m_files.begin(), m_files.end(), path), m_files.end());
        m_files.insert(m_files.begin(), path);
        if (m_files.size() > m_maxEntries) {
            m_files.resize(m_maxEntries);
        }
    }

    void clear() { m_files.clear(); }

    [[nodiscard]] const std::vector<std::string>& files() const { return m_files; }
    [[nodiscard]] size_t count() const { return m_files.size(); }
    [[nodiscard]] bool empty() const { return m_files.empty(); }

    void setMaxEntries(size_t max) { m_maxEntries = max; }

private:
    std::vector<std::string> m_files;
    size_t m_maxEntries = 10;
};

// ── Editor Theme ─────────────────────────────────────────────────

struct EditorTheme {
    // Panel colors
    uint32_t panelBackground     = 0x2B2B2BFF;
    uint32_t panelHeader         = 0x3C3C3CFF;
    uint32_t panelBorder         = 0x555555FF;
    uint32_t panelText           = 0xDCDCDCFF;

    // Selection colors
    uint32_t selectionHighlight  = 0x264F78FF;
    uint32_t selectionBorder     = 0x007ACCFF;
    uint32_t hoverHighlight      = 0x383838FF;

    // Input field colors
    uint32_t inputBackground     = 0x1E1E1EFF;
    uint32_t inputBorder         = 0x474747FF;
    uint32_t inputText           = 0xD4D4D4FF;
    uint32_t inputFocusBorder    = 0x007ACCFF;

    // Button colors
    uint32_t buttonBackground    = 0x3C3C3CFF;
    uint32_t buttonHover         = 0x505050FF;
    uint32_t buttonPressed       = 0x007ACCFF;
    uint32_t buttonText          = 0xCCCCCCFF;
    uint32_t buttonDisabledText  = 0x666666FF;

    // Toolbar
    uint32_t toolbarBackground   = 0x333333FF;
    uint32_t toolbarSeparator    = 0x4A4A4AFF;

    // Status bar
    uint32_t statusBarBackground = 0x007ACCFF;
    uint32_t statusBarText       = 0xFFFFFFFF;

    // Viewport
    uint32_t viewportBackground  = 0x1A1A1AFF;
    uint32_t gridColor           = 0x333333FF;

    // Inspector
    uint32_t propertyLabel       = 0xBBBBBBFF;
    uint32_t propertyValue       = 0xE0E0E0FF;
    uint32_t propertySeparator   = 0x404040FF;
    uint32_t dirtyIndicator      = 0xE8A435FF;

    // Font sizing
    float fontSize               = 14.f;
    float headerFontSize         = 16.f;
    float smallFontSize          = 12.f;

    // Spacing
    float panelPadding           = 8.f;
    float itemSpacing            = 4.f;
    float sectionSpacing         = 12.f;

    static EditorTheme dark() { return {}; }

    static EditorTheme light() {
        EditorTheme t;
        t.panelBackground     = 0xF0F0F0FF;
        t.panelHeader         = 0xE0E0E0FF;
        t.panelBorder         = 0xCCCCCCFF;
        t.panelText           = 0x1E1E1EFF;
        t.selectionHighlight  = 0xCCE8FFFF;
        t.selectionBorder     = 0x0078D4FF;
        t.hoverHighlight      = 0xE5E5E5FF;
        t.inputBackground     = 0xFFFFFFFF;
        t.inputBorder         = 0xCCCCCCFF;
        t.inputText           = 0x1E1E1EFF;
        t.buttonBackground    = 0xE0E0E0FF;
        t.buttonHover         = 0xD0D0D0FF;
        t.buttonText          = 0x1E1E1EFF;
        t.toolbarBackground   = 0xE8E8E8FF;
        t.statusBarBackground = 0x0078D4FF;
        t.viewportBackground  = 0xD4D4D4FF;
        t.gridColor           = 0xBBBBBBFF;
        t.propertyLabel       = 0x444444FF;
        t.propertyValue       = 0x1E1E1EFF;
        return t;
    }

    UITheme toUITheme() const {
        UITheme t;
        t.panelBackground    = panelBackground;
        t.panelHeader        = panelHeader;
        t.panelBorder        = panelBorder;
        t.panelText          = panelText;
        t.selectionHighlight = selectionHighlight;
        t.selectionBorder    = selectionBorder;
        t.hoverHighlight     = hoverHighlight;
        t.inputBackground    = inputBackground;
        t.inputBorder        = inputBorder;
        t.inputText          = inputText;
        t.inputFocusBorder   = inputFocusBorder;
        t.buttonBackground   = buttonBackground;
        t.buttonHover        = buttonHover;
        t.buttonPressed      = buttonPressed;
        t.buttonText         = buttonText;
        t.buttonDisabledText = buttonDisabledText;
        t.toolbarBackground  = toolbarBackground;
        t.toolbarSeparator   = toolbarSeparator;
        t.statusBarBackground= statusBarBackground;
        t.statusBarText      = statusBarText;
        t.viewportBackground = viewportBackground;
        t.gridColor          = gridColor;
        t.propertyLabel      = propertyLabel;
        t.propertyValue      = propertyValue;
        t.propertySeparator  = propertySeparator;
        t.dirtyIndicator     = dirtyIndicator;
        t.fontSize           = fontSize;
        t.headerFontSize     = headerFontSize;
        t.smallFontSize      = smallFontSize;
        t.panelPadding       = panelPadding;
        t.itemSpacing        = itemSpacing;
        t.sectionSpacing     = sectionSpacing;
        return t;
    }
};

// ── DockLayout ───────────────────────────────────────────────────

class DockLayout {
public:
    void addPanel(const std::string& name, DockSlot slot) {
        DockPanel panel;
        panel.name = name;
        panel.slot = slot;
        panel.visible = true;
        m_panels.push_back(std::move(panel));
        NF_LOG_INFO("Editor", "DockLayout: added panel '" + name + "'");
    }

    void removePanel(const std::string& name) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const DockPanel& p) { return p.name == name; });
        if (it != m_panels.end()) {
            m_panels.erase(it);
            NF_LOG_INFO("Editor", "DockLayout: removed panel '" + name + "'");
        }
    }

    [[nodiscard]] DockPanel* findPanel(const std::string& name) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const DockPanel& p) { return p.name == name; });
        return (it != m_panels.end()) ? &(*it) : nullptr;
    }

    void setPanelVisible(const std::string& name, bool visible) {
        if (auto* p = findPanel(name)) {
            p->visible = visible;
        }
    }

    void computeLayout(float width, float height, float toolbarHeight, float statusBarHeight) {
        float usableHeight = height - toolbarHeight - statusBarHeight;

        float leftWidth = 0.f;
        float rightWidth = 0.f;
        float topHeight = 0.f;
        float bottomHeight = 0.f;

        for (auto& p : m_panels) {
            if (!p.visible) continue;
            switch (p.slot) {
                case DockSlot::Left:   leftWidth   = m_leftWidth;   break;
                case DockSlot::Right:  rightWidth  = m_rightWidth;  break;
                case DockSlot::Top:    topHeight   = m_topHeight;   break;
                case DockSlot::Bottom: bottomHeight = m_bottomHeight; break;
                default: break;
            }
        }

        float centerX = leftWidth;
        float centerW = width - leftWidth - rightWidth;

        for (auto& p : m_panels) {
            if (!p.visible) continue;
            switch (p.slot) {
                case DockSlot::Left:
                    p.bounds = { 0.f, toolbarHeight, leftWidth, usableHeight };
                    break;
                case DockSlot::Right:
                    p.bounds = { width - rightWidth, toolbarHeight, rightWidth, usableHeight };
                    break;
                case DockSlot::Bottom:
                    p.bounds = { centerX, height - statusBarHeight - bottomHeight, centerW, bottomHeight };
                    break;
                case DockSlot::Top:
                    p.bounds = { centerX, toolbarHeight, centerW, topHeight };
                    break;
                case DockSlot::Center:
                    p.bounds = { centerX, toolbarHeight + topHeight, centerW,
                                 usableHeight - topHeight - bottomHeight };
                    break;
            }
        }
    }

    [[nodiscard]] size_t panelCount() const { return m_panels.size(); }
    [[nodiscard]] const std::vector<DockPanel>& panels() const { return m_panels; }

    static constexpr int kDockSlotCount = 5;

    // ── Splitter resizing ────────────────────────────────────────
    void beginResize(DockSlot slot, float mousePos) {
        m_resizingSlot = slot;
        m_resizeStart = mousePos;
        m_resizing = true;
    }

    void updateResize(float mousePos) {
        if (!m_resizing) return;
        float delta = mousePos - m_resizeStart;
        m_resizeStart = mousePos;
        switch (m_resizingSlot) {
            case DockSlot::Left:   m_leftWidth   = std::clamp(m_leftWidth + delta, kMinPanelSize, 600.f); break;
            case DockSlot::Right:  m_rightWidth  = std::clamp(m_rightWidth - delta, kMinPanelSize, 600.f); break;
            case DockSlot::Bottom: m_bottomHeight = std::clamp(m_bottomHeight - delta, kMinPanelSize, 500.f); break;
            case DockSlot::Top:    m_topHeight   = std::clamp(m_topHeight + delta, kMinPanelSize, 500.f); break;
            default: break;
        }
    }

    void endResize() { m_resizing = false; }
    [[nodiscard]] bool isResizing() const { return m_resizing; }
    [[nodiscard]] DockSlot resizingSlot() const { return m_resizingSlot; }

    // Reset to defaults
    void resetSizes() {
        m_leftWidth   = kDefaultLeftWidth;
        m_rightWidth  = kDefaultRightWidth;
        m_topHeight   = kDefaultTopHeight;
        m_bottomHeight = kDefaultBottomHeight;
    }

    // ── Tab groups ───────────────────────────────────────────────
    void addTab(const std::string& panelName, DockSlot slot) {
        m_tabGroups[static_cast<int>(slot)].push_back(panelName);
    }

    [[nodiscard]] std::string activeTab(DockSlot slot) const {
        int idx = static_cast<int>(slot);
        auto it = m_activeTabIndex.find(idx);
        int tabIdx = (it != m_activeTabIndex.end()) ? it->second : 0;
        if (idx < kDockSlotCount && tabIdx < static_cast<int>(m_tabGroups[idx].size())) {
            return m_tabGroups[idx][static_cast<size_t>(tabIdx)];
        }
        return {};
    }

    void selectTab(const std::string& panelName) {
        for (int s = 0; s < kDockSlotCount; ++s) {
            for (size_t i = 0; i < m_tabGroups[s].size(); ++i) {
                if (m_tabGroups[s][i] == panelName) {
                    m_activeTabIndex[s] = static_cast<int>(i);
                    return;
                }
            }
        }
    }

    [[nodiscard]] const std::vector<std::string>& tabGroup(DockSlot slot) const {
        return m_tabGroups[static_cast<int>(slot)];
    }

    // Accessors for current sizes
    [[nodiscard]] float leftWidth()    const { return m_leftWidth; }
    [[nodiscard]] float rightWidth()   const { return m_rightWidth; }
    [[nodiscard]] float topHeight()    const { return m_topHeight; }
    [[nodiscard]] float bottomHeight() const { return m_bottomHeight; }

    // Setters for restoring persisted sizes
    void setLeftWidth(float w)    { m_leftWidth    = std::clamp(w, kMinPanelSize, 600.f); }
    void setRightWidth(float w)   { m_rightWidth   = std::clamp(w, kMinPanelSize, 600.f); }
    void setTopHeight(float h)    { m_topHeight    = std::clamp(h, kMinPanelSize, 500.f); }
    void setBottomHeight(float h) { m_bottomHeight = std::clamp(h, kMinPanelSize, 500.f); }

    static constexpr float kMinPanelSize = 100.f;
    static constexpr float kDefaultLeftWidth   = 250.f;
    static constexpr float kDefaultRightWidth  = 300.f;
    static constexpr float kDefaultTopHeight   = 200.f;
    static constexpr float kDefaultBottomHeight = 200.f;

private:
    std::vector<DockPanel> m_panels;
    float m_leftWidth    = kDefaultLeftWidth;
    float m_rightWidth   = kDefaultRightWidth;
    float m_topHeight    = kDefaultTopHeight;
    float m_bottomHeight = kDefaultBottomHeight;
    bool  m_resizing     = false;
    DockSlot m_resizingSlot = DockSlot::Center;
    float m_resizeStart  = 0.f;
    std::vector<std::string> m_tabGroups[kDockSlotCount];  // indexed by DockSlot
    std::unordered_map<int, int> m_activeTabIndex;
};

// ── EditorPanel (abstract) ───────────────────────────────────────
// DEPRECATED: Legacy panel base class. New panels should derive from
// NF::UI::AtlasUI::PanelBase instead. See Source/UI/include/NF/UI/AtlasUI/Panels/
// for the canonical AtlasUI panel implementations.
// Migration phases U1-U7 provide AtlasUI equivalents for each panel below.

class EditorPanel {
public:
    virtual ~EditorPanel() = default;

    [[nodiscard]] virtual const std::string& name() const = 0;
    [[nodiscard]] virtual DockSlot slot() const = 0;
    virtual void update(float dt) = 0;
    virtual void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) = 0;
    virtual void renderUI(UIContext& ctx, const Rect& bounds) { (void)ctx; (void)bounds; }

    [[nodiscard]] bool isVisible() const { return m_visible; }
    void setVisible(bool v) { m_visible = v; }

private:
    bool m_visible = true;
};

// ── ViewportPanel ────────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::ViewportPanel instead (U7).

enum class RenderMode : uint8_t { Shaded, Wireframe, Unlit };
enum class ToolMode   : uint8_t { Select, Move, Rotate, Scale, Paint, Erase };

// Controls a fly-camera inside the viewport when the right mouse button is held.
// Uses the same yaw/pitch math as FPSCamera but is self-contained here so the
// Editor module does not depend on Game logic directly.
struct ViewportCameraController {
    float moveSpeed      = 10.f;   // world units per second
    float mouseSensitivity = 0.15f; // degrees per pixel
    float sprintMultiplier = 3.f;  // held Shift boost

    // Call each frame; mutates cameraPos, yaw, pitch.
    // Only moves the camera when Mouse2 (right button) is held.
    void update(float dt, const InputSystem& input,
                Vec3& cameraPos, float& yaw, float& pitch) {
        bool rmb = input.isKeyDown(KeyCode::Mouse2);
        if (!rmb) {
            m_active = false;
            return;
        }
        m_active = true;

        // -- Mouse look --
        float dx = input.state().mouse.deltaX * mouseSensitivity;
        float dy = input.state().mouse.deltaY * mouseSensitivity;
        yaw   += dx;
        pitch -= dy;  // invert Y so dragging up looks up
        if (pitch >  89.f) pitch =  89.f;
        if (pitch < -89.f) pitch = -89.f;

        // -- Rebuild forward/right vectors from yaw & pitch --
        float yawRad   = yaw   * (3.14159265f / 180.f);
        float pitchRad = pitch * (3.14159265f / 180.f);
        Vec3 fwd{
            std::cos(yawRad) * std::cos(pitchRad),
            std::sin(pitchRad),
            std::sin(yawRad) * std::cos(pitchRad)
        };
        fwd = fwd.normalized();
        Vec3 worldUp{0.f, 1.f, 0.f};
        Vec3 right = fwd.cross(worldUp).normalized();

        // -- WASD movement --
        float speed = moveSpeed;
        if (input.isKeyDown(KeyCode::LShift) || input.isKeyDown(KeyCode::RShift))
            speed *= sprintMultiplier;

        Vec3 move{0.f, 0.f, 0.f};
        if (input.isKeyDown(KeyCode::W)) move = move + fwd  *  speed * dt;
        if (input.isKeyDown(KeyCode::S)) move = move + fwd  * -speed * dt;
        if (input.isKeyDown(KeyCode::D)) move = move + right *  speed * dt;
        if (input.isKeyDown(KeyCode::A)) move = move + right * -speed * dt;
        // Q/E for up/down in world space
        if (input.isKeyDown(KeyCode::E)) move = move + worldUp *  speed * dt;
        if (input.isKeyDown(KeyCode::Q)) move = move + worldUp * -speed * dt;

        cameraPos = cameraPos + move;
    }

    [[nodiscard]] bool isActive() const { return m_active; }

private:
    bool m_active = false;
};

class ViewportPanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.viewportBackground);
        // Grid overlay
        if (m_gridEnabled) {
            float step = 40.f;
            for (float gx = bounds.x; gx < bounds.x + bounds.w; gx += step)
                ui.drawRect({gx, bounds.y, 1.f, bounds.h}, theme.gridColor);
            for (float gy = bounds.y; gy < bounds.y + bounds.h; gy += step)
                ui.drawRect({bounds.x, gy, bounds.w, 1.f}, theme.gridColor);
        }
        // Camera info overlay
        char camText[64];
        std::snprintf(camText, sizeof(camText), "Cam: %.1f, %.1f, %.1f",
                      m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
        ui.drawText(bounds.x + 8.f, bounds.y + bounds.h - 20.f, camText, 0x888888FF);
        // Tool mode indicator
        const char* modes[] = {"Select", "Move", "Rotate", "Scale", "Paint", "Erase"};
        ui.drawText(bounds.x + 8.f, bounds.y + 8.f,
                    modes[static_cast<int>(m_toolMode)], 0xAAAAAAFF);
        // Render mode indicator
        const char* rmodes[] = {"Shaded", "Wireframe", "Unlit"};
        ui.drawText(bounds.x + bounds.w - 80.f, bounds.y + 8.f,
                    rmodes[static_cast<int>(m_renderMode)], 0x888888FF);
        // Center label
        const char* vpLabel = "[ 3D Viewport ]";
        float labelW = 15.f * 8.f;
        ui.drawText(bounds.x + (bounds.w - labelW) * 0.5f,
                    bounds.y + (bounds.h - 14.f) * 0.5f, vpLabel, 0x444444FF);
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        (void)bounds;
        // Viewport-specific UI overlays rendered through widget system
        ctx.beginHorizontal();
        if (ctx.button("Grid", 50.f)) { m_gridEnabled = !m_gridEnabled; }
        ctx.endHorizontal();
    }

    // Called by EditorApp::update(dt, input) each frame.
    // Activates fly-camera only while right mouse button is held.
    void updateCamera(float dt, const InputSystem& input) {
        m_camController.update(dt, input, m_cameraPos, m_cameraYaw, m_cameraPitch);
    }

    [[nodiscard]] Vec3 cameraPosition() const { return m_cameraPos; }
    void setCameraPosition(Vec3 pos) { m_cameraPos = pos; }

    [[nodiscard]] float cameraYaw()   const { return m_cameraYaw; }
    [[nodiscard]] float cameraPitch() const { return m_cameraPitch; }
    void setCameraYaw(float y)   { m_cameraYaw   = y; }
    void setCameraPitch(float p) { m_cameraPitch = p; }

    [[nodiscard]] bool isFlyCamActive() const { return m_camController.isActive(); }

    [[nodiscard]] ViewportCameraController& cameraController() { return m_camController; }
    [[nodiscard]] const ViewportCameraController& cameraController() const { return m_camController; }

    [[nodiscard]] float cameraZoom() const { return m_cameraZoom; }
    void setCameraZoom(float z) { m_cameraZoom = z; }

    [[nodiscard]] bool gridEnabled() const { return m_gridEnabled; }
    void setGridEnabled(bool e) { m_gridEnabled = e; }

    [[nodiscard]] RenderMode renderMode() const { return m_renderMode; }
    void setRenderMode(RenderMode m) { m_renderMode = m; }

    [[nodiscard]] ToolMode toolMode() const { return m_toolMode; }
    void setToolMode(ToolMode m) { m_toolMode = m; }

private:
    std::string m_name = "Viewport";
    Vec3  m_cameraPos{0.f, 0.f, 0.f};
    float m_cameraYaw   = -90.f;  // degrees; -90 = look down -Z
    float m_cameraPitch =   0.f;  // degrees
    float m_cameraZoom  =   1.f;
    bool  m_gridEnabled = true;
    RenderMode m_renderMode = RenderMode::Shaded;
    ToolMode   m_toolMode   = ToolMode::Select;
    ViewportCameraController m_camController;
};

// ── InspectorPanel ───────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::InspectorPanel instead (U1).

class InspectorPanel : public EditorPanel {
public:
    InspectorPanel() = default;
    InspectorPanel(SelectionService* sel, TypeRegistry* reg)
        : m_selection(sel), m_typeRegistry(reg) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Right; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Inspector", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        if (m_selection && m_selection->hasSelection()) {
            EntityID id = m_selection->primarySelection();
            char idBuf[32];
            std::snprintf(idBuf, sizeof(idBuf), "Entity #%u", id);
            ui.drawText(bounds.x + 8.f, y, idBuf, theme.panelText);
            y += 20.f;
            ui.drawRect({bounds.x + 8.f, y, bounds.w - 16.f, 1.f}, theme.propertySeparator);
            y += 8.f;
            ui.drawText(bounds.x + 8.f, y, "Transform", theme.propertyLabel);
            y += 18.f;
            const char* axes[] = {"X: 0.00", "Y: 0.00", "Z: 0.00"};
            for (auto* a : axes) {
                ui.drawText(bounds.x + 16.f, y, a, theme.propertyValue);
                y += 16.f;
            }
        } else {
            ui.drawText(bounds.x + 8.f, y, "No entity selected", theme.propertyLabel);
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Inspector", bounds);
        if (m_selection && m_selection->hasSelection()) {
            char idBuf[32];
            std::snprintf(idBuf, sizeof(idBuf), "Entity #%u", m_selection->primarySelection());
            ctx.headerLabel(idBuf);
            ctx.separator();
            bool tExpanded = true;
            if (ctx.treeNode("Transform", tExpanded)) {
                ctx.indent(12.f);
                ctx.label("X: 0.00");
                ctx.label("Y: 0.00");
                ctx.label("Z: 0.00");
                ctx.unindent(12.f);
            }
        } else {
            ctx.label("No entity selected");
        }
        ctx.endPanel();
    }

    [[nodiscard]] SelectionService* selectionService() const { return m_selection; }
    void setSelectionService(SelectionService* s) { m_selection = s; }

    [[nodiscard]] TypeRegistry* typeRegistry() const { return m_typeRegistry; }
    void setTypeRegistry(TypeRegistry* r) { m_typeRegistry = r; }

private:
    std::string m_name = "Inspector";
    SelectionService* m_selection = nullptr;
    TypeRegistry* m_typeRegistry = nullptr;
};

// ── HierarchyPanel ──────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::HierarchyPanel instead (U2).

class HierarchyPanel : public EditorPanel {
public:
    HierarchyPanel() = default;
    explicit HierarchyPanel(SelectionService* sel) : m_selection(sel) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Left; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Hierarchy", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        // Search filter display
        if (!m_searchFilter.empty()) {
            ui.drawText(bounds.x + 8.f, y, "Filter: " + m_searchFilter, theme.propertyLabel);
            y += 18.f;
        }
        // Entity list
        for (auto id : m_entityList) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "Entity #%u", id);
            bool selected = m_selection && m_selection->isSelected(id);
            if (selected) {
                ui.drawRect({bounds.x, y, bounds.w, 18.f}, theme.selectionHighlight);
            }
            ui.drawText(bounds.x + 16.f, y + 2.f, buf,
                         selected ? theme.selectionBorder : theme.panelText);
            y += 18.f;
            if (y > bounds.y + bounds.h - 4.f) break;
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Hierarchy", bounds);
        ctx.textInput("Search", m_searchFilter);
        ctx.separator();
        float contentH = static_cast<float>(m_entityList.size()) * 20.f;
        Rect scrollR{bounds.x, bounds.y + 60.f, bounds.w, bounds.h - 64.f};
        ctx.beginScrollArea("hierarchy_scroll", scrollR, contentH);
        for (auto id : m_entityList) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "Entity #%u", id);
            bool expanded = true;
            ctx.treeNode(buf, expanded);
        }
        ctx.endScrollArea();
        ctx.endPanel();
    }

    [[nodiscard]] SelectionService* selectionService() const { return m_selection; }
    void setSelectionService(SelectionService* s) { m_selection = s; }

    [[nodiscard]] const std::string& searchFilter() const { return m_searchFilter; }
    void setSearchFilter(const std::string& f) { m_searchFilter = f; }

    void setEntityList(const std::vector<EntityID>& ids) { m_entityList = ids; }
    [[nodiscard]] const std::vector<EntityID>& entityList() const { return m_entityList; }

private:
    std::string m_name = "Hierarchy";
    SelectionService* m_selection = nullptr;
    std::string m_searchFilter;
    std::vector<EntityID> m_entityList;
};

// ── ConsolePanel ─────────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::ConsolePanel instead (U4).

enum class ConsoleMessageLevel : uint8_t { Info, Warning, Error };

struct ConsoleMessage {
    std::string text;
    ConsoleMessageLevel level = ConsoleMessageLevel::Info;
    float timestamp = 0.f;
};

class ConsolePanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Bottom; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, 0x1A1A1AFF);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Console", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        for (auto& msg : m_messages) {
            if (y > bounds.y + bounds.h - 4.f) break;
            uint32_t color = theme.panelText;
            if (msg.level == ConsoleMessageLevel::Warning) color = 0xE8A435FF;
            if (msg.level == ConsoleMessageLevel::Error)   color = 0xF44747FF;
            ui.drawText(bounds.x + 8.f, y, msg.text, color);
            y += 16.f;
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Console", bounds);
        ctx.beginHorizontal();
        if (ctx.button("Clear")) clearMessages();
        ctx.endHorizontal();
        ctx.separator();
        float contentH = static_cast<float>(m_messages.size()) * 18.f;
        Rect scrollR{bounds.x, bounds.y + 60.f, bounds.w, bounds.h - 64.f};
        ctx.beginScrollArea("console_scroll", scrollR, contentH);
        for (auto& msg : m_messages) {
            uint32_t color = 0;
            if (msg.level == ConsoleMessageLevel::Warning) color = 0xE8A435FF;
            else if (msg.level == ConsoleMessageLevel::Error) color = 0xF44747FF;
            ctx.label(msg.text, color);
        }
        ctx.endScrollArea();
        ctx.endPanel();
    }

    void addMessage(const std::string& text, ConsoleMessageLevel level, float timestamp) {
        ConsoleMessage msg;
        msg.text = text;
        msg.level = level;
        msg.timestamp = timestamp;
        m_messages.push_back(std::move(msg));
        if (m_messages.size() > kMaxMessages) {
            m_messages.erase(m_messages.begin());
        }
    }

    /// Called by the log sink to feed NF_LOG_* messages into the console.
    void addLogMessage(LogLevel level, std::string_view category, std::string_view message) {
        ConsoleMessageLevel cl = ConsoleMessageLevel::Info;
        if (level == LogLevel::Warn)  cl = ConsoleMessageLevel::Warning;
        if (level == LogLevel::Error || level == LogLevel::Fatal) cl = ConsoleMessageLevel::Error;
        std::string text = std::string("[") + std::string(category) + "] " + std::string(message);
        addMessage(text, cl, 0.f);
    }

    void clearMessages() { m_messages.clear(); }
    [[nodiscard]] size_t messageCount() const { return m_messages.size(); }
    [[nodiscard]] const std::vector<ConsoleMessage>& messages() const { return m_messages; }

    static constexpr size_t kMaxMessages = 1000;

private:
    std::string m_name = "Console";
    std::vector<ConsoleMessage> m_messages;
};

// ── ContentBrowserPanel ──────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::ContentBrowserPanel instead (U3).

enum class ContentViewMode : uint8_t { Grid, List };

class ContentBrowserPanel : public EditorPanel {
public:
    ContentBrowserPanel() = default;
    explicit ContentBrowserPanel(ContentBrowser* browser) : m_browser(browser) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Left; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Content Browser", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        if (m_browser) {
            ui.drawText(bounds.x + 8.f, y, m_browser->currentPath(), theme.propertyLabel);
            y += 18.f;
            for (auto& entry : m_browser->entries()) {
                if (y > bounds.y + bounds.h - 4.f) break;
                std::string icon = entry.isDirectory ? "[D] " : "[F] ";
                ui.drawText(bounds.x + 16.f, y, icon + entry.name, theme.panelText);
                y += 18.f;
            }
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Content Browser", bounds);
        if (m_browser) {
            ctx.label(m_browser->currentPath());
            ctx.beginHorizontal();
            if (ctx.button("Up")) m_browser->navigateUp();
            ctx.endHorizontal();
            ctx.separator();
            for (auto& entry : m_browser->entries()) {
                std::string icon = entry.isDirectory ? "[D] " : "[F] ";
                if (ctx.button(icon + entry.name)) {
                    if (entry.isDirectory)
                        m_browser->navigateTo(entry.name);
                }
            }
        }
        ctx.endPanel();
    }

    [[nodiscard]] ContentBrowser* contentBrowser() const { return m_browser; }
    void setContentBrowser(ContentBrowser* b) { m_browser = b; }

    [[nodiscard]] ContentViewMode viewMode() const { return m_viewMode; }
    void setViewMode(ContentViewMode m) { m_viewMode = m; }

private:
    std::string m_name = "ContentBrowser";
    ContentBrowser* m_browser = nullptr;
    ContentViewMode m_viewMode = ContentViewMode::Grid;
};

// ── EditorToolbar ────────────────────────────────────────────────

struct ToolbarItem {
    std::string name;
    std::string icon;
    std::string tooltip;
    std::function<void()> action;
    bool enabled = true;
    bool isSeparator = false;
};

class EditorToolbar {
public:
    void addItem(const std::string& name, const std::string& icon,
                 const std::string& tooltip, std::function<void()> action, bool enabled = true) {
        ToolbarItem item;
        item.name = name;
        item.icon = icon;
        item.tooltip = tooltip;
        item.action = std::move(action);
        item.enabled = enabled;
        item.isSeparator = false;
        m_items.push_back(std::move(item));
    }

    void addSeparator() {
        ToolbarItem sep;
        sep.isSeparator = true;
        m_items.push_back(std::move(sep));
    }

    [[nodiscard]] const std::vector<ToolbarItem>& items() const { return m_items; }
    [[nodiscard]] size_t itemCount() const { return m_items.size(); }

private:
    std::vector<ToolbarItem> m_items;
};

// ── Project Indexer ──────────────────────────────────────────────

enum class SourceFileType : uint8_t {
    Header, Source, Shader, Script, Data, Config, Unknown
};

struct IndexedFile {
    std::string path;
    SourceFileType fileType = SourceFileType::Unknown;
    std::string moduleName;
    uint32_t lineCount = 0;
    uint64_t lastModified = 0;
    std::vector<std::string> symbols;
};

class ProjectIndexer {
public:
    void indexDirectory(const std::string& rootPath) {
        namespace fs = std::filesystem;
        if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
            NF_LOG_WARN("IDE", "Directory not found: " + rootPath);
            return;
        }
        for (auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            SourceFileType ft = classifyExtension(ext);
            auto rel = fs::relative(entry.path(), rootPath).string();
            std::string mod;
            auto sep = rel.find_first_of("/\\");
            if (sep != std::string::npos) mod = rel.substr(0, sep);
            IndexedFile f;
            f.path = entry.path().string();
            f.fileType = ft;
            f.moduleName = mod;
            f.lineCount = 0;
            f.lastModified = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    entry.last_write_time().time_since_epoch()).count());
            m_files.push_back(std::move(f));
        }
        NF_LOG_INFO("IDE", "Indexed " + std::to_string(m_files.size()) + " files from " + rootPath);
    }

    void indexFile(const std::string& path, SourceFileType type, const std::string& moduleName) {
        IndexedFile f;
        f.path = path;
        f.fileType = type;
        f.moduleName = moduleName;
        m_files.push_back(std::move(f));
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByType(SourceFileType type) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.fileType == type) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByModule(const std::string& moduleName) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.moduleName == moduleName) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByName(const std::string& nameSubstring) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.path.find(nameSubstring) != std::string::npos) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] const std::vector<IndexedFile>& allFiles() const { return m_files; }
    [[nodiscard]] size_t fileCount() const { return m_files.size(); }

    void clear() { m_files.clear(); }

    void addSymbol(const std::string& filePath, const std::string& symbolName) {
        for (auto& f : m_files) {
            if (f.path == filePath) {
                f.symbols.push_back(symbolName);
                return;
            }
        }
    }

    [[nodiscard]] std::vector<const IndexedFile*> findSymbol(const std::string& symbolName) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            for (auto& s : f.symbols) {
                if (s == symbolName) {
                    result.push_back(&f);
                    break;
                }
            }
        }
        return result;
    }

private:
    static SourceFileType classifyExtension(const std::string& ext) {
        if (ext == ".h" || ext == ".hpp") return SourceFileType::Header;
        if (ext == ".cpp" || ext == ".cc" || ext == ".cxx") return SourceFileType::Source;
        if (ext == ".glsl" || ext == ".hlsl") return SourceFileType::Shader;
        if (ext == ".lua" || ext == ".py") return SourceFileType::Script;
        if (ext == ".json") return SourceFileType::Data;
        if (ext == ".cfg" || ext == ".ini") return SourceFileType::Config;
        return SourceFileType::Unknown;
    }

    std::vector<IndexedFile> m_files;
};

// ── Code Navigation ─────────────────────────────────────────────

enum class SymbolKind : uint8_t {
    Function, Class, Struct, Enum, Variable, Namespace, Macro, Type, Unknown
};

struct NavigationTarget {
    std::string filePath;
    uint32_t line = 0;
    uint32_t column = 0;
    std::string symbolName;
    SymbolKind kind = SymbolKind::Unknown;
};

struct NavigationEntry {
    std::string symbol;
    SymbolKind kind = SymbolKind::Unknown;
    std::string filePath;
    uint32_t line = 0;
};

class CodeNavigator {
public:
    void addEntry(NavigationEntry entry) {
        m_entries.push_back(std::move(entry));
    }

    [[nodiscard]] std::optional<NavigationTarget> goToDefinition(const std::string& symbolName) const {
        for (auto& e : m_entries) {
            if (e.symbol == symbolName) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                return t;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<NavigationTarget> findReferences(const std::string& symbolName) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.symbol == symbolName) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<NavigationTarget> findSymbolsByKind(SymbolKind kind) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.kind == kind) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<NavigationTarget> searchSymbols(const std::string& query) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.symbol.find(query) != std::string::npos) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    void clear() { m_entries.clear(); }

private:
    std::vector<NavigationEntry> m_entries;
};

// ── Breadcrumb Trail ─────────────────────────────────────────────

struct BreadcrumbItem {
    std::string label;
    std::string filePath;
    uint32_t line = 0;
};

class BreadcrumbTrail {
public:
    void push(BreadcrumbItem item) {
        if (m_trail.size() >= maxDepth) {
            m_trail.erase(m_trail.begin());
        }
        m_trail.push_back(std::move(item));
    }

    std::optional<BreadcrumbItem> pop() {
        if (m_trail.empty()) return std::nullopt;
        auto item = std::move(m_trail.back());
        m_trail.pop_back();
        return item;
    }

    [[nodiscard]] const BreadcrumbItem* current() const {
        if (m_trail.empty()) return nullptr;
        return &m_trail.back();
    }

    [[nodiscard]] const std::vector<BreadcrumbItem>& trail() const { return m_trail; }
    [[nodiscard]] size_t depth() const { return m_trail.size(); }
    void clear() { m_trail.clear(); }

private:
    std::vector<BreadcrumbItem> m_trail;
    static constexpr size_t maxDepth = 50;
};

// ── IDE Panel ────────────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::IDEPanel instead (U5).

class IDEPanel : public EditorPanel {
public:
    IDEPanel(ProjectIndexer* indexer, CodeNavigator* navigator)
        : m_indexer(indexer), m_navigator(navigator) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "IDE", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 30.f;
        if (!searchQuery.empty()) {
            ui.drawText(bounds.x + 8.f, y, "Search: " + searchQuery, theme.propertyLabel);
            y += 18.f;
        }
        for (auto& r : searchResults) {
            if (y > bounds.y + bounds.h - 4.f) break;
            ui.drawText(bounds.x + 8.f, y, r.symbolName + " @ " + r.filePath, theme.panelText);
            y += 16.f;
        }
    }

    [[nodiscard]] ProjectIndexer* indexer() const { return m_indexer; }
    [[nodiscard]] CodeNavigator* navigator() const { return m_navigator; }

    std::string searchQuery;
    std::vector<NavigationTarget> searchResults;

private:
    std::string m_name = "IDE";
    ProjectIndexer* m_indexer = nullptr;
    CodeNavigator* m_navigator = nullptr;
};

// ── IDE Service ──────────────────────────────────────────────────

class IDEService {
public:
    void init() {
        m_indexer = ProjectIndexer{};
        m_navigator = CodeNavigator{};
        m_breadcrumbs = BreadcrumbTrail{};
        m_initialized = true;
        NF_LOG_INFO("IDE", "IDEService initialized");
    }

    void shutdown() {
        m_indexer.clear();
        m_navigator.clear();
        m_breadcrumbs.clear();
        m_initialized = false;
        NF_LOG_INFO("IDE", "IDEService shutdown");
    }

    [[nodiscard]] ProjectIndexer& indexer() { return m_indexer; }
    [[nodiscard]] CodeNavigator& navigator() { return m_navigator; }
    [[nodiscard]] BreadcrumbTrail& breadcrumbs() { return m_breadcrumbs; }

    void navigateTo(const std::string& filePath, uint32_t line, const std::string& symbolName) {
        BreadcrumbItem item;
        item.label = symbolName;
        item.filePath = filePath;
        item.line = line;
        m_breadcrumbs.push(std::move(item));
        NF_LOG_INFO("IDE", "Navigate to " + symbolName + " at " + filePath + ":" + std::to_string(line));
    }

    bool goBack() {
        auto item = m_breadcrumbs.pop();
        if (!item) return false;
        NF_LOG_INFO("IDE", "Go back to " + item->label);
        return true;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

private:
    ProjectIndexer m_indexer;
    CodeNavigator m_navigator;
    BreadcrumbTrail m_breadcrumbs;
    bool m_initialized = false;
};

// ── MenuBar ──────────────────────────────────────────────────────

struct MenuItem {
    std::string name;
    std::string command;    // command name to execute via EditorCommandRegistry
    std::string hotkey;
    bool enabled = true;
    bool isSeparator = false;
    std::vector<MenuItem> children;

    static MenuItem separator() {
        MenuItem m;
        m.isSeparator = true;
        return m;
    }
};

struct MenuCategory {
    std::string name;
    std::vector<MenuItem> items;

    void addItem(const std::string& itemName, const std::string& command,
                 const std::string& hotkey = "", bool enabled = true) {
        MenuItem m;
        m.name = itemName;
        m.command = command;
        m.hotkey = hotkey;
        m.enabled = enabled;
        items.push_back(std::move(m));
    }

    void addSeparator() { items.push_back(MenuItem::separator()); }
};

class MenuBar {
public:
    MenuCategory& addCategory(const std::string& name) {
        m_categories.push_back(MenuCategory{name, {}});
        return m_categories.back();
    }

    MenuCategory* findCategory(const std::string& name) {
        for (auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    const MenuCategory* findCategory(const std::string& name) const {
        for (auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    const std::vector<MenuCategory>& categories() const { return m_categories; }
    size_t categoryCount() const { return m_categories.size(); }

private:
    std::vector<MenuCategory> m_categories;
};

// ── EditorStatusBar ──────────────────────────────────────────────

struct StatusBarState {
    std::string modeName;
    std::string worldPath;
    bool isDirty = false;
    int selectionCount = 0;
    float fps = 0.f;
    std::string statusMessage;
};

class EditorStatusBar {
public:
    void update(const std::string& mode, const std::string& worldPath,
                bool dirty, int selection, float fps,
                const std::string& msg = "") {
        m_state.modeName = mode;
        m_state.worldPath = worldPath;
        m_state.isDirty = dirty;
        m_state.selectionCount = selection;
        m_state.fps = fps;
        m_state.statusMessage = msg;
    }

    const StatusBarState& state() const { return m_state; }

    std::string buildText() const {
        std::string s = m_state.modeName;
        if (!m_state.worldPath.empty()) {
            s += "  |  " + m_state.worldPath;
            if (m_state.isDirty) s += " *";
        }
        if (m_state.selectionCount > 0)
            s += "  |  " + std::to_string(m_state.selectionCount) + " selected";
        s += "  |  " + std::to_string(static_cast<int>(m_state.fps)) + " FPS";
        if (!m_state.statusMessage.empty())
            s += "  |  " + m_state.statusMessage;
        return s;
    }

private:
    StatusBarState m_state;
};

// ── Notification System ──────────────────────────────────────────

enum class NotificationType : uint8_t { Info, Success, Warning, Error };

struct EditorNotification {
    NotificationType type = NotificationType::Info;
    std::string message;
    float ttl = 3.f;      // seconds before it expires
    float elapsed = 0.f;

    bool isExpired() const { return elapsed >= ttl; }
    float progress() const { return ttl > 0.f ? std::min(elapsed / ttl, 1.f) : 1.f; }
};

class NotificationQueue {
public:
    void push(NotificationType type, const std::string& message, float ttl = 3.f) {
        EditorNotification n;
        n.type = type;
        n.message = message;
        n.ttl = ttl;
        n.elapsed = 0.f;
        m_queue.push_back(std::move(n));
    }

    void tick(float dt) {
        for (auto& n : m_queue) n.elapsed += dt;
        m_queue.erase(
            std::remove_if(m_queue.begin(), m_queue.end(),
                           [](const EditorNotification& n){ return n.isExpired(); }),
            m_queue.end());
    }

    const EditorNotification* current() const {
        return m_queue.empty() ? nullptr : &m_queue.front();
    }

    bool hasActive() const { return !m_queue.empty(); }
    int count() const { return (int)m_queue.size(); }
    void clear() { m_queue.clear(); }

private:
    std::vector<EditorNotification> m_queue;
};

// ── Orbital Editor Camera ─────────────────────────────────────────

struct EditorCameraOrbit {
    Vec3  target    = {0.f, 0.f, 0.f};
    float distance  = 10.f;
    float yaw       = -90.f;   // degrees
    float pitch     = 30.f;    // degrees
    float fovDeg    = 60.f;
    float nearPlane = 0.1f;
    float farPlane  = 1000.f;

    Vec3 computePosition() const {
        float y = yaw   * (3.14159265f / 180.f);
        float p = pitch * (3.14159265f / 180.f);
        return Vec3{
            target.x + distance * std::cos(y) * std::cos(p),
            target.y + distance * std::sin(p),
            target.z + distance * std::sin(y) * std::cos(p)
        };
    }

    void orbit(float dyaw, float dpitch) {
        yaw   += dyaw;
        pitch += dpitch;
        if (pitch >  89.f) pitch =  89.f;
        if (pitch < -89.f) pitch = -89.f;
    }

    void zoom(float delta) {
        distance -= delta;
        if (distance < 0.5f) distance = 0.5f;
    }

    void pan(float dx, float dy) {
        float y = yaw * (3.14159265f / 180.f);
        Vec3 right{std::cos(y), 0.f, std::sin(y)};
        Vec3 up{0.f, 1.f, 0.f};
        target = target + right * (-dx * distance * 0.001f);
        target = target + up    * ( dy * distance * 0.001f);
    }

    // aspectRatio is not stored on Camera; it is passed per-frame to projectionMatrix().
    Camera buildCamera(float aspectRatio) const {
        Camera cam;
        cam.position = computePosition();
        cam.target   = target;
        cam.up       = Vec3{0.f, 1.f, 0.f};
        cam.fov      = fovDeg;
        cam.nearPlane = nearPlane;
        cam.farPlane  = farPlane;
        (void)aspectRatio;  // stored externally by caller for use with projectionMatrix()
        return cam;
    }
};

// ── Gizmo ─────────────────────────────────────────────────────────

enum class GizmoMode : uint8_t { Translate, Rotate, Scale };
enum class GizmoAxis : uint8_t { None, X, Y, Z, XY, YZ, XZ, All };

struct GizmoState {
    GizmoMode mode       = GizmoMode::Translate;
    GizmoAxis activeAxis = GizmoAxis::None;
    bool isDragging      = false;
    bool snapEnabled     = false;
    float snapValue      = 0.25f;   // world units for translate, degrees for rotate

    void activate(GizmoAxis axis) { activeAxis = axis; isDragging = true; }
    void deactivate() { activeAxis = GizmoAxis::None; isDragging = false; }
    void setMode(GizmoMode m) { mode = m; deactivate(); }
};

// ── Editor Settings ───────────────────────────────────────────────

struct SnapSettings {
    float gridSize  = 0.25f;   // world units
    float angleStep = 15.f;    // degrees
    float scaleStep = 0.1f;
    bool  enabled   = false;
};

struct EditorSettings {
    bool        darkMode             = true;
    SnapSettings snap;
    bool        showGrid             = true;
    bool        showGizmos           = true;
    float       cameraSpeed          = 10.f;
    bool        autosave             = true;
    float       autosaveIntervalSecs = 300.f;
    int         undoHistorySize      = 100;
};

class EditorSettingsService {
public:
    void reset() { m_settings = EditorSettings{}; }

    EditorSettings& settings() { return m_settings; }
    const EditorSettings& settings() const { return m_settings; }

    void applyTheme(EditorTheme& theme) const {
        if (m_settings.darkMode) theme = EditorTheme::dark();
        else                     theme = EditorTheme::light();
    }

    void setDarkMode(bool dark) { m_settings.darkMode = dark; }
    void setShowGrid(bool show) { m_settings.showGrid = show; }
    void setSnapEnabled(bool on) { m_settings.snap.enabled = on; }
    void setCameraSpeed(float s) { m_settings.cameraSpeed = s; }

private:
    EditorSettings m_settings;
};

// ── Hotkey Dispatcher ────────────────────────────────────────────

struct HotkeyBinding {
    std::string hotkey;      // e.g. "Ctrl+Z", "F12", "Ctrl+Shift+S"
    std::string commandName;
};

class HotkeyDispatcher {
public:
    void bind(const std::string& hotkey, const std::string& commandName) {
        m_bindings.push_back({hotkey, commandName});
    }

    void unbind(const std::string& hotkey) {
        m_bindings.erase(
            std::remove_if(m_bindings.begin(), m_bindings.end(),
                           [&](const HotkeyBinding& b){ return b.hotkey == hotkey; }),
            m_bindings.end());
    }

    // Returns command name matched for a given hotkey string (empty if none)
    std::string findCommand(const std::string& hotkey) const {
        for (auto& b : m_bindings)
            if (b.hotkey == hotkey) return b.commandName;
        return {};
    }

    // Dispatch all matching bindings given a pressed hotkey string.
    // Returns number of commands dispatched.
    int dispatch(const std::string& hotkey, EditorCommandRegistry& commands) {
        int count = 0;
        for (auto& b : m_bindings) {
            if (b.hotkey == hotkey) {
                commands.executeCommand(b.commandName);
                ++count;
            }
        }
        return count;
    }

    void loadDefaults(EditorCommandRegistry& commands) {
        // Mirror the hotkeys already registered on commands
        auto names = commands.allCommandNames();
        for (auto& n : names) {
            if (auto* info = commands.findCommand(n)) {
                if (!info->hotkey.empty())
                    bind(info->hotkey, n);
            }
        }
    }

    const std::vector<HotkeyBinding>& bindings() const { return m_bindings; }
    int bindingCount() const { return (int)m_bindings.size(); }

private:
    std::vector<HotkeyBinding> m_bindings;
};

// ── Graph Editor Panel ───────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::GraphEditorPanel instead (U6).

class GraphEditorPanel : public EditorPanel {
public:
    explicit GraphEditorPanel(GraphVM* vm = nullptr) : m_graphVM(vm) {}

    const std::string& name() const override { return m_name; }
    DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}

    void render(UIRenderer& ui, const Rect& bounds,
                const EditorTheme& theme) override {
        // Background
        ui.drawRect(bounds, theme.panelBackground);

        if (!m_currentGraph) {
            ui.drawText(bounds.x + 8.f, bounds.y + 8.f, "No graph open", theme.panelText);
            return;
        }

        // Draw nodes as simple labelled rectangles
        for (const auto& node : m_currentGraph->nodes()) {
            Rect nr{node.position.x + bounds.x, node.position.y + bounds.y, 120.f, 60.f};
            uint32_t nodeColor = (m_selectedNodeId >= 0 && node.id == static_cast<uint32_t>(m_selectedNodeId))
                                 ? theme.selectionHighlight : theme.toolbarBackground;
            ui.drawRect(nr, nodeColor);
            ui.drawRectOutline(nr, theme.panelText, 1.f);
            ui.drawText(nr.x + 4.f, nr.y + 4.f, node.name, theme.panelText);
        }

        // Draw link count annotation
        float ly = bounds.y + bounds.h - 20.f;
        ui.drawText(bounds.x + 4.f, ly,
                           std::to_string(m_currentGraph->links().size()) + " link(s)",
                           theme.propertyLabel);
    }

    void setGraphVM(GraphVM* vm) { m_graphVM = vm; }
    GraphVM* graphVM() const { return m_graphVM; }

    // Create a brand-new Graph and make it the current one.
    void newGraph(GraphType type, const std::string& graphName) {
        m_ownedGraph = std::make_unique<Graph>();
        m_currentGraph = m_ownedGraph.get();
        m_currentGraphName = graphName;
        m_currentGraphType = type;
        m_nextNodeId = 1;
        m_selectedNodeId = -1;
        NF_LOG_INFO("Editor", "GraphEditorPanel: new graph '" + graphName + "'");
    }

    // Open an externally-owned graph (e.g. loaded from disk).
    bool openGraph(const std::string& graphName) {
        if (!m_graphVM) return false;
        m_currentGraphName = graphName;
        NF_LOG_INFO("Editor", "GraphEditorPanel: opened graph '" + graphName + "'");
        return true;
    }

    // Add a node to the current graph; returns the assigned node ID or -1 on failure.
    int addNode(const std::string& nodeName, GraphType type = GraphType::World,
                float x = 0.f, float y = 0.f) {
        if (!m_currentGraph) return -1;
        GraphNode n;
        n.id       = m_nextNodeId++;
        n.name     = nodeName;
        n.type     = type;
        n.position = {x, y};
        m_currentGraph->addNode(n);
        NF_LOG_INFO("Editor", "GraphEditorPanel: added node '" + nodeName + "' id=" +
                    std::to_string(n.id));
        return n.id;
    }

    // Remove a node (and its connected links) from the current graph.
    bool removeNode(int nodeId) {
        if (!m_currentGraph) return false;
        if (m_currentGraph->findNode(nodeId) == nullptr) return false;
        m_currentGraph->removeNode(nodeId);
        if (m_selectedNodeId == nodeId) m_selectedNodeId = -1;
        return true;
    }

    // Add a link between two node ports.
    bool addLink(int srcNode, uint32_t srcPort, int dstNode, uint32_t dstPort) {
        if (!m_currentGraph) return false;
        GraphLink lk;
        lk.sourceNode = srcNode;
        lk.sourcePort = srcPort;
        lk.targetNode = dstNode;
        lk.targetPort = dstPort;
        m_currentGraph->addLink(lk);
        return true;
    }

    // Compile the current graph and load the resulting program into the VM.
    bool compileAndLoad() {
        if (!m_currentGraph || !m_graphVM) return false;
        auto prog = GraphCompiler::compile(*m_currentGraph);
        m_graphVM->loadProgram(prog);
        NF_LOG_INFO("Editor", "GraphEditorPanel: compiled graph '" + m_currentGraphName + "'");
        return true;
    }

    [[nodiscard]] Graph* currentGraph() { return m_currentGraph; }
    [[nodiscard]] const Graph* currentGraph() const { return m_currentGraph; }
    const std::string& currentGraphName() const { return m_currentGraphName; }
    bool hasOpenGraph() const { return m_currentGraph != nullptr; }

    int selectedNodeId() const { return m_selectedNodeId; }
    void selectNode(int id) { m_selectedNodeId = id; }
    void clearSelection() { m_selectedNodeId = -1; }

    int nodeCount() const { return m_currentGraph ? (int)m_currentGraph->nodes().size() : 0; }
    int linkCount() const { return m_currentGraph ? (int)m_currentGraph->links().size() : 0; }

private:
    std::string m_name = "GraphEditor";
    GraphVM* m_graphVM = nullptr;
    std::unique_ptr<Graph> m_ownedGraph;
    Graph* m_currentGraph = nullptr;
    std::string m_currentGraphName;
    GraphType m_currentGraphType = GraphType::World;
    int m_selectedNodeId = -1;
    int m_nextNodeId = 1;
};

// ── Tool Window Manager ──────────────────────────────────────────

enum class ToolLaunchMode : uint8_t { Embedded, External, Docked };

struct ToolDescriptor {
    std::string name;
    std::string executable;
    std::string icon;
    ToolLaunchMode mode = ToolLaunchMode::External;
    bool isRunning = false;
};

class ToolWindowManager {
public:
    void registerTool(ToolDescriptor desc) {
        m_tools.push_back(std::move(desc));
        NF_LOG_INFO("Editor", "ToolWindowManager: registered tool '" + m_tools.back().name + "'");
    }

    bool launchTool(const std::string& name) {
        for (auto& t : m_tools) {
            if (t.name == name) {
                t.isRunning = true;
                NF_LOG_INFO("Editor", "ToolWindowManager: launched '" + name + "'");
                return true;
            }
        }
        return false;
    }

    void stopTool(const std::string& name) {
        for (auto& t : m_tools) {
            if (t.name == name) { t.isRunning = false; return; }
        }
    }

    [[nodiscard]] const ToolDescriptor* findTool(const std::string& name) const {
        for (auto& t : m_tools) if (t.name == name) return &t;
        return nullptr;
    }

    [[nodiscard]] const std::vector<ToolDescriptor>& tools() const { return m_tools; }
    [[nodiscard]] size_t toolCount() const { return m_tools.size(); }

    [[nodiscard]] size_t runningCount() const {
        size_t c = 0;
        for (auto& t : m_tools) if (t.isRunning) ++c;
        return c;
    }

private:
    std::vector<ToolDescriptor> m_tools;
};

// ── Pipeline Monitor Panel ───────────────────────────────────────

struct PipelineEventEntry {
    std::string type;
    std::string source;
    std::string details;
    float timestamp = 0.f;
};

// DEPRECATED: Use NF::UI::AtlasUI::PipelineMonitorPanel instead.
class PipelineMonitorPanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Bottom; }
    void update(float /*dt*/) override {}

    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Pipeline Monitor", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        for (auto& ev : m_events) {
            if (y > bounds.y + bounds.h - 4.f) break;
            std::string line = "[" + ev.type + "] " + ev.source + ": " + ev.details;
            ui.drawText(bounds.x + 8.f, y, line, theme.panelText);
            y += 16.f;
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Pipeline Monitor", bounds);
        ctx.beginHorizontal();
        if (ctx.button("Clear")) m_events.clear();
        if (ctx.button("Refresh")) { /* poll pipeline */ }
        ctx.endHorizontal();
        ctx.separator();
        float contentH = static_cast<float>(m_events.size()) * 18.f;
        Rect scrollR{bounds.x, bounds.y + 60.f, bounds.w, bounds.h - 64.f};
        ctx.beginScrollArea("pipeline_scroll", scrollR, contentH);
        for (auto& ev : m_events) {
            std::string line = "[" + ev.type + "] " + ev.source + ": " + ev.details;
            ctx.label(line);
        }
        ctx.endScrollArea();
        ctx.endPanel();
    }

    void addEvent(const std::string& type, const std::string& source,
                  const std::string& details, float timestamp) {
        PipelineEventEntry e;
        e.type = type;
        e.source = source;
        e.details = details;
        e.timestamp = timestamp;
        m_events.push_back(std::move(e));
        if (m_events.size() > 500) m_events.erase(m_events.begin());
    }

    void clearEvents() { m_events.clear(); }
    [[nodiscard]] size_t eventCount() const { return m_events.size(); }
    [[nodiscard]] const std::vector<PipelineEventEntry>& events() const { return m_events; }

private:
    std::string m_name = "PipelineMonitor";
    std::vector<PipelineEventEntry> m_events;
};

// ── Frame Stats ──────────────────────────────────────────────────

struct FrameStats {
    float fps          = 0.f;
    float frameTimeMs  = 0.f;
    float updateTimeMs = 0.f;
    float renderTimeMs = 0.f;
    uint64_t frameCount = 0;
};

class FrameStatsTracker {
public:
    void beginFrame(float dtSeconds) {
        m_frameTimeMs = dtSeconds * 1000.f;
        // Exponential moving average for FPS
        float newFps = dtSeconds > 0.f ? 1.f / dtSeconds : 0.f;
        m_stats.fps = m_stats.fps * 0.9f + newFps * 0.1f;
        m_stats.frameTimeMs = m_frameTimeMs;
        ++m_stats.frameCount;
        m_updateStart = m_stats.frameCount;  // reuse as a simple "step" marker
    }

    void recordUpdateTime(float ms) { m_stats.updateTimeMs = ms; }
    void recordRenderTime(float ms) { m_stats.renderTimeMs = ms; }

    const FrameStats& stats() const { return m_stats; }

private:
    FrameStats m_stats;
    float m_frameTimeMs = 0.f;
    uint64_t m_updateStart = 0;
};

// ── M2/S1: Dev World Editing ─────────────────────────────────────

// Integer 3D vector for voxel coordinates.
struct Vec3i {
    int x = 0, y = 0, z = 0;
    bool operator==(const Vec3i& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vec3i& o) const { return !(*this == o); }
};

// ── PCG Tuning ──────────────────────────────────────────────────

struct NoiseParams {
    float frequency   = 1.0f;
    float amplitude   = 1.0f;
    int   octaves     = 4;
    float lacunarity  = 2.0f;
    float persistence = 0.5f;
    int   seed        = 42;

    bool operator==(const NoiseParams& o) const {
        return frequency == o.frequency && amplitude == o.amplitude &&
               octaves == o.octaves && lacunarity == o.lacunarity &&
               persistence == o.persistence && seed == o.seed;
    }
    bool operator!=(const NoiseParams& o) const { return !(*this == o); }
};

struct PCGPreset {
    std::string name;
    NoiseParams params;
};

class PCGTuningPanel : public EditorPanel {
public:
    PCGTuningPanel() { m_name = "PCGTuning"; }

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Right; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    void setNoiseParams(const NoiseParams& p) { m_params = p; m_dirty = true; }
    [[nodiscard]] const NoiseParams& noiseParams() const { return m_params; }

    void addPreset(const PCGPreset& preset) { m_presets.push_back(preset); }

    bool removePreset(const std::string& presetName) {
        auto it = std::find_if(m_presets.begin(), m_presets.end(),
                               [&](const PCGPreset& p) { return p.name == presetName; });
        if (it == m_presets.end()) return false;
        m_presets.erase(it);
        return true;
    }

    bool applyPreset(const std::string& presetName) {
        auto it = std::find_if(m_presets.begin(), m_presets.end(),
                               [&](const PCGPreset& p) { return p.name == presetName; });
        if (it == m_presets.end()) return false;
        m_params = it->params;
        m_dirty = true;
        return true;
    }

    [[nodiscard]] size_t presetCount() const { return m_presets.size(); }
    [[nodiscard]] const std::vector<PCGPreset>& presets() const { return m_presets; }

    void setSeed(int seed) { m_params.seed = seed; m_dirty = true; }

    void randomizeSeed() {
        m_params.seed = static_cast<int>(std::hash<size_t>{}(
            static_cast<size_t>(m_params.seed) ^ 0x9E3779B97F4A7C15ULL));
        m_dirty = true;
    }

    void markDirty() { m_dirty = true; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

private:
    std::string m_name;
    NoiseParams m_params;
    std::vector<PCGPreset> m_presets;
    bool m_dirty = false;
};

// ── Entity Placement ────────────────────────────────────────────

struct PlacedEntity {
    EntityID    entityId     = INVALID_ENTITY;
    std::string templateName;
    Vec3        position;
    Vec3        rotation;
    Vec3        scale{1.f, 1.f, 1.f};
};

class EntityPlacementTool {
public:
    void setActiveTemplate(const std::string& tplName) { m_activeTemplate = tplName; }
    [[nodiscard]] const std::string& activeTemplate() const { return m_activeTemplate; }

    EntityID placeEntity(const Vec3& pos, const Vec3& rot = {}, const Vec3& scl = {1.f, 1.f, 1.f}) {
        PlacedEntity e;
        e.entityId = m_nextId++;
        e.templateName = m_activeTemplate;
        e.position = m_gridSnap ? snapToGrid(pos) : pos;
        e.rotation = rot;
        e.scale = scl;
        m_entities.push_back(e);
        return e.entityId;
    }

    void addEntity(const PlacedEntity& e) {
        m_entities.push_back(e);
        if (e.entityId >= m_nextId) m_nextId = e.entityId + 1;
    }

    bool removeEntity(EntityID id) {
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
                               [id](const PlacedEntity& pe) { return pe.entityId == id; });
        if (it == m_entities.end()) return false;
        m_entities.erase(it);
        return true;
    }

    [[nodiscard]] const std::vector<PlacedEntity>& placedEntities() const { return m_entities; }
    [[nodiscard]] size_t placedCount() const { return m_entities.size(); }

    void setGridSnap(bool enabled) { m_gridSnap = enabled; }
    void setGridSize(float size) { m_gridSize = size; }
    [[nodiscard]] bool isGridSnapEnabled() const { return m_gridSnap; }
    [[nodiscard]] float gridSize() const { return m_gridSize; }

    [[nodiscard]] Vec3 snapToGrid(const Vec3& v) const {
        if (m_gridSize <= 0.f) return v;
        return {
            std::round(v.x / m_gridSize) * m_gridSize,
            std::round(v.y / m_gridSize) * m_gridSize,
            std::round(v.z / m_gridSize) * m_gridSize
        };
    }

    void clear() { m_entities.clear(); }

private:
    std::vector<PlacedEntity> m_entities;
    std::string m_activeTemplate;
    EntityID m_nextId = 1;
    bool  m_gridSnap = false;
    float m_gridSize = 1.0f;
};

// ── Voxel Paint ─────────────────────────────────────────────────

enum class VoxelBrushShape : uint8_t { Sphere, Cube, Cylinder };

struct VoxelBrushSettings {
    VoxelBrushShape shape = VoxelBrushShape::Sphere;
    int      radius     = 1;
    uint32_t materialId = 0;
    float    strength   = 1.0f;
};

struct PaintStroke {
    std::vector<Vec3i> positions;
    uint32_t materialId = 0;
    VoxelBrushSettings brush;
};

class VoxelPaintTool {
public:
    void setBrush(const VoxelBrushSettings& b) { m_brush = b; }
    [[nodiscard]] const VoxelBrushSettings& brush() const { return m_brush; }

    void beginStroke() {
        m_currentStroke = PaintStroke{};
        m_currentStroke.materialId = m_brush.materialId;
        m_currentStroke.brush = m_brush;
        m_stroking = true;
    }

    void addToStroke(const Vec3i& pos) {
        if (m_stroking) m_currentStroke.positions.push_back(pos);
    }

    void endStroke() {
        if (m_stroking) {
            m_strokes.push_back(std::move(m_currentStroke));
            m_currentStroke = PaintStroke{};
            m_stroking = false;
        }
    }

    void addStroke(const PaintStroke& stroke) { m_strokes.push_back(stroke); }

    bool removeLastStroke() {
        if (m_strokes.empty()) return false;
        m_strokes.pop_back();
        return true;
    }

    [[nodiscard]] bool isStroking() const { return m_stroking; }
    [[nodiscard]] const std::vector<PaintStroke>& strokes() const { return m_strokes; }
    [[nodiscard]] size_t strokeCount() const { return m_strokes.size(); }

    void clear() {
        m_strokes.clear();
        m_stroking = false;
        m_currentStroke = PaintStroke{};
    }

    static constexpr size_t kMaxPaletteSize = 32;

    void setPaletteSlot(int slot, uint32_t materialId) {
        if (slot < 0 || static_cast<size_t>(slot) >= kMaxPaletteSize) return;
        if (static_cast<size_t>(slot) >= m_palette.size())
            m_palette.resize(static_cast<size_t>(slot) + 1, 0);
        m_palette[static_cast<size_t>(slot)] = materialId;
    }

    [[nodiscard]] uint32_t getPaletteSlot(int slot) const {
        if (slot < 0 || static_cast<size_t>(slot) >= m_palette.size()) return 0;
        return m_palette[static_cast<size_t>(slot)];
    }

    [[nodiscard]] size_t paletteSize() const { return m_palette.size(); }

    void setActivePaletteSlot(int slot) {
        m_activePaletteSlot = slot;
        if (slot >= 0 && static_cast<size_t>(slot) < m_palette.size())
            m_brush.materialId = m_palette[static_cast<size_t>(slot)];
    }
    [[nodiscard]] int activePaletteSlot() const { return m_activePaletteSlot; }

private:
    VoxelBrushSettings m_brush;
    PaintStroke m_currentStroke;
    bool m_stroking = false;
    std::vector<PaintStroke> m_strokes;
    std::vector<uint32_t> m_palette;
    int m_activePaletteSlot = -1;
};

// ── Editor Undo System ──────────────────────────────────────────

class PlaceEntityCommand : public ICommand {
public:
    PlaceEntityCommand(EntityPlacementTool& tool, PlacedEntity entity)
        : m_tool(tool), m_entity(std::move(entity)) {}

    void execute() override { m_tool.addEntity(m_entity); }
    void undo() override    { m_tool.removeEntity(m_entity.entityId); }
    [[nodiscard]] std::string description() const override {
        return "Place Entity " + m_entity.templateName;
    }

private:
    EntityPlacementTool& m_tool;
    PlacedEntity m_entity;
};

class RemoveEntityCommand : public ICommand {
public:
    RemoveEntityCommand(EntityPlacementTool& tool, EntityID id)
        : m_tool(tool), m_id(id) {
        for (auto& e : m_tool.placedEntities()) {
            if (e.entityId == id) { m_entity = e; break; }
        }
    }

    void execute() override { m_tool.removeEntity(m_id); }
    void undo() override    { m_tool.addEntity(m_entity); }
    [[nodiscard]] std::string description() const override {
        return "Remove Entity " + std::to_string(m_id);
    }

private:
    EntityPlacementTool& m_tool;
    EntityID m_id;
    PlacedEntity m_entity;
};

class PaintStrokeCommand : public ICommand {
public:
    PaintStrokeCommand(VoxelPaintTool& tool, PaintStroke stroke)
        : m_tool(tool), m_stroke(std::move(stroke)) {}

    void execute() override { m_tool.addStroke(m_stroke); }
    void undo() override    { m_tool.removeLastStroke(); }
    [[nodiscard]] std::string description() const override {
        return "Paint Stroke (" + std::to_string(m_stroke.positions.size()) + " voxels)";
    }

private:
    VoxelPaintTool& m_tool;
    PaintStroke m_stroke;
};

class PCGParamChangeCommand : public ICommand {
public:
    PCGParamChangeCommand(PCGTuningPanel& panel, NoiseParams oldParams, NoiseParams newParams)
        : m_panel(panel), m_old(std::move(oldParams)), m_new(std::move(newParams)) {}

    void execute() override { m_panel.setNoiseParams(m_new); }
    void undo() override    { m_panel.setNoiseParams(m_old); }
    [[nodiscard]] std::string description() const override { return "Change PCG Parameters"; }

private:
    PCGTuningPanel& m_panel;
    NoiseParams m_old;
    NoiseParams m_new;
};

class EditorUndoSystem {
public:
    explicit EditorUndoSystem(CommandStack& stack) : m_stack(stack) {}

    void executePlaceEntity(EntityPlacementTool& tool, const PlacedEntity& entity) {
        m_stack.execute(std::make_unique<PlaceEntityCommand>(tool, entity));
    }

    void executeRemoveEntity(EntityPlacementTool& tool, EntityID id) {
        m_stack.execute(std::make_unique<RemoveEntityCommand>(tool, id));
    }

    void executePaintStroke(VoxelPaintTool& tool, const PaintStroke& stroke) {
        m_stack.execute(std::make_unique<PaintStrokeCommand>(tool, stroke));
    }

    void executePCGChange(PCGTuningPanel& panel, const NoiseParams& oldP, const NoiseParams& newP) {
        m_stack.execute(std::make_unique<PCGParamChangeCommand>(panel, oldP, newP));
    }

    bool undo()  { return m_stack.undo(); }
    bool redo()  { return m_stack.redo(); }
    [[nodiscard]] bool canUndo() const { return m_stack.canUndo(); }
    [[nodiscard]] bool canRedo() const { return m_stack.canRedo(); }
    [[nodiscard]] size_t undoCount() const { return m_stack.undoCount(); }
    [[nodiscard]] size_t redoCount() const { return m_stack.redoCount(); }

private:
    CommandStack& m_stack;
};

// ── World Preview Service ───────────────────────────────────────

enum class PreviewState : uint8_t { Idle, Loading, Ready, Error };

class WorldPreviewService {
public:
    void loadPreview(const std::string& path) {
        m_worldPath = path;
        if (path.empty()) {
            m_state = PreviewState::Error;
            m_lastError = "Empty path";
            return;
        }
        m_state = PreviewState::Loading;
        m_state = PreviewState::Ready;
        m_dirty = false;
    }

    void unloadPreview() {
        m_worldPath.clear();
        m_state = PreviewState::Idle;
        m_dirty = false;
        m_lastError.clear();
    }

    [[nodiscard]] PreviewState state() const { return m_state; }
    [[nodiscard]] const std::string& worldPath() const { return m_worldPath; }

    void setViewCenter(const Vec3& center) { m_viewCenter = center; }
    [[nodiscard]] const Vec3& viewCenter() const { return m_viewCenter; }

    void setViewRadius(float r) { m_viewRadius = r; }
    [[nodiscard]] float viewRadius() const { return m_viewRadius; }

    void setDirty() { m_dirty = true; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

    [[nodiscard]] const std::string& lastError() const { return m_lastError; }

private:
    PreviewState m_state = PreviewState::Idle;
    std::string m_worldPath;
    Vec3 m_viewCenter;
    float m_viewRadius = 100.0f;
    bool m_dirty = false;
    std::string m_lastError;
};

// ── M3/S2 Play-in-Editor ──────────────────────────────────────────

enum class PlayState : uint8_t {
    Stopped = 0,
    Running,
    Paused
};

inline const char* playStateName(PlayState s) {
    switch (s) {
        case PlayState::Stopped: return "Stopped";
        case PlayState::Running: return "Running";
        case PlayState::Paused:  return "Paused";
    }
    return "Unknown";
}

/// Snapshot of the world state captured before Play-in-Editor starts,
/// so that stop restores everything.
struct EditorWorldSnapshot {
    std::string worldPath;
    std::vector<PlacedEntity> placedEntities;
    NoiseParams pcgParams;
    Vec3 cameraPosition;
    float cameraYaw   = 0.f;
    float cameraPitch  = 0.f;
    bool valid = false;

    void capture(const std::string& path,
                 const std::vector<PlacedEntity>& entities,
                 const NoiseParams& params,
                 const Vec3& camPos, float yaw, float pitch) {
        worldPath       = path;
        placedEntities  = entities;
        pcgParams       = params;
        cameraPosition  = camPos;
        cameraYaw       = yaw;
        cameraPitch     = pitch;
        valid           = true;
    }

    void invalidate() { valid = false; }
};

/// Manages an in-editor play session (PIE).
///
/// Lifecycle:
///   1. User presses Play → snapshot captured, state → Running
///   2. User presses Pause → state → Paused (time frozen)
///   3. User presses Stop → state → Stopped, world restored from snapshot
class EditorWorldSession {
public:
    [[nodiscard]] PlayState state() const { return m_state; }
    [[nodiscard]] float elapsedTime() const { return m_elapsed; }
    [[nodiscard]] uint64_t frameCount() const { return m_frames; }
    [[nodiscard]] const EditorWorldSnapshot& snapshot() const { return m_snapshot; }
    [[nodiscard]] bool hasSnapshot() const { return m_snapshot.valid; }

    /// Begin a play session — captures the current snapshot.
    bool start(const std::string& worldPath,
               const std::vector<PlacedEntity>& entities,
               const NoiseParams& params,
               const Vec3& camPos, float yaw, float pitch) {
        if (m_state != PlayState::Stopped) return false;
        m_snapshot.capture(worldPath, entities, params, camPos, yaw, pitch);
        m_state   = PlayState::Running;
        m_elapsed = 0.f;
        m_frames  = 0;
        return true;
    }

    bool pause() {
        if (m_state != PlayState::Running) return false;
        m_state = PlayState::Paused;
        return true;
    }

    bool resume() {
        if (m_state != PlayState::Paused) return false;
        m_state = PlayState::Running;
        return true;
    }

    bool stop() {
        if (m_state == PlayState::Stopped) return false;
        m_state = PlayState::Stopped;
        // Snapshot remains valid for restoration
        return true;
    }

    void tick(float dt) {
        if (m_state == PlayState::Running) {
            m_elapsed += dt;
            ++m_frames;
        }
    }

private:
    PlayState m_state = PlayState::Stopped;
    float m_elapsed   = 0.f;
    uint64_t m_frames = 0;
    EditorWorldSnapshot m_snapshot;
};

/// High-level Play-in-Editor controller that integrates with the rest of
/// the editor — captures/restores entity placement, PCG params, camera.
class PlayInEditorSystem {
public:
    PlayInEditorSystem() = default;

    explicit PlayInEditorSystem(EntityPlacementTool* placement,
                                PCGTuningPanel* pcg,
                                ViewportPanel* viewport)
        : m_placement(placement), m_pcg(pcg), m_viewport(viewport) {}

    void setPlacementTool(EntityPlacementTool* t) { m_placement = t; }
    void setPCGTuningPanel(PCGTuningPanel* p) { m_pcg = p; }
    void setViewportPanel(ViewportPanel* v) { m_viewport = v; }

    [[nodiscard]] PlayState state() const { return m_session.state(); }
    [[nodiscard]] float elapsedTime() const { return m_session.elapsedTime(); }
    [[nodiscard]] uint64_t frameCount() const { return m_session.frameCount(); }
    [[nodiscard]] const EditorWorldSession& session() const { return m_session; }

    [[nodiscard]] bool isRunning() const { return m_session.state() == PlayState::Running; }
    [[nodiscard]] bool isPaused()  const { return m_session.state() == PlayState::Paused; }
    [[nodiscard]] bool isStopped() const { return m_session.state() == PlayState::Stopped; }

    /// Start play — snapshots the current editor state.
    bool start(const std::string& worldPath = "") {
        // Gather current state
        std::vector<PlacedEntity> entities;
        NoiseParams params;
        Vec3 camPos{};
        float yaw = 0.f, pitch = 0.f;

        if (m_placement)
            entities = m_placement->placedEntities();
        if (m_pcg)
            params = m_pcg->noiseParams();
        if (m_viewport) {
            camPos = m_viewport->cameraPosition();
            yaw    = m_viewport->cameraYaw();
            pitch  = m_viewport->cameraPitch();
        }

        bool ok = m_session.start(worldPath, entities, params, camPos, yaw, pitch);
        if (ok) {
            NF_LOG_INFO("PIE", "Play-in-Editor started");
        }
        return ok;
    }

    bool pause() {
        bool ok = m_session.pause();
        if (ok) NF_LOG_INFO("PIE", "Play-in-Editor paused");
        return ok;
    }

    bool resume() {
        bool ok = m_session.resume();
        if (ok) NF_LOG_INFO("PIE", "Play-in-Editor resumed");
        return ok;
    }

    /// Stop play — restores the pre-play snapshot to the editor state.
    bool stop() {
        bool ok = m_session.stop();
        if (!ok) return false;

        // Restore from snapshot
        const auto& snap = m_session.snapshot();
        if (snap.valid) {
            if (m_placement) {
                m_placement->clear();
                for (auto& e : snap.placedEntities)
                    m_placement->addEntity(e);
            }
            if (m_pcg)
                m_pcg->setNoiseParams(snap.pcgParams);
            if (m_viewport) {
                m_viewport->setCameraPosition(snap.cameraPosition);
                m_viewport->setCameraYaw(snap.cameraYaw);
                m_viewport->setCameraPitch(snap.cameraPitch);
            }
        }
        NF_LOG_INFO("PIE", "Play-in-Editor stopped — world restored");
        return true;
    }

    /// Toggle: Start/Resume if stopped/paused, Pause if running.
    void togglePlay() {
        switch (m_session.state()) {
            case PlayState::Stopped: start(); break;
            case PlayState::Running: pause(); break;
            case PlayState::Paused:  resume(); break;
        }
    }

    void tick(float dt) { m_session.tick(dt); }

private:
    EditorWorldSession m_session;
    EntityPlacementTool* m_placement = nullptr;
    PCGTuningPanel*      m_pcg       = nullptr;
    ViewportPanel*       m_viewport  = nullptr;
};

// ── M4/S3 Asset Pipeline ──────────────────────────────────────────

/// 128-bit asset GUID for stable references across renames and moves.
struct AssetGuid {
    uint64_t hi = 0;
    uint64_t lo = 0;

    [[nodiscard]] bool isNull() const { return hi == 0 && lo == 0; }

    bool operator==(const AssetGuid& o) const { return hi == o.hi && lo == o.lo; }
    bool operator!=(const AssetGuid& o) const { return !(*this == o); }
    bool operator<(const AssetGuid& o) const {
        return hi < o.hi || (hi == o.hi && lo < o.lo);
    }

    /// Generate a deterministic GUID from a path string (FNV-1a based).
    static AssetGuid fromPath(const std::string& path) {
        AssetGuid g;
        // FNV-1a 64-bit for hi
        uint64_t h = 14695981039346656037ULL;
        for (char c : path) {
            h ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
            h *= 1099511628211ULL;
        }
        g.hi = h;
        // Second pass with different seed for lo
        h = 17316040143175676883ULL;
        for (auto it = path.rbegin(); it != path.rend(); ++it) {
            h ^= static_cast<uint64_t>(static_cast<unsigned char>(*it));
            h *= 1099511628211ULL;
        }
        g.lo = h;
        return g;
    }

    /// Generate a unique GUID from a counter (for testing / new assets).
    static AssetGuid generate(uint64_t counter) {
        AssetGuid g;
        g.hi = 0x4F00FACE00000000ULL | (counter >> 32);
        g.lo = (counter & 0xFFFFFFFFULL) | 0xA55E700000000000ULL;
        return g;
    }

    [[nodiscard]] std::string toString() const {
        char buf[40];
        std::snprintf(buf, sizeof(buf), "%016llx-%016llx",
                      static_cast<unsigned long long>(hi),
                      static_cast<unsigned long long>(lo));
        return buf;
    }
};

enum class AssetType : uint8_t {
    Unknown  = 0,
    Mesh     = 1,
    Texture  = 2,
    Material = 3,
    Sound    = 4,
    Script   = 5,
    Graph    = 6,
    World    = 7
};

inline const char* assetTypeName(AssetType t) {
    switch (t) {
        case AssetType::Mesh:     return "Mesh";
        case AssetType::Texture:  return "Texture";
        case AssetType::Material: return "Material";
        case AssetType::Sound:    return "Sound";
        case AssetType::Script:   return "Script";
        case AssetType::Graph:    return "Graph";
        case AssetType::World:    return "World";
        default:                  return "Unknown";
    }
}

inline AssetType classifyAssetExtension(const std::string& ext) {
    if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") return AssetType::Mesh;
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") return AssetType::Texture;
    if (ext == ".mat" || ext == ".nfmat") return AssetType::Material;
    if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") return AssetType::Sound;
    if (ext == ".lua" || ext == ".nfs") return AssetType::Script;
    if (ext == ".nfg") return AssetType::Graph;
    if (ext == ".nfw") return AssetType::World;
    return AssetType::Unknown;
}

/// An entry in the asset database.
struct AssetEntry {
    AssetGuid   guid;
    std::string path;          // relative to content root
    std::string name;          // filename without extension
    AssetType   type       = AssetType::Unknown;
    uint64_t    lastModified = 0;  // epoch seconds
    size_t      sizeBytes  = 0;
    bool        imported   = false;
};

/// Central GUID-based asset registry.  Maps paths↔GUIDs and tracks import state.
class AssetDatabase {
public:
    /// Register an asset.  If the path already exists the existing entry is updated.
    AssetGuid registerAsset(const std::string& relativePath, AssetType type,
                            size_t sizeBytes = 0, uint64_t lastMod = 0) {
        // Check if path already registered
        for (auto& e : m_entries) {
            if (e.path == relativePath) {
                e.type = type;
                e.sizeBytes = sizeBytes;
                e.lastModified = lastMod;
                return e.guid;
            }
        }
        AssetEntry entry;
        entry.guid = AssetGuid::fromPath(relativePath);
        entry.path = relativePath;
        entry.type = type;
        entry.sizeBytes = sizeBytes;
        entry.lastModified = lastMod;

        // Extract name from path
        auto pos = relativePath.find_last_of("/\\");
        std::string filename = (pos != std::string::npos) ? relativePath.substr(pos + 1) : relativePath;
        auto dotPos = filename.find_last_of('.');
        entry.name = (dotPos != std::string::npos) ? filename.substr(0, dotPos) : filename;

        m_entries.push_back(entry);
        return entry.guid;
    }

    /// Remove an asset by GUID.
    bool removeAsset(const AssetGuid& guid) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->guid == guid) { m_entries.erase(it); return true; }
        }
        return false;
    }

    /// Find by GUID.
    [[nodiscard]] AssetEntry* findByGuid(const AssetGuid& guid) {
        for (auto& e : m_entries) if (e.guid == guid) return &e;
        return nullptr;
    }
    [[nodiscard]] const AssetEntry* findByGuid(const AssetGuid& guid) const {
        for (auto& e : m_entries) if (e.guid == guid) return &e;
        return nullptr;
    }

    /// Find by relative path.
    [[nodiscard]] AssetEntry* findByPath(const std::string& path) {
        for (auto& e : m_entries) if (e.path == path) return &e;
        return nullptr;
    }

    /// Mark an asset as imported.
    bool markImported(const AssetGuid& guid) {
        if (auto* e = findByGuid(guid)) { e->imported = true; return true; }
        return false;
    }

    /// Scan a directory tree and register all recognised asset files.
    size_t scanDirectory(const std::string& rootPath) {
        size_t count = 0;
        if (!std::filesystem::exists(rootPath)) return 0;
        for (auto& entry : std::filesystem::recursive_directory_iterator(rootPath)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            AssetType type = classifyAssetExtension(ext);
            if (type == AssetType::Unknown) continue;

            std::string relPath = entry.path().string();
            // Normalise to forward slashes
            for (char& c : relPath) if (c == '\\') c = '/';

            uint64_t lastMod = 0;
            auto ftime = std::filesystem::last_write_time(entry);
            lastMod = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());

            registerAsset(relPath, type, static_cast<size_t>(entry.file_size()), lastMod);
            ++count;
        }
        return count;
    }

    [[nodiscard]] const std::vector<AssetEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t assetCount() const { return m_entries.size(); }

    /// Return all assets of a given type.
    [[nodiscard]] std::vector<const AssetEntry*> assetsOfType(AssetType type) const {
        std::vector<const AssetEntry*> result;
        for (auto& e : m_entries) if (e.type == type) result.push_back(&e);
        return result;
    }

    [[nodiscard]] size_t importedCount() const {
        size_t n = 0;
        for (auto& e : m_entries) if (e.imported) ++n;
        return n;
    }

    void clear() { m_entries.clear(); }

private:
    std::vector<AssetEntry> m_entries;
};

/// Import settings for mesh assets.
struct MeshImportSettings {
    float scaleFactor       = 1.0f;
    bool  generateNormals   = true;
    bool  generateTangents  = false;
    bool  flipWindingOrder  = false;
    bool  mergeMeshes       = false;
    int   maxVertices       = 0;  // 0 = no limit
};

/// Import settings for texture assets.
struct TextureImportSettings {
    bool  generateMipmaps    = true;
    bool  sRGB               = true;
    int   maxResolution      = 0;   // 0 = no limit
    bool  premultiplyAlpha   = false;
    bool  flipVertically     = true;
    float compressionQuality = 0.8f; // 0-1
};

/// Mesh importer — validates and "imports" mesh files into the asset database.
class MeshImporter {
public:
    void setSettings(const MeshImportSettings& s) { m_settings = s; }
    [[nodiscard]] const MeshImportSettings& settings() const { return m_settings; }

    /// Validate that the given path is a supported mesh format.
    [[nodiscard]] bool canImport(const std::string& path) const {
        auto ext = std::filesystem::path(path).extension().string();
        return ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb";
    }

    /// Import a mesh into the asset database.  Returns the GUID.
    AssetGuid import(AssetDatabase& db, const std::string& relativePath) {
        if (!canImport(relativePath)) return {};
        AssetGuid guid = db.registerAsset(relativePath, AssetType::Mesh);
        db.markImported(guid);
        ++m_importCount;
        NF_LOG_INFO("MeshImporter", "Imported mesh: " + relativePath);
        return guid;
    }

    [[nodiscard]] size_t importCount() const { return m_importCount; }

private:
    MeshImportSettings m_settings;
    size_t m_importCount = 0;
};

/// Texture importer — validates and "imports" texture files into the asset database.
class TextureImporter {
public:
    void setSettings(const TextureImportSettings& s) { m_settings = s; }
    [[nodiscard]] const TextureImportSettings& settings() const { return m_settings; }

    /// Validate that the given path is a supported texture format.
    [[nodiscard]] bool canImport(const std::string& path) const {
        auto ext = std::filesystem::path(path).extension().string();
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
               ext == ".tga" || ext == ".bmp";
    }

    /// Import a texture into the asset database.  Returns the GUID.
    AssetGuid import(AssetDatabase& db, const std::string& relativePath) {
        if (!canImport(relativePath)) return {};
        AssetGuid guid = db.registerAsset(relativePath, AssetType::Texture);
        db.markImported(guid);
        ++m_importCount;
        NF_LOG_INFO("TextureImporter", "Imported texture: " + relativePath);
        return guid;
    }

    [[nodiscard]] size_t importCount() const { return m_importCount; }

private:
    TextureImportSettings m_settings;
    size_t m_importCount = 0;
};

/// Watches for asset changes and tracks which GUIDs need re-import (hot-reload).
class AssetWatcher {
public:
    /// Poll the asset database for changes.  Returns number of dirty assets detected.
    size_t pollChanges(AssetDatabase& db) {
        size_t detected = 0;
        for (auto& entry : db.entries()) {
            if (!std::filesystem::exists(entry.path)) continue;
            auto ftime = std::filesystem::last_write_time(entry.path);
            uint64_t modTime = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());
            if (modTime > entry.lastModified) {
                m_dirtyAssets.insert(entry.guid);
                ++detected;
            }
        }
        return detected;
    }

    /// Mark a GUID as dirty (needs re-import).
    void markDirty(const AssetGuid& guid) { m_dirtyAssets.insert(guid); }

    /// Clear a GUID from dirty set after re-import.
    void clearDirty(const AssetGuid& guid) { m_dirtyAssets.erase(guid); }

    /// Check if an asset is dirty.
    [[nodiscard]] bool isDirty(const AssetGuid& guid) const {
        return m_dirtyAssets.count(guid) > 0;
    }

    [[nodiscard]] size_t dirtyCount() const { return m_dirtyAssets.size(); }

    void clearAll() { m_dirtyAssets.clear(); }

    [[nodiscard]] const std::set<AssetGuid>& dirtyAssets() const { return m_dirtyAssets; }

private:
    std::set<AssetGuid> m_dirtyAssets;
};

// ── S4 Blender Bridge ────────────────────────────────────────────

/// Supported Blender export formats.
enum class BlenderExportFormat : uint8_t {
    FBX  = 0,
    GLTF = 1,
    OBJ  = 2,
    GLB  = 3
};

inline const char* blenderExportFormatName(BlenderExportFormat f) {
    switch (f) {
        case BlenderExportFormat::FBX:  return "FBX";
        case BlenderExportFormat::GLTF: return "GLTF";
        case BlenderExportFormat::OBJ:  return "OBJ";
        case BlenderExportFormat::GLB:  return "GLB";
    }
    return "Unknown";
}

inline const char* blenderExportFormatExtension(BlenderExportFormat f) {
    switch (f) {
        case BlenderExportFormat::FBX:  return ".fbx";
        case BlenderExportFormat::GLTF: return ".gltf";
        case BlenderExportFormat::OBJ:  return ".obj";
        case BlenderExportFormat::GLB:  return ".glb";
    }
    return "";
}

/// Record of a single Blender export that arrived in the watched directory.
struct BlenderExportEntry {
    std::string sourcePath;                      // path inside export dir
    BlenderExportFormat format = BlenderExportFormat::FBX;
    uint64_t exportedAt        = 0;              // epoch seconds
    bool autoImported          = false;          // true once auto-imported
    AssetGuid importedGuid;                      // GUID after import (null if not yet)
};

/// Watches a Blender export directory and auto-imports new/changed assets
/// into the editor's AssetDatabase via MeshImporter.
class BlenderAutoImporter {
public:
    /// Set the directory to watch for Blender exports.
    void setExportDirectory(const std::string& dir) { m_exportDir = dir; }
    [[nodiscard]] const std::string& exportDirectory() const { return m_exportDir; }

    /// Enable or disable auto-import.
    void setAutoImportEnabled(bool enabled) { m_autoImport = enabled; }
    [[nodiscard]] bool isAutoImportEnabled() const { return m_autoImport; }

    /// Scan the export directory for new or changed files.
    /// Returns number of new exports detected.
    size_t scanExports() {
        if (m_exportDir.empty() || !std::filesystem::exists(m_exportDir)) return 0;

        size_t detected = 0;
        for (auto& entry : std::filesystem::directory_iterator(m_exportDir)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            BlenderExportFormat fmt;
            if      (ext == ".fbx")  fmt = BlenderExportFormat::FBX;
            else if (ext == ".gltf") fmt = BlenderExportFormat::GLTF;
            else if (ext == ".obj")  fmt = BlenderExportFormat::OBJ;
            else if (ext == ".glb")  fmt = BlenderExportFormat::GLB;
            else continue;

            std::string relPath = entry.path().string();
            // Skip already-known files
            bool found = false;
            for (auto& e : m_exports) {
                if (e.sourcePath == relPath) { found = true; break; }
            }
            if (found) continue;

            BlenderExportEntry be;
            be.sourcePath = relPath;
            be.format = fmt;
            auto ftime = std::filesystem::last_write_time(entry);
            be.exportedAt = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());
            m_exports.push_back(be);
            ++detected;
        }
        return detected;
    }

    /// Auto-import all pending exports into the given asset database.
    /// Returns number of assets imported.
    size_t importPending(AssetDatabase& db, MeshImporter& meshImporter) {
        size_t imported = 0;
        for (auto& ex : m_exports) {
            if (ex.autoImported) continue;
            if (!meshImporter.canImport(ex.sourcePath)) continue;
            AssetGuid guid = meshImporter.import(db, ex.sourcePath);
            if (!guid.isNull()) {
                ex.autoImported = true;
                ex.importedGuid = guid;
                ++imported;
                NF_LOG_INFO("BlenderBridge", "Auto-imported: " + ex.sourcePath);
            }
        }
        return imported;
    }

    /// Poll: scan + auto-import in one call (convenience for editor tick).
    size_t poll(AssetDatabase& db, MeshImporter& meshImporter) {
        scanExports();
        if (!m_autoImport) return 0;
        return importPending(db, meshImporter);
    }

    [[nodiscard]] const std::vector<BlenderExportEntry>& exports() const { return m_exports; }
    [[nodiscard]] size_t exportCount() const { return m_exports.size(); }

    [[nodiscard]] size_t importedCount() const {
        size_t n = 0;
        for (auto& e : m_exports) if (e.autoImported) ++n;
        return n;
    }

    [[nodiscard]] size_t pendingCount() const {
        return m_exports.size() - importedCount();
    }

    void clearHistory() { m_exports.clear(); }

private:
    std::string m_exportDir;
    bool m_autoImport = true;
    std::vector<BlenderExportEntry> m_exports;
};

// ── Editor application ───────────────────────────────────────────

class EditorApp {
public:
    // Full init with an explicit executable path so the editor can locate the project root.
    bool init(int width, int height, const std::string& executablePath) {
        // Initialise project path service first so commands can reference paths.
        m_projectPaths.init(executablePath);

        // Wire ContentBrowser to the project's Content directory.
        if (!m_projectPaths.contentPath().empty()) {
            m_contentBrowser.setRootPath(m_projectPaths.contentPath());
            m_contentBrowser.refresh();
        }

        m_renderer.init(width, height);
        m_ui.init();

        // Register core editor commands
        m_commands.registerCommand("file.new", [this]() {
            NF_LOG_INFO("Editor", "New world");
            m_currentWorldPath.clear();
            m_commandStack.clear();
        }, "New World", "Ctrl+N");

        m_commands.registerCommand("file.open", [this]() {
            NF_LOG_INFO("Editor", "Open world");
        }, "Open World", "Ctrl+O");

        m_commands.registerCommand("file.save", [this]() {
            NF_LOG_INFO("Editor", "Save world");
            if (!m_currentWorldPath.empty()) {
                m_commandStack.markClean();
            }
        }, "Save", "Ctrl+S");

        m_commands.registerCommand("file.save_as", [this]() {
            NF_LOG_INFO("Editor", "Save world as...");
        }, "Save As", "Ctrl+Shift+S");

        m_commands.registerCommand("edit.undo", [this]() {
            if (m_commandStack.canUndo()) {
                std::string desc = m_commandStack.undoDescription();
                m_commandStack.undo();
                NF_LOG_INFO("Editor", "Undo: " + desc);
            }
        }, "Undo", "Ctrl+Z");
        m_commands.setEnabledCheck("edit.undo", [this]() { return m_commandStack.canUndo(); });

        m_commands.registerCommand("edit.redo", [this]() {
            if (m_commandStack.canRedo()) {
                std::string desc = m_commandStack.redoDescription();
                m_commandStack.redo();
                NF_LOG_INFO("Editor", "Redo: " + desc);
            }
        }, "Redo", "Ctrl+Y");
        m_commands.setEnabledCheck("edit.redo", [this]() { return m_commandStack.canRedo(); });

        m_commands.registerCommand("edit.select_all", [this]() {
            NF_LOG_INFO("Editor", "Select all");
        }, "Select All", "Ctrl+A");

        m_commands.registerCommand("edit.deselect", [this]() {
            m_selection.clearSelection();
        }, "Deselect All", "Ctrl+D");

        m_commands.registerCommand("view.reset_layout", [this]() {
            NF_LOG_INFO("Editor", "Reset panel layout");
        }, "Reset Layout", "");

        // View toggle commands
        m_commands.registerCommand("view.toggle_inspector", [this]() {
            togglePanelVisibility("Inspector");
        }, "Toggle Inspector", "");

        m_commands.registerCommand("view.toggle_hierarchy", [this]() {
            togglePanelVisibility("Hierarchy");
        }, "Toggle Hierarchy", "");

        m_commands.registerCommand("view.toggle_console", [this]() {
            togglePanelVisibility("Console");
        }, "Toggle Console", "");

        m_commands.registerCommand("view.toggle_content_browser", [this]() {
            togglePanelVisibility("ContentBrowser");
        }, "Toggle Content Browser", "");

        // Graph commands
        m_commands.registerCommand("graph.new_graph", [this]() {
            if (auto* gep = graphEditorPanel()) {
                gep->newGraph(GraphType::World, "Untitled");
                m_notifications.push(NotificationType::Success, "New graph created");
            }
            NF_LOG_INFO("Editor", "New graph");
        }, "New Graph", "");

        m_commands.registerCommand("graph.open_graph", [this]() {
            if (auto* gep = graphEditorPanel()) {
                gep->openGraph("Untitled");
            }
            NF_LOG_INFO("Editor", "Open graph");
        }, "Open Graph", "");

        m_commands.registerCommand("graph.add_node", [this]() {
            if (auto* gep = graphEditorPanel()) {
                int id = gep->addNode("NewNode");
                if (id >= 0)
                    m_notifications.push(NotificationType::Info, "Added node #" + std::to_string(id));
            }
        }, "Add Node", "");

        m_commands.registerCommand("graph.remove_node", [this]() {
            if (auto* gep = graphEditorPanel()) {
                if (gep->selectedNodeId() >= 0) {
                    gep->removeNode(gep->selectedNodeId());
                    m_notifications.push(NotificationType::Info, "Node removed");
                }
            }
        }, "Remove Node", "Delete");
        m_commands.setEnabledCheck("graph.remove_node",
            [this]() {
                auto* gep = graphEditorPanel();
                return gep && gep->selectedNodeId() >= 0;
            });

        m_commands.registerCommand("graph.compile", [this]() {
            if (auto* gep = graphEditorPanel()) {
                if (gep->compileAndLoad())
                    m_notifications.push(NotificationType::Success, "Graph compiled");
            }
        }, "Compile Graph", "F7");

        // IDE commands
        m_ideService.init();

        m_commands.registerCommand("ide.go_to_definition", [this]() {
            NF_LOG_INFO("IDE", "Go to definition");
        }, "Go To Definition", "F12");

        m_commands.registerCommand("ide.find_references", [this]() {
            NF_LOG_INFO("IDE", "Find references");
        }, "Find References", "Shift+F12");

        m_commands.registerCommand("ide.go_back", [this]() {
            m_ideService.goBack();
        }, "Go Back", "Alt+Left");

        m_commands.registerCommand("ide.index_project", [this]() {
            // Index Source/, Content/, and Config/ so the entire repo is searchable.
            auto& indexer = m_ideService.indexer();
            indexer.clear();
            for (const std::string rel : {"Source", "Content", "Config"}) {
                std::string dir = m_projectPaths.resolvePath(rel);
                indexer.indexDirectory(dir);
            }
            // Populate CodeNavigator from all indexed symbols.
            auto& nav = m_ideService.navigator();
            nav.clear();
            for (auto& f : indexer.allFiles()) {
                for (auto& sym : f.symbols) {
                    NavigationEntry entry;
                    entry.symbol = sym;
                    entry.kind = SymbolKind::Unknown;
                    entry.filePath = f.path;
                    entry.line = 0;
                    nav.addEntry(std::move(entry));
                }
            }
            NF_LOG_INFO("IDE", "Project indexed: " +
                std::to_string(indexer.fileCount()) + " files, " +
                std::to_string(nav.entryCount()) + " symbols");
            m_notifications.push(NotificationType::Success,
                "Project indexed: " + std::to_string(indexer.fileCount()) + " files");
        }, "Index Project", "");

        // Entity commands
        m_commands.registerCommand("entity.create", [this]() {
            NF_LOG_INFO("Editor", "Create entity");
            m_notifications.push(NotificationType::Success, "Entity created");
        }, "Create Entity", "Ctrl+Shift+N");

        m_commands.registerCommand("entity.delete", [this]() {
            if (!m_selection.hasSelection()) return;
            NF_LOG_INFO("Editor", "Delete selected entities");
            m_selection.clearSelection();
            m_notifications.push(NotificationType::Info, "Entity deleted");
        }, "Delete Entity", "Delete");
        m_commands.setEnabledCheck("entity.delete",
            [this]() { return m_selection.hasSelection(); });

        m_commands.registerCommand("entity.duplicate", [this]() {
            if (!m_selection.hasSelection()) return;
            NF_LOG_INFO("Editor", "Duplicate selected entity");
            m_notifications.push(NotificationType::Success, "Entity duplicated");
        }, "Duplicate Entity", "Ctrl+D");
        m_commands.setEnabledCheck("entity.duplicate",
            [this]() { return m_selection.hasSelection(); });

        // Gizmo mode commands
        m_commands.registerCommand("gizmo.translate", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        }, "Translate Gizmo", "W");

        m_commands.registerCommand("gizmo.rotate", [this]() {
            m_gizmo.setMode(GizmoMode::Rotate);
        }, "Rotate Gizmo", "E");

        m_commands.registerCommand("gizmo.scale", [this]() {
            m_gizmo.setMode(GizmoMode::Scale);
        }, "Scale Gizmo", "R");

        // Toggle graph editor panel
        m_commands.registerCommand("view.toggle_graph_editor", [this]() {
            togglePanelVisibility("GraphEditor");
        }, "Toggle Graph Editor", "");

        // Toggle settings
        m_commands.registerCommand("view.toggle_dark_mode", [this]() {
            m_editorSettings.setDarkMode(!m_editorSettings.settings().darkMode);
            m_editorSettings.applyTheme(m_theme);
            NF_LOG_INFO("Editor", std::string("Dark mode: ") +
                (m_editorSettings.settings().darkMode ? "on" : "off"));
        }, "Toggle Dark Mode", "");

        // Create default panels
        {
            auto viewport = std::make_unique<ViewportPanel>();
            m_dockLayout.addPanel(viewport->name(), viewport->slot());
            m_editorPanels.push_back(std::move(viewport));
        }
        {
            auto inspector = std::make_unique<InspectorPanel>(&m_selection, nullptr);
            m_dockLayout.addPanel(inspector->name(), inspector->slot());
            m_editorPanels.push_back(std::move(inspector));
        }
        {
            auto hierarchy = std::make_unique<HierarchyPanel>(&m_selection);
            m_dockLayout.addPanel(hierarchy->name(), hierarchy->slot());
            m_editorPanels.push_back(std::move(hierarchy));
        }
        {
            auto console = std::make_unique<ConsolePanel>();
            m_dockLayout.addPanel(console->name(), console->slot());
            m_editorPanels.push_back(std::move(console));
        }
        {
            auto cb = std::make_unique<ContentBrowserPanel>(&m_contentBrowser);
            m_dockLayout.addPanel(cb->name(), cb->slot());
            m_editorPanels.push_back(std::move(cb));
        }
        {
            auto graphEd = std::make_unique<GraphEditorPanel>(m_graphVM);
            m_dockLayout.addPanel(graphEd->name(), graphEd->slot());
            m_dockLayout.setPanelVisible(graphEd->name(), false);  // hidden by default
            m_editorPanels.push_back(std::move(graphEd));
        }
        {
            auto pcg = std::make_unique<PCGTuningPanel>();
            m_dockLayout.addPanel(pcg->name(), pcg->slot());
            m_dockLayout.setPanelVisible(pcg->name(), false);  // hidden by default
            m_editorPanels.push_back(std::move(pcg));
        }

        // M2/S1 undo system
        m_editorUndo = std::make_unique<EditorUndoSystem>(m_commandStack);

        // M3/S2 Play-in-Editor
        m_playInEditor = std::make_unique<PlayInEditorSystem>(
            &m_entityPlacement, pcgTuningPanel(), viewportPanel());

        // M3/S2 PIE commands
        m_commands.registerCommand("play.start", [this]() {
            if (m_playInEditor->isStopped())
                m_playInEditor->start(m_currentWorldPath);
            else if (m_playInEditor->isPaused())
                m_playInEditor->resume();
            m_notifications.push(NotificationType::Success, "Play");
        }, "Play", "F5");

        m_commands.registerCommand("play.pause", [this]() {
            if (m_playInEditor->isRunning()) {
                m_playInEditor->pause();
                m_notifications.push(NotificationType::Info, "Paused");
            }
        }, "Pause", "F6");
        m_commands.setEnabledCheck("play.pause", [this]() { return m_playInEditor->isRunning(); });

        m_commands.registerCommand("play.stop", [this]() {
            if (!m_playInEditor->isStopped()) {
                m_playInEditor->stop();
                m_notifications.push(NotificationType::Info, "Stopped — world restored");
            }
        }, "Stop", "Shift+F5");
        m_commands.setEnabledCheck("play.stop", [this]() { return !m_playInEditor->isStopped(); });

        // M2/S1 tool commands
        m_commands.registerCommand("view.toggle_pcg_tuning", [this]() {
            togglePanelVisibility("PCGTuning");
        }, "Toggle PCG Tuning", "");

        m_commands.registerCommand("tools.entity_placement", [this]() {
            NF_LOG_INFO("Editor", "Entity Placement tool activated");
        }, "Entity Placement Tool", "");

        m_commands.registerCommand("tools.voxel_paint", [this]() {
            NF_LOG_INFO("Editor", "Voxel Paint tool activated");
        }, "Voxel Paint Tool", "");

        // M4/S3 Asset Pipeline commands
        m_commands.registerCommand("assets.scan", [this]() {
            if (!m_projectPaths.contentPath().empty()) {
                size_t n = m_assetDatabase.scanDirectory(m_projectPaths.contentPath());
                NF_LOG_INFO("Assets", "Scanned " + std::to_string(n) + " assets");
                m_notifications.push(NotificationType::Success,
                    "Asset scan: " + std::to_string(n) + " files found");
            }
        }, "Scan Assets", "");

        m_commands.registerCommand("assets.reimport", [this]() {
            size_t reimported = 0;
            for (auto& guid : m_assetWatcher.dirtyAssets()) {
                auto* entry = m_assetDatabase.findByGuid(guid);
                if (!entry) continue;
                if (m_meshImporter.canImport(entry->path))
                    m_meshImporter.import(m_assetDatabase, entry->path);
                else if (m_textureImporter.canImport(entry->path))
                    m_textureImporter.import(m_assetDatabase, entry->path);
                ++reimported;
            }
            m_assetWatcher.clearAll();
            if (reimported > 0) {
                NF_LOG_INFO("Assets", "Re-imported " + std::to_string(reimported) + " assets");
                m_notifications.push(NotificationType::Success,
                    "Re-imported " + std::to_string(reimported) + " assets");
            }
        }, "Reimport Changed Assets", "");
        m_commands.setEnabledCheck("assets.reimport",
            [this]() { return m_assetWatcher.dirtyCount() > 0; });

        // S4 Blender Bridge commands
        m_commands.registerCommand("blender.set_export_dir", [this]() {
            // In a real editor, this would open a folder picker.
            // For now, default to Content/BlenderExports/
            std::string dir = m_projectPaths.contentPath() + "/BlenderExports";
            std::filesystem::create_directories(dir);
            m_blenderImporter.setExportDirectory(dir);
            NF_LOG_INFO("BlenderBridge", "Export dir: " + dir);
            m_notifications.push(NotificationType::Success,
                "Blender export dir set: " + dir);
        }, "Set Blender Export Dir", "");

        m_commands.registerCommand("blender.scan_exports", [this]() {
            size_t n = m_blenderImporter.scanExports();
            NF_LOG_INFO("BlenderBridge", "Scanned: " + std::to_string(n) + " new exports");
            if (n > 0) {
                m_notifications.push(NotificationType::Info,
                    std::to_string(n) + " new Blender exports found");
            }
        }, "Scan Blender Exports", "");

        m_commands.registerCommand("blender.import_pending", [this]() {
            size_t n = m_blenderImporter.importPending(m_assetDatabase, m_meshImporter);
            if (n > 0) {
                NF_LOG_INFO("BlenderBridge", "Imported " + std::to_string(n) + " assets");
                m_notifications.push(NotificationType::Success,
                    "Imported " + std::to_string(n) + " Blender assets");
            }
        }, "Import Pending Blender Assets", "");
        m_commands.setEnabledCheck("blender.import_pending",
            [this]() { return m_blenderImporter.pendingCount() > 0; });

        m_commands.registerCommand("blender.toggle_auto_import", [this]() {
            bool enabled = !m_blenderImporter.isAutoImportEnabled();
            m_blenderImporter.setAutoImportEnabled(enabled);
            NF_LOG_INFO("BlenderBridge", std::string("Auto-import: ") +
                (enabled ? "ON" : "OFF"));
        }, "Toggle Blender Auto-Import", "");

        // Create default toolbar items
        m_toolbar.addItem("Select", "select", "Select tool", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        });
        m_toolbar.addItem("Move", "move", "Move tool", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        });
        m_toolbar.addItem("Rotate", "rotate", "Rotate tool", [this]() {
            m_gizmo.setMode(GizmoMode::Rotate);
        });
        m_toolbar.addItem("Scale", "scale", "Scale tool", [this]() {
            m_gizmo.setMode(GizmoMode::Scale);
        });
        m_toolbar.addSeparator();
        m_toolbar.addItem("Play", "play", "Play (F5)", [this]() {
            m_commands.executeCommand("play.start");
        });
        m_toolbar.addItem("Pause", "pause", "Pause (F6)", [this]() {
            m_commands.executeCommand("play.pause");
        });
        m_toolbar.addItem("Stop", "stop", "Stop (Shift+F5)", [this]() {
            m_commands.executeCommand("play.stop");
        });

        // Build menu bar
        initMenuBar();

        // Load default hotkeys from registered commands
        m_hotkeyDispatcher.loadDefaults(m_commands);

        // ── Register log sink to feed ConsolePanel ──────────────────
        m_logSinkId = Logger::instance().addSink(
            [this](LogLevel level, std::string_view category, std::string_view message) {
                // Feed the console panel
                for (auto& p : m_editorPanels) {
                    if (auto* cp = dynamic_cast<ConsolePanel*>(p.get())) {
                        cp->addLogMessage(level, category, message);
                        break;
                    }
                }
                // Write to log file (thread-safe)
                {
                    std::lock_guard<std::mutex> lock(m_logFileMutex);
                    if (m_logFile.is_open()) {
                        m_logFile << "[" << Logger::levelTag(level) << "] ["
                                  << category << "] " << message << "\n";
                        m_logFile.flush();
                    }
                }
            });

        // ── Open log file in Logs/ directory ────────────────────────
        {
            std::string logDir = m_projectPaths.resolvePath("Logs");
            std::filesystem::create_directories(logDir);
            std::string logPath = logDir + "/editor.log";
            m_logFile.open(logPath, std::ios::out | std::ios::app);
            if (m_logFile.is_open()) {
                auto now = std::chrono::system_clock::now();
                auto time = std::chrono::system_clock::to_time_t(now);
                m_logFile << "\n=== Editor Session Started: " << std::ctime(&time);
            }
        }

        // ── Load persisted layout from Saved/ ───────────────────────
        loadEditorState();

        NF_LOG_INFO("Editor", "NovaForge Editor initialized");
        return true;
    }

    // Convenience overload: uses current working directory as the project root.
    bool init(int width, int height) { return init(width, height, "."); }

    void shutdown() {
        // ── Save editor state before shutdown ───────────────────────
        saveEditorState();

        // ── Remove log sink ─────────────────────────────────────────
        if (m_logSinkId != 0) {
            Logger::instance().removeSink(m_logSinkId);
            m_logSinkId = 0;
        }
        if (m_logFile.is_open()) {
            m_logFile << "=== Editor Session Ended ===\n";
            m_logFile.close();
        }

        m_ideService.shutdown();
        m_editorPanels.clear();
        m_ui.shutdown();
        m_renderer.shutdown();
        NF_LOG_INFO("Editor", "NovaForge Editor shutdown");
    }

    void update() {}
    void render() {}

    // Render the full editor UI through the UIRenderer pipeline.
    // Replaces the old paintEditorGDI() function.
    void renderAll(float width, float height) {
        m_dockLayout.computeLayout(width, height, 56.f, 24.f);
        m_ui.beginFrame(width, height);

        // Menu bar
        m_ui.drawRect({0.f, 0.f, width, 28.f}, m_theme.toolbarBackground);
        float mx = 8.f;
        for (auto& cat : m_menuBar.categories()) {
            m_ui.drawText(mx, 7.f, cat.name, m_theme.panelText);
            mx += static_cast<float>(cat.name.size()) * 8.f + 16.f;
        }

        // Toolbar
        m_ui.drawRect({0.f, 28.f, width, 28.f}, m_theme.toolbarBackground);
        float tx = 8.f;
        for (auto& item : m_toolbar.items()) {
            if (item.isSeparator) {
                m_ui.drawRect({tx, 32.f, 1.f, 20.f}, m_theme.toolbarSeparator);
                tx += 12.f;
            } else {
                m_ui.drawText(tx, 35.f, item.name, item.enabled ? m_theme.buttonText : m_theme.buttonDisabledText);
                tx += static_cast<float>(item.name.size()) * 8.f + 12.f;
            }
        }

        // Panels
        for (auto& panel : m_editorPanels) {
            if (!panel->isVisible()) continue;
            auto* dp = m_dockLayout.findPanel(panel->name());
            if (!dp || !dp->visible) continue;
            panel->render(m_ui, dp->bounds, m_theme);
        }

        // Splitter dividers
        for (auto& dp : m_dockLayout.panels()) {
            if (!dp.visible) continue;
            m_ui.drawRectOutline(dp.bounds, m_theme.panelBorder, 1.f);
        }

        // Status bar
        m_ui.drawRect({0.f, height - 24.f, width, 24.f}, m_theme.statusBarBackground);
        m_ui.drawText(8.f, height - 19.f, m_statusBar.buildText(), m_theme.statusBarText);

        // Notifications overlay
        if (m_notifications.hasActive()) {
            auto* n = m_notifications.current();
            if (n) {
                uint32_t bg = 0x333333FF;
                if (n->type == NotificationType::Error)   bg = 0x8B0000FF;
                if (n->type == NotificationType::Success) bg = 0x006400FF;
                if (n->type == NotificationType::Warning) bg = 0x8B8000FF;
                float nw = static_cast<float>(n->message.size()) * 8.f + 24.f;
                Rect nr{width - nw - 12.f, 60.f, nw, 28.f};
                m_ui.drawRect(nr, bg);
                m_ui.drawRectOutline(nr, m_theme.panelBorder, 1.f);
                m_ui.drawText(nr.x + 8.f, nr.y + 7.f, n->message, 0xFFFFFFFF);
            }
        }

        m_ui.endFrame();
    }

    // Per-frame update with input: routes right-click WASD fly-cam to the viewport,
    // dispatches hotkeys, ticks notifications, updates status bar and frame stats.
    void update(float dt, InputSystem& input) {
        input.update();
        if (auto* vp = viewportPanel())
            vp->updateCamera(dt, input);

        m_notifications.tick(dt);
        m_frameStats.beginFrame(dt);

        // M3/S2: tick Play-in-Editor
        if (m_playInEditor)
            m_playInEditor->tick(dt);

        // Update status bar mode to reflect PIE state
        std::string mode = "Editor";
        if (m_playInEditor && m_playInEditor->isRunning()) mode = "Playing";
        else if (m_playInEditor && m_playInEditor->isPaused()) mode = "Paused";

        m_statusBar.update(
            mode,
            m_currentWorldPath,
            m_commandStack.isDirty(),
            static_cast<int>(m_selection.selectionCount()),
            m_frameStats.stats().fps);
    }

    // Process a hotkey string and dispatch matching commands.
    int processHotkey(const std::string& hotkey) {
        return m_hotkeyDispatcher.dispatch(hotkey, m_commands);
    }

    // Returns a pointer to the first ViewportPanel, or nullptr.
    [[nodiscard]] ViewportPanel* viewportPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* vp = dynamic_cast<ViewportPanel*>(p.get())) return vp;
        }
        return nullptr;
    }
    [[nodiscard]] const ViewportPanel* viewportPanel() const {
        for (auto& p : m_editorPanels) {
            if (auto* vp = dynamic_cast<const ViewportPanel*>(p.get())) return vp;
        }
        return nullptr;
    }

    [[nodiscard]] GraphEditorPanel* graphEditorPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* gep = dynamic_cast<GraphEditorPanel*>(p.get())) return gep;
        }
        return nullptr;
    }

    // Accessors for editor services
    EditorCommandRegistry& commands() { return m_commands; }
    CommandStack& commandStack() { return m_commandStack; }
    SelectionService& selection() { return m_selection; }
    ContentBrowser& contentBrowser() { return m_contentBrowser; }
    RecentFilesList& recentFiles() { return m_recentFiles; }
    LaunchService& launchService() { return m_launchService; }
    ProjectPathService& projectPaths() { return m_projectPaths; }
    const ProjectPathService& projectPaths() const { return m_projectPaths; }
    DockLayout& dockLayout() { return m_dockLayout; }
    EditorToolbar& toolbar() { return m_toolbar; }
    MenuBar& menuBar() { return m_menuBar; }
    EditorStatusBar& statusBar() { return m_statusBar; }
    NotificationQueue& notifications() { return m_notifications; }
    EditorCameraOrbit& editorCamera() { return m_editorCamera; }
    GizmoState& gizmo() { return m_gizmo; }
    EditorSettingsService& settingsService() { return m_editorSettings; }
    HotkeyDispatcher& hotkeyDispatcher() { return m_hotkeyDispatcher; }
    FrameStatsTracker& frameStatsTracker() { return m_frameStats; }
    EditorTheme& theme() { return m_theme; }
    UIContext& uiContext() { return m_uiContext; }
    ToolWindowManager& toolManager() { return m_toolManager; }
    UIRenderer& uiRenderer() { return m_ui; }

    [[nodiscard]] const std::string& currentWorldPath() const { return m_currentWorldPath; }
    void setCurrentWorldPath(const std::string& path) {
        m_currentWorldPath = path;
        m_recentFiles.addFile(path);
    }

    [[nodiscard]] bool isDirty() const { return m_commandStack.isDirty(); }

    void addPanel(std::unique_ptr<EditorPanel> panel) {
        m_dockLayout.addPanel(panel->name(), panel->slot());
        m_editorPanels.push_back(std::move(panel));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<EditorPanel>>& editorPanels() const {
        return m_editorPanels;
    }

    void setGraphVM(GraphVM* vm) {
        m_graphVM = vm;
        if (auto* gep = graphEditorPanel()) gep->setGraphVM(vm);
    }
    [[nodiscard]] GraphVM* graphVM() const { return m_graphVM; }

    IDEService& ideService() { return m_ideService; }

    // M2/S1 accessors
    EntityPlacementTool& entityPlacementTool() { return m_entityPlacement; }
    VoxelPaintTool& voxelPaintTool() { return m_voxelPaint; }
    EditorUndoSystem& editorUndoSystem() { return *m_editorUndo; }
    WorldPreviewService& worldPreview() { return m_worldPreview; }

    // M3/S2 accessors
    PlayInEditorSystem& playInEditor() { return *m_playInEditor; }
    const PlayInEditorSystem& playInEditor() const { return *m_playInEditor; }

    // M4/S3 accessors
    AssetDatabase& assetDatabase() { return m_assetDatabase; }
    const AssetDatabase& assetDatabase() const { return m_assetDatabase; }
    MeshImporter& meshImporter() { return m_meshImporter; }
    TextureImporter& textureImporter() { return m_textureImporter; }
    AssetWatcher& assetWatcher() { return m_assetWatcher; }

    // S4 accessors
    BlenderAutoImporter& blenderAutoImporter() { return m_blenderImporter; }
    const BlenderAutoImporter& blenderAutoImporter() const { return m_blenderImporter; }

    [[nodiscard]] PCGTuningPanel* pcgTuningPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* pcg = dynamic_cast<PCGTuningPanel*>(p.get())) return pcg;
        }
        return nullptr;
    }

private:
    void togglePanelVisibility(const std::string& name) {
        if (auto* p = m_dockLayout.findPanel(name)) {
            p->visible = !p->visible;
            NF_LOG_INFO("Editor", "Toggle panel '" + name + "' visible=" +
                        (p->visible ? "true" : "false"));
        }
    }

    void initMenuBar() {
        // File menu
        auto& file = m_menuBar.addCategory("File");
        file.addItem("New World",   "file.new",     "Ctrl+N");
        file.addItem("Open World",  "file.open",    "Ctrl+O");
        file.addItem("Save",        "file.save",    "Ctrl+S");
        file.addItem("Save As",     "file.save_as", "Ctrl+Shift+S");
        file.addSeparator();
        file.addItem("Exit",        "file.exit",    "Alt+F4");

        // Edit menu
        auto& edit = m_menuBar.addCategory("Edit");
        edit.addItem("Undo",        "edit.undo",    "Ctrl+Z");
        edit.addItem("Redo",        "edit.redo",    "Ctrl+Y");
        edit.addSeparator();
        edit.addItem("Select All",  "edit.select_all",  "Ctrl+A");
        edit.addItem("Deselect",    "edit.deselect",     "Ctrl+D");
        edit.addSeparator();
        edit.addItem("Create Entity",   "entity.create",    "Ctrl+Shift+N");
        edit.addItem("Delete Entity",   "entity.delete",    "Delete");
        edit.addItem("Duplicate Entity","entity.duplicate", "Ctrl+D");
        edit.addSeparator();
        edit.addItem("PCG Tuning",      "view.toggle_pcg_tuning", "");
        edit.addSeparator();
        edit.addItem("Play",            "play.start",            "F5");
        edit.addItem("Pause",           "play.pause",            "F6");
        edit.addItem("Stop",            "play.stop",             "Shift+F5");

        // View menu
        auto& view = m_menuBar.addCategory("View");
        view.addItem("Inspector",      "view.toggle_inspector",       "");
        view.addItem("Hierarchy",      "view.toggle_hierarchy",       "");
        view.addItem("Console",        "view.toggle_console",         "");
        view.addItem("Content Browser","view.toggle_content_browser", "");
        view.addItem("Graph Editor",   "view.toggle_graph_editor",    "");
        view.addSeparator();
        view.addItem("Reset Layout",   "view.reset_layout",           "");
        view.addItem("Dark Mode",      "view.toggle_dark_mode",       "");

        // Graph menu
        auto& graph = m_menuBar.addCategory("Graph");
        graph.addItem("New Graph",    "graph.new_graph",    "");
        graph.addItem("Open Graph",   "graph.open_graph",   "");
        graph.addSeparator();
        graph.addItem("Add Node",     "graph.add_node",     "");
        graph.addItem("Remove Node",  "graph.remove_node",  "");
        graph.addSeparator();
        graph.addItem("Compile",      "graph.compile",      "F7");

        // Code menu
        auto& code = m_menuBar.addCategory("Code");
        code.addItem("Go To Definition", "ide.go_to_definition", "F12");
        code.addItem("Find References",  "ide.find_references",  "Shift+F12");
        code.addItem("Go Back",          "ide.go_back",          "Alt+Left");
        code.addItem("Index Project",    "ide.index_project",    "");

        // Tools menu
        auto& tools = m_menuBar.addCategory("Tools");
        tools.addItem("Blender Bridge",     "tools.launch_blender_bridge");
        tools.addItem("Contract Scanner",   "tools.launch_contract_scanner");
        tools.addItem("Replay Minimizer",   "tools.launch_replay_minimizer");
        tools.addItem("Atlas AI",           "tools.launch_atlas_ai");
        tools.addSeparator();
        tools.addItem("Pipeline Monitor",   "tools.pipeline_monitor");
        tools.addSeparator();
        tools.addItem("Entity Placement",   "tools.entity_placement");
        tools.addItem("Voxel Paint",        "tools.voxel_paint");
        tools.addSeparator();
        tools.addItem("Scan Assets",        "assets.scan");
        tools.addItem("Reimport Changed",   "assets.reimport");
        tools.addSeparator();
        tools.addItem("Set Blender Export Dir",  "blender.set_export_dir");
        tools.addItem("Scan Blender Exports",    "blender.scan_exports");
        tools.addItem("Import Pending",          "blender.import_pending");
        tools.addItem("Toggle Auto-Import",      "blender.toggle_auto_import");
    }

    Renderer m_renderer;
    UIRenderer m_ui;
    EditorCommandRegistry m_commands;
    CommandStack m_commandStack;
    SelectionService m_selection;
    ContentBrowser m_contentBrowser;
    RecentFilesList m_recentFiles;
    LaunchService m_launchService;
    ProjectPathService m_projectPaths;
    DockLayout m_dockLayout;
    EditorToolbar m_toolbar;
    std::vector<std::unique_ptr<EditorPanel>> m_editorPanels;
    std::string m_currentWorldPath;
    GraphVM* m_graphVM = nullptr;
    IDEService m_ideService;

    // New systems
    MenuBar m_menuBar;
    EditorStatusBar m_statusBar;
    NotificationQueue m_notifications;
    EditorCameraOrbit m_editorCamera;
    GizmoState m_gizmo;
    EditorSettingsService m_editorSettings;
    HotkeyDispatcher m_hotkeyDispatcher;
    FrameStatsTracker m_frameStats;
    EditorTheme m_theme;
    UIContext m_uiContext;
    ToolWindowManager m_toolManager;
    size_t m_logSinkId = 0;
    std::ofstream m_logFile;
    std::mutex m_logFileMutex;

    // M2/S1 world-editing systems
    EntityPlacementTool m_entityPlacement;
    VoxelPaintTool m_voxelPaint;
    std::unique_ptr<EditorUndoSystem> m_editorUndo;
    WorldPreviewService m_worldPreview;

    // M3/S2 Play-in-Editor
    std::unique_ptr<PlayInEditorSystem> m_playInEditor;

    // M4/S3 Asset Pipeline
    AssetDatabase m_assetDatabase;
    MeshImporter m_meshImporter;
    TextureImporter m_textureImporter;
    AssetWatcher m_assetWatcher;

    // S4 Blender Bridge
    BlenderAutoImporter m_blenderImporter;

    // ── State persistence ───────────────────────────────────────

    /// Escape a string for safe JSON embedding (handles quotes and backslashes).
    static std::string escapeJson(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else out += c;
        }
        return out;
    }

    /// Resolve the Saved/ directory path relative to project root.
    std::string savedDir() const {
        std::string dir = m_projectPaths.resolvePath("Saved");
        std::filesystem::create_directories(dir);
        return dir;
    }

    /// Save editor state (panel visibility, dock sizes, settings) to Saved/editor_state.json.
    void saveEditorState() {
        std::string path = savedDir() + "/editor_state.json";
        std::ofstream out(path);
        if (!out.is_open()) return;

        out << "{\n";
        out << "  \"version\": 1,\n";
        out << "  \"darkMode\": " << (m_editorSettings.settings().darkMode ? "true" : "false") << ",\n";
        out << "  \"dockSizes\": {\n";
        out << "    \"left\": " << m_dockLayout.leftWidth() << ",\n";
        out << "    \"right\": " << m_dockLayout.rightWidth() << ",\n";
        out << "    \"top\": " << m_dockLayout.topHeight() << ",\n";
        out << "    \"bottom\": " << m_dockLayout.bottomHeight() << "\n";
        out << "  },\n";
        out << "  \"panels\": [\n";
        auto& panels = m_dockLayout.panels();
        for (size_t i = 0; i < panels.size(); ++i) {
            out << "    {\"name\": \"" << escapeJson(panels[i].name)
                << "\", \"visible\": " << (panels[i].visible ? "true" : "false") << "}";
            if (i + 1 < panels.size()) out << ",";
            out << "\n";
        }
        out << "  ],\n";
        out << "  \"lastWorldPath\": \"" << escapeJson(m_currentWorldPath) << "\",\n";

        // Save recent files
        out << "  \"recentFiles\": [\n";
        auto& recent = m_recentFiles.files();
        for (size_t i = 0; i < recent.size(); ++i) {
            out << "    \"" << escapeJson(recent[i]) << "\"";
            if (i + 1 < recent.size()) out << ",";
            out << "\n";
        }
        out << "  ]\n";
        out << "}\n";

        NF_LOG_INFO("Editor", "Editor state saved to " + path);
    }

    /// Load editor state from Saved/editor_state.json if it exists.
    void loadEditorState() {
        std::string path = savedDir() + "/editor_state.json";
        std::ifstream in(path);
        if (!in.is_open()) {
            NF_LOG_INFO("Editor", "No saved editor state found (first run)");
            return;
        }

        std::string json((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());

        // Minimal JSON extraction — we use simple key scanning since
        // the format is our own well-known structure.
        auto extractFloat = [&](const std::string& key) -> float {
            auto pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos) return -1.f;
            pos = json.find(':', pos);
            if (pos == std::string::npos) return -1.f;
            try { return std::stof(json.substr(pos + 1)); }
            catch (...) { return -1.f; }
        };

        auto extractBool = [&](const std::string& key) -> int {
            auto pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos) return -1;
            pos = json.find(':', pos);
            if (pos == std::string::npos) return -1;
            auto val = json.substr(pos + 1, 10);
            if (val.find("true") != std::string::npos) return 1;
            if (val.find("false") != std::string::npos) return 0;
            return -1;
        };

        // Restore dark mode
        int darkMode = extractBool("darkMode");
        if (darkMode >= 0) {
            m_editorSettings.setDarkMode(darkMode == 1);
            m_editorSettings.applyTheme(m_theme);
        }

        // Restore dock sizes
        float left = extractFloat("left");
        float right = extractFloat("right");
        float top = extractFloat("top");
        float bottom = extractFloat("bottom");
        if (left > 0.f)   m_dockLayout.setLeftWidth(left);
        if (right > 0.f)  m_dockLayout.setRightWidth(right);
        if (top > 0.f)    m_dockLayout.setTopHeight(top);
        if (bottom > 0.f) m_dockLayout.setBottomHeight(bottom);

        // Restore panel visibility
        for (auto& dp : m_dockLayout.panels()) {
            std::string key = "\"name\": \"" + dp.name + "\"";
            auto pos = json.find(key);
            if (pos != std::string::npos) {
                auto vis = json.find("\"visible\"", pos);
                if (vis != std::string::npos && vis < pos + 200) {
                    auto colon = json.find(':', vis);
                    if (colon != std::string::npos) {
                        auto val = json.substr(colon + 1, 10);
                        bool visible = val.find("true") != std::string::npos;
                        m_dockLayout.setPanelVisible(dp.name, visible);
                    }
                }
            }
        }

        NF_LOG_INFO("Editor", "Editor state restored from " + path);
    }
};

// ── Property Editor ──────────────────────────────────────────────
// Reads and writes property values via reflection offsets.

class PropertyEditor {
public:
    // Read a property value from a struct via offset, returning a JSON-like representation
    static JsonValue readProperty(const void* obj, const PropertyInfo& prop) {
        const uint8_t* base = static_cast<const uint8_t*>(obj) + prop.offset;

        switch (prop.type) {
            case PropertyType::Bool:
                return JsonValue(*reinterpret_cast<const bool*>(base));
            case PropertyType::Int32:
                return JsonValue(*reinterpret_cast<const int32_t*>(base));
            case PropertyType::Float:
                return JsonValue(*reinterpret_cast<const float*>(base));
            case PropertyType::String:
                return JsonValue(*reinterpret_cast<const std::string*>(base));
            case PropertyType::Vec3: {
                const auto* v = reinterpret_cast<const Vec3*>(base);
                auto arr = JsonValue::array();
                arr.push(JsonValue(v->x));
                arr.push(JsonValue(v->y));
                arr.push(JsonValue(v->z));
                return arr;
            }
            case PropertyType::Color: {
                const auto* c = reinterpret_cast<const Color*>(base);
                auto arr = JsonValue::array();
                arr.push(JsonValue(c->r));
                arr.push(JsonValue(c->g));
                arr.push(JsonValue(c->b));
                arr.push(JsonValue(c->a));
                return arr;
            }
            default:
                return JsonValue();
        }
    }

    // Write a property value to a struct via offset
    static bool writeProperty(void* obj, const PropertyInfo& prop, const JsonValue& value) {
        uint8_t* base = static_cast<uint8_t*>(obj) + prop.offset;

        switch (prop.type) {
            case PropertyType::Bool:
                if (!value.isBool()) return false;
                *reinterpret_cast<bool*>(base) = value.asBool();
                return true;
            case PropertyType::Int32:
                if (!value.isNumber()) return false;
                *reinterpret_cast<int32_t*>(base) = value.asInt();
                return true;
            case PropertyType::Float:
                if (!value.isNumber()) return false;
                *reinterpret_cast<float*>(base) = value.asFloat();
                return true;
            case PropertyType::String:
                if (!value.isString()) return false;
                *reinterpret_cast<std::string*>(base) = value.asString();
                return true;
            case PropertyType::Vec3: {
                if (!value.isArray() || value.size() < 3) return false;
                auto* v = reinterpret_cast<Vec3*>(base);
                v->x = value[static_cast<size_t>(0)].asFloat();
                v->y = value[static_cast<size_t>(1)].asFloat();
                v->z = value[static_cast<size_t>(2)].asFloat();
                return true;
            }
            case PropertyType::Color: {
                if (!value.isArray() || value.size() < 4) return false;
                auto* c = reinterpret_cast<Color*>(base);
                c->r = value[static_cast<size_t>(0)].asFloat();
                c->g = value[static_cast<size_t>(1)].asFloat();
                c->b = value[static_cast<size_t>(2)].asFloat();
                c->a = value[static_cast<size_t>(3)].asFloat();
                return true;
            }
            default:
                return false;
        }
    }

    // Create an undo-safe property change for a float property
    static std::unique_ptr<ICommand> makeFloatChange(float* target, float newValue,
                                                      const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<float>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for an int property
    static std::unique_ptr<ICommand> makeIntChange(int32_t* target, int32_t newValue,
                                                    const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<int32_t>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a bool property
    static std::unique_ptr<ICommand> makeBoolChange(bool* target, bool newValue,
                                                     const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<bool>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a Vec3 property
    static std::unique_ptr<ICommand> makeVec3Change(Vec3* target, Vec3 newValue,
                                                     const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<Vec3>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a Color property
    static std::unique_ptr<ICommand> makeColorChange(Color* target, Color newValue,
                                                      const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<Color>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a string property
    static std::unique_ptr<ICommand> makeStringChange(std::string* target,
                                                       const std::string& newValue,
                                                       const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<std::string>>(
            target, *target, newValue, "Change " + propName);
    }
};

// ── S6 — PCG World Tuning ────────────────────────────────────────

enum class BiomeBrushType : uint8_t {
    Paint, Erase, Smooth, Raise, Lower, Flatten, Noise, Fill
};

inline const char* biomeBrushTypeName(BiomeBrushType t) {
    switch (t) {
        case BiomeBrushType::Paint:   return "Paint";
        case BiomeBrushType::Erase:   return "Erase";
        case BiomeBrushType::Smooth:  return "Smooth";
        case BiomeBrushType::Raise:   return "Raise";
        case BiomeBrushType::Lower:   return "Lower";
        case BiomeBrushType::Flatten: return "Flatten";
        case BiomeBrushType::Noise:   return "Noise";
        case BiomeBrushType::Fill:    return "Fill";
    }
    return "Unknown";
}

struct BiomePaintCell {
    int x = 0;
    int y = 0;
    int biomeIndex = 0;
    float intensity = 1.f;
};

class BiomePainter {
public:
    explicit BiomePainter(int gridSize = 64)
        : m_gridSize(gridSize > 256 ? 256 : (gridSize < 1 ? 1 : gridSize)) {}

    void paint(int x, int y) {
        if (x < 0 || x >= m_gridSize || y < 0 || y >= m_gridSize) return;
        auto* existing = cellAtMut(x, y);
        if (existing) {
            existing->biomeIndex = m_activeBiome;
            existing->intensity = m_brushIntensity;
            m_dirty = true;
            return;
        }
        if (m_cells.size() >= kMaxCells) return; // at capacity
        {
            BiomePaintCell c;
            c.x = x;
            c.y = y;
            c.biomeIndex = m_activeBiome;
            c.intensity = m_brushIntensity;
            m_cells.push_back(c);
        }
        m_dirty = true;
    }

    void erase(int x, int y) {
        if (x < 0 || x >= m_gridSize || y < 0 || y >= m_gridSize) return;
        auto* existing = cellAtMut(x, y);
        if (existing) {
            existing->biomeIndex = 0;
            m_dirty = true;
        }
    }

    void fill(int biomeIdx) {
        m_cells.clear();
        m_cells.reserve(static_cast<size_t>(m_gridSize) * static_cast<size_t>(m_gridSize));
        for (int cy = 0; cy < m_gridSize; ++cy) {
            for (int cx = 0; cx < m_gridSize; ++cx) {
                BiomePaintCell c;
                c.x = cx;
                c.y = cy;
                c.biomeIndex = biomeIdx;
                c.intensity = m_brushIntensity;
                m_cells.push_back(c);
            }
        }
        m_dirty = true;
    }

    [[nodiscard]] const BiomePaintCell* cellAt(int x, int y) const {
        for (auto& c : m_cells) {
            if (c.x == x && c.y == y) return &c;
        }
        return nullptr;
    }

    void clear() { m_cells.clear(); m_dirty = true; }

    [[nodiscard]] size_t cellCount() const { return m_cells.size(); }
    [[nodiscard]] int gridSize() const { return m_gridSize; }

    void setActiveBrush(BiomeBrushType b) { m_activeBrush = b; }
    [[nodiscard]] BiomeBrushType activeBrush() const { return m_activeBrush; }

    void setBrushRadius(float r) { m_brushRadius = (r < 0.f ? 0.f : (r > 16.f ? 16.f : r)); }
    [[nodiscard]] float brushRadius() const { return m_brushRadius; }

    void setBrushIntensity(float i) { m_brushIntensity = (i < 0.f ? 0.f : (i > 1.f ? 1.f : i)); }
    [[nodiscard]] float brushIntensity() const { return m_brushIntensity; }

    void setActiveBiome(int idx) { m_activeBiome = idx; }
    [[nodiscard]] int activeBiome() const { return m_activeBiome; }

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }
    void markDirty() { m_dirty = true; }

    static constexpr size_t kMaxCells = 256 * 256;

private:
    BiomePaintCell* cellAtMut(int x, int y) {
        for (auto& c : m_cells) {
            if (c.x == x && c.y == y) return &c;
        }
        return nullptr;
    }

    int m_gridSize = 64;
    BiomeBrushType m_activeBrush = BiomeBrushType::Paint;
    float m_brushRadius = 1.f;
    float m_brushIntensity = 1.f;
    int m_activeBiome = 0;
    bool m_dirty = false;
    std::vector<BiomePaintCell> m_cells;
};

// ── StructureSeedBank ────────────────────────────────────────────

struct StructureSeedOverride {
    std::string structureId;
    uint64_t    overrideSeed = 0;
    bool        locked = false;
    std::string notes;
};

class StructureSeedBank {
public:
    static constexpr size_t kMaxOverrides = 128;

    bool addOverride(StructureSeedOverride ov) {
        if (m_overrides.size() >= kMaxOverrides) return false;
        for (auto& o : m_overrides) {
            if (o.structureId == ov.structureId) return false;
        }
        m_overrides.push_back(std::move(ov));
        return true;
    }

    bool removeOverride(const std::string& structureId) {
        auto it = std::find_if(m_overrides.begin(), m_overrides.end(),
                               [&](const StructureSeedOverride& o) { return o.structureId == structureId; });
        if (it == m_overrides.end()) return false;
        m_overrides.erase(it);
        return true;
    }

    [[nodiscard]] const StructureSeedOverride* findOverride(const std::string& structureId) const {
        for (auto& o : m_overrides) {
            if (o.structureId == structureId) return &o;
        }
        return nullptr;
    }

    StructureSeedOverride* findOverride(const std::string& structureId) {
        for (auto& o : m_overrides) {
            if (o.structureId == structureId) return &o;
        }
        return nullptr;
    }

    bool lockOverride(const std::string& structureId) {
        auto* ov = findOverride(structureId);
        if (!ov) return false;
        ov->locked = true;
        return true;
    }

    bool unlockOverride(const std::string& structureId) {
        auto* ov = findOverride(structureId);
        if (!ov) return false;
        ov->locked = false;
        return true;
    }

    [[nodiscard]] size_t overrideCount() const { return m_overrides.size(); }
    [[nodiscard]] const std::vector<StructureSeedOverride>& overrides() const { return m_overrides; }
    void clear() { m_overrides.clear(); }

    [[nodiscard]] size_t lockedCount() const {
        size_t n = 0;
        for (auto& o : m_overrides) { if (o.locked) ++n; }
        return n;
    }

private:
    std::vector<StructureSeedOverride> m_overrides;
};

// ── OreSeamEditor ────────────────────────────────────────────────

enum class OreSeamType : uint8_t {
    Iron, Copper, Gold, Silver, Titanium, Uranium, Crystal, Exotic
};

inline const char* oreSeamTypeName(OreSeamType t) {
    switch (t) {
        case OreSeamType::Iron:     return "Iron";
        case OreSeamType::Copper:   return "Copper";
        case OreSeamType::Gold:     return "Gold";
        case OreSeamType::Silver:   return "Silver";
        case OreSeamType::Titanium: return "Titanium";
        case OreSeamType::Uranium:  return "Uranium";
        case OreSeamType::Crystal:  return "Crystal";
        case OreSeamType::Exotic:   return "Exotic";
    }
    return "Unknown";
}

struct OreSeamDef {
    std::string id;
    OreSeamType type = OreSeamType::Iron;
    Vec3 position;
    float radius = 5.f;
    float density = 0.5f;
    float depth = 10.f;
    uint64_t seed = 0;

    [[nodiscard]] float volume() const { return 4.f / 3.f * 3.14159265f * radius * radius * radius * density; }
};

class OreSeamEditor {
public:
    static constexpr size_t kMaxSeams = 64;

    bool addSeam(OreSeamDef seam) {
        if (m_seams.size() >= kMaxSeams) return false;
        for (auto& s : m_seams) {
            if (s.id == seam.id) return false;
        }
        m_seams.push_back(std::move(seam));
        return true;
    }

    bool removeSeam(const std::string& id) {
        auto it = std::find_if(m_seams.begin(), m_seams.end(),
                               [&](const OreSeamDef& s) { return s.id == id; });
        if (it == m_seams.end()) return false;
        m_seams.erase(it);
        return true;
    }

    [[nodiscard]] const OreSeamDef* findSeam(const std::string& id) const {
        for (auto& s : m_seams) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    OreSeamDef* findSeam(const std::string& id) {
        for (auto& s : m_seams) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    [[nodiscard]] std::vector<const OreSeamDef*> seamsOfType(OreSeamType type) const {
        std::vector<const OreSeamDef*> result;
        for (auto& s : m_seams) {
            if (s.type == type) result.push_back(&s);
        }
        return result;
    }

    [[nodiscard]] float totalVolume() const {
        float v = 0.f;
        for (auto& s : m_seams) v += s.volume();
        return v;
    }

    [[nodiscard]] size_t seamCount() const { return m_seams.size(); }
    [[nodiscard]] const std::vector<OreSeamDef>& seams() const { return m_seams; }
    void clear() { m_seams.clear(); }

private:
    std::vector<OreSeamDef> m_seams;
};

// ── PCGPreviewRenderer ──────────────────────────────────────────

enum class PCGPreviewMode : uint8_t {
    Heightmap, Biome, Moisture, OreDeposits, Structures, Combined, Wireframe, Heatmap
};

inline const char* pcgPreviewModeName(PCGPreviewMode m) {
    switch (m) {
        case PCGPreviewMode::Heightmap:   return "Heightmap";
        case PCGPreviewMode::Biome:       return "Biome";
        case PCGPreviewMode::Moisture:    return "Moisture";
        case PCGPreviewMode::OreDeposits: return "OreDeposits";
        case PCGPreviewMode::Structures:  return "Structures";
        case PCGPreviewMode::Combined:    return "Combined";
        case PCGPreviewMode::Wireframe:   return "Wireframe";
        case PCGPreviewMode::Heatmap:     return "Heatmap";
    }
    return "Unknown";
}

struct PCGPreviewSettings {
    PCGPreviewMode mode = PCGPreviewMode::Combined;
    int resolution = 128;
    float zoom = 1.f;
    bool autoRefresh = true;
    bool showGrid = true;
    bool showLabels = false;
    uint64_t seed = 0;
};

class PCGPreviewRenderer {
public:
    void setSettings(const PCGPreviewSettings& s) {
        m_settings = s;
        clampSettings();
    }
    [[nodiscard]] const PCGPreviewSettings& settings() const { return m_settings; }

    void setResolution(int r) {
        m_settings.resolution = (r < 32 ? 32 : (r > 512 ? 512 : r));
    }

    void setZoom(float z) {
        m_settings.zoom = (z < 0.1f ? 0.1f : (z > 10.f ? 10.f : z));
    }

    void setMode(PCGPreviewMode mode) { m_settings.mode = mode; }

    void refresh() { m_stale = true; ++m_refreshCount; }
    [[nodiscard]] size_t refreshCount() const { return m_refreshCount; }
    [[nodiscard]] bool isStale() const { return m_stale; }
    void markFresh() { m_stale = false; }

    void setPreviewData(std::vector<float> data) { m_previewData = std::move(data); }
    [[nodiscard]] const std::vector<float>& previewData() const { return m_previewData; }
    [[nodiscard]] size_t previewPixelCount() const {
        return static_cast<size_t>(m_settings.resolution) * static_cast<size_t>(m_settings.resolution);
    }

    void clear() {
        m_settings = PCGPreviewSettings{};
        m_previewData.clear();
        m_stale = false;
        m_refreshCount = 0;
    }

private:
    void clampSettings() {
        if (m_settings.resolution < 32) m_settings.resolution = 32;
        if (m_settings.resolution > 512) m_settings.resolution = 512;
        if (m_settings.zoom < 0.1f) m_settings.zoom = 0.1f;
        if (m_settings.zoom > 10.f) m_settings.zoom = 10.f;
    }

    PCGPreviewSettings m_settings;
    std::vector<float> m_previewData;
    bool m_stale = false;
    size_t m_refreshCount = 0;
};

// ---------------------------------------------------------------------------
// S7 — Logic Wiring UI
// ---------------------------------------------------------------------------

enum class LogicPinType : uint8_t {
    Flow, Bool, Int, Float, String, Vector, Event, Object
};

inline const char* logicPinTypeName(LogicPinType t) {
    switch (t) {
        case LogicPinType::Flow:   return "Flow";
        case LogicPinType::Bool:   return "Bool";
        case LogicPinType::Int:    return "Int";
        case LogicPinType::Float:  return "Float";
        case LogicPinType::String: return "String";
        case LogicPinType::Vector: return "Vector";
        case LogicPinType::Event:  return "Event";
        case LogicPinType::Object: return "Object";
    }
    return "Unknown";
}

struct LogicPin {
    std::string   id;
    std::string   name;
    LogicPinType  type      = LogicPinType::Flow;
    bool          isOutput  = false;
    bool          connected = false;
    float         value     = 0.f;
};

enum class LogicNodeType : uint8_t {
    AndGate, OrGate, NotGate, Latch, Delay, Switch, Compare, MathOp
};

inline const char* logicNodeTypeName(LogicNodeType t) {
    switch (t) {
        case LogicNodeType::AndGate: return "AND Gate";
        case LogicNodeType::OrGate:  return "OR Gate";
        case LogicNodeType::NotGate: return "NOT Gate";
        case LogicNodeType::Latch:   return "Latch";
        case LogicNodeType::Delay:   return "Delay";
        case LogicNodeType::Switch:  return "Switch";
        case LogicNodeType::Compare: return "Compare";
        case LogicNodeType::MathOp:  return "Math Op";
    }
    return "Unknown";
}

struct LogicNodeDef {
    std::string   name;
    LogicNodeType nodeType = LogicNodeType::AndGate;
    std::vector<LogicPin> inputs;
    std::vector<LogicPin> outputs;
    std::string   description;
};

class LogicWireNode {
public:
    void setId(int id) { m_id = id; }
    [[nodiscard]] int id() const { return m_id; }

    void setName(const std::string& name) { m_name = name; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    void setNodeType(LogicNodeType type) { m_nodeType = type; }
    [[nodiscard]] LogicNodeType nodeType() const { return m_nodeType; }

    bool addInput(const LogicPin& pin) {
        if (m_inputs.size() >= kMaxPins) return false;
        m_inputs.push_back(pin);
        return true;
    }

    bool addOutput(const LogicPin& pin) {
        if (m_outputs.size() >= kMaxPins) return false;
        m_outputs.push_back(pin);
        return true;
    }

    [[nodiscard]] const std::vector<LogicPin>& inputs() const { return m_inputs; }
    [[nodiscard]] const std::vector<LogicPin>& outputs() const { return m_outputs; }

    inline LogicPin* findInput(const std::string& pinId) {
        for (auto& p : m_inputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }
    inline LogicPin* findOutput(const std::string& pinId) {
        for (auto& p : m_outputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }
    inline const LogicPin* findInput(const std::string& pinId) const {
        for (const auto& p : m_inputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }
    inline const LogicPin* findOutput(const std::string& pinId) const {
        for (const auto& p : m_outputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }

    [[nodiscard]] size_t inputCount() const { return m_inputs.size(); }
    [[nodiscard]] size_t outputCount() const { return m_outputs.size(); }

    inline void evaluate() {
        float result = 0.f;
        switch (m_nodeType) {
            case LogicNodeType::AndGate: {
                result = 1.f;
                for (const auto& in : m_inputs) {
                    if (in.value <= 0.5f) { result = 0.f; break; }
                }
                if (m_inputs.empty()) result = 0.f;
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::OrGate: {
                result = 0.f;
                for (const auto& in : m_inputs) {
                    if (in.value > 0.5f) { result = 1.f; break; }
                }
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::NotGate: {
                if (m_inputs.empty()) {
                    result = 1.f;
                } else {
                    result = (m_inputs[0].value <= 0.5f) ? 1.f : 0.f;
                }
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::Latch: {
                if (!m_inputs.empty()) {
                    result = m_inputs[0].value;
                    for (auto& out : m_outputs) out.value = result;
                }
                break;
            }
            case LogicNodeType::Compare: {
                if (m_inputs.size() >= 2) {
                    result = (m_inputs[0].value == m_inputs[1].value) ? 1.f : 0.f;
                } else {
                    result = 0.f;
                }
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::MathOp: {
                result = 0.f;
                for (const auto& in : m_inputs) result += in.value;
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::Delay:
            case LogicNodeType::Switch:
            default: {
                result = m_inputs.empty() ? 0.f : m_inputs[0].value;
                for (auto& out : m_outputs) out.value = result;
                break;
            }
        }
    }

    static constexpr size_t kMaxPins = 16;

private:
    int m_id = 0;
    std::string m_name;
    LogicNodeType m_nodeType = LogicNodeType::AndGate;
    std::vector<LogicPin> m_inputs;
    std::vector<LogicPin> m_outputs;
};

struct LogicWire {
    int sourceNodeId = -1;
    std::string sourcePin;
    int targetNodeId = -1;
    std::string targetPin;
};

class LogicWireGraph {
public:
    inline int addNode(LogicWireNode node) {
        if (m_nodes.size() >= kMaxNodes) return -1;
        int nid = m_nextId++;
        node.setId(nid);
        m_nodes.push_back(std::move(node));
        return nid;
    }

    inline bool removeNode(int id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [id](const LogicWireNode& n) { return n.id() == id; });
        if (it == m_nodes.end()) return false;
        // Remove wires referencing this node
        m_wires.erase(
            std::remove_if(m_wires.begin(), m_wires.end(),
                [id](const LogicWire& w) {
                    return w.sourceNodeId == id || w.targetNodeId == id;
                }),
            m_wires.end());
        m_nodes.erase(it);
        return true;
    }

    [[nodiscard]] inline const LogicWireNode* findNode(int id) const {
        for (const auto& n : m_nodes) { if (n.id() == id) return &n; }
        return nullptr;
    }

    inline LogicWireNode* findNode(int id) {
        for (auto& n : m_nodes) { if (n.id() == id) return &n; }
        return nullptr;
    }

    inline bool addWire(const LogicWire& wire) {
        if (m_wires.size() >= kMaxWires) return false;
        if (!findNode(wire.sourceNodeId) || !findNode(wire.targetNodeId)) return false;
        m_wires.push_back(wire);
        return true;
    }

    inline bool removeWire(size_t index) {
        if (index >= m_wires.size()) return false;
        m_wires.erase(m_wires.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t wireCount() const { return m_wires.size(); }

    [[nodiscard]] const std::vector<LogicWireNode>& nodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<LogicWire>& wires() const { return m_wires; }

    void clear() {
        m_nodes.clear();
        m_wires.clear();
        m_nextId = 1;
    }

    [[nodiscard]] inline bool isValid() const {
        for (const auto& w : m_wires) {
            if (!findNode(w.sourceNodeId) || !findNode(w.targetNodeId))
                return false;
        }
        return true;
    }

    inline void evaluate() {
        for (auto& n : m_nodes) n.evaluate();
    }

    static constexpr size_t kMaxNodes = 128;
    static constexpr size_t kMaxWires = 256;

private:
    std::vector<LogicWireNode> m_nodes;
    std::vector<LogicWire> m_wires;
    int m_nextId = 1;
};

struct LogicGraphTemplate {
    std::string name;
    std::string description;
    std::string category;
    std::vector<LogicNodeDef> nodeDefs;
};

class LogicTemplateLibrary {
public:
    inline bool addTemplate(const LogicGraphTemplate& tmpl) {
        if (m_templates.size() >= kMaxTemplates) return false;
        for (const auto& t : m_templates) {
            if (t.name == tmpl.name) return false;
        }
        m_templates.push_back(tmpl);
        return true;
    }

    inline bool removeTemplate(const std::string& name) {
        auto it = std::find_if(m_templates.begin(), m_templates.end(),
            [&name](const LogicGraphTemplate& t) { return t.name == name; });
        if (it == m_templates.end()) return false;
        m_templates.erase(it);
        return true;
    }

    [[nodiscard]] inline const LogicGraphTemplate* findTemplate(const std::string& name) const {
        for (const auto& t : m_templates) {
            if (t.name == name) return &t;
        }
        return nullptr;
    }

    [[nodiscard]] inline std::vector<const LogicGraphTemplate*>
    templatesInCategory(const std::string& category) const {
        std::vector<const LogicGraphTemplate*> result;
        for (const auto& t : m_templates) {
            if (t.category == category) result.push_back(&t);
        }
        return result;
    }

    [[nodiscard]] size_t templateCount() const { return m_templates.size(); }
    [[nodiscard]] const std::vector<LogicGraphTemplate>& templates() const { return m_templates; }

    void clear() { m_templates.clear(); }

    [[nodiscard]] inline size_t categoryCount() const {
        std::set<std::string> cats;
        for (const auto& t : m_templates) cats.insert(t.category);
        return cats.size();
    }

    static constexpr size_t kMaxTemplates = 64;

private:
    std::vector<LogicGraphTemplate> m_templates;
};

// ── S8 — Tool Ecosystem ──────────────────────────────────────────

enum class ToolStatus : uint8_t {
    Stopped, Starting, Running, Unhealthy, Stopping, Crashed, Unknown, Disabled
};

inline const char* toolStatusName(ToolStatus s) {
    switch (s) {
        case ToolStatus::Stopped:   return "Stopped";
        case ToolStatus::Starting:  return "Starting";
        case ToolStatus::Running:   return "Running";
        case ToolStatus::Unhealthy: return "Unhealthy";
        case ToolStatus::Stopping:  return "Stopping";
        case ToolStatus::Crashed:   return "Crashed";
        case ToolStatus::Unknown:   return "Unknown";
        case ToolStatus::Disabled:  return "Disabled";
    }
    return "Unknown";
}

struct ToolInstanceInfo {
    std::string name;
    std::string executablePath;
    ToolStatus status = ToolStatus::Stopped;
    int pid = -1;
    float uptimeSeconds = 0.f;
    size_t eventsHandled = 0;
    float lastHeartbeatAge = 0.f;  // seconds since last heartbeat
};

struct ToolEcosystemConfig {
    std::string pipelineDir = ".novaforge/pipeline";
    float heartbeatIntervalSec = 5.f;
    float unhealthyThresholdSec = 15.f;
    float crashThresholdSec = 30.f;
    size_t maxEventsPerTick = 16;
    bool autoRestart = true;
};

class StandaloneToolRunner {
public:
    void setName(const std::string& n) { m_info.name = n; }
    void setExecutablePath(const std::string& p) { m_info.executablePath = p; }

    [[nodiscard]] const std::string& name() const { return m_info.name; }
    [[nodiscard]] const std::string& executablePath() const { return m_info.executablePath; }
    [[nodiscard]] ToolStatus status() const { return m_info.status; }
    [[nodiscard]] const ToolInstanceInfo& info() const { return m_info; }
    [[nodiscard]] float uptimeSeconds() const { return m_info.uptimeSeconds; }
    [[nodiscard]] size_t eventsHandled() const { return m_info.eventsHandled; }

    bool start() {
        if (m_info.status == ToolStatus::Running || m_info.status == ToolStatus::Starting) return false;
        m_info.status = ToolStatus::Starting;
        m_info.pid = static_cast<int>(std::hash<std::string>{}(m_info.name) % 65536);
        m_info.uptimeSeconds = 0.f;
        m_info.eventsHandled = 0;
        m_info.status = ToolStatus::Running;
        return true;
    }

    bool stop() {
        if (m_info.status != ToolStatus::Running && m_info.status != ToolStatus::Unhealthy) return false;
        m_info.status = ToolStatus::Stopping;
        m_info.pid = -1;
        m_info.status = ToolStatus::Stopped;
        return true;
    }

    void recordHeartbeat() {
        m_info.lastHeartbeatAge = 0.f;
    }

    void recordEvent() {
        m_info.eventsHandled++;
    }

    void tickUptime(float dt) {
        if (m_info.status == ToolStatus::Running || m_info.status == ToolStatus::Unhealthy) {
            m_info.uptimeSeconds += dt;
            m_info.lastHeartbeatAge += dt;
        }
    }

    void markCrashed() {
        m_info.status = ToolStatus::Crashed;
        m_info.pid = -1;
    }

    void markUnhealthy() {
        if (m_info.status == ToolStatus::Running)
            m_info.status = ToolStatus::Unhealthy;
    }

    [[nodiscard]] bool isAlive() const {
        return m_info.status == ToolStatus::Running || m_info.status == ToolStatus::Unhealthy;
    }

private:
    ToolInstanceInfo m_info;
};

class ToolHealthMonitor {
public:
    void setConfig(const ToolEcosystemConfig& config) { m_config = config; }

    void addRunner(StandaloneToolRunner* runner) {
        if (m_runners.size() < kMaxTools && runner)
            m_runners.push_back(runner);
    }

    void removeRunner(const std::string& name) {
        m_runners.erase(
            std::remove_if(m_runners.begin(), m_runners.end(),
                [&name](const StandaloneToolRunner* r) { return r->name() == name; }),
            m_runners.end());
    }

    void checkHealth() {
        for (auto* r : m_runners) {
            if (!r->isAlive()) continue;
            if (r->info().lastHeartbeatAge >= m_config.crashThresholdSec) {
                r->markCrashed();
            } else if (r->info().lastHeartbeatAge >= m_config.unhealthyThresholdSec) {
                r->markUnhealthy();
            }
        }
    }

    [[nodiscard]] size_t healthyCount() const {
        size_t count = 0;
        for (const auto* r : m_runners) {
            if (r->status() == ToolStatus::Running) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t unhealthyCount() const {
        size_t count = 0;
        for (const auto* r : m_runners) {
            if (r->status() == ToolStatus::Unhealthy) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t crashedCount() const {
        size_t count = 0;
        for (const auto* r : m_runners) {
            if (r->status() == ToolStatus::Crashed) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t runnerCount() const { return m_runners.size(); }

    static constexpr size_t kMaxTools = 8;

private:
    std::vector<StandaloneToolRunner*> m_runners;
    ToolEcosystemConfig m_config;
};

class ToolOrchestrator {
public:
    ToolOrchestrator() {
        m_swissAgent.setName("SwissAgent");
        m_swissAgent.setExecutablePath("Atlas/Workspace/SwissAgent/cli.py");
        m_arbiter.setName("ArbiterAI");
        m_arbiter.setExecutablePath("Atlas/Workspace/Arbiter/arbiter_cli.py");
        m_contractScanner.setName("ContractScanner");
        m_contractScanner.setExecutablePath("tools/contract_scanner");
        m_replayMinimizer.setName("ReplayMinimizer");
        m_replayMinimizer.setExecutablePath("tools/replay_minimizer");
    }

    bool startAll() {
        bool ok = true;
        ok &= m_swissAgent.start();
        ok &= m_arbiter.start();
        ok &= m_contractScanner.start();
        ok &= m_replayMinimizer.start();
        return ok;
    }

    bool stopAll() {
        bool ok = true;
        ok &= m_swissAgent.stop();
        ok &= m_arbiter.stop();
        ok &= m_contractScanner.stop();
        ok &= m_replayMinimizer.stop();
        return ok;
    }

    StandaloneToolRunner* runner(const std::string& name) {
        if (name == "SwissAgent") return &m_swissAgent;
        if (name == "ArbiterAI") return &m_arbiter;
        if (name == "ContractScanner") return &m_contractScanner;
        if (name == "ReplayMinimizer") return &m_replayMinimizer;
        return nullptr;
    }

    const StandaloneToolRunner* runner(const std::string& name) const {
        if (name == "SwissAgent") return &m_swissAgent;
        if (name == "ArbiterAI") return &m_arbiter;
        if (name == "ContractScanner") return &m_contractScanner;
        if (name == "ReplayMinimizer") return &m_replayMinimizer;
        return nullptr;
    }

    [[nodiscard]] size_t runningCount() const {
        size_t c = 0;
        if (m_swissAgent.isAlive()) ++c;
        if (m_arbiter.isAlive()) ++c;
        if (m_contractScanner.isAlive()) ++c;
        if (m_replayMinimizer.isAlive()) ++c;
        return c;
    }

    [[nodiscard]] size_t totalEventsHandled() const {
        return m_swissAgent.eventsHandled() + m_arbiter.eventsHandled() +
               m_contractScanner.eventsHandled() + m_replayMinimizer.eventsHandled();
    }

    void tickAll(float dt) {
        m_swissAgent.tickUptime(dt);
        m_arbiter.tickUptime(dt);
        m_contractScanner.tickUptime(dt);
        m_replayMinimizer.tickUptime(dt);
    }

    static constexpr size_t kToolCount = 4;

private:
    StandaloneToolRunner m_swissAgent;
    StandaloneToolRunner m_arbiter;
    StandaloneToolRunner m_contractScanner;
    StandaloneToolRunner m_replayMinimizer;
};

class ToolEcosystem {
public:
    void init(const ToolEcosystemConfig& config = {}) {
        m_config = config;
        m_monitor.setConfig(config);
        m_monitor.addRunner(m_orchestrator.runner("SwissAgent"));
        m_monitor.addRunner(m_orchestrator.runner("ArbiterAI"));
        m_monitor.addRunner(m_orchestrator.runner("ContractScanner"));
        m_monitor.addRunner(m_orchestrator.runner("ReplayMinimizer"));
        m_initialized = true;
    }

    void shutdown() {
        m_orchestrator.stopAll();
        m_initialized = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool startAll() { return m_orchestrator.startAll(); }
    bool stopAll() { return m_orchestrator.stopAll(); }

    void tick(float dt) {
        m_orchestrator.tickAll(dt);
        m_monitor.checkHealth();
        m_tickCount++;

        if (m_config.autoRestart) {
            autoRestartCrashed();
        }
    }

    [[nodiscard]] const ToolOrchestrator& orchestrator() const { return m_orchestrator; }
    [[nodiscard]] ToolOrchestrator& orchestrator() { return m_orchestrator; }
    [[nodiscard]] const ToolHealthMonitor& monitor() const { return m_monitor; }
    [[nodiscard]] const ToolEcosystemConfig& config() const { return m_config; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

    [[nodiscard]] size_t healthyToolCount() const { return m_monitor.healthyCount(); }
    [[nodiscard]] size_t totalEventsHandled() const { return m_orchestrator.totalEventsHandled(); }

private:
    void autoRestartCrashed() {
        for (const char* name : {"SwissAgent", "ArbiterAI", "ContractScanner", "ReplayMinimizer"}) {
            auto* r = m_orchestrator.runner(name);
            if (r && r->status() == ToolStatus::Crashed) {
                r->start();
            }
        }
    }

    ToolEcosystemConfig m_config;
    ToolOrchestrator m_orchestrator;
    ToolHealthMonitor m_monitor;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ── S9 — AtlasAI Integration ────────────────────────────────────

enum class AIInsightType : uint8_t {
    CodeQuality,
    PerformanceHint,
    AssetOptimization,
    LogicBug,
    SecurityRisk,
    Refactoring,
    Documentation,
    General
};

inline const char* aiInsightTypeName(AIInsightType t) noexcept {
    switch (t) {
        case AIInsightType::CodeQuality:       return "CodeQuality";
        case AIInsightType::PerformanceHint:   return "PerformanceHint";
        case AIInsightType::AssetOptimization: return "AssetOptimization";
        case AIInsightType::LogicBug:          return "LogicBug";
        case AIInsightType::SecurityRisk:      return "SecurityRisk";
        case AIInsightType::Refactoring:       return "Refactoring";
        case AIInsightType::Documentation:     return "Documentation";
        case AIInsightType::General:           return "General";
        default:                                return "Unknown";
    }
}

struct AIInsight {
    std::string id;
    AIInsightType type = AIInsightType::General;
    std::string title;
    std::string description;
    std::string sourceTool;
    std::string affectedPath;
    float confidence = 0.f;
    float severity = 0.f;
    int64_t timestamp = 0;
    bool dismissed = false;
    bool applied = false;

    [[nodiscard]] bool isActionable() const { return !dismissed && !applied && confidence > 0.5f; }
    void dismiss() { dismissed = true; }
    void markApplied() { applied = true; }
};

enum class AIQueryPriority : uint8_t { Low, Normal, High, Critical };

struct AIQueryRequest {
    std::string queryId;
    std::string prompt;
    std::string context;
    AIQueryPriority priority = AIQueryPriority::Normal;
    std::string targetTool;
    float timeoutSeconds = 30.f;

    [[nodiscard]] bool isValid() const { return !queryId.empty() && !prompt.empty(); }
};

struct AIQueryResponse {
    std::string queryId;
    bool success = false;
    std::string result;
    std::string error;
    float processingTimeSec = 0.f;
    std::vector<AIInsight> insights;

    [[nodiscard]] bool hasInsights() const { return !insights.empty(); }
    [[nodiscard]] size_t insightCount() const { return insights.size(); }
};

class AIAnalysisEngine {
public:
    void analyzeEvent(const std::string& eventType, const std::string& path,
                      const std::string& /*metadata*/) {
        AIInsight insight;
        insight.id = "insight_" + std::to_string(m_nextId++);
        insight.timestamp = static_cast<int64_t>(m_nextId);
        insight.affectedPath = path;
        insight.sourceTool = "AIAnalysisEngine";

        if (eventType == "AssetImported") {
            insight.type = AIInsightType::AssetOptimization;
            insight.title = "Asset import analysis";
            insight.description = "Imported asset at " + path + " may benefit from optimization";
            insight.confidence = 0.7f;
            insight.severity = 0.3f;
        } else if (eventType == "ScriptUpdated") {
            insight.type = AIInsightType::CodeQuality;
            insight.title = "Script quality check";
            insight.description = "Updated script at " + path + " should be reviewed";
            insight.confidence = 0.6f;
            insight.severity = 0.4f;
        } else if (eventType == "ContractIssue") {
            insight.type = AIInsightType::SecurityRisk;
            insight.title = "Contract issue detected";
            insight.description = "Contract issue at " + path + " requires attention";
            insight.confidence = 0.9f;
            insight.severity = 0.8f;
        } else {
            insight.type = AIInsightType::General;
            insight.title = "Pipeline event analysis";
            insight.description = "Event " + eventType + " at " + path;
            insight.confidence = 0.5f;
            insight.severity = 0.2f;
        }

        m_insights.push_back(insight);
    }

    [[nodiscard]] const std::vector<AIInsight>& insights() const { return m_insights; }
    [[nodiscard]] size_t insightCount() const { return m_insights.size(); }

    [[nodiscard]] std::vector<AIInsight> actionableInsights() const {
        std::vector<AIInsight> result;
        for (const auto& i : m_insights) {
            if (i.isActionable()) result.push_back(i);
        }
        return result;
    }

    [[nodiscard]] std::vector<AIInsight> insightsByType(AIInsightType type) const {
        std::vector<AIInsight> result;
        for (const auto& i : m_insights) {
            if (i.type == type) result.push_back(i);
        }
        return result;
    }

    void dismissInsight(const std::string& id) {
        for (auto& i : m_insights) {
            if (i.id == id) { i.dismiss(); return; }
        }
    }

    void clearDismissed() {
        m_insights.erase(
            std::remove_if(m_insights.begin(), m_insights.end(),
                [](const AIInsight& i) { return i.dismissed; }),
            m_insights.end());
    }

    void clear() { m_insights.clear(); m_nextId = 1; }

    static constexpr size_t kMaxInsights = 256;

private:
    std::vector<AIInsight> m_insights;
    size_t m_nextId = 1;
};

struct AISuggestion {
    std::string id;
    std::string title;
    std::string description;
    AIInsightType category = AIInsightType::General;
    float priority = 0.f;
    bool accepted = false;
    bool rejected = false;

    [[nodiscard]] bool isPending() const { return !accepted && !rejected; }
    void accept() { accepted = true; }
    void reject() { rejected = true; }
};

class AIProactiveSuggester {
public:
    void generateSuggestions(const AIAnalysisEngine& engine) {
        auto actionable = engine.actionableInsights();
        for (const auto& insight : actionable) {
            if (m_suggestions.size() >= kMaxSuggestions) break;
            bool alreadySuggested = false;
            for (const auto& s : m_suggestions) {
                if (s.title == insight.title) { alreadySuggested = true; break; }
            }
            if (alreadySuggested) continue;

            AISuggestion suggestion;
            suggestion.id = "sug_" + std::to_string(m_nextId++);
            suggestion.title = insight.title;
            suggestion.description = insight.description;
            suggestion.category = insight.type;
            suggestion.priority = insight.severity * insight.confidence;
            m_suggestions.push_back(suggestion);
        }
    }

    [[nodiscard]] const std::vector<AISuggestion>& suggestions() const { return m_suggestions; }
    [[nodiscard]] size_t suggestionCount() const { return m_suggestions.size(); }

    [[nodiscard]] size_t pendingCount() const {
        size_t count = 0;
        for (const auto& s : m_suggestions) {
            if (s.isPending()) ++count;
        }
        return count;
    }

    [[nodiscard]] AISuggestion* findSuggestion(const std::string& id) {
        for (auto& s : m_suggestions) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    void acceptSuggestion(const std::string& id) {
        auto* s = findSuggestion(id);
        if (s) s->accept();
    }

    void rejectSuggestion(const std::string& id) {
        auto* s = findSuggestion(id);
        if (s) s->reject();
    }

    void clearResolved() {
        m_suggestions.erase(
            std::remove_if(m_suggestions.begin(), m_suggestions.end(),
                [](const AISuggestion& s) { return !s.isPending(); }),
            m_suggestions.end());
    }

    void clear() { m_suggestions.clear(); m_nextId = 1; }

    static constexpr size_t kMaxSuggestions = 64;

private:
    std::vector<AISuggestion> m_suggestions;
    size_t m_nextId = 1;
};

class AIPipelineBridge {
public:
    explicit AIPipelineBridge(const std::string& pipelineDir = ".novaforge/pipeline")
        : m_pipelineDir(pipelineDir) {}

    void processEvent(const std::string& eventType, const std::string& path,
                      const std::string& metadata = "") {
        m_engine.analyzeEvent(eventType, path, metadata);
        m_eventsProcessed++;
        if (m_eventsProcessed % m_suggestionInterval == 0) {
            m_suggester.generateSuggestions(m_engine);
        }
    }

    AIQueryResponse submitQuery(const AIQueryRequest& request) {
        AIQueryResponse response;
        response.queryId = request.queryId;
        if (!request.isValid()) {
            response.success = false;
            response.error = "Invalid query request";
            return response;
        }
        m_queriesProcessed++;
        response.success = true;
        response.result = "Processed query: " + request.prompt;
        response.processingTimeSec = 0.01f;
        m_engine.analyzeEvent("AIAnalysis", request.context, request.prompt);
        response.insights = m_engine.insightsByType(AIInsightType::General);
        return response;
    }

    [[nodiscard]] const AIAnalysisEngine& engine() const { return m_engine; }
    [[nodiscard]] AIAnalysisEngine& engine() { return m_engine; }
    [[nodiscard]] const AIProactiveSuggester& suggester() const { return m_suggester; }
    [[nodiscard]] AIProactiveSuggester& suggester() { return m_suggester; }
    [[nodiscard]] size_t eventsProcessed() const { return m_eventsProcessed; }
    [[nodiscard]] size_t queriesProcessed() const { return m_queriesProcessed; }
    [[nodiscard]] const std::string& pipelineDir() const { return m_pipelineDir; }

    void setSuggestionInterval(size_t interval) { m_suggestionInterval = interval > 0 ? interval : 1; }

private:
    std::string m_pipelineDir;
    AIAnalysisEngine m_engine;
    AIProactiveSuggester m_suggester;
    size_t m_eventsProcessed = 0;
    size_t m_queriesProcessed = 0;
    size_t m_suggestionInterval = 5;
};

struct AtlasAIConfig {
    std::string pipelineDir = ".novaforge/pipeline";
    bool proactiveSuggestions = true;
    size_t suggestionInterval = 5;
    float analysisTickRate = 1.f;
    size_t maxInsights = 256;
    size_t maxSuggestions = 64;
};

class AtlasAIIntegration {
public:
    void init(const AtlasAIConfig& config = {}) {
        m_config = config;
        m_bridge = AIPipelineBridge(config.pipelineDir);
        m_bridge.setSuggestionInterval(config.suggestionInterval);
        m_initialized = true;
    }

    void shutdown() {
        m_bridge.engine().clear();
        m_bridge.suggester().clear();
        m_initialized = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    void processEvent(const std::string& eventType, const std::string& path,
                      const std::string& metadata = "") {
        if (!m_initialized) return;
        m_bridge.processEvent(eventType, path, metadata);
    }

    AIQueryResponse submitQuery(const AIQueryRequest& request) {
        if (!m_initialized) {
            AIQueryResponse r;
            r.queryId = request.queryId;
            r.success = false;
            r.error = "AtlasAI not initialized";
            return r;
        }
        return m_bridge.submitQuery(request);
    }

    void tick(float dt) {
        if (!m_initialized) return;
        m_tickAccumulator += dt;
        m_tickCount++;
        if (m_config.proactiveSuggestions && m_tickAccumulator >= m_config.analysisTickRate) {
            m_bridge.suggester().generateSuggestions(m_bridge.engine());
            m_tickAccumulator -= m_config.analysisTickRate;
        }
    }

    [[nodiscard]] const AIPipelineBridge& bridge() const { return m_bridge; }
    [[nodiscard]] AIPipelineBridge& bridge() { return m_bridge; }
    [[nodiscard]] const AtlasAIConfig& config() const { return m_config; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] size_t totalInsights() const { return m_bridge.engine().insightCount(); }
    [[nodiscard]] size_t totalSuggestions() const { return m_bridge.suggester().suggestionCount(); }
    [[nodiscard]] size_t pendingSuggestions() const { return m_bridge.suggester().pendingCount(); }
    [[nodiscard]] size_t totalEvents() const { return m_bridge.eventsProcessed(); }
    [[nodiscard]] size_t totalQueries() const { return m_bridge.queriesProcessed(); }

private:
    AtlasAIConfig m_config;
    AIPipelineBridge m_bridge;
    bool m_initialized = false;
    size_t m_tickCount = 0;
    float m_tickAccumulator = 0.f;
};

// ── S10 — Performance Profiler ───────────────────────────────────

enum class ProfileMetricType : uint8_t {
    FrameTime,
    CpuUsage,
    GpuUsage,
    MemoryAlloc,
    DrawCalls,
    TriangleCount,
    ScriptTime,
    NetworkLatency
};

inline const char* profileMetricTypeName(ProfileMetricType t) noexcept {
    switch (t) {
        case ProfileMetricType::FrameTime:       return "FrameTime";
        case ProfileMetricType::CpuUsage:        return "CpuUsage";
        case ProfileMetricType::GpuUsage:        return "GpuUsage";
        case ProfileMetricType::MemoryAlloc:     return "MemoryAlloc";
        case ProfileMetricType::DrawCalls:        return "DrawCalls";
        case ProfileMetricType::TriangleCount:   return "TriangleCount";
        case ProfileMetricType::ScriptTime:      return "ScriptTime";
        case ProfileMetricType::NetworkLatency:  return "NetworkLatency";
        default:                                  return "Unknown";
    }
}

struct ProfileSample {
    ProfileMetricType type = ProfileMetricType::FrameTime;
    float value = 0.f;
    double timestamp = 0.0;
    std::string tag;

    [[nodiscard]] bool hasTag() const { return !tag.empty(); }
};

struct ProfileSession {
    std::string name;
    double startTime = 0.0;
    double endTime = 0.0;
    bool active = false;
    size_t sampleCount = 0;

    void start(double time) { startTime = time; active = true; sampleCount = 0; }
    void stop(double time) { endTime = time; active = false; }
    [[nodiscard]] double duration() const { return active ? 0.0 : endTime - startTime; }
    [[nodiscard]] bool isActive() const { return active; }
};

class FrameProfiler {
public:
    void beginFrame(double timestamp) {
        m_frameStart = timestamp;
        m_inFrame = true;
    }

    void endFrame(double timestamp) {
        if (!m_inFrame) return;
        float frameDuration = static_cast<float>(timestamp - m_frameStart);
        ProfileSample sample;
        sample.type = ProfileMetricType::FrameTime;
        sample.value = frameDuration;
        sample.timestamp = timestamp;
        if (m_samples.size() < kMaxSamples) {
            m_samples.push_back(sample);
        }
        m_totalFrames++;
        m_totalFrameTime += frameDuration;
        if (frameDuration > m_peakFrameTime) m_peakFrameTime = frameDuration;
        m_inFrame = false;
    }

    void recordMetric(ProfileMetricType type, float value, double timestamp,
                      const std::string& tag = "") {
        ProfileSample sample;
        sample.type = type;
        sample.value = value;
        sample.timestamp = timestamp;
        sample.tag = tag;
        if (m_samples.size() < kMaxSamples) {
            m_samples.push_back(sample);
        }
    }

    [[nodiscard]] const std::vector<ProfileSample>& samples() const { return m_samples; }
    [[nodiscard]] size_t sampleCount() const { return m_samples.size(); }
    [[nodiscard]] size_t totalFrames() const { return m_totalFrames; }

    [[nodiscard]] float averageFrameTime() const {
        return m_totalFrames > 0 ? m_totalFrameTime / static_cast<float>(m_totalFrames) : 0.f;
    }

    [[nodiscard]] float peakFrameTime() const { return m_peakFrameTime; }

    [[nodiscard]] std::vector<ProfileSample> samplesByType(ProfileMetricType type) const {
        std::vector<ProfileSample> result;
        for (const auto& s : m_samples) {
            if (s.type == type) result.push_back(s);
        }
        return result;
    }

    void clear() {
        m_samples.clear();
        m_totalFrames = 0;
        m_totalFrameTime = 0.f;
        m_peakFrameTime = 0.f;
        m_inFrame = false;
    }

    static constexpr size_t kMaxSamples = 4096;

private:
    std::vector<ProfileSample> m_samples;
    size_t m_totalFrames = 0;
    float m_totalFrameTime = 0.f;
    float m_peakFrameTime = 0.f;
    double m_frameStart = 0.0;
    bool m_inFrame = false;
};

class MemoryProfiler {
public:
    void trackAllocation(size_t bytes, const std::string& tag = "") {
        m_currentUsage += bytes;
        m_allocationCount++;
        m_totalAllocated += bytes;
        if (m_currentUsage > m_peakUsage) m_peakUsage = m_currentUsage;
        if (!tag.empty()) m_taggedUsage[tag] += bytes;
    }

    void trackFree(size_t bytes, const std::string& tag = "") {
        m_currentUsage = (bytes > m_currentUsage) ? 0 : m_currentUsage - bytes;
        m_freeCount++;
        if (!tag.empty()) {
            auto it = m_taggedUsage.find(tag);
            if (it != m_taggedUsage.end()) {
                it->second = (bytes > it->second) ? 0 : it->second - bytes;
            }
        }
    }

    [[nodiscard]] size_t currentUsage() const { return m_currentUsage; }
    [[nodiscard]] size_t peakUsage() const { return m_peakUsage; }
    [[nodiscard]] size_t allocationCount() const { return m_allocationCount; }
    [[nodiscard]] size_t freeCount() const { return m_freeCount; }
    [[nodiscard]] size_t totalAllocated() const { return m_totalAllocated; }

    [[nodiscard]] size_t taggedUsage(const std::string& tag) const {
        auto it = m_taggedUsage.find(tag);
        return it != m_taggedUsage.end() ? it->second : 0;
    }

    void reset() {
        m_currentUsage = 0;
        m_peakUsage = 0;
        m_allocationCount = 0;
        m_freeCount = 0;
        m_totalAllocated = 0;
        m_taggedUsage.clear();
    }

private:
    size_t m_currentUsage = 0;
    size_t m_peakUsage = 0;
    size_t m_allocationCount = 0;
    size_t m_freeCount = 0;
    size_t m_totalAllocated = 0;
    std::map<std::string, size_t> m_taggedUsage;
};

struct ProfileMarker {
    std::string label;
    double timestamp = 0.0;
    float duration = 0.f;
    std::string category;

    [[nodiscard]] double endTime() const { return timestamp + static_cast<double>(duration); }
};

class ProfilerTimeline {
public:
    void addMarker(const std::string& label, double timestamp, float duration,
                   const std::string& category = "") {
        if (m_markers.size() >= kMaxMarkers) return;
        ProfileMarker m;
        m.label = label;
        m.timestamp = timestamp;
        m.duration = duration;
        m.category = category;
        m_markers.push_back(m);
    }

    [[nodiscard]] std::vector<ProfileMarker> markersInRange(double start, double end) const {
        std::vector<ProfileMarker> result;
        for (const auto& m : m_markers) {
            if (m.timestamp >= start && m.timestamp <= end) result.push_back(m);
        }
        return result;
    }

    [[nodiscard]] std::vector<ProfileMarker> markersByCategory(const std::string& category) const {
        std::vector<ProfileMarker> result;
        for (const auto& m : m_markers) {
            if (m.category == category) result.push_back(m);
        }
        return result;
    }

    [[nodiscard]] size_t markerCount() const { return m_markers.size(); }
    [[nodiscard]] const std::vector<ProfileMarker>& markers() const { return m_markers; }

    void clear() { m_markers.clear(); }

    static constexpr size_t kMaxMarkers = 2048;

private:
    std::vector<ProfileMarker> m_markers;
};

struct PerformanceProfilerConfig {
    bool autoCapture = true;
    size_t maxFrameSamples = 4096;
    size_t maxTimelineMarkers = 2048;
    float warningFrameTimeMs = 33.33f;
    float criticalFrameTimeMs = 50.f;
};

class PerformanceProfiler {
public:
    void init(const PerformanceProfilerConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_frameProfiler.clear();
        m_memoryProfiler.reset();
        m_timeline.clear();
        m_initialized = false;
        m_sessionCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    void startSession(const std::string& name, double time) {
        if (!m_initialized) return;
        m_session.name = name;
        m_session.start(time);
        m_sessionCount++;
    }

    void stopSession(double time) {
        if (!m_initialized || !m_session.isActive()) return;
        m_session.stop(time);
    }

    void beginFrame(double timestamp) {
        if (!m_initialized) return;
        m_frameProfiler.beginFrame(timestamp);
    }

    void endFrame(double timestamp) {
        if (!m_initialized) return;
        m_frameProfiler.endFrame(timestamp);
    }

    void recordMetric(ProfileMetricType type, float value, double timestamp,
                      const std::string& tag = "") {
        if (!m_initialized) return;
        m_frameProfiler.recordMetric(type, value, timestamp, tag);
    }

    void trackAllocation(size_t bytes, const std::string& tag = "") {
        if (!m_initialized) return;
        m_memoryProfiler.trackAllocation(bytes, tag);
    }

    void trackFree(size_t bytes, const std::string& tag = "") {
        if (!m_initialized) return;
        m_memoryProfiler.trackFree(bytes, tag);
    }

    void addTimelineMarker(const std::string& label, double timestamp, float duration,
                           const std::string& category = "") {
        if (!m_initialized) return;
        m_timeline.addMarker(label, timestamp, duration, category);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] const FrameProfiler& frameProfiler() const { return m_frameProfiler; }
    [[nodiscard]] const MemoryProfiler& memoryProfiler() const { return m_memoryProfiler; }
    [[nodiscard]] const ProfilerTimeline& timeline() const { return m_timeline; }
    [[nodiscard]] const ProfileSession& session() const { return m_session; }
    [[nodiscard]] const PerformanceProfilerConfig& config() const { return m_config; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] size_t sessionCount() const { return m_sessionCount; }
    [[nodiscard]] size_t frameSampleCount() const { return m_frameProfiler.sampleCount(); }
    [[nodiscard]] size_t memoryPeakBytes() const { return m_memoryProfiler.peakUsage(); }
    [[nodiscard]] size_t timelineMarkerCount() const { return m_timeline.markerCount(); }

private:
    PerformanceProfilerConfig m_config;
    FrameProfiler m_frameProfiler;
    MemoryProfiler m_memoryProfiler;
    ProfilerTimeline m_timeline;
    ProfileSession m_session;
    bool m_initialized = false;
    size_t m_tickCount = 0;
    size_t m_sessionCount = 0;
};

// ── S11 — Live Collaboration System ─────────────────────────────

enum class CollabUserRole : uint8_t {
    Owner,
    Admin,
    Editor,
    Reviewer,
    Viewer,
    Builder,
    Tester,
    Guest
};

inline const char* collabUserRoleName(CollabUserRole r) noexcept {
    switch (r) {
        case CollabUserRole::Owner:    return "Owner";
        case CollabUserRole::Admin:    return "Admin";
        case CollabUserRole::Editor:   return "Editor";
        case CollabUserRole::Reviewer: return "Reviewer";
        case CollabUserRole::Viewer:   return "Viewer";
        case CollabUserRole::Builder:  return "Builder";
        case CollabUserRole::Tester:   return "Tester";
        case CollabUserRole::Guest:    return "Guest";
        default:                       return "Unknown";
    }
}

struct CollabUser {
    std::string userId;
    std::string displayName;
    CollabUserRole role = CollabUserRole::Guest;
    bool connected = false;
    double lastActivityTime = 0.0;

    [[nodiscard]] bool canEdit() const {
        return role == CollabUserRole::Owner ||
               role == CollabUserRole::Admin ||
               role == CollabUserRole::Editor;
    }

    [[nodiscard]] bool canReview() const {
        return canEdit() || role == CollabUserRole::Reviewer;
    }

    [[nodiscard]] bool isConnected() const { return connected; }
    void connect(double time) { connected = true; lastActivityTime = time; }
    void disconnect() { connected = false; }
    void touch(double time) { lastActivityTime = time; }
};

enum class CollabEditType : uint8_t {
    Insert,
    Delete,
    Modify,
    Move,
    Rename,
    Create,
    Lock,
    Unlock
};

inline const char* collabEditTypeName(CollabEditType t) noexcept {
    switch (t) {
        case CollabEditType::Insert:  return "Insert";
        case CollabEditType::Delete:  return "Delete";
        case CollabEditType::Modify:  return "Modify";
        case CollabEditType::Move:    return "Move";
        case CollabEditType::Rename:  return "Rename";
        case CollabEditType::Create:  return "Create";
        case CollabEditType::Lock:    return "Lock";
        case CollabEditType::Unlock:  return "Unlock";
        default:                      return "Unknown";
    }
}

struct CollabEditAction {
    std::string actionId;
    std::string userId;
    CollabEditType type = CollabEditType::Modify;
    std::string targetPath;
    std::string payload;
    double timestamp = 0.0;
    size_t sequenceNum = 0;
    bool applied = false;
    bool conflicted = false;

    [[nodiscard]] bool isValid() const { return !actionId.empty() && !userId.empty() && !targetPath.empty(); }
    void markApplied() { applied = true; }
    void markConflicted() { conflicted = true; }
};

class CollabSession {
public:
    explicit CollabSession(const std::string& sessionName)
        : m_name(sessionName) {}

    bool addUser(const CollabUser& user) {
        if (m_users.size() >= kMaxUsers) return false;
        for (const auto& u : m_users) {
            if (u.userId == user.userId) return false;
        }
        m_users.push_back(user);
        return true;
    }

    bool removeUser(const std::string& userId) {
        for (auto it = m_users.begin(); it != m_users.end(); ++it) {
            if (it->userId == userId) {
                m_users.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] CollabUser* findUser(const std::string& userId) {
        for (auto& u : m_users) {
            if (u.userId == userId) return &u;
        }
        return nullptr;
    }

    [[nodiscard]] const CollabUser* findUser(const std::string& userId) const {
        for (const auto& u : m_users) {
            if (u.userId == userId) return &u;
        }
        return nullptr;
    }

    bool submitAction(const CollabEditAction& action) {
        if (!action.isValid()) return false;
        auto* user = findUser(action.userId);
        if (!user || !user->canEdit()) return false;
        if (m_actions.size() >= kMaxActions) return false;

        CollabEditAction a = action;
        a.sequenceNum = m_nextSeqNum++;
        a.applied = true;

        // Check for conflicts on same path
        for (const auto& existing : m_actions) {
            if (existing.targetPath == a.targetPath && !existing.conflicted &&
                existing.applied && existing.sequenceNum > 0 &&
                existing.userId != a.userId) {
                // Potential conflict detected if same path edited by different user recently
                if (a.timestamp - existing.timestamp < m_conflictWindowSec) {
                    a.markConflicted();
                    m_conflictCount++;
                    break;
                }
            }
        }

        m_actions.push_back(a);
        user->touch(a.timestamp);
        return true;
    }

    [[nodiscard]] size_t userCount() const { return m_users.size(); }
    [[nodiscard]] size_t actionCount() const { return m_actions.size(); }
    [[nodiscard]] size_t conflictCount() const { return m_conflictCount; }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::vector<CollabUser>& users() const { return m_users; }
    [[nodiscard]] const std::vector<CollabEditAction>& actions() const { return m_actions; }

    [[nodiscard]] size_t connectedCount() const {
        size_t count = 0;
        for (const auto& u : m_users) {
            if (u.isConnected()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t editorCount() const {
        size_t count = 0;
        for (const auto& u : m_users) {
            if (u.canEdit()) ++count;
        }
        return count;
    }

    void setConflictWindow(double seconds) { m_conflictWindowSec = seconds; }

    static constexpr size_t kMaxUsers = 32;
    static constexpr size_t kMaxActions = 4096;

private:
    std::string m_name;
    std::vector<CollabUser> m_users;
    std::vector<CollabEditAction> m_actions;
    size_t m_nextSeqNum = 1;
    size_t m_conflictCount = 0;
    double m_conflictWindowSec = 2.0;
};

class CollabConflictResolver {
public:
    struct Resolution {
        std::string actionId;
        bool autoResolved = false;
        std::string strategy;
    };

    Resolution resolve(const CollabEditAction& a, const CollabEditAction& b) {
        Resolution res;
        res.actionId = a.actionId;
        m_totalResolutions++;

        if (a.targetPath != b.targetPath) {
            res.autoResolved = true;
            res.strategy = "no_conflict";
            m_autoResolved++;
            return res;
        }

        // Same path — last-writer-wins if same edit type
        if (a.type == b.type) {
            res.autoResolved = true;
            res.strategy = "last_writer_wins";
            m_autoResolved++;
            return res;
        }

        // Different types on same path — needs manual resolution
        res.autoResolved = false;
        res.strategy = "manual";
        m_manualRequired++;
        return res;
    }

    [[nodiscard]] size_t totalResolutions() const { return m_totalResolutions; }
    [[nodiscard]] size_t autoResolved() const { return m_autoResolved; }
    [[nodiscard]] size_t manualRequired() const { return m_manualRequired; }

    void reset() {
        m_totalResolutions = 0;
        m_autoResolved = 0;
        m_manualRequired = 0;
    }

private:
    size_t m_totalResolutions = 0;
    size_t m_autoResolved = 0;
    size_t m_manualRequired = 0;
};

struct LiveCollabConfig {
    std::string serverAddress = "localhost";
    uint16_t port = 9090;
    double heartbeatIntervalSec = 5.0;
    double inactivityTimeoutSec = 300.0;
    size_t maxSessions = 16;
    size_t maxUsersPerSession = 32;
};

class LiveCollaborationSystem {
public:
    void init(const LiveCollabConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_sessions.clear();
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    int createSession(const std::string& sessionName) {
        if (!m_initialized) return -1;
        if (m_sessions.size() >= m_config.maxSessions) return -1;
        for (const auto& s : m_sessions) {
            if (s.name() == sessionName) return -1;
        }
        m_sessions.emplace_back(sessionName);
        return static_cast<int>(m_sessions.size()) - 1;
    }

    [[nodiscard]] CollabSession* session(int index) {
        if (index < 0 || index >= static_cast<int>(m_sessions.size())) return nullptr;
        return &m_sessions[static_cast<size_t>(index)];
    }

    [[nodiscard]] CollabSession* sessionByName(const std::string& name) {
        for (auto& s : m_sessions) {
            if (s.name() == name) return &s;
        }
        return nullptr;
    }

    bool joinSession(const std::string& sessionName, const CollabUser& user) {
        auto* s = sessionByName(sessionName);
        if (!s) return false;
        return s->addUser(user);
    }

    bool leaveSession(const std::string& sessionName, const std::string& userId) {
        auto* s = sessionByName(sessionName);
        if (!s) return false;
        return s->removeUser(userId);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t sessionCount() const { return m_sessions.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] const LiveCollabConfig& config() const { return m_config; }
    [[nodiscard]] CollabConflictResolver& resolver() { return m_resolver; }
    [[nodiscard]] const CollabConflictResolver& resolver() const { return m_resolver; }

    [[nodiscard]] size_t totalConnectedUsers() const {
        size_t count = 0;
        for (const auto& s : m_sessions) count += s.connectedCount();
        return count;
    }

    [[nodiscard]] size_t totalActions() const {
        size_t count = 0;
        for (const auto& s : m_sessions) count += s.actionCount();
        return count;
    }

    [[nodiscard]] size_t totalConflicts() const {
        size_t count = 0;
        for (const auto& s : m_sessions) count += s.conflictCount();
        return count;
    }

private:
    LiveCollabConfig m_config;
    std::vector<CollabSession> m_sessions;
    CollabConflictResolver m_resolver;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ── S12 — Version Control Integration ───────────────────────────

enum class VCSProviderType : uint8_t {
    Git,
    SVN,
    Perforce,
    Mercurial,
    Plastic,
    Fossil,
    Custom,
    None
};

inline const char* vcsProviderTypeName(VCSProviderType t) noexcept {
    switch (t) {
        case VCSProviderType::Git:       return "Git";
        case VCSProviderType::SVN:       return "SVN";
        case VCSProviderType::Perforce:  return "Perforce";
        case VCSProviderType::Mercurial: return "Mercurial";
        case VCSProviderType::Plastic:   return "Plastic";
        case VCSProviderType::Fossil:    return "Fossil";
        case VCSProviderType::Custom:    return "Custom";
        case VCSProviderType::None:      return "None";
        default:                         return "Unknown";
    }
}

enum class VCSFileStatus : uint8_t {
    Untracked,
    Added,
    Modified,
    Deleted,
    Renamed,
    Conflicted,
    Ignored,
    Unchanged
};

inline const char* vcsFileStatusName(VCSFileStatus s) noexcept {
    switch (s) {
        case VCSFileStatus::Untracked:  return "Untracked";
        case VCSFileStatus::Added:      return "Added";
        case VCSFileStatus::Modified:   return "Modified";
        case VCSFileStatus::Deleted:    return "Deleted";
        case VCSFileStatus::Renamed:    return "Renamed";
        case VCSFileStatus::Conflicted: return "Conflicted";
        case VCSFileStatus::Ignored:    return "Ignored";
        case VCSFileStatus::Unchanged:  return "Unchanged";
        default:                        return "Unknown";
    }
}

struct VCSCommitInfo {
    std::string hash;
    std::string author;
    std::string message;
    double timestamp = 0.0;
    std::string parentHash;
    size_t fileCount = 0;

    [[nodiscard]] bool isValid() const { return !hash.empty() && !author.empty(); }
    [[nodiscard]] bool isRoot() const { return parentHash.empty(); }
    [[nodiscard]] bool hasMessage() const { return !message.empty(); }
};

struct VCSBranchInfo {
    std::string name;
    bool isActive = false;
    bool isRemote = false;
    std::string lastCommitHash;
    size_t aheadCount = 0;
    size_t behindCount = 0;

    [[nodiscard]] bool isSynced() const { return aheadCount == 0 && behindCount == 0; }
    [[nodiscard]] bool isLocal() const { return !isRemote; }
    [[nodiscard]] bool hasCommits() const { return !lastCommitHash.empty(); }
};

struct VCSDiffEntry {
    std::string filePath;
    VCSFileStatus status = VCSFileStatus::Unchanged;
    size_t additions = 0;
    size_t deletions = 0;
    bool isBinary = false;

    [[nodiscard]] size_t totalChanges() const { return additions + deletions; }
    [[nodiscard]] bool hasChanges() const { return additions > 0 || deletions > 0; }
};

class VCSRepository {
public:
    explicit VCSRepository(const std::string& repoName, VCSProviderType provider = VCSProviderType::Git)
        : m_name(repoName), m_provider(provider) {}

    bool addBranch(const VCSBranchInfo& branch) {
        if (m_branches.size() >= kMaxBranches) return false;
        for (const auto& b : m_branches) {
            if (b.name == branch.name) return false;
        }
        m_branches.push_back(branch);
        return true;
    }

    bool removeBranch(const std::string& branchName) {
        for (auto it = m_branches.begin(); it != m_branches.end(); ++it) {
            if (it->name == branchName && !it->isActive) {
                m_branches.erase(it);
                return true;
            }
        }
        return false;
    }

    bool switchBranch(const std::string& branchName) {
        VCSBranchInfo* target = nullptr;
        for (auto& b : m_branches) {
            if (b.name == branchName) target = &b;
        }
        if (!target) return false;
        for (auto& b : m_branches) b.isActive = false;
        target->isActive = true;
        return true;
    }

    [[nodiscard]] VCSBranchInfo* activeBranch() {
        for (auto& b : m_branches) {
            if (b.isActive) return &b;
        }
        return nullptr;
    }

    [[nodiscard]] const VCSBranchInfo* activeBranch() const {
        for (const auto& b : m_branches) {
            if (b.isActive) return &b;
        }
        return nullptr;
    }

    [[nodiscard]] VCSBranchInfo* findBranch(const std::string& name) {
        for (auto& b : m_branches) {
            if (b.name == name) return &b;
        }
        return nullptr;
    }

    bool addCommit(const VCSCommitInfo& commit) {
        if (!commit.isValid()) return false;
        if (m_commits.size() >= kMaxCommits) return false;
        m_commits.push_back(commit);
        return true;
    }

    bool trackFile(const std::string& path, VCSFileStatus status) {
        for (auto& d : m_diffs) {
            if (d.filePath == path) {
                d.status = status;
                return true;
            }
        }
        if (m_diffs.size() >= kMaxDiffs) return false;
        VCSDiffEntry entry;
        entry.filePath = path;
        entry.status = status;
        m_diffs.push_back(entry);
        return true;
    }

    [[nodiscard]] const VCSDiffEntry* findDiff(const std::string& path) const {
        for (const auto& d : m_diffs) {
            if (d.filePath == path) return &d;
        }
        return nullptr;
    }

    [[nodiscard]] size_t branchCount() const { return m_branches.size(); }
    [[nodiscard]] size_t commitCount() const { return m_commits.size(); }
    [[nodiscard]] size_t diffCount() const { return m_diffs.size(); }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] VCSProviderType provider() const { return m_provider; }
    [[nodiscard]] const std::vector<VCSBranchInfo>& branches() const { return m_branches; }
    [[nodiscard]] const std::vector<VCSCommitInfo>& commits() const { return m_commits; }
    [[nodiscard]] const std::vector<VCSDiffEntry>& diffs() const { return m_diffs; }

    [[nodiscard]] size_t modifiedFileCount() const {
        size_t count = 0;
        for (const auto& d : m_diffs) {
            if (d.status != VCSFileStatus::Unchanged && d.status != VCSFileStatus::Ignored) ++count;
        }
        return count;
    }

    static constexpr size_t kMaxBranches = 64;
    static constexpr size_t kMaxCommits = 1024;
    static constexpr size_t kMaxDiffs = 512;

private:
    std::string m_name;
    VCSProviderType m_provider;
    std::vector<VCSBranchInfo> m_branches;
    std::vector<VCSCommitInfo> m_commits;
    std::vector<VCSDiffEntry> m_diffs;
};

struct VCSConfig {
    VCSProviderType defaultProvider = VCSProviderType::Git;
    bool autoDetect = true;
    bool watchFileChanges = true;
    double pollIntervalSec = 2.0;
    size_t maxRepositories = 8;
};

class VersionControlSystem {
public:
    void init(const VCSConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_repos.clear();
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    int openRepository(const std::string& repoName, VCSProviderType provider = VCSProviderType::Git) {
        if (!m_initialized) return -1;
        if (m_repos.size() >= m_config.maxRepositories) return -1;
        for (const auto& r : m_repos) {
            if (r.name() == repoName) return -1;
        }
        m_repos.emplace_back(repoName, provider);
        return static_cast<int>(m_repos.size()) - 1;
    }

    [[nodiscard]] VCSRepository* repository(int index) {
        if (index < 0 || index >= static_cast<int>(m_repos.size())) return nullptr;
        return &m_repos[static_cast<size_t>(index)];
    }

    [[nodiscard]] VCSRepository* repositoryByName(const std::string& name) {
        for (auto& r : m_repos) {
            if (r.name() == name) return &r;
        }
        return nullptr;
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t repositoryCount() const { return m_repos.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] const VCSConfig& config() const { return m_config; }

    [[nodiscard]] size_t totalBranches() const {
        size_t count = 0;
        for (const auto& r : m_repos) count += r.branchCount();
        return count;
    }

    [[nodiscard]] size_t totalCommits() const {
        size_t count = 0;
        for (const auto& r : m_repos) count += r.commitCount();
        return count;
    }

    [[nodiscard]] size_t totalModifiedFiles() const {
        size_t count = 0;
        for (const auto& r : m_repos) count += r.modifiedFileCount();
        return count;
    }

private:
    VCSConfig m_config;
    std::vector<VCSRepository> m_repos;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ── S13 — Localization System ───────────────────────────────────

enum class LocaleId : uint8_t {
    English,
    Spanish,
    French,
    German,
    Japanese,
    Chinese,
    Korean,
    Russian
};

inline const char* localeIdName(LocaleId id) noexcept {
    switch (id) {
        case LocaleId::English:  return "English";
        case LocaleId::Spanish:  return "Spanish";
        case LocaleId::French:   return "French";
        case LocaleId::German:   return "German";
        case LocaleId::Japanese: return "Japanese";
        case LocaleId::Chinese:  return "Chinese";
        case LocaleId::Korean:   return "Korean";
        case LocaleId::Russian:  return "Russian";
        default:                 return "Unknown";
    }
}

struct LocalizedString {
    std::string key;
    std::string value;
    LocaleId locale = LocaleId::English;
    std::string context;
    bool verified = false;

    [[nodiscard]] bool isValid() const { return !key.empty() && !value.empty(); }
    [[nodiscard]] bool isVerified() const { return verified; }
    void verify() { verified = true; }
};

struct TranslationEntry {
    std::string key;
    std::string context;
    std::unordered_map<uint8_t, std::string> translations; // LocaleId -> text

    void set(LocaleId locale, const std::string& text) {
        translations[static_cast<uint8_t>(locale)] = text;
    }

    [[nodiscard]] const std::string* get(LocaleId locale) const {
        auto it = translations.find(static_cast<uint8_t>(locale));
        return (it != translations.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] bool has(LocaleId locale) const {
        return translations.count(static_cast<uint8_t>(locale)) > 0;
    }

    [[nodiscard]] size_t localeCount() const { return translations.size(); }
};

class TranslationTable {
public:
    explicit TranslationTable(const std::string& tableName)
        : m_name(tableName) {}

    bool addEntry(const std::string& key, const std::string& context = "") {
        if (m_entries.size() >= kMaxEntries) return false;
        for (const auto& e : m_entries) {
            if (e.key == key) return false;
        }
        TranslationEntry entry;
        entry.key = key;
        entry.context = context;
        m_entries.push_back(entry);
        return true;
    }

    bool removeEntry(const std::string& key) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->key == key) {
                m_entries.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TranslationEntry* findEntry(const std::string& key) {
        for (auto& e : m_entries) {
            if (e.key == key) return &e;
        }
        return nullptr;
    }

    [[nodiscard]] const TranslationEntry* findEntry(const std::string& key) const {
        for (const auto& e : m_entries) {
            if (e.key == key) return &e;
        }
        return nullptr;
    }

    bool setTranslation(const std::string& key, LocaleId locale, const std::string& text) {
        auto* entry = findEntry(key);
        if (!entry) return false;
        entry->set(locale, text);
        return true;
    }

    [[nodiscard]] const std::string* lookup(const std::string& key, LocaleId locale) const {
        const auto* entry = findEntry(key);
        if (!entry) return nullptr;
        return entry->get(locale);
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::vector<TranslationEntry>& entries() const { return m_entries; }

    [[nodiscard]] size_t translatedCount(LocaleId locale) const {
        size_t count = 0;
        for (const auto& e : m_entries) {
            if (e.has(locale)) ++count;
        }
        return count;
    }

    [[nodiscard]] float completionRate(LocaleId locale) const {
        if (m_entries.empty()) return 0.f;
        return static_cast<float>(translatedCount(locale)) / static_cast<float>(m_entries.size());
    }

    static constexpr size_t kMaxEntries = 4096;

private:
    std::string m_name;
    std::vector<TranslationEntry> m_entries;
};

class LocaleManager {
public:
    explicit LocaleManager(LocaleId activeLocale = LocaleId::English)
        : m_active(activeLocale), m_fallback(LocaleId::English) {}

    void setActive(LocaleId locale) { m_active = locale; m_switchCount++; }
    void setFallback(LocaleId locale) { m_fallback = locale; }

    [[nodiscard]] LocaleId active() const { return m_active; }
    [[nodiscard]] LocaleId fallback() const { return m_fallback; }
    [[nodiscard]] size_t switchCount() const { return m_switchCount; }

    [[nodiscard]] const std::string* resolve(const TranslationTable& table, const std::string& key) const {
        const auto* result = table.lookup(key, m_active);
        if (result) return result;
        if (m_active != m_fallback) {
            return table.lookup(key, m_fallback);
        }
        return nullptr;
    }

private:
    LocaleId m_active;
    LocaleId m_fallback;
    size_t m_switchCount = 0;
};

struct LocalizationConfig {
    LocaleId defaultLocale = LocaleId::English;
    LocaleId fallbackLocale = LocaleId::English;
    size_t maxTables = 16;
    bool autoDetectLocale = false;
};

class LocalizationSystem {
public:
    void init(const LocalizationConfig& config = {}) {
        m_config = config;
        m_manager = LocaleManager(config.defaultLocale);
        m_manager.setFallback(config.fallbackLocale);
        m_initialized = true;
    }

    void shutdown() {
        m_tables.clear();
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    int createTable(const std::string& tableName) {
        if (!m_initialized) return -1;
        if (m_tables.size() >= m_config.maxTables) return -1;
        for (const auto& t : m_tables) {
            if (t.name() == tableName) return -1;
        }
        m_tables.emplace_back(tableName);
        return static_cast<int>(m_tables.size()) - 1;
    }

    [[nodiscard]] TranslationTable* table(int index) {
        if (index < 0 || index >= static_cast<int>(m_tables.size())) return nullptr;
        return &m_tables[static_cast<size_t>(index)];
    }

    [[nodiscard]] TranslationTable* tableByName(const std::string& name) {
        for (auto& t : m_tables) {
            if (t.name() == name) return &t;
        }
        return nullptr;
    }

    [[nodiscard]] const std::string* translate(const std::string& tableName, const std::string& key) const {
        for (const auto& t : m_tables) {
            if (t.name() == tableName) {
                return m_manager.resolve(t, key);
            }
        }
        return nullptr;
    }

    void setLocale(LocaleId locale) { m_manager.setActive(locale); }
    [[nodiscard]] LocaleId activeLocale() const { return m_manager.active(); }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t tableCount() const { return m_tables.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] const LocalizationConfig& config() const { return m_config; }
    [[nodiscard]] LocaleManager& localeManager() { return m_manager; }
    [[nodiscard]] const LocaleManager& localeManager() const { return m_manager; }

    [[nodiscard]] size_t totalEntries() const {
        size_t count = 0;
        for (const auto& t : m_tables) count += t.entryCount();
        return count;
    }

    [[nodiscard]] size_t totalTranslated(LocaleId locale) const {
        size_t count = 0;
        for (const auto& t : m_tables) count += t.translatedCount(locale);
        return count;
    }

private:
    LocalizationConfig m_config;
    std::vector<TranslationTable> m_tables;
    LocaleManager m_manager;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// S14 — Plugin System
// ─────────────────────────────────────────────────────────────────────────────

enum class PluginState : uint8_t {
    Unloaded = 0,
    Loading,
    Loaded,
    Active,
    Suspended,
    Error,
    Disabled,
    Unloading
};

inline const char* pluginStateName(PluginState s) {
    switch (s) {
        case PluginState::Unloaded:   return "Unloaded";
        case PluginState::Loading:    return "Loading";
        case PluginState::Loaded:     return "Loaded";
        case PluginState::Active:     return "Active";
        case PluginState::Suspended:  return "Suspended";
        case PluginState::Error:      return "Error";
        case PluginState::Disabled:   return "Disabled";
        case PluginState::Unloading:  return "Unloading";
        default:                      return "Unknown";
    }
}

struct PluginManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty() && !version.empty(); }
};

struct PluginInstance {
    PluginManifest manifest;
    PluginState state = PluginState::Unloaded;
    float loadTime = 0.f;
    std::string errorMessage;

    [[nodiscard]] bool isLoaded()   const { return state == PluginState::Loaded || state == PluginState::Active || state == PluginState::Suspended; }
    [[nodiscard]] bool isActive()   const { return state == PluginState::Active; }
    [[nodiscard]] bool hasError()   const { return state == PluginState::Error; }
    [[nodiscard]] bool isDisabled() const { return state == PluginState::Disabled; }

    bool activate() {
        if (state == PluginState::Loaded || state == PluginState::Suspended) {
            state = PluginState::Active;
            return true;
        }
        return false;
    }

    bool suspend() {
        if (state == PluginState::Active) {
            state = PluginState::Suspended;
            return true;
        }
        return false;
    }

    bool disable() {
        if (state != PluginState::Unloaded && state != PluginState::Error) {
            state = PluginState::Disabled;
            return true;
        }
        return false;
    }

    void setError(const std::string& msg) {
        state = PluginState::Error;
        errorMessage = msg;
    }
};

class PluginRegistry {
public:
    static constexpr size_t kMaxPlugins = 64;

    bool registerPlugin(const PluginManifest& manifest) {
        if (!manifest.isValid()) return false;
        if (m_plugins.size() >= kMaxPlugins) return false;
        for (const auto& p : m_plugins) {
            if (p.manifest.id == manifest.id) return false;
        }
        PluginInstance inst;
        inst.manifest = manifest;
        inst.state = PluginState::Unloaded;
        m_plugins.push_back(std::move(inst));
        return true;
    }

    bool unregisterPlugin(const std::string& pluginId) {
        for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
            if (it->manifest.id == pluginId) {
                m_plugins.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] PluginInstance* findPlugin(const std::string& pluginId) {
        for (auto& p : m_plugins) {
            if (p.manifest.id == pluginId) return &p;
        }
        return nullptr;
    }

    [[nodiscard]] const PluginInstance* findPlugin(const std::string& pluginId) const {
        for (const auto& p : m_plugins) {
            if (p.manifest.id == pluginId) return &p;
        }
        return nullptr;
    }

    [[nodiscard]] size_t pluginCount() const { return m_plugins.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t count = 0;
        for (const auto& p : m_plugins) {
            if (p.state != PluginState::Disabled && p.state != PluginState::Error) ++count;
        }
        return count;
    }

    [[nodiscard]] std::vector<const PluginInstance*> pluginsByState(PluginState s) const {
        std::vector<const PluginInstance*> result;
        for (const auto& p : m_plugins) {
            if (p.state == s) result.push_back(&p);
        }
        return result;
    }

    [[nodiscard]] const std::vector<PluginInstance>& plugins() const { return m_plugins; }

private:
    std::vector<PluginInstance> m_plugins;
};

class PluginLoader {
public:
    bool load(PluginRegistry& registry, const std::string& pluginId) {
        auto* inst = registry.findPlugin(pluginId);
        if (!inst) return false;
        if (inst->state != PluginState::Unloaded && inst->state != PluginState::Disabled) return false;
        inst->state = PluginState::Loading;
        inst->state = PluginState::Loaded;
        inst->loadTime = 0.f;
        inst->errorMessage.clear();
        m_loadCount++;
        return true;
    }

    bool unload(PluginRegistry& registry, const std::string& pluginId) {
        auto* inst = registry.findPlugin(pluginId);
        if (!inst) return false;
        if (!inst->isLoaded()) return false;
        inst->state = PluginState::Unloading;
        inst->state = PluginState::Unloaded;
        m_unloadCount++;
        return true;
    }

    bool reload(PluginRegistry& registry, const std::string& pluginId) {
        if (!unload(registry, pluginId)) return false;
        return load(registry, pluginId);
    }

    [[nodiscard]] size_t loadCount()   const { return m_loadCount; }
    [[nodiscard]] size_t unloadCount() const { return m_unloadCount; }
    [[nodiscard]] size_t errorCount()  const { return m_errorCount; }

private:
    size_t m_loadCount   = 0;
    size_t m_unloadCount = 0;
    size_t m_errorCount  = 0;
};

struct PluginSystemConfig {
    size_t maxPlugins = 32;
    bool autoActivateOnLoad = false;
};

class PluginSystem {
public:
    void init(const PluginSystemConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_registry = PluginRegistry{};
        m_loader = PluginLoader{};
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool registerPlugin(const PluginManifest& manifest) {
        if (!m_initialized) return false;
        if (m_registry.pluginCount() >= m_config.maxPlugins) return false;
        return m_registry.registerPlugin(manifest);
    }

    bool loadPlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        if (!m_loader.load(m_registry, pluginId)) return false;
        if (m_config.autoActivateOnLoad) {
            auto* inst = m_registry.findPlugin(pluginId);
            if (inst) inst->activate();
        }
        return true;
    }

    bool unloadPlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        return m_loader.unload(m_registry, pluginId);
    }

    bool activatePlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        auto* inst = m_registry.findPlugin(pluginId);
        if (!inst) return false;
        return inst->activate();
    }

    bool suspendPlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        auto* inst = m_registry.findPlugin(pluginId);
        if (!inst) return false;
        return inst->suspend();
    }

    [[nodiscard]] PluginInstance* findPlugin(const std::string& pluginId) {
        return m_registry.findPlugin(pluginId);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t activePluginCount() const {
        return m_registry.pluginsByState(PluginState::Active).size();
    }

    [[nodiscard]] size_t totalPluginCount() const { return m_registry.pluginCount(); }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }
    [[nodiscard]] PluginRegistry&       registry()       { return m_registry; }
    [[nodiscard]] const PluginRegistry& registry() const { return m_registry; }
    [[nodiscard]] PluginLoader&         loader()         { return m_loader; }

private:
    PluginSystemConfig m_config;
    PluginRegistry     m_registry;
    PluginLoader       m_loader;
    bool               m_initialized = false;
    size_t             m_tickCount   = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// S15 — Scripting Console
// ─────────────────────────────────────────────────────────────────────────────

enum class ScriptLanguage : uint8_t {
    Lua        = 0,
    Python     = 1,
    JavaScript = 2,
    TypeScript = 3,
    Bash       = 4,
    Ruby       = 5,
    CSharp     = 6,
    DSL        = 7
};

inline const char* scriptLanguageName(ScriptLanguage lang) {
    switch (lang) {
        case ScriptLanguage::Lua:        return "Lua";
        case ScriptLanguage::Python:     return "Python";
        case ScriptLanguage::JavaScript: return "JavaScript";
        case ScriptLanguage::TypeScript: return "TypeScript";
        case ScriptLanguage::Bash:       return "Bash";
        case ScriptLanguage::Ruby:       return "Ruby";
        case ScriptLanguage::CSharp:     return "CSharp";
        case ScriptLanguage::DSL:        return "DSL";
        default:                         return "Unknown";
    }
}

struct ScriptVariable {
    std::string name;
    std::string value;
    std::string typeName;
    bool        readOnly = false;

    [[nodiscard]] bool isValid()    const { return !name.empty(); }
    [[nodiscard]] bool isReadOnly() const { return readOnly; }

    bool set(const std::string& newValue) {
        if (readOnly) return false;
        value = newValue;
        return true;
    }
};

struct ScriptResult {
    std::string output;
    std::string errorMessage;
    int         exitCode    = 0;
    float       durationMs  = 0.f;

    [[nodiscard]] bool isSuccess()  const { return exitCode == 0 && errorMessage.empty(); }
    [[nodiscard]] bool hasOutput()  const { return !output.empty(); }
    [[nodiscard]] bool hasError()   const { return !errorMessage.empty(); }
};

class ScriptContext {
public:
    static constexpr size_t kMaxVariables = 128;

    explicit ScriptContext(const std::string& name, ScriptLanguage lang = ScriptLanguage::Lua)
        : m_name(name), m_language(lang) {}

    [[nodiscard]] const std::string& name()     const { return m_name; }
    [[nodiscard]] ScriptLanguage     language()  const { return m_language; }

    bool setVariable(const ScriptVariable& var) {
        for (auto& v : m_variables) {
            if (v.name == var.name) {
                if (v.readOnly) return false;
                v = var;
                return true;
            }
        }
        if (m_variables.size() >= kMaxVariables) return false;
        m_variables.push_back(var);
        return true;
    }

    [[nodiscard]] ScriptVariable* getVariable(const std::string& varName) {
        for (auto& v : m_variables) { if (v.name == varName) return &v; }
        return nullptr;
    }

    bool removeVariable(const std::string& varName) {
        for (auto it = m_variables.begin(); it != m_variables.end(); ++it) {
            if (it->name == varName) { m_variables.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] bool hasVariable(const std::string& varName) const {
        for (const auto& v : m_variables) { if (v.name == varName) return true; }
        return false;
    }

    void clear() { m_variables.clear(); }

    [[nodiscard]] size_t variableCount() const { return m_variables.size(); }

private:
    std::string                m_name;
    ScriptLanguage             m_language;
    std::vector<ScriptVariable> m_variables;
};

class ScriptConsole {
public:
    static constexpr size_t kMaxContexts = 16;

    void init() { m_initialized = true; }

    void shutdown() {
        m_contexts.clear();
        m_executionCount = 0;
        m_errorCount     = 0;
        m_tickCount      = 0;
        m_initialized    = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    ScriptContext* createContext(const std::string& name, ScriptLanguage lang = ScriptLanguage::Lua) {
        if (!m_initialized) return nullptr;
        if (m_contexts.size() >= kMaxContexts) return nullptr;
        for (const auto& c : m_contexts) { if (c.name() == name) return nullptr; }
        m_contexts.emplace_back(name, lang);
        return &m_contexts.back();
    }

    [[nodiscard]] ScriptContext* contextByName(const std::string& name) {
        for (auto& c : m_contexts) { if (c.name() == name) return &c; }
        return nullptr;
    }

    ScriptResult execute(const std::string& code, ScriptContext* context = nullptr) {
        ScriptResult result;
        if (!m_initialized) {
            result.exitCode    = -1;
            result.errorMessage = "Console not initialized";
            m_errorCount++;
            return result;
        }
        if (code.empty()) {
            result.exitCode    = -1;
            result.errorMessage = "Empty script";
            m_errorCount++;
            return result;
        }
        // Simulate successful execution
        result.output     = "ok";
        result.exitCode   = 0;
        result.durationMs = 1.f;
        m_executionCount++;
        return result;
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t totalContexts()   const { return m_contexts.size(); }
    [[nodiscard]] size_t executionCount()  const { return m_executionCount; }
    [[nodiscard]] size_t errorCount()      const { return m_errorCount; }
    [[nodiscard]] size_t tickCount()       const { return m_tickCount; }

private:
    std::vector<ScriptContext> m_contexts;
    bool   m_initialized  = false;
    size_t m_executionCount = 0;
    size_t m_errorCount     = 0;
    size_t m_tickCount      = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// S16 — Hot-Reload System
// ─────────────────────────────────────────────────────────────────────────────

enum class HotReloadAssetType : uint8_t {
    Script   = 0,
    Shader   = 1,
    Texture  = 2,
    Mesh     = 3,
    Audio    = 4,
    Config   = 5,
    Level    = 6,
    Material = 7
};

inline const char* hotReloadAssetTypeName(HotReloadAssetType t) {
    switch (t) {
        case HotReloadAssetType::Script:   return "Script";
        case HotReloadAssetType::Shader:   return "Shader";
        case HotReloadAssetType::Texture:  return "Texture";
        case HotReloadAssetType::Mesh:     return "Mesh";
        case HotReloadAssetType::Audio:    return "Audio";
        case HotReloadAssetType::Config:   return "Config";
        case HotReloadAssetType::Level:    return "Level";
        case HotReloadAssetType::Material: return "Material";
        default:                           return "Unknown";
    }
}

enum class HotReloadStatus : uint8_t {
    Idle      = 0,
    Pending   = 1,
    Reloading = 2,
    Success   = 3,
    Failed    = 4
};

struct HotReloadEntry {
    std::string       assetPath;
    HotReloadAssetType assetType  = HotReloadAssetType::Script;
    HotReloadStatus   status      = HotReloadStatus::Idle;
    size_t            reloadCount = 0;
    std::string       errorMessage;

    [[nodiscard]] bool isPending()   const { return status == HotReloadStatus::Pending; }
    [[nodiscard]] bool isReloading() const { return status == HotReloadStatus::Reloading; }
    [[nodiscard]] bool hasError()    const { return status == HotReloadStatus::Failed; }
    [[nodiscard]] bool isSuccess()   const { return status == HotReloadStatus::Success; }

    void markPending() { status = HotReloadStatus::Pending; errorMessage.clear(); }
    void markSuccess() { status = HotReloadStatus::Success; reloadCount++; errorMessage.clear(); }
    void markFailed(const std::string& err) { status = HotReloadStatus::Failed; errorMessage = err; }
};

class HotReloadWatcher {
public:
    static constexpr size_t kMaxEntries = 256;

    bool watch(const std::string& assetPath, HotReloadAssetType type) {
        if (m_entries.size() >= kMaxEntries) return false;
        for (const auto& e : m_entries) { if (e.assetPath == assetPath) return false; }
        HotReloadEntry entry;
        entry.assetPath = assetPath;
        entry.assetType = type;
        m_entries.push_back(entry);
        return true;
    }

    bool unwatch(const std::string& assetPath) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->assetPath == assetPath) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] HotReloadEntry* findEntry(const std::string& assetPath) {
        for (auto& e : m_entries) { if (e.assetPath == assetPath) return &e; }
        return nullptr;
    }

    bool triggerReload(const std::string& assetPath) {
        auto* e = findEntry(assetPath);
        if (!e) return false;
        e->markPending();
        return true;
    }

    [[nodiscard]] size_t entryCount()   const { return m_entries.size(); }
    [[nodiscard]] size_t pendingCount() const {
        size_t n = 0;
        for (const auto& e : m_entries) { if (e.isPending()) ++n; }
        return n;
    }

    [[nodiscard]] const std::vector<HotReloadEntry>& entries() const { return m_entries; }

private:
    std::vector<HotReloadEntry> m_entries;
};

class HotReloadDispatcher {
public:
    size_t dispatchPending(HotReloadWatcher& watcher) {
        size_t dispatched = 0;
        for (auto& e : const_cast<std::vector<HotReloadEntry>&>(watcher.entries())) {
            if (e.isPending()) {
                e.status = HotReloadStatus::Reloading;
                // Simulate successful reload
                e.markSuccess();
                m_totalDispatched++;
                dispatched++;
            }
        }
        return dispatched;
    }

    [[nodiscard]] size_t totalDispatched() const { return m_totalDispatched; }

private:
    size_t m_totalDispatched = 0;
};

class HotReloadSystem {
public:
    void init() { m_initialized = true; }

    void shutdown() {
        m_watcher    = HotReloadWatcher{};
        m_dispatcher = HotReloadDispatcher{};
        m_tickCount  = 0;
        m_initialized = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool watch(const std::string& assetPath, HotReloadAssetType type) {
        if (!m_initialized) return false;
        return m_watcher.watch(assetPath, type);
    }

    bool unwatch(const std::string& assetPath) {
        if (!m_initialized) return false;
        return m_watcher.unwatch(assetPath);
    }

    bool triggerReload(const std::string& assetPath) {
        if (!m_initialized) return false;
        return m_watcher.triggerReload(assetPath);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_dispatcher.dispatchPending(m_watcher);
        m_tickCount++;
    }

    [[nodiscard]] HotReloadEntry* findEntry(const std::string& assetPath) {
        return m_watcher.findEntry(assetPath);
    }

    [[nodiscard]] size_t watchedCount()      const { return m_watcher.entryCount(); }
    [[nodiscard]] size_t pendingCount()      const { return m_watcher.pendingCount(); }
    [[nodiscard]] size_t totalDispatched()   const { return m_dispatcher.totalDispatched(); }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }
    [[nodiscard]] HotReloadWatcher&          watcher()          { return m_watcher; }
    [[nodiscard]] const HotReloadWatcher&    watcher()    const { return m_watcher; }
    [[nodiscard]] HotReloadDispatcher&       dispatcher()       { return m_dispatcher; }

private:
    HotReloadWatcher    m_watcher;
    HotReloadDispatcher m_dispatcher;
    bool   m_initialized = false;
    size_t m_tickCount   = 0;
};

// ============================================================
// S17 — Asset Dependency Tracker
// ============================================================

enum class AssetDepType : uint8_t {
    Texture   = 0,
    Mesh      = 1,
    Shader    = 2,
    Script    = 3,
    Audio     = 4,
    Material  = 5,
    Animation = 6,
    Level     = 7,
};

inline const char* assetDepTypeName(AssetDepType t) {
    switch (t) {
        case AssetDepType::Texture:   return "Texture";
        case AssetDepType::Mesh:      return "Mesh";
        case AssetDepType::Shader:    return "Shader";
        case AssetDepType::Script:    return "Script";
        case AssetDepType::Audio:     return "Audio";
        case AssetDepType::Material:  return "Material";
        case AssetDepType::Animation: return "Animation";
        case AssetDepType::Level:     return "Level";
        default:                      return "Unknown";
    }
}

enum class AssetDepStatus : uint8_t {
    Unknown  = 0,
    Resolved = 1,
    Missing  = 2,
    Circular = 3,
};

struct AssetDepNode {
    std::string   assetId;
    std::string   assetPath;
    AssetDepType  type   = AssetDepType::Texture;
    AssetDepStatus status = AssetDepStatus::Unknown;

    std::vector<std::string> dependencies; // ids of direct deps

    [[nodiscard]] bool isResolved() const { return status == AssetDepStatus::Resolved; }
    [[nodiscard]] bool isMissing()  const { return status == AssetDepStatus::Missing;  }
    [[nodiscard]] bool isCircular() const { return status == AssetDepStatus::Circular; }

    bool addDependency(const std::string& depId) {
        if (depId == assetId) return false; // no self-dep
        for (auto& d : dependencies) if (d == depId) return false;
        dependencies.push_back(depId);
        return true;
    }

    [[nodiscard]] bool hasDependency(const std::string& depId) const {
        for (auto& d : dependencies) if (d == depId) return true;
        return false;
    }

    [[nodiscard]] size_t dependencyCount() const { return dependencies.size(); }
};

class AssetDepGraph {
public:
    static constexpr size_t MAX_NODES = 512;

    bool addNode(const AssetDepNode& node) {
        if (m_nodes.size() >= MAX_NODES) return false;
        for (auto& n : m_nodes) if (n.assetId == node.assetId) return false;
        m_nodes.push_back(node);
        return true;
    }

    bool removeNode(const std::string& assetId) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->assetId == assetId) {
                m_nodes.erase(it);
                // remove references to this node from other nodes
                for (auto& n : m_nodes) {
                    auto& deps = n.dependencies;
                    deps.erase(std::remove(deps.begin(), deps.end(), assetId), deps.end());
                }
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AssetDepNode* findNode(const std::string& assetId) {
        for (auto& n : m_nodes) if (n.assetId == assetId) return &n;
        return nullptr;
    }
    [[nodiscard]] const AssetDepNode* findNode(const std::string& assetId) const {
        for (auto& n : m_nodes) if (n.assetId == assetId) return &n;
        return nullptr;
    }

    bool addEdge(const std::string& sourceId, const std::string& depId) {
        AssetDepNode* src = findNode(sourceId);
        if (!src) return false;
        if (!findNode(depId)) return false;
        return src->addDependency(depId);
    }

    [[nodiscard]] bool hasEdge(const std::string& sourceId, const std::string& depId) const {
        const AssetDepNode* src = findNode(sourceId);
        if (!src) return false;
        return src->hasDependency(depId);
    }

    void resolveAll() {
        for (auto& n : m_nodes)
            if (n.status == AssetDepStatus::Unknown)
                n.status = AssetDepStatus::Resolved;
    }

    void detectCircular() {
        // Simple DFS cycle detection; mark involved nodes as Circular
        std::vector<std::string> visited;
        std::vector<std::string> stack;

        std::function<bool(const std::string&)> dfs = [&](const std::string& id) -> bool {
            visited.push_back(id);
            stack.push_back(id);
            const AssetDepNode* node = findNode(id);
            if (node) {
                for (auto& dep : node->dependencies) {
                    bool inStack = false;
                    for (auto& s : stack) if (s == dep) { inStack = true; break; }
                    if (inStack) {
                        // mark all nodes in the current stack as Circular
                        for (auto& s : stack) {
                            AssetDepNode* n = findNode(s);
                            if (n) n->status = AssetDepStatus::Circular;
                        }
                        AssetDepNode* n = findNode(dep);
                        if (n) n->status = AssetDepStatus::Circular;
                        stack.pop_back();
                        return true;
                    }
                    bool inVisited = false;
                    for (auto& v : visited) if (v == dep) { inVisited = true; break; }
                    if (!inVisited) dfs(dep);
                }
            }
            stack.pop_back();
            return false;
        };

        for (auto& n : m_nodes) {
            bool inVisited = false;
            for (auto& v : visited) if (v == n.assetId) { inVisited = true; break; }
            if (!inVisited) dfs(n.assetId);
        }
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }

    [[nodiscard]] size_t unresolvedCount() const {
        size_t c = 0;
        for (auto& n : m_nodes)
            if (n.status == AssetDepStatus::Unknown || n.status == AssetDepStatus::Missing)
                c++;
        return c;
    }

    [[nodiscard]] size_t totalEdgeCount() const {
        size_t c = 0;
        for (auto& n : m_nodes) c += n.dependencyCount();
        return c;
    }

    [[nodiscard]] const std::vector<AssetDepNode>& nodes() const { return m_nodes; }

private:
    std::vector<AssetDepNode> m_nodes;
};

class AssetDependencyTracker {
public:
    bool registerAsset(const std::string& assetId, const std::string& assetPath, AssetDepType type) {
        AssetDepNode node;
        node.assetId   = assetId;
        node.assetPath = assetPath;
        node.type      = type;
        node.status    = AssetDepStatus::Unknown;
        return m_graph.addNode(node);
    }

    bool unregisterAsset(const std::string& assetId) {
        return m_graph.removeNode(assetId);
    }

    bool addDependency(const std::string& sourceId, const std::string& depId) {
        return m_graph.addEdge(sourceId, depId);
    }

    [[nodiscard]] bool hasDependency(const std::string& sourceId, const std::string& depId) const {
        return m_graph.hasEdge(sourceId, depId);
    }

    void resolveAll() {
        m_graph.resolveAll();
    }

    void detectCircular() {
        m_graph.detectCircular();
    }

    [[nodiscard]] AssetDepNode* findAsset(const std::string& assetId) {
        return m_graph.findNode(assetId);
    }

    [[nodiscard]] size_t assetCount()        const { return m_graph.nodeCount(); }
    [[nodiscard]] size_t unresolvedCount()   const { return m_graph.unresolvedCount(); }
    [[nodiscard]] size_t totalDependencies() const { return m_graph.totalEdgeCount(); }

    [[nodiscard]] AssetDepGraph&       graph()       { return m_graph; }
    [[nodiscard]] const AssetDepGraph& graph() const { return m_graph; }

private:
    AssetDepGraph m_graph;
};

// ============================================================
// S18 — Build Configuration System
// ============================================================

enum class BuildTarget : uint8_t {
    Executable   = 0,
    SharedLib    = 1,
    StaticLib    = 2,
    HeaderOnly   = 3,
    TestSuite    = 4,
    Plugin       = 5,
    Shader       = 6,
    ContentPack  = 7,
};

inline const char* buildTargetName(BuildTarget t) {
    switch (t) {
        case BuildTarget::Executable:  return "Executable";
        case BuildTarget::SharedLib:   return "SharedLib";
        case BuildTarget::StaticLib:   return "StaticLib";
        case BuildTarget::HeaderOnly:  return "HeaderOnly";
        case BuildTarget::TestSuite:   return "TestSuite";
        case BuildTarget::Plugin:      return "Plugin";
        case BuildTarget::Shader:      return "Shader";
        case BuildTarget::ContentPack: return "ContentPack";
        default:                       return "Unknown";
    }
}

enum class BuildPlatform : uint8_t {
    Windows  = 0,
    Linux    = 1,
    MacOS    = 2,
    WebAsm   = 3,
    Console  = 4,
};

inline const char* buildPlatformName(BuildPlatform p) {
    switch (p) {
        case BuildPlatform::Windows: return "Windows";
        case BuildPlatform::Linux:   return "Linux";
        case BuildPlatform::MacOS:   return "MacOS";
        case BuildPlatform::WebAsm:  return "WebAsm";
        case BuildPlatform::Console: return "Console";
        default:                     return "Unknown";
    }
}

struct BuildConfig {
    std::string   name;
    BuildTarget   target   = BuildTarget::Executable;
    BuildPlatform platform = BuildPlatform::Windows;
    bool          debugSymbols   = false;
    bool          optimized      = false;
    bool          sanitizers     = false;

    std::vector<std::string> defines;
    std::vector<std::string> includePaths;

    [[nodiscard]] bool isDebug()   const { return debugSymbols && !optimized; }
    [[nodiscard]] bool isRelease() const { return optimized && !debugSymbols; }

    bool addDefine(const std::string& def) {
        for (auto& d : defines) if (d == def) return false;
        defines.push_back(def);
        return true;
    }

    bool addIncludePath(const std::string& path) {
        for (auto& p : includePaths) if (p == path) return false;
        includePaths.push_back(path);
        return true;
    }

    [[nodiscard]] size_t defineCount()      const { return defines.size(); }
    [[nodiscard]] size_t includePathCount() const { return includePaths.size(); }
};

class BuildProfile {
public:
    static constexpr size_t MAX_CONFIGS = 64;

    bool addConfig(const BuildConfig& cfg) {
        if (m_configs.size() >= MAX_CONFIGS) return false;
        for (auto& c : m_configs) if (c.name == cfg.name) return false;
        m_configs.push_back(cfg);
        return true;
    }

    bool removeConfig(const std::string& name) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->name == name) { m_configs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] BuildConfig* findConfig(const std::string& name) {
        for (auto& c : m_configs) if (c.name == name) return &c;
        return nullptr;
    }

    [[nodiscard]] const BuildConfig* findConfig(const std::string& name) const {
        for (auto& c : m_configs) if (c.name == name) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t configCount() const { return m_configs.size(); }

    [[nodiscard]] size_t debugConfigCount() const {
        size_t c = 0;
        for (auto& cfg : m_configs) if (cfg.isDebug()) c++;
        return c;
    }

    [[nodiscard]] size_t releaseConfigCount() const {
        size_t c = 0;
        for (auto& cfg : m_configs) if (cfg.isRelease()) c++;
        return c;
    }

    [[nodiscard]] const std::vector<BuildConfig>& configs() const { return m_configs; }

private:
    std::vector<BuildConfig> m_configs;
};

class BuildConfigurationSystem {
public:
    void init() { m_initialized = true; m_activeProfile.clear(); }
    void shutdown() { m_profiles.clear(); m_activeProfile.clear(); m_initialized = false; }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool createProfile(const std::string& name) {
        if (!m_initialized) return false;
        if (m_profiles.size() >= 16) return false;
        for (auto& p : m_profiles) if (p.first == name) return false;
        m_profiles.push_back({name, BuildProfile{}});
        return true;
    }

    bool removeProfile(const std::string& name) {
        if (!m_initialized) return false;
        for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
            if (it->first == name) {
                if (m_activeProfile == name) m_activeProfile.clear();
                m_profiles.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] BuildProfile* findProfile(const std::string& name) {
        for (auto& p : m_profiles) if (p.first == name) return &p.second;
        return nullptr;
    }

    bool setActiveProfile(const std::string& name) {
        if (!m_initialized) return false;
        for (auto& p : m_profiles) {
            if (p.first == name) { m_activeProfile = name; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::string& activeProfileName() const { return m_activeProfile; }

    [[nodiscard]] BuildProfile* activeProfile() {
        if (m_activeProfile.empty()) return nullptr;
        return findProfile(m_activeProfile);
    }

    [[nodiscard]] size_t profileCount() const { return m_profiles.size(); }

    [[nodiscard]] size_t totalConfigCount() const {
        size_t c = 0;
        for (auto& p : m_profiles) c += p.second.configCount();
        return c;
    }

private:
    std::vector<std::pair<std::string, BuildProfile>> m_profiles;
    std::string m_activeProfile;
    bool m_initialized = false;
};

// ============================================================
// S19 — Scene Snapshot System
// ============================================================

enum class SceneSnapshotType : uint8_t {
    Full      = 0,
    Delta     = 1,
    Lighting  = 2,
    Physics   = 3,
    AI        = 4,
    Audio     = 5,
    Visual    = 6,
    Meta      = 7,
};

inline const char* sceneSnapshotTypeName(SceneSnapshotType t) {
    switch (t) {
        case SceneSnapshotType::Full:     return "Full";
        case SceneSnapshotType::Delta:    return "Delta";
        case SceneSnapshotType::Lighting: return "Lighting";
        case SceneSnapshotType::Physics:  return "Physics";
        case SceneSnapshotType::AI:       return "AI";
        case SceneSnapshotType::Audio:    return "Audio";
        case SceneSnapshotType::Visual:   return "Visual";
        case SceneSnapshotType::Meta:     return "Meta";
        default:                          return "Unknown";
    }
}

enum class SceneSnapshotState : uint8_t {
    Valid     = 0,
    Outdated  = 1,
    Corrupted = 2,
    Partial   = 3,
};

struct SceneSnapshotFrame {
    std::string        id;
    std::string        label;
    SceneSnapshotType  type  = SceneSnapshotType::Full;
    SceneSnapshotState state = SceneSnapshotState::Valid;
    uint64_t           timestamp = 0;
    size_t             dataSize  = 0;   // bytes captured

    [[nodiscard]] bool isValid()     const { return state == SceneSnapshotState::Valid;     }
    [[nodiscard]] bool isOutdated()  const { return state == SceneSnapshotState::Outdated;  }
    [[nodiscard]] bool isCorrupted() const { return state == SceneSnapshotState::Corrupted; }
    [[nodiscard]] bool isPartial()   const { return state == SceneSnapshotState::Partial;   }

    void markOutdated()  { if (state == SceneSnapshotState::Valid) state = SceneSnapshotState::Outdated;  }
    void markCorrupted() { state = SceneSnapshotState::Corrupted; }
};

class SceneSnapshotHistory {
public:
    static constexpr size_t MAX_FRAMES = 128;

    bool push(const SceneSnapshotFrame& frame) {
        if (m_frames.size() >= MAX_FRAMES) return false;
        for (auto& f : m_frames) if (f.id == frame.id) return false;
        m_frames.push_back(frame);
        return true;
    }

    bool remove(const std::string& id) {
        for (auto it = m_frames.begin(); it != m_frames.end(); ++it) {
            if (it->id == id) { m_frames.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SceneSnapshotFrame* find(const std::string& id) {
        for (auto& f : m_frames) if (f.id == id) return &f;
        return nullptr;
    }

    [[nodiscard]] const SceneSnapshotFrame* find(const std::string& id) const {
        for (auto& f : m_frames) if (f.id == id) return &f;
        return nullptr;
    }

    [[nodiscard]] SceneSnapshotFrame* latest() {
        if (m_frames.empty()) return nullptr;
        return &m_frames.back();
    }

    void markAllOutdated() {
        for (auto& f : m_frames) f.markOutdated();
    }

    [[nodiscard]] size_t frameCount()     const { return m_frames.size(); }
    [[nodiscard]] bool   empty()          const { return m_frames.empty(); }

    [[nodiscard]] size_t validCount() const {
        size_t c = 0;
        for (auto& f : m_frames) if (f.isValid()) c++;
        return c;
    }

    [[nodiscard]] size_t corruptedCount() const {
        size_t c = 0;
        for (auto& f : m_frames) if (f.isCorrupted()) c++;
        return c;
    }

    [[nodiscard]] size_t totalDataSize() const {
        size_t total = 0;
        for (auto& f : m_frames) total += f.dataSize;
        return total;
    }

    [[nodiscard]] const std::vector<SceneSnapshotFrame>& frames() const { return m_frames; }

private:
    std::vector<SceneSnapshotFrame> m_frames;
};

class SceneSnapshotSystem {
public:
    void init()     { m_initialized = true;  m_history = SceneSnapshotHistory{}; }
    void shutdown() { m_initialized = false; m_history = SceneSnapshotHistory{}; }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool capture(const SceneSnapshotFrame& frame) {
        if (!m_initialized) return false;
        return m_history.push(frame);
    }

    bool discard(const std::string& id) {
        if (!m_initialized) return false;
        return m_history.remove(id);
    }

    [[nodiscard]] SceneSnapshotFrame* find(const std::string& id) {
        return m_history.find(id);
    }

    [[nodiscard]] SceneSnapshotFrame* latest() {
        return m_history.latest();
    }

    void invalidateAll() {
        m_history.markAllOutdated();
    }

    [[nodiscard]] size_t frameCount()     const { return m_history.frameCount();     }
    [[nodiscard]] size_t validCount()     const { return m_history.validCount();     }
    [[nodiscard]] size_t corruptedCount() const { return m_history.corruptedCount(); }
    [[nodiscard]] size_t totalDataSize()  const { return m_history.totalDataSize();  }

    [[nodiscard]] SceneSnapshotHistory&       history()       { return m_history; }
    [[nodiscard]] const SceneSnapshotHistory& history() const { return m_history; }

private:
    SceneSnapshotHistory m_history;
    bool                 m_initialized = false;
};

// ============================================================
// S20 — Resource Monitor System
// ============================================================

enum class ResourceMonitorMetric : uint8_t {
    CPU        = 0,
    GPU        = 1,
    Memory     = 2,
    DiskIO     = 3,
    NetworkIO  = 4,
    FrameTime  = 5,
    DrawCalls  = 6,
    ThreadLoad = 7,
};

inline const char* resourceMonitorMetricName(ResourceMonitorMetric m) {
    switch (m) {
        case ResourceMonitorMetric::CPU:        return "CPU";
        case ResourceMonitorMetric::GPU:        return "GPU";
        case ResourceMonitorMetric::Memory:     return "Memory";
        case ResourceMonitorMetric::DiskIO:     return "DiskIO";
        case ResourceMonitorMetric::NetworkIO:  return "NetworkIO";
        case ResourceMonitorMetric::FrameTime:  return "FrameTime";
        case ResourceMonitorMetric::DrawCalls:  return "DrawCalls";
        case ResourceMonitorMetric::ThreadLoad: return "ThreadLoad";
        default:                                return "Unknown";
    }
}

enum class ResourceMonitorLevel : uint8_t {
    Normal   = 0,
    Warning  = 1,
    Critical = 2,
    Overflow = 3,
};

struct ResourceMonitorSample {
    ResourceMonitorMetric metric    = ResourceMonitorMetric::CPU;
    ResourceMonitorLevel  level     = ResourceMonitorLevel::Normal;
    float                 value     = 0.0f;
    uint64_t              timestamp = 0;

    [[nodiscard]] bool isWarning()  const { return level == ResourceMonitorLevel::Warning;  }
    [[nodiscard]] bool isCritical() const { return level == ResourceMonitorLevel::Critical; }
    [[nodiscard]] bool isOverflow() const { return level == ResourceMonitorLevel::Overflow; }
    [[nodiscard]] bool isHealthy()  const { return level == ResourceMonitorLevel::Normal;   }

    [[nodiscard]] ResourceMonitorLevel computeLevel(float warnThreshold, float critThreshold) const {
        if (value >= critThreshold) return ResourceMonitorLevel::Critical;
        if (value >= warnThreshold) return ResourceMonitorLevel::Warning;
        return ResourceMonitorLevel::Normal;
    }
};

class ResourceMonitorChannel {
public:
    static constexpr size_t MAX_SAMPLES = 256;

    explicit ResourceMonitorChannel(ResourceMonitorMetric metric) : m_metric(metric) {}

    [[nodiscard]] ResourceMonitorMetric metric() const { return m_metric; }

    bool push(const ResourceMonitorSample& s) {
        if (s.metric != m_metric) return false;
        if (m_samples.size() >= MAX_SAMPLES) m_samples.erase(m_samples.begin());
        m_samples.push_back(s);
        return true;
    }

    [[nodiscard]] const ResourceMonitorSample* latest() const {
        if (m_samples.empty()) return nullptr;
        return &m_samples.back();
    }

    [[nodiscard]] size_t sampleCount() const { return m_samples.size(); }
    [[nodiscard]] bool   empty()       const { return m_samples.empty(); }

    [[nodiscard]] float average() const {
        if (m_samples.empty()) return 0.0f;
        float sum = 0.0f;
        for (auto& s : m_samples) sum += s.value;
        return sum / static_cast<float>(m_samples.size());
    }

    [[nodiscard]] float peak() const {
        float p = 0.0f;
        for (auto& s : m_samples) if (s.value > p) p = s.value;
        return p;
    }

    [[nodiscard]] size_t warningCount() const {
        size_t c = 0;
        for (auto& s : m_samples) if (s.isWarning() || s.isCritical()) c++;
        return c;
    }

    void clear() { m_samples.clear(); }

    [[nodiscard]] const std::vector<ResourceMonitorSample>& samples() const { return m_samples; }

private:
    ResourceMonitorMetric              m_metric;
    std::vector<ResourceMonitorSample> m_samples;
};

class ResourceMonitorSystem {
public:
    void init() {
        m_initialized = true;
        m_channels.clear();
        for (uint8_t i = 0; i <= static_cast<uint8_t>(ResourceMonitorMetric::ThreadLoad); ++i) {
            m_channels.emplace_back(static_cast<ResourceMonitorMetric>(i));
        }
    }

    void shutdown() {
        m_initialized = false;
        m_channels.clear();
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool record(const ResourceMonitorSample& s) {
        if (!m_initialized) return false;
        auto* ch = channelFor(s.metric);
        if (!ch) return false;
        return ch->push(s);
    }

    [[nodiscard]] ResourceMonitorChannel* channelFor(ResourceMonitorMetric m) {
        for (auto& ch : m_channels) if (ch.metric() == m) return &ch;
        return nullptr;
    }

    [[nodiscard]] const ResourceMonitorChannel* channelFor(ResourceMonitorMetric m) const {
        for (auto& ch : m_channels) if (ch.metric() == m) return &ch;
        return nullptr;
    }

    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

    [[nodiscard]] size_t totalSamples() const {
        size_t t = 0;
        for (auto& ch : m_channels) t += ch.sampleCount();
        return t;
    }

    [[nodiscard]] size_t totalWarnings() const {
        size_t t = 0;
        for (auto& ch : m_channels) t += ch.warningCount();
        return t;
    }

    void clearAll() {
        for (auto& ch : m_channels) ch.clear();
    }

private:
    std::vector<ResourceMonitorChannel> m_channels;
    bool                                m_initialized = false;
};

// ============================================================
// S21 — Event Bus System
// ============================================================

enum class EditorEventPriority : uint8_t {
    Lowest  = 0,
    Low     = 1,
    Normal  = 2,
    High    = 3,
    Highest = 4,
    System  = 5,
    Critical = 6,
    Realtime = 7,
};

inline const char* editorEventPriorityName(EditorEventPriority p) {
    switch (p) {
        case EditorEventPriority::Lowest:   return "Lowest";
        case EditorEventPriority::Low:      return "Low";
        case EditorEventPriority::Normal:   return "Normal";
        case EditorEventPriority::High:     return "High";
        case EditorEventPriority::Highest:  return "Highest";
        case EditorEventPriority::System:   return "System";
        case EditorEventPriority::Critical: return "Critical";
        case EditorEventPriority::Realtime: return "Realtime";
        default:                            return "Unknown";
    }
}

enum class EditorBusState : uint8_t {
    Idle      = 0,
    Posting   = 1,
    Flushing  = 2,
    Suspended = 3,
};

struct EditorBusEvent {
    std::string          topic;
    std::string          payload;
    EditorEventPriority  priority  = EditorEventPriority::Normal;
    uint64_t             timestamp = 0;
    bool                 consumed  = false;

    void consume() { consumed = true; }
    [[nodiscard]] bool isConsumed() const { return consumed; }
    [[nodiscard]] bool isHighPrio() const { return priority >= EditorEventPriority::High; }
    [[nodiscard]] bool isCritical() const { return priority >= EditorEventPriority::Critical; }
};

class EditorEventSubscription {
public:
    using Handler = std::function<void(const EditorBusEvent&)>;

    EditorEventSubscription(std::string topic, EditorEventPriority minPrio, Handler handler)
        : m_topic(std::move(topic)), m_minPriority(minPrio), m_handler(std::move(handler)) {}

    [[nodiscard]] const std::string& topic()      const { return m_topic;       }
    [[nodiscard]] EditorEventPriority minPriority() const { return m_minPriority; }
    [[nodiscard]] size_t             callCount()  const { return m_callCount;   }
    [[nodiscard]] bool               isActive()   const { return m_active;      }

    void deliver(const EditorBusEvent& ev) {
        if (!m_active) return;
        if (ev.priority < m_minPriority) return;
        if (m_handler) { m_handler(ev); ++m_callCount; }
    }

    void cancel() { m_active = false; }

private:
    std::string          m_topic;
    EditorEventPriority  m_minPriority;
    Handler              m_handler;
    size_t               m_callCount = 0;
    bool                 m_active    = true;
};

class EditorEventBus {
public:
    static constexpr size_t MAX_SUBSCRIPTIONS = 256;
    static constexpr size_t MAX_QUEUE         = 512;

    EditorEventSubscription* subscribe(const std::string& topic, EditorEventPriority minPrio,
                                        EditorEventSubscription::Handler handler) {
        if (m_subscriptions.size() >= MAX_SUBSCRIPTIONS) return nullptr;
        m_subscriptions.emplace_back(topic, minPrio, std::move(handler));
        return &m_subscriptions.back();
    }

    bool post(const EditorBusEvent& ev) {
        if (m_state == EditorBusState::Suspended) return false;
        if (m_queue.size() >= MAX_QUEUE) return false;
        m_queue.push_back(ev);
        return true;
    }

    size_t flush() {
        if (m_state == EditorBusState::Suspended) return 0;
        m_state = EditorBusState::Flushing;
        size_t dispatched = 0;
        for (auto& ev : m_queue) {
            for (auto& sub : m_subscriptions) {
                if (sub.topic() == ev.topic || sub.topic() == "*") {
                    sub.deliver(ev);
                    ++dispatched;
                }
            }
        }
        m_queue.clear();
        m_state = EditorBusState::Idle;
        return dispatched;
    }

    void suspend()    { m_state = EditorBusState::Suspended; }
    void resume()     { if (m_state == EditorBusState::Suspended) m_state = EditorBusState::Idle; }
    void clearQueue() { m_queue.clear(); }

    [[nodiscard]] EditorBusState state()             const { return m_state;                 }
    [[nodiscard]] size_t         queueSize()          const { return m_queue.size();          }
    [[nodiscard]] size_t         subscriptionCount()  const { return m_subscriptions.size(); }
    [[nodiscard]] bool           isSuspended()        const { return m_state == EditorBusState::Suspended; }

private:
    std::vector<EditorEventSubscription> m_subscriptions;
    std::vector<EditorBusEvent>          m_queue;
    EditorBusState                       m_state = EditorBusState::Idle;
};

// ============================================================
// S22 — Workspace Layout Manager
// ============================================================

enum class LayoutPanelType : uint8_t {
    Viewport    = 0,
    Inspector   = 1,
    Hierarchy   = 2,
    ContentBrowser = 3,
    Console     = 4,
    Profiler    = 5,
    Timeline    = 6,
    Custom      = 7,
};

inline const char* layoutPanelTypeName(LayoutPanelType t) {
    switch (t) {
        case LayoutPanelType::Viewport:       return "Viewport";
        case LayoutPanelType::Inspector:      return "Inspector";
        case LayoutPanelType::Hierarchy:      return "Hierarchy";
        case LayoutPanelType::ContentBrowser: return "ContentBrowser";
        case LayoutPanelType::Console:        return "Console";
        case LayoutPanelType::Profiler:       return "Profiler";
        case LayoutPanelType::Timeline:       return "Timeline";
        case LayoutPanelType::Custom:         return "Custom";
        default:                              return "Unknown";
    }
}

enum class LayoutDockZone : uint8_t {
    Left   = 0,
    Right  = 1,
    Top    = 2,
    Bottom = 3,
};

inline const char* layoutDockZoneName(LayoutDockZone z) {
    switch (z) {
        case LayoutDockZone::Left:   return "Left";
        case LayoutDockZone::Right:  return "Right";
        case LayoutDockZone::Top:    return "Top";
        case LayoutDockZone::Bottom: return "Bottom";
        default:                     return "Unknown";
    }
}

struct LayoutPanel {
    std::string     id;
    std::string     title;
    LayoutPanelType type     = LayoutPanelType::Custom;
    LayoutDockZone  dockZone = LayoutDockZone::Left;
    float           width    = 0.f;
    float           height   = 0.f;
    bool            visible  = true;
    bool            pinned   = false;

    void show()    { visible = true;  }
    void hide()    { visible = false; }
    void pin()     { pinned = true;   }
    void unpin()   { pinned = false;  }

    [[nodiscard]] bool isVisible() const { return visible; }
    [[nodiscard]] bool isPinned()  const { return pinned;  }
    [[nodiscard]] bool hasSize()   const { return width > 0.f && height > 0.f; }
};

struct LayoutSplit {
    std::string firstPanelId;
    std::string secondPanelId;
    bool        isHorizontal = true;
    float       ratio        = 0.5f;

    [[nodiscard]] bool isValid() const {
        return !firstPanelId.empty() && !secondPanelId.empty() && ratio > 0.f && ratio < 1.f;
    }
    void flipOrientation() { isHorizontal = !isHorizontal; }
};

class WorkspaceLayout {
public:
    explicit WorkspaceLayout(std::string name) : m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] size_t             panelCount()  const { return m_panels.size();  }
    [[nodiscard]] size_t             splitCount()  const { return m_splits.size();  }

    bool addPanel(const LayoutPanel& p) {
        for (auto& existing : m_panels) if (existing.id == p.id) return false;
        m_panels.push_back(p);
        return true;
    }

    bool removePanel(const std::string& id) {
        for (auto it = m_panels.begin(); it != m_panels.end(); ++it) {
            if (it->id == id) { m_panels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] LayoutPanel* findPanel(const std::string& id) {
        for (auto& p : m_panels) if (p.id == id) return &p;
        return nullptr;
    }

    bool addSplit(const LayoutSplit& s) {
        if (!s.isValid()) return false;
        m_splits.push_back(s);
        return true;
    }

    [[nodiscard]] size_t visiblePanelCount() const {
        size_t c = 0;
        for (auto& p : m_panels) if (p.isVisible()) c++;
        return c;
    }

    [[nodiscard]] size_t pinnedPanelCount() const {
        size_t c = 0;
        for (auto& p : m_panels) if (p.isPinned()) c++;
        return c;
    }

    void showAll() { for (auto& p : m_panels) p.show(); }
    void hideAll() { for (auto& p : m_panels) p.hide(); }

private:
    std::string              m_name;
    std::vector<LayoutPanel> m_panels;
    std::vector<LayoutSplit> m_splits;
};

class WorkspaceLayoutManager {
public:
    static constexpr size_t MAX_LAYOUTS = 32;

    WorkspaceLayout* createLayout(const std::string& name) {
        if (m_layouts.size() >= MAX_LAYOUTS) return nullptr;
        for (auto& l : m_layouts) if (l.name() == name) return nullptr;
        m_layouts.emplace_back(name);
        return &m_layouts.back();
    }

    bool removeLayout(const std::string& name) {
        for (auto it = m_layouts.begin(); it != m_layouts.end(); ++it) {
            if (it->name() == name) {
                if (m_activeLayout == name) m_activeLayout.clear();
                m_layouts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] WorkspaceLayout* findLayout(const std::string& name) {
        for (auto& l : m_layouts) if (l.name() == name) return &l;
        return nullptr;
    }

    bool setActive(const std::string& name) {
        if (!findLayout(name)) return false;
        m_activeLayout = name;
        return true;
    }

    [[nodiscard]] WorkspaceLayout* activeLayout() {
        return findLayout(m_activeLayout);
    }

    [[nodiscard]] const std::string& activeName()  const { return m_activeLayout; }
    [[nodiscard]] size_t             layoutCount() const { return m_layouts.size(); }
    [[nodiscard]] bool               hasActive()   const { return !m_activeLayout.empty(); }

private:
    std::vector<WorkspaceLayout> m_layouts;
    std::string                  m_activeLayout;
};

// ============================================================
// S23 — Shortcut Manager
// ============================================================

enum class ShortcutCategory : uint8_t {
    File, Edit, View, Navigate, Select, Debug, Tool, Custom
};

[[nodiscard]] inline const char* shortcutCategoryName(ShortcutCategory c) {
    switch (c) {
        case ShortcutCategory::File:     return "File";
        case ShortcutCategory::Edit:     return "Edit";
        case ShortcutCategory::View:     return "View";
        case ShortcutCategory::Navigate: return "Navigate";
        case ShortcutCategory::Select:   return "Select";
        case ShortcutCategory::Debug:    return "Debug";
        case ShortcutCategory::Tool:     return "Tool";
        case ShortcutCategory::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class ShortcutState : uint8_t {
    Inactive, Active, Pressed, Blocked
};

[[nodiscard]] inline const char* shortcutStateName(ShortcutState s) {
    switch (s) {
        case ShortcutState::Inactive: return "Inactive";
        case ShortcutState::Active:   return "Active";
        case ShortcutState::Pressed:  return "Pressed";
        case ShortcutState::Blocked:  return "Blocked";
    }
    return "Unknown";
}

struct ShortcutBinding {
    std::string      id;
    std::string      name;
    ShortcutCategory category  = ShortcutCategory::File;
    std::string      key;
    uint8_t          modifiers = 0;
    bool             enabled   = true;
    ShortcutState    state     = ShortcutState::Inactive;

    void enable()   { enabled = true; }
    void disable()  { enabled = false; }
    void trigger()  { state = ShortcutState::Pressed; }
    void reset()    { state = ShortcutState::Inactive; }

    [[nodiscard]] bool isEnabled()  const { return enabled; }
    [[nodiscard]] bool isActive()   const { return state == ShortcutState::Pressed; }
    [[nodiscard]] bool hasKey()     const { return !key.empty(); }
};

class ShortcutContext {
public:
    explicit ShortcutContext(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name()         const { return m_name; }
    [[nodiscard]] size_t             bindingCount() const { return m_bindings.size(); }

    bool addBinding(const ShortcutBinding& b) {
        for (auto& existing : m_bindings) if (existing.id == b.id) return false;
        m_bindings.push_back(b);
        return true;
    }

    bool removeBinding(const std::string& id) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->id == id) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ShortcutBinding* findBinding(const std::string& id) {
        for (auto& b : m_bindings) if (b.id == id) return &b;
        return nullptr;
    }

    void enableAll()  { for (auto& b : m_bindings) b.enable(); }
    void disableAll() { for (auto& b : m_bindings) b.disable(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& b : m_bindings) if (b.isEnabled()) c++;
        return c;
    }

private:
    std::string                  m_name;
    std::vector<ShortcutBinding> m_bindings;
};

class ShortcutManager {
public:
    static constexpr size_t MAX_CONTEXTS = 32;

    ShortcutContext* createContext(const std::string& name) {
        if (m_contexts.size() >= MAX_CONTEXTS) return nullptr;
        for (auto& c : m_contexts) if (c.name() == name) return nullptr;
        m_contexts.emplace_back(name);
        return &m_contexts.back();
    }

    bool removeContext(const std::string& name) {
        for (auto it = m_contexts.begin(); it != m_contexts.end(); ++it) {
            if (it->name() == name) {
                if (m_activeName == name) m_activeName.clear();
                m_contexts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ShortcutContext* findContext(const std::string& name) {
        for (auto& c : m_contexts) if (c.name() == name) return &c;
        return nullptr;
    }

    bool setActiveContext(const std::string& name) {
        if (!findContext(name)) return false;
        m_activeName = name;
        return true;
    }

    [[nodiscard]] ShortcutContext* activeContext() { return findContext(m_activeName); }
    [[nodiscard]] const std::string& activeName()  const { return m_activeName; }
    [[nodiscard]] bool               hasActive()   const { return !m_activeName.empty(); }
    [[nodiscard]] size_t             contextCount() const { return m_contexts.size(); }

private:
    std::vector<ShortcutContext> m_contexts;
    std::string                  m_activeName;
};

// ============================================================
// S24 — Notification System
// ============================================================

enum class NotificationSeverity : uint8_t { Info, Success, Warning, Error, Critical, Debug, Trace, System };

inline const char* notificationSeverityName(NotificationSeverity s) {
    switch (s) {
        case NotificationSeverity::Info:     return "Info";
        case NotificationSeverity::Success:  return "Success";
        case NotificationSeverity::Warning:  return "Warning";
        case NotificationSeverity::Error:    return "Error";
        case NotificationSeverity::Critical: return "Critical";
        case NotificationSeverity::Debug:    return "Debug";
        case NotificationSeverity::Trace:    return "Trace";
        case NotificationSeverity::System:   return "System";
        default:                             return "Unknown";
    }
}

enum class NotificationState : uint8_t { Pending, Shown, Dismissed, Expired };

inline const char* notificationStateName(NotificationState s) {
    switch (s) {
        case NotificationState::Pending:   return "Pending";
        case NotificationState::Shown:     return "Shown";
        case NotificationState::Dismissed: return "Dismissed";
        case NotificationState::Expired:   return "Expired";
        default:                           return "Unknown";
    }
}

struct Notification {
    std::string          id;
    std::string          title;
    std::string          message;
    NotificationSeverity severity   = NotificationSeverity::Info;
    NotificationState    state      = NotificationState::Pending;
    uint32_t             durationMs = 3000;
    bool                 persistent = false;

    void dismiss() { state = NotificationState::Dismissed; }
    void expire()  { state = NotificationState::Expired;   }
    void show()    { state = NotificationState::Shown;     }

    [[nodiscard]] bool isDismissed() const { return state == NotificationState::Dismissed; }
    [[nodiscard]] bool isExpired()   const { return state == NotificationState::Expired;   }
    [[nodiscard]] bool isVisible()   const { return state == NotificationState::Shown;     }
    [[nodiscard]] bool isError()     const { return severity >= NotificationSeverity::Error; }
    [[nodiscard]] bool isCritical()  const { return severity == NotificationSeverity::Critical; }
};

class NotificationChannel {
public:
    explicit NotificationChannel(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name()              const { return m_name; }
    [[nodiscard]] size_t             notificationCount() const { return m_notifications.size(); }

    bool post(Notification n) {
        for (auto& existing : m_notifications) if (existing.id == n.id) return false;
        n.show();
        m_notifications.push_back(std::move(n));
        return true;
    }

    bool dismiss(const std::string& id) {
        for (auto& n : m_notifications) {
            if (n.id == id) { n.dismiss(); return true; }
        }
        return false;
    }

    [[nodiscard]] Notification* find(const std::string& id) {
        for (auto& n : m_notifications) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& n : m_notifications) if (n.isVisible()) c++;
        return c;
    }

    [[nodiscard]] size_t errorCount() const {
        size_t c = 0;
        for (auto& n : m_notifications) if (n.isError()) c++;
        return c;
    }

    size_t clearDismissed() {
        size_t before = m_notifications.size();
        m_notifications.erase(
            std::remove_if(m_notifications.begin(), m_notifications.end(),
                [](const Notification& n){ return n.isDismissed() || n.isExpired(); }),
            m_notifications.end());
        return before - m_notifications.size();
    }

private:
    std::string               m_name;
    std::vector<Notification> m_notifications;
};

class NotificationSystem {
public:
    static constexpr size_t MAX_CHANNELS = 16;

    NotificationChannel* createChannel(const std::string& name) {
        if (m_channels.size() >= MAX_CHANNELS) return nullptr;
        for (auto& c : m_channels) if (c.name() == name) return nullptr;
        m_channels.emplace_back(name);
        return &m_channels.back();
    }

    bool removeChannel(const std::string& name) {
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            if (it->name() == name) { m_channels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] NotificationChannel* findChannel(const std::string& name) {
        for (auto& c : m_channels) if (c.name() == name) return &c;
        return nullptr;
    }

    bool post(const std::string& channelName, Notification n) {
        auto* ch = findChannel(channelName);
        if (!ch) return false;
        return ch->post(std::move(n));
    }

    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

    [[nodiscard]] size_t totalActive() const {
        size_t c = 0;
        for (auto& ch : m_channels) c += ch.activeCount();
        return c;
    }

private:
    std::vector<NotificationChannel> m_channels;
};

// ============================================================
// S25 — Undo/Redo System
// ============================================================

enum class UndoActionType : uint8_t { Create, Delete, Move, Resize, Rename, Modify, Group, Ungroup };

inline const char* undoActionTypeName(UndoActionType t) {
    switch (t) {
        case UndoActionType::Create:   return "Create";
        case UndoActionType::Delete:   return "Delete";
        case UndoActionType::Move:     return "Move";
        case UndoActionType::Resize:   return "Resize";
        case UndoActionType::Rename:   return "Rename";
        case UndoActionType::Modify:   return "Modify";
        case UndoActionType::Group:    return "Group";
        case UndoActionType::Ungroup:  return "Ungroup";
        default:                       return "Unknown";
    }
}

enum class UndoActionState : uint8_t { Pending, Applied, Undone, Invalid };

inline const char* undoActionStateName(UndoActionState s) {
    switch (s) {
        case UndoActionState::Pending:  return "Pending";
        case UndoActionState::Applied:  return "Applied";
        case UndoActionState::Undone:   return "Undone";
        case UndoActionState::Invalid:  return "Invalid";
        default:                        return "Unknown";
    }
}

struct UndoAction {
    std::string     id;
    std::string     description;
    UndoActionType  type  = UndoActionType::Create;
    UndoActionState state = UndoActionState::Pending;

    void apply()      { state = UndoActionState::Applied; }
    void undo()       { if (state == UndoActionState::Applied) state = UndoActionState::Undone; }
    void invalidate() { state = UndoActionState::Invalid; }

    [[nodiscard]] bool isApplied() const { return state == UndoActionState::Applied; }
    [[nodiscard]] bool isUndone()  const { return state == UndoActionState::Undone;  }
    [[nodiscard]] bool isValid()   const { return state != UndoActionState::Invalid; }
    [[nodiscard]] bool canUndo()   const { return isApplied(); }
    [[nodiscard]] bool canRedo()   const { return isUndone();  }
};

class UndoGroup {
public:
    explicit UndoGroup(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool addAction(UndoAction a) {
        for (auto& existing : m_actions) if (existing.id == a.id) return false;
        m_actions.push_back(std::move(a));
        return true;
    }

    bool removeAction(const std::string& id) {
        for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
            if (it->id == id) { m_actions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] UndoAction* find(const std::string& id) {
        for (auto& a : m_actions) if (a.id == id) return &a;
        return nullptr;
    }

    void applyAll() { for (auto& a : m_actions) a.apply(); }

    void undoAll() {
        for (auto& a : m_actions) if (a.isApplied()) a.undo();
    }

    [[nodiscard]] size_t actionCount() const { return m_actions.size(); }

    [[nodiscard]] size_t appliedCount() const {
        size_t c = 0;
        for (auto& a : m_actions) if (a.isApplied()) c++;
        return c;
    }

private:
    std::string              m_name;
    std::vector<UndoAction>  m_actions;
};

class UndoRedoSystem {
public:
    static constexpr size_t MAX_GROUPS = 64;

    bool pushGroup(UndoGroup g) {
        if (m_undoStack.size() >= MAX_GROUPS) return false;
        m_redoStack.clear();
        m_undoStack.push_back(std::move(g));
        return true;
    }

    bool undo() {
        if (m_undoStack.empty()) return false;
        UndoGroup g = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        g.undoAll();
        m_redoStack.push_back(std::move(g));
        return true;
    }

    bool redo() {
        if (m_redoStack.empty()) return false;
        UndoGroup g = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        g.applyAll();
        m_undoStack.push_back(std::move(g));
        return true;
    }

    [[nodiscard]] bool   canUndo()    const { return !m_undoStack.empty(); }
    [[nodiscard]] bool   canRedo()    const { return !m_redoStack.empty(); }
    [[nodiscard]] size_t undoDepth()  const { return m_undoStack.size();   }
    [[nodiscard]] size_t redoDepth()  const { return m_redoStack.size();   }

    void clear() { m_undoStack.clear(); m_redoStack.clear(); }

private:
    std::vector<UndoGroup> m_undoStack;
    std::vector<UndoGroup> m_redoStack;
};

// ============================================================
// S26 — Command Palette
// ============================================================

enum class CommandPaletteCategory : uint8_t { File, Edit, View, Navigate, Debug, Build, Tools, Help };

inline const char* commandPaletteCategoryName(CommandPaletteCategory c) {
    switch (c) {
        case CommandPaletteCategory::File:     return "File";
        case CommandPaletteCategory::Edit:     return "Edit";
        case CommandPaletteCategory::View:     return "View";
        case CommandPaletteCategory::Navigate: return "Navigate";
        case CommandPaletteCategory::Debug:    return "Debug";
        case CommandPaletteCategory::Build:    return "Build";
        case CommandPaletteCategory::Tools:    return "Tools";
        case CommandPaletteCategory::Help:     return "Help";
        default:                               return "Unknown";
    }
}

enum class CommandPaletteState : uint8_t { Idle, Open, Searching, Executing };

inline const char* commandPaletteStateName(CommandPaletteState s) {
    switch (s) {
        case CommandPaletteState::Idle:      return "Idle";
        case CommandPaletteState::Open:      return "Open";
        case CommandPaletteState::Searching: return "Searching";
        case CommandPaletteState::Executing: return "Executing";
        default:                             return "Unknown";
    }
}

struct PaletteCommand {
    std::string            id;
    std::string            label;
    CommandPaletteCategory category     = CommandPaletteCategory::File;
    bool                   enabled      = true;
    size_t                 executeCount = 0;

    void execute()  { if (enabled) executeCount++; }
    void disable()  { enabled = false; }
    void enable()   { enabled = true;  }

    [[nodiscard]] bool   hasBeenExecuted() const { return executeCount > 0; }
    [[nodiscard]] bool   isEnabled()       const { return enabled;          }
    [[nodiscard]] size_t timesExecuted()   const { return executeCount;     }
};

class PaletteCommandGroup {
public:
    explicit PaletteCommandGroup(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool addCommand(PaletteCommand cmd) {
        for (auto& existing : m_commands) if (existing.id == cmd.id) return false;
        m_commands.push_back(std::move(cmd));
        return true;
    }

    bool removeCommand(const std::string& id) {
        for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
            if (it->id == id) { m_commands.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] PaletteCommand* find(const std::string& id) {
        for (auto& c : m_commands) if (c.id == id) return &c;
        return nullptr;
    }

    void enableAll()  { for (auto& c : m_commands) c.enable();  }
    void disableAll() { for (auto& c : m_commands) c.disable(); }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& c : m_commands) if (c.isEnabled()) n++;
        return n;
    }

private:
    std::string                  m_name;
    std::vector<PaletteCommand>  m_commands;
};

class CommandPalette {
public:
    static constexpr size_t MAX_COMMANDS = 128;

    bool registerCommand(PaletteCommand cmd) {
        if (m_commands.size() >= MAX_COMMANDS) return false;
        for (auto& existing : m_commands) if (existing.id == cmd.id) return false;
        m_commands.push_back(std::move(cmd));
        return true;
    }

    bool unregisterCommand(const std::string& id) {
        for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
            if (it->id == id) { m_commands.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] PaletteCommand* find(const std::string& id) {
        for (auto& c : m_commands) if (c.id == id) return &c;
        return nullptr;
    }

    bool execute(const std::string& id) {
        auto* cmd = find(id);
        if (!cmd) return false;
        cmd->execute();
        return true;
    }

    [[nodiscard]] std::vector<PaletteCommand*> search(const std::string& query) {
        std::vector<PaletteCommand*> results;
        for (auto& c : m_commands)
            if (c.label.find(query) != std::string::npos) results.push_back(&c);
        return results;
    }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& c : m_commands) if (c.isEnabled()) n++;
        return n;
    }

    void setState(CommandPaletteState s) { m_state = s; }
    [[nodiscard]] CommandPaletteState state() const { return m_state; }

    void open()  { m_state = CommandPaletteState::Open; }
    void close() { m_state = CommandPaletteState::Idle; }

private:
    std::vector<PaletteCommand> m_commands;
    CommandPaletteState         m_state = CommandPaletteState::Idle;
};

// ============================================================
// S27 — Theme Manager
// ============================================================

enum class ThemeMode : uint8_t { Light, Dark, HighContrast, Custom };

inline const char* themeModeName(ThemeMode m) {
    switch (m) {
        case ThemeMode::Light:       return "Light";
        case ThemeMode::Dark:        return "Dark";
        case ThemeMode::HighContrast:return "HighContrast";
        case ThemeMode::Custom:      return "Custom";
        default:                     return "Unknown";
    }
}

enum class ThemeColor : uint8_t { Background, Foreground, Primary, Secondary, Accent, Border, Error, Warning };

inline const char* themeColorName(ThemeColor c) {
    switch (c) {
        case ThemeColor::Background: return "Background";
        case ThemeColor::Foreground: return "Foreground";
        case ThemeColor::Primary:    return "Primary";
        case ThemeColor::Secondary:  return "Secondary";
        case ThemeColor::Accent:     return "Accent";
        case ThemeColor::Border:     return "Border";
        case ThemeColor::Error:      return "Error";
        case ThemeColor::Warning:    return "Warning";
        default:                     return "Unknown";
    }
}

struct ThemeToken {
    std::string key;
    std::string value;
    ThemeMode   mode = ThemeMode::Dark;

    [[nodiscard]] bool matches(ThemeMode m) const { return mode == m; }
    void update(const std::string& newValue)      { value = newValue; }
};

struct Theme {
    std::string name;
    ThemeMode   m_mode    = ThemeMode::Dark;
    uint32_t    version   = 1;

    bool addToken(ThemeToken t) {
        for (auto& existing : m_tokens) if (existing.key == t.key) return false;
        m_tokens.push_back(std::move(t));
        return true;
    }

    bool removeToken(const std::string& key) {
        for (auto it = m_tokens.begin(); it != m_tokens.end(); ++it) {
            if (it->key == key) { m_tokens.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ThemeToken* findToken(const std::string& key) {
        for (auto& t : m_tokens) if (t.key == key) return &t;
        return nullptr;
    }

    [[nodiscard]] size_t    tokenCount() const { return m_tokens.size(); }
    [[nodiscard]] ThemeMode mode()       const { return m_mode;          }
    [[nodiscard]] const std::string& name_() const { return name;        }

    void setMode(ThemeMode m) { m_mode = m; }
    void bumpVersion()        { version++;  }

private:
    std::vector<ThemeToken> m_tokens;
};

class ThemeManager {
public:
    static constexpr size_t MAX_THEMES = 32;

    bool addTheme(Theme t) {
        if (m_themes.size() >= MAX_THEMES) return false;
        for (auto& existing : m_themes) if (existing.name == t.name) return false;
        m_themes.push_back(std::move(t));
        return true;
    }

    bool removeTheme(const std::string& name) {
        for (auto it = m_themes.begin(); it != m_themes.end(); ++it) {
            if (it->name == name) {
                if (m_active == &*it) m_active = nullptr;
                m_themes.erase(it);
                return true;
            }
        }
        return false;
    }

    bool setActive(const std::string& name) {
        for (auto& t : m_themes) {
            if (t.name == name) { m_active = &t; return true; }
        }
        return false;
    }

    [[nodiscard]] Theme* active() { return m_active; }

    [[nodiscard]] Theme* find(const std::string& name) {
        for (auto& t : m_themes) if (t.name == name) return &t;
        return nullptr;
    }

    [[nodiscard]] size_t themeCount() const { return m_themes.size(); }
    [[nodiscard]] bool   hasActive()  const { return m_active != nullptr; }

    bool applyMode(ThemeMode m) {
        if (!m_active) return false;
        m_active->setMode(m);
        return true;
    }

private:
    std::vector<Theme> m_themes;
    Theme*             m_active = nullptr;
};

// ============================================================
// S28 — Keyframe Animation Editor
// ============================================================

enum class KeyframeInterpolation : uint8_t { Linear, Step, Bezier, CubicSpline, EaseIn, EaseOut, EaseInOut, Custom };

inline const char* keyframeInterpolationName(KeyframeInterpolation i) {
    switch (i) {
        case KeyframeInterpolation::Linear:      return "Linear";
        case KeyframeInterpolation::Step:        return "Step";
        case KeyframeInterpolation::Bezier:      return "Bezier";
        case KeyframeInterpolation::CubicSpline: return "CubicSpline";
        case KeyframeInterpolation::EaseIn:      return "EaseIn";
        case KeyframeInterpolation::EaseOut:     return "EaseOut";
        case KeyframeInterpolation::EaseInOut:   return "EaseInOut";
        case KeyframeInterpolation::Custom:      return "Custom";
        default:                                 return "Unknown";
    }
}

enum class AnimationTrackType : uint8_t { Position, Rotation, Scale, Opacity, Color, Float, Bool, Event };

inline const char* animationTrackTypeName(AnimationTrackType t) {
    switch (t) {
        case AnimationTrackType::Position: return "Position";
        case AnimationTrackType::Rotation: return "Rotation";
        case AnimationTrackType::Scale:    return "Scale";
        case AnimationTrackType::Opacity:  return "Opacity";
        case AnimationTrackType::Color:    return "Color";
        case AnimationTrackType::Float:    return "Float";
        case AnimationTrackType::Bool:     return "Bool";
        case AnimationTrackType::Event:    return "Event";
        default:                           return "Unknown";
    }
}

struct Keyframe {
    float                  time         = 0.f;
    float                  value        = 0.f;
    KeyframeInterpolation  interpolation = KeyframeInterpolation::Linear;
    bool                   selected     = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setTime(float t)  { time  = t; }
    void setValue(float v) { value = v; }
};

class AnimationTrack {
public:
    explicit AnimationTrack(const std::string& name, AnimationTrackType type = AnimationTrackType::Float)
        : m_name(name), m_type(type) {}

    bool addKeyframe(Keyframe kf) {
        for (auto& existing : m_keyframes) if (existing.time == kf.time) return false;
        m_keyframes.push_back(kf);
        return true;
    }

    bool removeKeyframe(float time) {
        for (auto it = m_keyframes.begin(); it != m_keyframes.end(); ++it) {
            if (it->time == time) { m_keyframes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Keyframe* findKeyframe(float time) {
        for (auto& kf : m_keyframes) if (kf.time == time) return &kf;
        return nullptr;
    }

    [[nodiscard]] size_t keyframeCount() const { return m_keyframes.size(); }

    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0;
        for (auto& kf : m_keyframes) if (kf.selected) c++;
        return c;
    }

    void selectAll()   { for (auto& kf : m_keyframes) kf.select();   }
    void deselectAll() { for (auto& kf : m_keyframes) kf.deselect(); }

    [[nodiscard]] const std::string&    name() const { return m_name; }
    [[nodiscard]] AnimationTrackType    type() const { return m_type; }

    [[nodiscard]] float duration() const {
        float d = 0.f;
        for (auto& kf : m_keyframes) if (kf.time > d) d = kf.time;
        return d;
    }

private:
    std::string            m_name;
    AnimationTrackType     m_type;
    std::vector<Keyframe>  m_keyframes;
};

class KeyframeAnimationEditor {
public:
    static constexpr size_t MAX_TRACKS = 64;

    bool addTrack(AnimationTrack track) {
        if (m_tracks.size() >= MAX_TRACKS) return false;
        for (auto& existing : m_tracks) if (existing.name() == track.name()) return false;
        m_tracks.push_back(std::move(track));
        return true;
    }

    bool removeTrack(const std::string& name) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->name() == name) { m_tracks.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] AnimationTrack* findTrack(const std::string& name) {
        for (auto& t : m_tracks) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] size_t trackCount() const { return m_tracks.size(); }

    [[nodiscard]] float totalDuration() const {
        float d = 0.f;
        for (auto& t : m_tracks) if (t.duration() > d) d = t.duration();
        return d;
    }

    void setPlayhead(float time) { m_playhead = time; }
    [[nodiscard]] float playhead() const { return m_playhead; }

    void play()  { m_playing = true;  }
    void pause() { m_playing = false; }
    void stop()  { m_playing = false; m_playhead = 0.f; }

    [[nodiscard]] bool isPlaying() const { return m_playing; }

    void selectAllKeyframes()   { for (auto& t : m_tracks) t.selectAll();   }
    void deselectAllKeyframes() { for (auto& t : m_tracks) t.deselectAll(); }

private:
    std::vector<AnimationTrack> m_tracks;
    float                       m_playhead = 0.f;
    bool                        m_playing  = false;
};

// ============================================================
// S29 — Curve Editor
// ============================================================

enum class CurveType : uint8_t {
    Linear    = 0,
    Bezier    = 1,
    Hermite   = 2,
    CatmullRom= 3,
    Step      = 4,
    Sine      = 5,
    Cosine    = 6,
    Custom    = 7
};

inline const char* curveTypeName(CurveType t) {
    switch (t) {
        case CurveType::Linear:     return "Linear";
        case CurveType::Bezier:     return "Bezier";
        case CurveType::Hermite:    return "Hermite";
        case CurveType::CatmullRom: return "CatmullRom";
        case CurveType::Step:       return "Step";
        case CurveType::Sine:       return "Sine";
        case CurveType::Cosine:     return "Cosine";
        case CurveType::Custom:     return "Custom";
        default:                    return "Unknown";
    }
}

enum class CurveHandleMode : uint8_t {
    Free     = 0,
    Aligned  = 1,
    Vector   = 2,
    Auto     = 3
};

inline const char* curveHandleModeName(CurveHandleMode m) {
    switch (m) {
        case CurveHandleMode::Free:    return "Free";
        case CurveHandleMode::Aligned: return "Aligned";
        case CurveHandleMode::Vector:  return "Vector";
        case CurveHandleMode::Auto:    return "Auto";
        default:                       return "Unknown";
    }
}

struct CurveControlPoint {
    float           time     = 0.f;
    float           value    = 0.f;
    float           handleL  = 0.f;
    float           handleR  = 0.f;
    CurveHandleMode mode     = CurveHandleMode::Auto;
    bool            selected = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setTime(float t)  { time  = t; }
    void setValue(float v) { value = v; }
};

class Curve {
public:
    explicit Curve(const std::string& name, CurveType type = CurveType::Linear)
        : m_name(name), m_type(type) {}

    bool addPoint(CurveControlPoint cp) {
        for (auto& existing : m_points) if (existing.time == cp.time) return false;
        m_points.push_back(cp);
        return true;
    }

    bool removePoint(float time) {
        for (auto it = m_points.begin(); it != m_points.end(); ++it) {
            if (it->time == time) { m_points.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] CurveControlPoint* findPoint(float time) {
        for (auto& cp : m_points) if (cp.time == time) return &cp;
        return nullptr;
    }

    [[nodiscard]] size_t pointCount()    const { return m_points.size(); }

    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0;
        for (auto& cp : m_points) if (cp.selected) c++;
        return c;
    }

    void selectAll()   { for (auto& cp : m_points) cp.select();   }
    void deselectAll() { for (auto& cp : m_points) cp.deselect(); }

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] CurveType          type() const { return m_type; }

    [[nodiscard]] float duration() const {
        float d = 0.f;
        for (auto& cp : m_points) if (cp.time > d) d = cp.time;
        return d;
    }

private:
    std::string                   m_name;
    CurveType                     m_type;
    std::vector<CurveControlPoint> m_points;
};

class CurveEditorPanel {
public:
    static constexpr size_t MAX_CURVES = 32;

    bool addCurve(Curve curve) {
        if (m_curves.size() >= MAX_CURVES) return false;
        for (auto& existing : m_curves) if (existing.name() == curve.name()) return false;
        m_curves.push_back(std::move(curve));
        return true;
    }

    bool removeCurve(const std::string& name) {
        for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
            if (it->name() == name) {
                if (m_activeCurve == name) m_activeCurve.clear();
                m_curves.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Curve* findCurve(const std::string& name) {
        for (auto& c : m_curves) if (c.name() == name) return &c;
        return nullptr;
    }

    bool setActiveCurve(const std::string& name) {
        for (auto& c : m_curves) {
            if (c.name() == name) { m_activeCurve = name; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::string& activeCurve() const { return m_activeCurve; }
    [[nodiscard]] size_t             curveCount()  const { return m_curves.size(); }
    [[nodiscard]] bool               isLooping()   const { return m_looping; }
    void                             setLooping(bool v) { m_looping = v; }

    void selectAllPoints()   { for (auto& c : m_curves) c.selectAll();   }
    void deselectAllPoints() { for (auto& c : m_curves) c.deselectAll(); }

private:
    std::vector<Curve> m_curves;
    std::string        m_activeCurve;
    bool               m_looping = false;
};

// ── S30 — Gradient Editor ────────────────────────────────────────

enum class GradientType : uint8_t {
    Linear, Radial, Angular, Diamond, Square, Reflected, Conical, Custom
};

inline const char* gradientTypeName(GradientType t) {
    switch (t) {
        case GradientType::Linear:    return "Linear";
        case GradientType::Radial:    return "Radial";
        case GradientType::Angular:   return "Angular";
        case GradientType::Diamond:   return "Diamond";
        case GradientType::Square:    return "Square";
        case GradientType::Reflected: return "Reflected";
        case GradientType::Conical:   return "Conical";
        case GradientType::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class GradientInterpolation : uint8_t {
    Linear, Step, Spline, Constant
};

inline const char* gradientInterpolationName(GradientInterpolation i) {
    switch (i) {
        case GradientInterpolation::Linear:   return "Linear";
        case GradientInterpolation::Step:     return "Step";
        case GradientInterpolation::Spline:   return "Spline";
        case GradientInterpolation::Constant: return "Constant";
    }
    return "Unknown";
}

struct GradientColorStop {
    float                 position      = 0.0f; // [0,1]
    float                 r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    GradientInterpolation interpolation = GradientInterpolation::Linear;
    bool                  selected      = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setPosition(float p) { position = p; }
};

class GradientRamp {
public:
    static constexpr size_t MAX_STOPS = 64;

    explicit GradientRamp(const std::string& name, GradientType type = GradientType::Linear)
        : m_name(name), m_type(type) {}

    [[nodiscard]] bool addStop(const GradientColorStop& s) {
        for (auto& e : m_stops) {
            if (e.position == s.position) return false;
        }
        if (m_stops.size() >= MAX_STOPS) return false;
        m_stops.push_back(s);
        return true;
    }

    [[nodiscard]] bool removeStop(float position) {
        for (auto it = m_stops.begin(); it != m_stops.end(); ++it) {
            if (it->position == position) { m_stops.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] GradientColorStop* findStop(float position) {
        for (auto& s : m_stops)
            if (s.position == position) return &s;
        return nullptr;
    }

    void selectAll()   { for (auto& s : m_stops) s.select();   }
    void deselectAll() { for (auto& s : m_stops) s.deselect(); }

    [[nodiscard]] size_t stopCount()     const { return m_stops.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& s : m_stops) if (s.selected) ++c; return c;
    }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] GradientType       type() const { return m_type; }

private:
    std::string               m_name;
    GradientType              m_type;
    std::vector<GradientColorStop> m_stops;
};

class GradientEditorPanel {
public:
    static constexpr size_t MAX_RAMPS = 32;

    [[nodiscard]] bool addRamp(const GradientRamp& ramp) {
        for (auto& r : m_ramps)
            if (r.name() == ramp.name()) return false;
        if (m_ramps.size() >= MAX_RAMPS) return false;
        m_ramps.push_back(ramp);
        return true;
    }

    [[nodiscard]] bool removeRamp(const std::string& name) {
        for (auto it = m_ramps.begin(); it != m_ramps.end(); ++it) {
            if (it->name() == name) {
                if (m_activeRamp == name) m_activeRamp.clear();
                m_ramps.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] GradientRamp* findRamp(const std::string& name) {
        for (auto& r : m_ramps)
            if (r.name() == name) return &r;
        return nullptr;
    }

    [[nodiscard]] bool setActiveRamp(const std::string& name) {
        for (auto& r : m_ramps) {
            if (r.name() == name) { m_activeRamp = name; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::string& activeRamp()  const { return m_activeRamp; }
    [[nodiscard]] size_t             rampCount()   const { return m_ramps.size(); }
    [[nodiscard]] bool               isSymmetric() const { return m_symmetric; }
    void                             setSymmetric(bool v) { m_symmetric = v; }

    void selectAllStops()   { for (auto& r : m_ramps) r.selectAll();   }
    void deselectAllStops() { for (auto& r : m_ramps) r.deselectAll(); }

private:
    std::vector<GradientRamp> m_ramps;
    std::string               m_activeRamp;
    bool                      m_symmetric = false;
};

// ── S31 — Timeline Editor ────────────────────────────────────────

enum class TimelineEventType : uint8_t {
    Keyframe, Marker, Clip, Trigger, Label, Camera, Audio, Custom
};

inline const char* timelineEventTypeName(TimelineEventType t) {
    switch (t) {
        case TimelineEventType::Keyframe: return "Keyframe";
        case TimelineEventType::Marker:   return "Marker";
        case TimelineEventType::Clip:     return "Clip";
        case TimelineEventType::Trigger:  return "Trigger";
        case TimelineEventType::Label:    return "Label";
        case TimelineEventType::Camera:   return "Camera";
        case TimelineEventType::Audio:    return "Audio";
        case TimelineEventType::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class TimelineTrackKind : uint8_t {
    Animation, Audio, Event, Camera
};

inline const char* timelineTrackKindName(TimelineTrackKind k) {
    switch (k) {
        case TimelineTrackKind::Animation: return "Animation";
        case TimelineTrackKind::Audio:     return "Audio";
        case TimelineTrackKind::Event:     return "Event";
        case TimelineTrackKind::Camera:    return "Camera";
    }
    return "Unknown";
}

struct TimelineEvent {
    std::string       id;
    TimelineEventType type     = TimelineEventType::Keyframe;
    float             time     = 0.0f;
    float             duration = 0.0f;
    bool              selected = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setTime(float t)     { time = t; }
    void setDuration(float d) { duration = d; }
};

class TimelineTrack {
public:
    static constexpr size_t MAX_EVENTS = 128;

    explicit TimelineTrack(const std::string& name, TimelineTrackKind kind = TimelineTrackKind::Animation)
        : m_name(name), m_kind(kind) {}

    [[nodiscard]] bool addEvent(const TimelineEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        if (m_events.size() >= MAX_EVENTS) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] TimelineEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void selectAll()   { for (auto& e : m_events) e.select();   }
    void deselectAll() { for (auto& e : m_events) e.deselect(); }

    [[nodiscard]] size_t eventCount()    const { return m_events.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.selected) ++c; return c;
    }
    [[nodiscard]] float duration() const {
        float d = 0.0f;
        for (auto& e : m_events) { float end = e.time + e.duration; if (end > d) d = end; }
        return d;
    }
    [[nodiscard]] bool                muted()  const { return m_muted; }
    void                              setMuted(bool v) { m_muted = v; }
    [[nodiscard]] const std::string&  name()   const { return m_name; }
    [[nodiscard]] TimelineTrackKind   kind()   const { return m_kind; }

private:
    std::string              m_name;
    TimelineTrackKind        m_kind;
    std::vector<TimelineEvent> m_events;
    bool                     m_muted = false;
};

class TimelineEditorPanel {
public:
    static constexpr size_t MAX_TRACKS = 64;

    [[nodiscard]] bool addTrack(const TimelineTrack& track) {
        for (auto& t : m_tracks) if (t.name() == track.name()) return false;
        if (m_tracks.size() >= MAX_TRACKS) return false;
        m_tracks.push_back(track);
        return true;
    }

    [[nodiscard]] bool removeTrack(const std::string& name) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->name() == name) {
                if (m_activeTrack == name) m_activeTrack.clear();
                m_tracks.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TimelineTrack* findTrack(const std::string& name) {
        for (auto& t : m_tracks) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] bool setActiveTrack(const std::string& name) {
        for (auto& t : m_tracks) {
            if (t.name() == name) { m_activeTrack = name; return true; }
        }
        return false;
    }

    void setPlayhead(float t)  { m_playhead = t; }
    void play()  { m_playing = true;  }
    void pause() { m_playing = false; }
    void stop()  { m_playing = false; m_playhead = 0.0f; }

    [[nodiscard]] float              playhead()     const { return m_playhead; }
    [[nodiscard]] bool               isPlaying()    const { return m_playing; }
    [[nodiscard]] const std::string& activeTrack()  const { return m_activeTrack; }
    [[nodiscard]] size_t             trackCount()   const { return m_tracks.size(); }

    void selectAllEvents()   { for (auto& t : m_tracks) t.selectAll();   }
    void deselectAllEvents() { for (auto& t : m_tracks) t.deselectAll(); }

private:
    std::vector<TimelineTrack> m_tracks;
    std::string                m_activeTrack;
    float                      m_playhead = 0.0f;
    bool                       m_playing  = false;
};

// ── S32 — Particle Effect Editor ─────────────────────────────────

enum class ParticleEmitterShape : uint8_t {
    Point, Circle, Rectangle, Cone, Sphere, Ring, Line, Custom
};

inline const char* particleEmitterShapeName(ParticleEmitterShape s) {
    switch (s) {
        case ParticleEmitterShape::Point:     return "Point";
        case ParticleEmitterShape::Circle:    return "Circle";
        case ParticleEmitterShape::Rectangle: return "Rectangle";
        case ParticleEmitterShape::Cone:      return "Cone";
        case ParticleEmitterShape::Sphere:    return "Sphere";
        case ParticleEmitterShape::Ring:      return "Ring";
        case ParticleEmitterShape::Line:      return "Line";
        case ParticleEmitterShape::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class ParticleBlendMode : uint8_t {
    Additive, Alpha, Multiply, Screen
};

inline const char* particleBlendModeName(ParticleBlendMode b) {
    switch (b) {
        case ParticleBlendMode::Additive: return "Additive";
        case ParticleBlendMode::Alpha:    return "Alpha";
        case ParticleBlendMode::Multiply: return "Multiply";
        case ParticleBlendMode::Screen:   return "Screen";
    }
    return "Unknown";
}

struct ParticleEmitterConfig {
    std::string          id;
    ParticleEmitterShape shape       = ParticleEmitterShape::Point;
    ParticleBlendMode    blendMode   = ParticleBlendMode::Additive;
    float                emitRate    = 10.0f;
    float                lifetime    = 1.0f;
    float                speed       = 1.0f;
    float                size        = 1.0f;
    bool                 looping     = true;

    void setEmitRate(float r) { emitRate = r; }
    void setLifetime(float l) { lifetime = l; }
    void setSpeed(float s)    { speed = s; }
    void setSize(float s)     { size = s; }

    [[nodiscard]] bool isValid() const {
        return emitRate > 0.0f && lifetime > 0.0f && size > 0.0f;
    }
};

class ParticleEffectLayer {
public:
    static constexpr size_t MAX_EMITTERS = 64;

    explicit ParticleEffectLayer(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEmitter(const ParticleEmitterConfig& cfg) {
        for (auto& e : m_emitters) if (e.id == cfg.id) return false;
        if (m_emitters.size() >= MAX_EMITTERS) return false;
        m_emitters.push_back(cfg);
        return true;
    }

    [[nodiscard]] bool removeEmitter(const std::string& id) {
        for (auto it = m_emitters.begin(); it != m_emitters.end(); ++it) {
            if (it->id == id) { m_emitters.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ParticleEmitterConfig* findEmitter(const std::string& id) {
        for (auto& e : m_emitters) if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] size_t emitterCount()   const { return m_emitters.size(); }
    [[nodiscard]] bool   visible()        const { return m_visible; }
    void                 setVisible(bool v)     { m_visible = v; }
    [[nodiscard]] const std::string& name() const { return m_name; }

private:
    std::string                      m_name;
    std::vector<ParticleEmitterConfig> m_emitters;
    bool                             m_visible = true;
};

class ParticleEffectEditor {
public:
    static constexpr size_t MAX_LAYERS = 32;

    [[nodiscard]] bool addLayer(const ParticleEffectLayer& layer) {
        for (auto& l : m_layers) if (l.name() == layer.name()) return false;
        if (m_layers.size() >= MAX_LAYERS) return false;
        m_layers.push_back(layer);
        return true;
    }

    [[nodiscard]] bool removeLayer(const std::string& name) {
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (it->name() == name) {
                if (m_activeLayer == name) m_activeLayer.clear();
                m_layers.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ParticleEffectLayer* findLayer(const std::string& name) {
        for (auto& l : m_layers) if (l.name() == name) return &l;
        return nullptr;
    }

    [[nodiscard]] bool setActiveLayer(const std::string& name) {
        for (auto& l : m_layers) {
            if (l.name() == name) { m_activeLayer = name; return true; }
        }
        return false;
    }

    void preview()  { m_previewing = true;  }
    void stopPreview() { m_previewing = false; }

    [[nodiscard]] bool               isPreviewing() const { return m_previewing; }
    [[nodiscard]] const std::string& activeLayer()  const { return m_activeLayer; }
    [[nodiscard]] size_t             layerCount()   const { return m_layers.size(); }

    [[nodiscard]] size_t totalEmitterCount() const {
        size_t c = 0;
        for (auto& l : m_layers) c += l.emitterCount();
        return c;
    }

private:
    std::vector<ParticleEffectLayer> m_layers;
    std::string                      m_activeLayer;
    bool                             m_previewing = false;
};

// ── S33 — Shader Graph Editor ─────────────────────────────────────

enum class ShaderNodeType : uint8_t {
    Input, Output, Math, Texture, Color, Vector, Blend, Custom
};

inline const char* shaderNodeTypeName(ShaderNodeType t) {
    switch (t) {
        case ShaderNodeType::Input:   return "Input";
        case ShaderNodeType::Output:  return "Output";
        case ShaderNodeType::Math:    return "Math";
        case ShaderNodeType::Texture: return "Texture";
        case ShaderNodeType::Color:   return "Color";
        case ShaderNodeType::Vector:  return "Vector";
        case ShaderNodeType::Blend:   return "Blend";
        case ShaderNodeType::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class ShaderPortKind : uint8_t {
    Float, Vector2, Vector3, Vector4
};

inline const char* shaderPortKindName(ShaderPortKind k) {
    switch (k) {
        case ShaderPortKind::Float:   return "Float";
        case ShaderPortKind::Vector2: return "Vector2";
        case ShaderPortKind::Vector3: return "Vector3";
        case ShaderPortKind::Vector4: return "Vector4";
    }
    return "Unknown";
}

struct ShaderNode {
    std::string    id;
    ShaderNodeType type     = ShaderNodeType::Math;
    float          posX     = 0.0f;
    float          posY     = 0.0f;
    bool           selected = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setPosition(float x, float y) { posX = x; posY = y; }
};

struct ShaderGraphEdge {
    std::string  id;
    std::string  fromNode;
    std::string  toNode;
    ShaderPortKind portKind = ShaderPortKind::Float;
};

class ShaderGraphEditor {
public:
    static constexpr size_t MAX_NODES = 256;
    static constexpr size_t MAX_EDGES = 512;

    [[nodiscard]] bool addNode(const ShaderNode& node) {
        for (auto& n : m_nodes) if (n.id == node.id) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        m_nodes.push_back(node);
        return true;
    }

    [[nodiscard]] bool removeNode(const std::string& id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) {
                // remove edges connected to this node
                m_edges.erase(
                    std::remove_if(m_edges.begin(), m_edges.end(),
                        [&id](const ShaderGraphEdge& e) {
                            return e.fromNode == id || e.toNode == id;
                        }),
                    m_edges.end());
                if (m_activeNode == id) m_activeNode.clear();
                m_nodes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ShaderNode* findNode(const std::string& id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] bool addEdge(const ShaderGraphEdge& edge) {
        for (auto& e : m_edges) if (e.id == edge.id) return false;
        if (m_edges.size() >= MAX_EDGES) return false;
        // both endpoints must exist
        if (!findNode(edge.fromNode) || !findNode(edge.toNode)) return false;
        m_edges.push_back(edge);
        return true;
    }

    [[nodiscard]] bool removeEdge(const std::string& id) {
        for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
            if (it->id == id) { m_edges.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] bool setActiveNode(const std::string& id) {
        for (auto& n : m_nodes) {
            if (n.id == id) { m_activeNode = id; return true; }
        }
        return false;
    }

    void selectAll()   { for (auto& n : m_nodes) n.select();   }
    void deselectAll() { for (auto& n : m_nodes) n.deselect(); }

    [[nodiscard]] size_t nodeCount()     const { return m_nodes.size(); }
    [[nodiscard]] size_t edgeCount()     const { return m_edges.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (n.selected) ++c; return c;
    }
    [[nodiscard]] const std::string& activeNode() const { return m_activeNode; }

private:
    std::vector<ShaderNode>      m_nodes;
    std::vector<ShaderGraphEdge> m_edges;
    std::string                  m_activeNode;
};

// ── S34 — Material Editor ─────────────────────────────────────────

enum class MaterialShadingModel : uint8_t {
    Unlit, Lambert, Phong, BlinnPhong, PBR, Toon, Subsurface, Custom
};

inline const char* materialShadingModelName(MaterialShadingModel m) {
    switch (m) {
        case MaterialShadingModel::Unlit:      return "Unlit";
        case MaterialShadingModel::Lambert:    return "Lambert";
        case MaterialShadingModel::Phong:      return "Phong";
        case MaterialShadingModel::BlinnPhong: return "BlinnPhong";
        case MaterialShadingModel::PBR:        return "PBR";
        case MaterialShadingModel::Toon:       return "Toon";
        case MaterialShadingModel::Subsurface: return "Subsurface";
        case MaterialShadingModel::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class MaterialBlendMode : uint8_t {
    Opaque, Masked, Translucent, Additive
};

inline const char* materialBlendModeName(MaterialBlendMode b) {
    switch (b) {
        case MaterialBlendMode::Opaque:       return "Opaque";
        case MaterialBlendMode::Masked:       return "Masked";
        case MaterialBlendMode::Translucent:  return "Translucent";
        case MaterialBlendMode::Additive:     return "Additive";
    }
    return "Unknown";
}

struct MaterialParameter {
    std::string name;
    float       value = 0.0f;
    bool        exposed = false;

    void expose()  { exposed = true;  }
    void hide()    { exposed = false; }
};

class MaterialAsset {
public:
    explicit MaterialAsset(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addParameter(const MaterialParameter& p) {
        for (auto& existing : m_params) if (existing.name == p.name) return false;
        m_params.push_back(p);
        return true;
    }

    [[nodiscard]] bool removeParameter(const std::string& name) {
        for (auto it = m_params.begin(); it != m_params.end(); ++it) {
            if (it->name == name) { m_params.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] MaterialParameter* findParameter(const std::string& name) {
        for (auto& p : m_params) if (p.name == name) return &p;
        return nullptr;
    }

    void setShadingModel(MaterialShadingModel m) { m_shadingModel = m; }
    void setBlendMode(MaterialBlendMode b)        { m_blendMode = b; }
    void setDirty(bool d)                         { m_dirty = d; }

    [[nodiscard]] MaterialShadingModel shadingModel() const { return m_shadingModel; }
    [[nodiscard]] MaterialBlendMode    blendMode()    const { return m_blendMode; }
    [[nodiscard]] bool                 isDirty()      const { return m_dirty; }
    [[nodiscard]] size_t               paramCount()   const { return m_params.size(); }
    [[nodiscard]] size_t               exposedParamCount() const {
        size_t c = 0; for (auto& p : m_params) if (p.exposed) ++c; return c;
    }
    [[nodiscard]] const std::string&   name()         const { return m_name; }

private:
    std::string                   m_name;
    MaterialShadingModel          m_shadingModel = MaterialShadingModel::PBR;
    MaterialBlendMode             m_blendMode    = MaterialBlendMode::Opaque;
    bool                          m_dirty        = false;
    std::vector<MaterialParameter> m_params;
};

class MaterialEditor {
public:
    static constexpr size_t MAX_ASSETS = 128;

    [[nodiscard]] bool addAsset(const MaterialAsset& asset) {
        for (auto& a : m_assets) if (a.name() == asset.name()) return false;
        if (m_assets.size() >= MAX_ASSETS) return false;
        m_assets.push_back(asset);
        return true;
    }

    [[nodiscard]] bool removeAsset(const std::string& name) {
        for (auto it = m_assets.begin(); it != m_assets.end(); ++it) {
            if (it->name() == name) {
                if (m_activeAsset == name) m_activeAsset.clear();
                m_assets.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] MaterialAsset* findAsset(const std::string& name) {
        for (auto& a : m_assets) if (a.name() == name) return &a;
        return nullptr;
    }

    [[nodiscard]] bool setActiveAsset(const std::string& name) {
        for (auto& a : m_assets) {
            if (a.name() == name) { m_activeAsset = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t            assetCount()   const { return m_assets.size(); }
    [[nodiscard]] const std::string& activeAsset() const { return m_activeAsset; }
    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0; for (auto& a : m_assets) if (a.isDirty()) ++c; return c;
    }

private:
    std::vector<MaterialAsset> m_assets;
    std::string                m_activeAsset;
};

// ── S35 — Texture Editor ──────────────────────────────────────────

enum class TextureFormat : uint8_t {
    R8, RG8, RGB8, RGBA8, R16F, RG16F, RGB16F, RGBA16F
};

inline const char* textureFormatName(TextureFormat f) {
    switch (f) {
        case TextureFormat::R8:      return "R8";
        case TextureFormat::RG8:     return "RG8";
        case TextureFormat::RGB8:    return "RGB8";
        case TextureFormat::RGBA8:   return "RGBA8";
        case TextureFormat::R16F:    return "R16F";
        case TextureFormat::RG16F:   return "RG16F";
        case TextureFormat::RGB16F:  return "RGB16F";
        case TextureFormat::RGBA16F: return "RGBA16F";
    }
    return "Unknown";
}

enum class TextureFilter : uint8_t {
    Nearest, Linear, NearestMipmapNearest, LinearMipmapLinear
};

inline const char* textureFilterName(TextureFilter f) {
    switch (f) {
        case TextureFilter::Nearest:               return "Nearest";
        case TextureFilter::Linear:                return "Linear";
        case TextureFilter::NearestMipmapNearest:  return "NearestMipmapNearest";
        case TextureFilter::LinearMipmapLinear:    return "LinearMipmapLinear";
    }
    return "Unknown";
}

enum class TextureWrapMode : uint8_t {
    Repeat, MirroredRepeat, ClampToEdge, ClampToBorder
};

inline const char* textureWrapModeName(TextureWrapMode w) {
    switch (w) {
        case TextureWrapMode::Repeat:          return "Repeat";
        case TextureWrapMode::MirroredRepeat:  return "MirroredRepeat";
        case TextureWrapMode::ClampToEdge:     return "ClampToEdge";
        case TextureWrapMode::ClampToBorder:   return "ClampToBorder";
    }
    return "Unknown";
}

class TextureAsset {
public:
    explicit TextureAsset(const std::string& name,
                          uint32_t width  = 1,
                          uint32_t height = 1)
        : m_name(name), m_width(width), m_height(height) {}

    void setFormat(TextureFormat f)       { m_format = f; }
    void setFilter(TextureFilter f)       { m_filter = f; }
    void setWrapMode(TextureWrapMode w)   { m_wrap   = w; }
    void setMipmapsEnabled(bool v)        { m_mipmaps = v; }
    void setDirty(bool d)                 { m_dirty   = d; }
    void resize(uint32_t w, uint32_t h)   { m_width = w; m_height = h; }

    [[nodiscard]] TextureFormat   format()        const { return m_format; }
    [[nodiscard]] TextureFilter   filter()        const { return m_filter; }
    [[nodiscard]] TextureWrapMode wrapMode()      const { return m_wrap;   }
    [[nodiscard]] bool            mipmapsEnabled()const { return m_mipmaps; }
    [[nodiscard]] bool            isDirty()       const { return m_dirty;   }
    [[nodiscard]] uint32_t        width()         const { return m_width;   }
    [[nodiscard]] uint32_t        height()        const { return m_height;  }
    [[nodiscard]] uint64_t        pixelCount()    const { return static_cast<uint64_t>(m_width) * m_height; }
    [[nodiscard]] const std::string& name()       const { return m_name;    }

    [[nodiscard]] bool isHDR() const {
        return m_format == TextureFormat::R16F   ||
               m_format == TextureFormat::RG16F  ||
               m_format == TextureFormat::RGB16F ||
               m_format == TextureFormat::RGBA16F;
    }

private:
    std::string       m_name;
    uint32_t          m_width   = 1;
    uint32_t          m_height  = 1;
    TextureFormat     m_format  = TextureFormat::RGBA8;
    TextureFilter     m_filter  = TextureFilter::Linear;
    TextureWrapMode   m_wrap    = TextureWrapMode::Repeat;
    bool              m_mipmaps = false;
    bool              m_dirty   = false;
};

class TextureEditor {
public:
    static constexpr size_t MAX_TEXTURES = 256;

    [[nodiscard]] bool addTexture(const TextureAsset& tex) {
        for (auto& t : m_textures) if (t.name() == tex.name()) return false;
        if (m_textures.size() >= MAX_TEXTURES) return false;
        m_textures.push_back(tex);
        return true;
    }

    [[nodiscard]] bool removeTexture(const std::string& name) {
        for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
            if (it->name() == name) {
                if (m_activeTexture == name) m_activeTexture.clear();
                m_textures.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TextureAsset* findTexture(const std::string& name) {
        for (auto& t : m_textures) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] bool setActiveTexture(const std::string& name) {
        for (auto& t : m_textures) {
            if (t.name() == name) { m_activeTexture = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t textureCount() const { return m_textures.size(); }
    [[nodiscard]] const std::string& activeTexture() const { return m_activeTexture; }
    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0; for (auto& t : m_textures) if (t.isDirty()) ++c; return c;
    }
    [[nodiscard]] size_t hdrCount() const {
        size_t c = 0; for (auto& t : m_textures) if (t.isHDR()) ++c; return c;
    }
    [[nodiscard]] size_t mipmapCount() const {
        size_t c = 0; for (auto& t : m_textures) if (t.mipmapsEnabled()) ++c; return c;
    }

private:
    std::vector<TextureAsset> m_textures;
    std::string               m_activeTexture;
};

// ── S36 — Font Editor ─────────────────────────────────────────────

enum class FontStyle : uint8_t {
    Normal, Italic, Oblique, Inherit
};

inline const char* fontStyleName(FontStyle s) {
    switch (s) {
        case FontStyle::Normal:  return "Normal";
        case FontStyle::Italic:  return "Italic";
        case FontStyle::Oblique: return "Oblique";
        case FontStyle::Inherit: return "Inherit";
    }
    return "Unknown";
}

enum class FontWeight : uint8_t {
    Thin, ExtraLight, Light, Regular, Medium, Bold
};

inline const char* fontWeightName(FontWeight w) {
    switch (w) {
        case FontWeight::Thin:       return "Thin";
        case FontWeight::ExtraLight: return "ExtraLight";
        case FontWeight::Light:      return "Light";
        case FontWeight::Regular:    return "Regular";
        case FontWeight::Medium:     return "Medium";
        case FontWeight::Bold:       return "Bold";
    }
    return "Unknown";
}

enum class FontVariant : uint8_t {
    Normal, SmallCaps, AllSmallCaps, PetiteCaps
};

inline const char* fontVariantName(FontVariant v) {
    switch (v) {
        case FontVariant::Normal:       return "Normal";
        case FontVariant::SmallCaps:    return "SmallCaps";
        case FontVariant::AllSmallCaps: return "AllSmallCaps";
        case FontVariant::PetiteCaps:   return "PetiteCaps";
    }
    return "Unknown";
}

class FontAsset {
public:
    explicit FontAsset(const std::string& family,
                       float              size = 12.0f)
        : m_family(family), m_size(size) {}

    void setStyle(FontStyle s)    { m_style   = s; }
    void setWeight(FontWeight w)  { m_weight  = w; }
    void setVariant(FontVariant v){ m_variant = v; }
    void setSize(float s)         { m_size    = s; }
    void setLineHeight(float lh)  { m_lineHeight = lh; }
    void setLetterSpacing(float ls){ m_letterSpacing = ls; }
    void setEmbedded(bool e)      { m_embedded = e; }
    void setDirty(bool d)         { m_dirty    = d; }

    [[nodiscard]] FontStyle   style()         const { return m_style;         }
    [[nodiscard]] FontWeight  weight()        const { return m_weight;        }
    [[nodiscard]] FontVariant variant()       const { return m_variant;       }
    [[nodiscard]] float       size()          const { return m_size;          }
    [[nodiscard]] float       lineHeight()    const { return m_lineHeight;    }
    [[nodiscard]] float       letterSpacing() const { return m_letterSpacing; }
    [[nodiscard]] bool        isEmbedded()    const { return m_embedded;      }
    [[nodiscard]] bool        isDirty()       const { return m_dirty;         }
    [[nodiscard]] const std::string& family() const { return m_family;        }

    [[nodiscard]] bool isBold()   const { return m_weight == FontWeight::Bold || m_weight == FontWeight::Medium; }
    [[nodiscard]] bool isItalic() const { return m_style  == FontStyle::Italic || m_style == FontStyle::Oblique; }

private:
    std::string m_family;
    float       m_size          = 12.0f;
    float       m_lineHeight    = 1.2f;
    float       m_letterSpacing = 0.0f;
    FontStyle   m_style         = FontStyle::Normal;
    FontWeight  m_weight        = FontWeight::Regular;
    FontVariant m_variant       = FontVariant::Normal;
    bool        m_embedded      = false;
    bool        m_dirty         = false;
};

class FontEditor {
public:
    static constexpr size_t MAX_FONTS = 128;

    [[nodiscard]] bool addFont(const FontAsset& font) {
        for (auto& f : m_fonts) if (f.family() == font.family()) return false;
        if (m_fonts.size() >= MAX_FONTS) return false;
        m_fonts.push_back(font);
        return true;
    }

    [[nodiscard]] bool removeFont(const std::string& family) {
        for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it) {
            if (it->family() == family) {
                if (m_activeFont == family) m_activeFont.clear();
                m_fonts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] FontAsset* findFont(const std::string& family) {
        for (auto& f : m_fonts) if (f.family() == family) return &f;
        return nullptr;
    }

    [[nodiscard]] bool setActiveFont(const std::string& family) {
        for (auto& f : m_fonts) {
            if (f.family() == family) { m_activeFont = family; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t fontCount()     const { return m_fonts.size(); }
    [[nodiscard]] const std::string& activeFont() const { return m_activeFont; }

    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isDirty()) ++c; return c;
    }
    [[nodiscard]] size_t embeddedCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isEmbedded()) ++c; return c;
    }
    [[nodiscard]] size_t boldCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isBold()) ++c; return c;
    }
    [[nodiscard]] size_t italicCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isItalic()) ++c; return c;
    }

private:
    std::vector<FontAsset> m_fonts;
    std::string            m_activeFont;
};

// ── S37 — Icon Editor ─────────────────────────────────────────────

enum class IconSize : uint8_t {
    XSmall, Small, Medium, Large, XLarge
};

inline const char* iconSizeName(IconSize s) {
    switch (s) {
        case IconSize::XSmall: return "XSmall";
        case IconSize::Small:  return "Small";
        case IconSize::Medium: return "Medium";
        case IconSize::Large:  return "Large";
        case IconSize::XLarge: return "XLarge";
    }
    return "Unknown";
}

enum class IconTheme : uint8_t {
    Light, Dark, HighContrast, Monochrome
};

inline const char* iconThemeName(IconTheme t) {
    switch (t) {
        case IconTheme::Light:        return "Light";
        case IconTheme::Dark:         return "Dark";
        case IconTheme::HighContrast: return "HighContrast";
        case IconTheme::Monochrome:   return "Monochrome";
    }
    return "Unknown";
}

enum class IconState : uint8_t {
    Normal, Hover, Pressed, Disabled, Selected
};

inline const char* iconStateName(IconState s) {
    switch (s) {
        case IconState::Normal:   return "Normal";
        case IconState::Hover:    return "Hover";
        case IconState::Pressed:  return "Pressed";
        case IconState::Disabled: return "Disabled";
        case IconState::Selected: return "Selected";
    }
    return "Unknown";
}

class IconAsset {
public:
    explicit IconAsset(const std::string& name,
                       IconSize           size = IconSize::Medium)
        : m_name(name), m_size(size) {}

    void setTheme(IconTheme t)  { m_theme = t; }
    void setState(IconState s)  { m_state = s; }
    void setSize(IconSize s)    { m_size  = s; }
    void setScalable(bool v)    { m_scalable = v; }
    void setDirty(bool v)       { m_dirty    = v; }
    void setPixelDensity(float f){ m_pixelDensity = f; }

    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] IconSize           size()          const { return m_size;         }
    [[nodiscard]] IconTheme          theme()         const { return m_theme;        }
    [[nodiscard]] IconState          state()         const { return m_state;        }
    [[nodiscard]] bool               isScalable()   const { return m_scalable;     }
    [[nodiscard]] bool               isDirty()      const { return m_dirty;        }
    [[nodiscard]] float              pixelDensity() const { return m_pixelDensity; }

    [[nodiscard]] bool isDisabled()  const { return m_state == IconState::Disabled; }
    [[nodiscard]] bool isSelected()  const { return m_state == IconState::Selected; }
    [[nodiscard]] bool isHighDPI()   const { return m_pixelDensity >= 2.0f; }

private:
    std::string m_name;
    IconSize    m_size         = IconSize::Medium;
    IconTheme   m_theme        = IconTheme::Light;
    IconState   m_state        = IconState::Normal;
    float       m_pixelDensity = 1.0f;
    bool        m_scalable     = false;
    bool        m_dirty        = false;
};

class IconEditor {
public:
    static constexpr size_t MAX_ICONS = 256;

    [[nodiscard]] bool addIcon(const IconAsset& icon) {
        for (auto& i : m_icons) if (i.name() == icon.name()) return false;
        if (m_icons.size() >= MAX_ICONS) return false;
        m_icons.push_back(icon);
        return true;
    }

    [[nodiscard]] bool removeIcon(const std::string& name) {
        for (auto it = m_icons.begin(); it != m_icons.end(); ++it) {
            if (it->name() == name) {
                if (m_activeIcon == name) m_activeIcon.clear();
                m_icons.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] IconAsset* findIcon(const std::string& name) {
        for (auto& i : m_icons) if (i.name() == name) return &i;
        return nullptr;
    }

    [[nodiscard]] bool setActiveIcon(const std::string& name) {
        for (auto& i : m_icons) {
            if (i.name() == name) { m_activeIcon = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t iconCount()      const { return m_icons.size(); }
    [[nodiscard]] const std::string& activeIcon() const { return m_activeIcon; }

    [[nodiscard]] size_t dirtyCount()    const {
        size_t c = 0; for (auto& i : m_icons) if (i.isDirty())    ++c; return c;
    }
    [[nodiscard]] size_t scalableCount() const {
        size_t c = 0; for (auto& i : m_icons) if (i.isScalable()) ++c; return c;
    }
    [[nodiscard]] size_t disabledCount() const {
        size_t c = 0; for (auto& i : m_icons) if (i.isDisabled()) ++c; return c;
    }
    [[nodiscard]] size_t highDPICount()  const {
        size_t c = 0; for (auto& i : m_icons) if (i.isHighDPI())  ++c; return c;
    }
    [[nodiscard]] size_t countByTheme(IconTheme t) const {
        size_t c = 0; for (auto& i : m_icons) if (i.theme() == t) ++c; return c;
    }
    [[nodiscard]] size_t countBySize(IconSize s) const {
        size_t c = 0; for (auto& i : m_icons) if (i.size() == s)  ++c; return c;
    }

private:
    std::vector<IconAsset> m_icons;
    std::string            m_activeIcon;
};

// ── S38 — Sprite Editor ───────────────────────────────────────────

enum class SpriteOrigin : uint8_t {
    TopLeft, TopCenter, Center, BottomLeft, BottomCenter
};

inline const char* spriteOriginName(SpriteOrigin o) {
    switch (o) {
        case SpriteOrigin::TopLeft:      return "TopLeft";
        case SpriteOrigin::TopCenter:    return "TopCenter";
        case SpriteOrigin::Center:       return "Center";
        case SpriteOrigin::BottomLeft:   return "BottomLeft";
        case SpriteOrigin::BottomCenter: return "BottomCenter";
    }
    return "Unknown";
}

enum class SpriteBlendMode : uint8_t {
    Normal, Additive, Multiply, Screen, Overlay
};

inline const char* spriteBlendModeName(SpriteBlendMode b) {
    switch (b) {
        case SpriteBlendMode::Normal:   return "Normal";
        case SpriteBlendMode::Additive: return "Additive";
        case SpriteBlendMode::Multiply: return "Multiply";
        case SpriteBlendMode::Screen:   return "Screen";
        case SpriteBlendMode::Overlay:  return "Overlay";
    }
    return "Unknown";
}

enum class SpriteAnimState : uint8_t {
    Idle, Playing, Paused, Stopped, Finished
};

inline const char* spriteAnimStateName(SpriteAnimState s) {
    switch (s) {
        case SpriteAnimState::Idle:     return "Idle";
        case SpriteAnimState::Playing:  return "Playing";
        case SpriteAnimState::Paused:   return "Paused";
        case SpriteAnimState::Stopped:  return "Stopped";
        case SpriteAnimState::Finished: return "Finished";
    }
    return "Unknown";
}

class SpriteAsset {
public:
    explicit SpriteAsset(const std::string& name,
                         uint32_t width  = 32,
                         uint32_t height = 32)
        : m_name(name), m_width(width), m_height(height) {}

    void setOrigin(SpriteOrigin o)      { m_origin    = o; }
    void setBlendMode(SpriteBlendMode b){ m_blendMode = b; }
    void setAnimState(SpriteAnimState s){ m_animState = s; }
    void setFrameCount(uint32_t n)      { m_frameCount = n; }
    void setFrameRate(float fps)        { m_frameRate  = fps; }
    void setLooping(bool v)             { m_looping    = v; }
    void setFlippedH(bool v)            { m_flippedH   = v; }
    void setFlippedV(bool v)            { m_flippedV   = v; }
    void setDirty(bool v)               { m_dirty      = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] uint32_t           width()      const { return m_width;      }
    [[nodiscard]] uint32_t           height()     const { return m_height;     }
    [[nodiscard]] SpriteOrigin       origin()     const { return m_origin;     }
    [[nodiscard]] SpriteBlendMode    blendMode()  const { return m_blendMode;  }
    [[nodiscard]] SpriteAnimState    animState()  const { return m_animState;  }
    [[nodiscard]] uint32_t           frameCount() const { return m_frameCount; }
    [[nodiscard]] float              frameRate()  const { return m_frameRate;  }
    [[nodiscard]] bool               isLooping()  const { return m_looping;    }
    [[nodiscard]] bool               isFlippedH() const { return m_flippedH;   }
    [[nodiscard]] bool               isFlippedV() const { return m_flippedV;   }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isAnimated()    const { return m_frameCount > 1; }
    [[nodiscard]] bool isPlaying()     const { return m_animState == SpriteAnimState::Playing; }
    [[nodiscard]] bool isPaused()      const { return m_animState == SpriteAnimState::Paused;  }
    [[nodiscard]] bool isFinished()    const { return m_animState == SpriteAnimState::Finished;}
    [[nodiscard]] uint32_t area()      const { return m_width * m_height; }

private:
    std::string      m_name;
    uint32_t         m_width      = 32;
    uint32_t         m_height     = 32;
    SpriteOrigin     m_origin     = SpriteOrigin::Center;
    SpriteBlendMode  m_blendMode  = SpriteBlendMode::Normal;
    SpriteAnimState  m_animState  = SpriteAnimState::Idle;
    uint32_t         m_frameCount = 1;
    float            m_frameRate  = 24.0f;
    bool             m_looping    = false;
    bool             m_flippedH   = false;
    bool             m_flippedV   = false;
    bool             m_dirty      = false;
};

class SpriteEditor {
public:
    static constexpr size_t MAX_SPRITES = 512;

    [[nodiscard]] bool addSprite(const SpriteAsset& sprite) {
        for (auto& s : m_sprites) if (s.name() == sprite.name()) return false;
        if (m_sprites.size() >= MAX_SPRITES) return false;
        m_sprites.push_back(sprite);
        return true;
    }

    [[nodiscard]] bool removeSprite(const std::string& name) {
        for (auto it = m_sprites.begin(); it != m_sprites.end(); ++it) {
            if (it->name() == name) {
                if (m_activeSprite == name) m_activeSprite.clear();
                m_sprites.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] SpriteAsset* findSprite(const std::string& name) {
        for (auto& s : m_sprites) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] bool setActiveSprite(const std::string& name) {
        for (auto& s : m_sprites) {
            if (s.name() == name) { m_activeSprite = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t spriteCount()   const { return m_sprites.size();  }
    [[nodiscard]] const std::string& activeSprite() const { return m_activeSprite; }

    [[nodiscard]] size_t dirtyCount()    const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isDirty())    ++c; return c;
    }
    [[nodiscard]] size_t animatedCount() const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isAnimated()) ++c; return c;
    }
    [[nodiscard]] size_t playingCount()  const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isPlaying())  ++c; return c;
    }
    [[nodiscard]] size_t loopingCount()  const {
        size_t c = 0; for (auto& s : m_sprites) if (s.isLooping())  ++c; return c;
    }
    [[nodiscard]] size_t countByBlendMode(SpriteBlendMode b) const {
        size_t c = 0; for (auto& s : m_sprites) if (s.blendMode() == b) ++c; return c;
    }
    [[nodiscard]] size_t countByOrigin(SpriteOrigin o) const {
        size_t c = 0; for (auto& s : m_sprites) if (s.origin() == o) ++c; return c;
    }

private:
    std::vector<SpriteAsset> m_sprites;
    std::string              m_activeSprite;
};

// ── S39 — Tilemap Editor ─────────────────────────────────────────

enum class TileFlipMode : uint8_t {
    None, Horizontal, Vertical, Both, Rotate90
};
inline const char* tileFlipModeName(TileFlipMode m) {
    switch (m) {
        case TileFlipMode::None:       return "None";
        case TileFlipMode::Horizontal: return "Horizontal";
        case TileFlipMode::Vertical:   return "Vertical";
        case TileFlipMode::Both:       return "Both";
        case TileFlipMode::Rotate90:   return "Rotate90";
    }
    return "Unknown";
}

enum class TileLayerType : uint8_t {
    Background, Midground, Foreground, Object, Collision
};
inline const char* tileLayerTypeName(TileLayerType t) {
    switch (t) {
        case TileLayerType::Background: return "Background";
        case TileLayerType::Midground:  return "Midground";
        case TileLayerType::Foreground: return "Foreground";
        case TileLayerType::Object:     return "Object";
        case TileLayerType::Collision:  return "Collision";
    }
    return "Unknown";
}

enum class TileAnimMode : uint8_t {
    Static, Loop, PingPong, Once, Random
};
inline const char* tileAnimModeName(TileAnimMode m) {
    switch (m) {
        case TileAnimMode::Static:   return "Static";
        case TileAnimMode::Loop:     return "Loop";
        case TileAnimMode::PingPong: return "PingPong";
        case TileAnimMode::Once:     return "Once";
        case TileAnimMode::Random:   return "Random";
    }
    return "Unknown";
}

class TileAsset {
public:
    explicit TileAsset(const std::string& name,
                       uint32_t tileWidth  = 16,
                       uint32_t tileHeight = 16)
        : m_name(name), m_tileWidth(tileWidth), m_tileHeight(tileHeight) {}

    void setTilesetName(const std::string& ts) { m_tilesetName = ts; }
    void setTileId(uint32_t id)                { m_tileId      = id; }
    void setFlipMode(TileFlipMode f)           { m_flipMode    = f;  }
    void setLayerType(TileLayerType l)         { m_layerType   = l;  }
    void setAnimMode(TileAnimMode a)           { m_animMode    = a;  }
    void setAnimated(bool v)                   { m_animated    = v;  }
    void setCollider(bool v)                   { m_collider    = v;  }
    void setDirty(bool v)                      { m_dirty       = v;  }

    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] const std::string& tilesetName() const { return m_tilesetName; }
    [[nodiscard]] uint32_t           tileId()      const { return m_tileId;      }
    [[nodiscard]] uint32_t           tileWidth()   const { return m_tileWidth;   }
    [[nodiscard]] uint32_t           tileHeight()  const { return m_tileHeight;  }
    [[nodiscard]] TileFlipMode       flipMode()    const { return m_flipMode;    }
    [[nodiscard]] TileLayerType      layerType()   const { return m_layerType;   }
    [[nodiscard]] TileAnimMode       animMode()    const { return m_animMode;    }
    [[nodiscard]] bool               isAnimated()  const { return m_animated;    }
    [[nodiscard]] bool               hasCollider() const { return m_collider;    }
    [[nodiscard]] bool               isDirty()     const { return m_dirty;       }

    [[nodiscard]] uint32_t area() const { return m_tileWidth * m_tileHeight; }

private:
    std::string   m_name;
    std::string   m_tilesetName;
    uint32_t      m_tileId      = 0;
    uint32_t      m_tileWidth;
    uint32_t      m_tileHeight;
    TileFlipMode  m_flipMode    = TileFlipMode::None;
    TileLayerType m_layerType   = TileLayerType::Background;
    TileAnimMode  m_animMode    = TileAnimMode::Static;
    bool          m_animated    = false;
    bool          m_collider    = false;
    bool          m_dirty       = false;
};

class TilemapEditor {
public:
    static constexpr size_t MAX_TILES = 256;

    [[nodiscard]] bool addTile(const TileAsset& tile) {
        if (m_tiles.size() >= MAX_TILES) return false;
        for (auto& t : m_tiles) if (t.name() == tile.name()) return false;
        m_tiles.push_back(tile);
        return true;
    }

    [[nodiscard]] bool removeTile(const std::string& name) {
        for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it) {
            if (it->name() == name) {
                if (m_activeTile == name) m_activeTile.clear();
                m_tiles.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TileAsset* findTile(const std::string& name) {
        for (auto& t : m_tiles) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] bool setActiveTile(const std::string& name) {
        for (auto& t : m_tiles)
            if (t.name() == name) { m_activeTile = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeTile() const { return m_activeTile; }
    [[nodiscard]] size_t tileCount()   const { return m_tiles.size(); }
    [[nodiscard]] size_t dirtyCount()  const {
        size_t c = 0; for (auto& t : m_tiles) if (t.isDirty())     ++c; return c;
    }
    [[nodiscard]] size_t animatedCount() const {
        size_t c = 0; for (auto& t : m_tiles) if (t.isAnimated())  ++c; return c;
    }
    [[nodiscard]] size_t colliderCount() const {
        size_t c = 0; for (auto& t : m_tiles) if (t.hasCollider()) ++c; return c;
    }
    [[nodiscard]] size_t countByLayerType(TileLayerType l) const {
        size_t c = 0; for (auto& t : m_tiles) if (t.layerType() == l) ++c; return c;
    }
    [[nodiscard]] size_t countByFlipMode(TileFlipMode f) const {
        size_t c = 0; for (auto& t : m_tiles) if (t.flipMode()    == f) ++c; return c;
    }

private:
    std::vector<TileAsset> m_tiles;
    std::string            m_activeTile;
};

// ── S40 — Audio Clip Editor ──────────────────────────────────────

enum class AudioClipFormat : uint8_t {
    WAV, OGG, MP3, FLAC, AIFF
};
inline const char* audioClipFormatName(AudioClipFormat f) {
    switch (f) {
        case AudioClipFormat::WAV:  return "WAV";
        case AudioClipFormat::OGG:  return "OGG";
        case AudioClipFormat::MP3:  return "MP3";
        case AudioClipFormat::FLAC: return "FLAC";
        case AudioClipFormat::AIFF: return "AIFF";
    }
    return "Unknown";
}

enum class AudioClipState : uint8_t {
    Idle, Playing, Paused, Stopped, Finished
};
inline const char* audioClipStateName(AudioClipState s) {
    switch (s) {
        case AudioClipState::Idle:     return "Idle";
        case AudioClipState::Playing:  return "Playing";
        case AudioClipState::Paused:   return "Paused";
        case AudioClipState::Stopped:  return "Stopped";
        case AudioClipState::Finished: return "Finished";
    }
    return "Unknown";
}

enum class AudioLoopMode : uint8_t {
    None, Loop, PingPong, LoopPoint, Shuffle
};
inline const char* audioLoopModeName(AudioLoopMode m) {
    switch (m) {
        case AudioLoopMode::None:      return "None";
        case AudioLoopMode::Loop:      return "Loop";
        case AudioLoopMode::PingPong:  return "PingPong";
        case AudioLoopMode::LoopPoint: return "LoopPoint";
        case AudioLoopMode::Shuffle:   return "Shuffle";
    }
    return "Unknown";
}

class AudioClipAsset {
public:
    explicit AudioClipAsset(const std::string& name,
                             float durationSec = 1.0f,
                             uint32_t sampleRate = 44100)
        : m_name(name), m_durationSec(durationSec), m_sampleRate(sampleRate) {}

    void setFormat(AudioClipFormat f)   { m_format   = f; }
    void setState(AudioClipState s)     { m_state    = s; }
    void setLoopMode(AudioLoopMode m)   { m_loopMode = m; }
    void setVolume(float v)             { m_volume   = v; }
    void setPitch(float p)              { m_pitch    = p; }
    void setStreaming(bool v)           { m_streaming = v; }
    void setDirty(bool v)               { m_dirty     = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] AudioClipFormat    format()     const { return m_format;     }
    [[nodiscard]] AudioClipState     state()      const { return m_state;      }
    [[nodiscard]] AudioLoopMode      loopMode()   const { return m_loopMode;   }
    [[nodiscard]] float              durationSec()const { return m_durationSec;}
    [[nodiscard]] uint32_t           sampleRate() const { return m_sampleRate; }
    [[nodiscard]] float              volume()     const { return m_volume;     }
    [[nodiscard]] float              pitch()      const { return m_pitch;      }
    [[nodiscard]] bool               isStreaming()const { return m_streaming;  }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isPlaying()  const { return m_state == AudioClipState::Playing;  }
    [[nodiscard]] bool isPaused()   const { return m_state == AudioClipState::Paused;   }
    [[nodiscard]] bool isFinished() const { return m_state == AudioClipState::Finished; }
    [[nodiscard]] bool isLooping()  const { return m_loopMode != AudioLoopMode::None;   }

private:
    std::string      m_name;
    AudioClipFormat  m_format     = AudioClipFormat::WAV;
    AudioClipState   m_state      = AudioClipState::Idle;
    AudioLoopMode    m_loopMode   = AudioLoopMode::None;
    float            m_durationSec;
    uint32_t         m_sampleRate;
    float            m_volume     = 1.0f;
    float            m_pitch      = 1.0f;
    bool             m_streaming  = false;
    bool             m_dirty      = false;
};

class AudioClipEditor {
public:
    static constexpr size_t MAX_CLIPS = 512;

    [[nodiscard]] bool addClip(const AudioClipAsset& clip) {
        if (m_clips.size() >= MAX_CLIPS) return false;
        for (auto& c : m_clips) if (c.name() == clip.name()) return false;
        m_clips.push_back(clip);
        return true;
    }

    [[nodiscard]] bool removeClip(const std::string& name) {
        for (auto it = m_clips.begin(); it != m_clips.end(); ++it) {
            if (it->name() == name) {
                if (m_activeClip == name) m_activeClip.clear();
                m_clips.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AudioClipAsset* findClip(const std::string& name) {
        for (auto& c : m_clips) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] bool setActiveClip(const std::string& name) {
        for (auto& c : m_clips)
            if (c.name() == name) { m_activeClip = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeClip() const { return m_activeClip; }
    [[nodiscard]] size_t clipCount()     const { return m_clips.size(); }
    [[nodiscard]] size_t dirtyCount()    const {
        size_t n = 0; for (auto& c : m_clips) if (c.isDirty())     ++n; return n;
    }
    [[nodiscard]] size_t playingCount()  const {
        size_t n = 0; for (auto& c : m_clips) if (c.isPlaying())   ++n; return n;
    }
    [[nodiscard]] size_t streamingCount()const {
        size_t n = 0; for (auto& c : m_clips) if (c.isStreaming())  ++n; return n;
    }
    [[nodiscard]] size_t loopingCount()  const {
        size_t n = 0; for (auto& c : m_clips) if (c.isLooping())   ++n; return n;
    }
    [[nodiscard]] size_t countByFormat(AudioClipFormat f) const {
        size_t n = 0; for (auto& c : m_clips) if (c.format() == f) ++n; return n;
    }
    [[nodiscard]] size_t countByState(AudioClipState s) const {
        size_t n = 0; for (auto& c : m_clips) if (c.state()  == s) ++n; return n;
    }

private:
    std::vector<AudioClipAsset> m_clips;
    std::string                 m_activeClip;
};

// ── S41 — Video Clip Editor ──────────────────────────────────────

enum class VideoClipCodec : uint8_t {
    H264, H265, VP8, VP9, AV1
};
inline const char* videoClipCodecName(VideoClipCodec c) {
    switch (c) {
        case VideoClipCodec::H264: return "H264";
        case VideoClipCodec::H265: return "H265";
        case VideoClipCodec::VP8:  return "VP8";
        case VideoClipCodec::VP9:  return "VP9";
        case VideoClipCodec::AV1:  return "AV1";
    }
    return "Unknown";
}

enum class VideoClipState : uint8_t {
    Idle, Playing, Paused, Stopped, Finished
};
inline const char* videoClipStateName(VideoClipState s) {
    switch (s) {
        case VideoClipState::Idle:     return "Idle";
        case VideoClipState::Playing:  return "Playing";
        case VideoClipState::Paused:   return "Paused";
        case VideoClipState::Stopped:  return "Stopped";
        case VideoClipState::Finished: return "Finished";
    }
    return "Unknown";
}

enum class VideoAspectRatio : uint8_t {
    Ratio4x3, Ratio16x9, Ratio21x9, Ratio1x1, Custom
};
inline const char* videoAspectRatioName(VideoAspectRatio r) {
    switch (r) {
        case VideoAspectRatio::Ratio4x3:  return "4x3";
        case VideoAspectRatio::Ratio16x9: return "16x9";
        case VideoAspectRatio::Ratio21x9: return "21x9";
        case VideoAspectRatio::Ratio1x1:  return "1x1";
        case VideoAspectRatio::Custom:    return "Custom";
    }
    return "Unknown";
}

class VideoClipAsset {
public:
    explicit VideoClipAsset(const std::string& name,
                             float durationSec = 1.0f,
                             uint32_t fps = 30)
        : m_name(name), m_durationSec(durationSec), m_fps(fps) {}

    void setCodec(VideoClipCodec c)         { m_codec       = c; }
    void setState(VideoClipState s)         { m_state       = s; }
    void setAspectRatio(VideoAspectRatio r) { m_aspectRatio = r; }
    void setWidth(uint32_t w)               { m_width       = w; }
    void setHeight(uint32_t h)              { m_height      = h; }
    void setLooping(bool v)                 { m_looping     = v; }
    void setStreaming(bool v)               { m_streaming   = v; }
    void setDirty(bool v)                   { m_dirty       = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] VideoClipCodec     codec()      const { return m_codec;      }
    [[nodiscard]] VideoClipState     state()      const { return m_state;      }
    [[nodiscard]] VideoAspectRatio   aspectRatio()const { return m_aspectRatio;}
    [[nodiscard]] float              durationSec()const { return m_durationSec;}
    [[nodiscard]] uint32_t           fps()        const { return m_fps;        }
    [[nodiscard]] uint32_t           width()      const { return m_width;      }
    [[nodiscard]] uint32_t           height()     const { return m_height;     }
    [[nodiscard]] bool               isLooping()  const { return m_looping;    }
    [[nodiscard]] bool               isStreaming()const { return m_streaming;  }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isPlaying()  const { return m_state == VideoClipState::Playing;  }
    [[nodiscard]] bool isPaused()   const { return m_state == VideoClipState::Paused;   }
    [[nodiscard]] bool isFinished() const { return m_state == VideoClipState::Finished; }
    [[nodiscard]] bool isHD()       const { return m_width >= 1280 && m_height >= 720;  }

private:
    std::string      m_name;
    VideoClipCodec   m_codec       = VideoClipCodec::H264;
    VideoClipState   m_state       = VideoClipState::Idle;
    VideoAspectRatio m_aspectRatio = VideoAspectRatio::Ratio16x9;
    float            m_durationSec;
    uint32_t         m_fps;
    uint32_t         m_width       = 1920;
    uint32_t         m_height      = 1080;
    bool             m_looping     = false;
    bool             m_streaming   = false;
    bool             m_dirty       = false;
};

class VideoClipEditor {
public:
    static constexpr size_t MAX_CLIPS = 256;

    [[nodiscard]] bool addClip(const VideoClipAsset& clip) {
        if (m_clips.size() >= MAX_CLIPS) return false;
        for (auto& c : m_clips) if (c.name() == clip.name()) return false;
        m_clips.push_back(clip);
        return true;
    }

    [[nodiscard]] bool removeClip(const std::string& name) {
        for (auto it = m_clips.begin(); it != m_clips.end(); ++it) {
            if (it->name() == name) {
                if (m_activeClip == name) m_activeClip.clear();
                m_clips.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] VideoClipAsset* findClip(const std::string& name) {
        for (auto& c : m_clips) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] bool setActiveClip(const std::string& name) {
        for (auto& c : m_clips)
            if (c.name() == name) { m_activeClip = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeClip()   const { return m_activeClip;  }
    [[nodiscard]] size_t clipCount()                const { return m_clips.size(); }
    [[nodiscard]] size_t dirtyCount()               const {
        size_t n = 0; for (auto& c : m_clips) if (c.isDirty())     ++n; return n;
    }
    [[nodiscard]] size_t playingCount()             const {
        size_t n = 0; for (auto& c : m_clips) if (c.isPlaying())   ++n; return n;
    }
    [[nodiscard]] size_t streamingCount()           const {
        size_t n = 0; for (auto& c : m_clips) if (c.isStreaming())  ++n; return n;
    }
    [[nodiscard]] size_t loopingCount()             const {
        size_t n = 0; for (auto& c : m_clips) if (c.isLooping())   ++n; return n;
    }
    [[nodiscard]] size_t hdCount()                  const {
        size_t n = 0; for (auto& c : m_clips) if (c.isHD())        ++n; return n;
    }
    [[nodiscard]] size_t countByCodec(VideoClipCodec codec) const {
        size_t n = 0; for (auto& c : m_clips) if (c.codec() == codec) ++n; return n;
    }
    [[nodiscard]] size_t countByState(VideoClipState s) const {
        size_t n = 0; for (auto& c : m_clips) if (c.state()  == s) ++n; return n;
    }

private:
    std::vector<VideoClipAsset> m_clips;
    std::string                 m_activeClip;
};

// ── S42 — Scene Prefab Editor ──────────────────────────────────

enum class PrefabCategory : uint8_t {
    Prop, Character, Vehicle, Structure, Effect
};
inline const char* prefabCategoryName(PrefabCategory c) {
    switch (c) {
        case PrefabCategory::Prop:      return "Prop";
        case PrefabCategory::Character: return "Character";
        case PrefabCategory::Vehicle:   return "Vehicle";
        case PrefabCategory::Structure: return "Structure";
        case PrefabCategory::Effect:    return "Effect";
    }
    return "Unknown";
}

enum class PrefabState : uint8_t {
    Unloaded, Loaded, Instanced, Modified, Broken
};
inline const char* prefabStateName(PrefabState s) {
    switch (s) {
        case PrefabState::Unloaded:  return "Unloaded";
        case PrefabState::Loaded:    return "Loaded";
        case PrefabState::Instanced: return "Instanced";
        case PrefabState::Modified:  return "Modified";
        case PrefabState::Broken:    return "Broken";
    }
    return "Unknown";
}

enum class PrefabLOD : uint8_t {
    Full, High, Medium, Low, Proxy
};
inline const char* prefabLODName(PrefabLOD l) {
    switch (l) {
        case PrefabLOD::Full:   return "Full";
        case PrefabLOD::High:   return "High";
        case PrefabLOD::Medium: return "Medium";
        case PrefabLOD::Low:    return "Low";
        case PrefabLOD::Proxy:  return "Proxy";
    }
    return "Unknown";
}

class PrefabAsset {
public:
    explicit PrefabAsset(const std::string& name,
                          uint32_t componentCount = 0,
                          uint32_t childCount = 0)
        : m_name(name), m_componentCount(componentCount), m_childCount(childCount) {}

    void setCategory(PrefabCategory c)  { m_category       = c; }
    void setState(PrefabState s)        { m_state           = s; }
    void setLOD(PrefabLOD l)            { m_lod             = l; }
    void setComponentCount(uint32_t v)  { m_componentCount  = v; }
    void setChildCount(uint32_t v)      { m_childCount      = v; }
    void setOverridable(bool v)         { m_overridable     = v; }
    void setNested(bool v)              { m_nested          = v; }
    void setDirty(bool v)               { m_dirty           = v; }

    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] PrefabCategory     category()       const { return m_category;       }
    [[nodiscard]] PrefabState        state()          const { return m_state;          }
    [[nodiscard]] PrefabLOD          lod()            const { return m_lod;            }
    [[nodiscard]] uint32_t           componentCount() const { return m_componentCount; }
    [[nodiscard]] uint32_t           childCount()     const { return m_childCount;     }
    [[nodiscard]] bool               isOverridable()  const { return m_overridable;    }
    [[nodiscard]] bool               isNested()       const { return m_nested;         }
    [[nodiscard]] bool               isDirty()        const { return m_dirty;          }

    [[nodiscard]] bool isInstanced() const { return m_state == PrefabState::Instanced; }
    [[nodiscard]] bool isModified()  const { return m_state == PrefabState::Modified;  }
    [[nodiscard]] bool isBroken()    const { return m_state == PrefabState::Broken;    }
    [[nodiscard]] bool isComplex()   const { return m_componentCount >= 10; }

private:
    std::string     m_name;
    PrefabCategory  m_category       = PrefabCategory::Prop;
    PrefabState     m_state          = PrefabState::Unloaded;
    PrefabLOD       m_lod            = PrefabLOD::Full;
    uint32_t        m_componentCount;
    uint32_t        m_childCount;
    bool            m_overridable    = false;
    bool            m_nested         = false;
    bool            m_dirty          = false;
};

class ScenePrefabEditor {
public:
    static constexpr size_t MAX_PREFABS = 256;

    [[nodiscard]] bool addPrefab(const PrefabAsset& prefab) {
        if (m_prefabs.size() >= MAX_PREFABS) return false;
        for (auto& p : m_prefabs) if (p.name() == prefab.name()) return false;
        m_prefabs.push_back(prefab);
        return true;
    }

    [[nodiscard]] bool removePrefab(const std::string& name) {
        for (auto it = m_prefabs.begin(); it != m_prefabs.end(); ++it) {
            if (it->name() == name) {
                if (m_activePrefab == name) m_activePrefab.clear();
                m_prefabs.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] PrefabAsset* findPrefab(const std::string& name) {
        for (auto& p : m_prefabs) if (p.name() == name) return &p;
        return nullptr;
    }

    [[nodiscard]] bool setActivePrefab(const std::string& name) {
        for (auto& p : m_prefabs)
            if (p.name() == name) { m_activePrefab = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activePrefab()    const { return m_activePrefab;   }
    [[nodiscard]] size_t prefabCount()                 const { return m_prefabs.size();  }
    [[nodiscard]] size_t dirtyCount()                  const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isDirty())       ++n; return n;
    }
    [[nodiscard]] size_t instancedCount()              const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isInstanced())   ++n; return n;
    }
    [[nodiscard]] size_t overridableCount()            const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isOverridable()) ++n; return n;
    }
    [[nodiscard]] size_t nestedCount()                 const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isNested())      ++n; return n;
    }
    [[nodiscard]] size_t complexCount()                const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isComplex())     ++n; return n;
    }
    [[nodiscard]] size_t countByCategory(PrefabCategory cat) const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.category() == cat) ++n; return n;
    }
    [[nodiscard]] size_t countByState(PrefabState st) const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.state() == st)  ++n; return n;
    }

private:
    std::vector<PrefabAsset> m_prefabs;
    std::string              m_activePrefab;
};

// ── S43 — Animation Blueprint Editor ───────────────────────────

enum class AnimBPNodeType : uint8_t {
    StateMachine, BlendSpace, Selector, Sequence, Pose
};
inline const char* animBPNodeTypeName(AnimBPNodeType t) {
    switch (t) {
        case AnimBPNodeType::StateMachine: return "StateMachine";
        case AnimBPNodeType::BlendSpace:   return "BlendSpace";
        case AnimBPNodeType::Selector:     return "Selector";
        case AnimBPNodeType::Sequence:     return "Sequence";
        case AnimBPNodeType::Pose:         return "Pose";
    }
    return "Unknown";
}

enum class AnimBPState : uint8_t {
    Inactive, Compiling, Ready, Running, Error
};
inline const char* animBPStateName(AnimBPState s) {
    switch (s) {
        case AnimBPState::Inactive:  return "Inactive";
        case AnimBPState::Compiling: return "Compiling";
        case AnimBPState::Ready:     return "Ready";
        case AnimBPState::Running:   return "Running";
        case AnimBPState::Error:     return "Error";
    }
    return "Unknown";
}

enum class AnimBPBlendMode : uint8_t {
    Override, Additive, Layered, Masked, Blended
};
inline const char* animBPBlendModeName(AnimBPBlendMode m) {
    switch (m) {
        case AnimBPBlendMode::Override: return "Override";
        case AnimBPBlendMode::Additive: return "Additive";
        case AnimBPBlendMode::Layered:  return "Layered";
        case AnimBPBlendMode::Masked:   return "Masked";
        case AnimBPBlendMode::Blended:  return "Blended";
    }
    return "Unknown";
}

class AnimBlueprintAsset {
public:
    explicit AnimBlueprintAsset(const std::string& name,
                                 uint32_t nodeCount = 0,
                                 uint32_t layerCount = 0)
        : m_name(name), m_nodeCount(nodeCount), m_layerCount(layerCount) {}

    void setNodeType(AnimBPNodeType t)    { m_nodeType   = t; }
    void setState(AnimBPState s)          { m_state      = s; }
    void setBlendMode(AnimBPBlendMode m)  { m_blendMode  = m; }
    void setNodeCount(uint32_t v)         { m_nodeCount  = v; }
    void setLayerCount(uint32_t v)        { m_layerCount = v; }
    void setLooping(bool v)               { m_looping    = v; }
    void setOptimized(bool v)             { m_optimized  = v; }
    void setDirty(bool v)                 { m_dirty      = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] AnimBPNodeType     nodeType()   const { return m_nodeType;   }
    [[nodiscard]] AnimBPState        state()      const { return m_state;      }
    [[nodiscard]] AnimBPBlendMode    blendMode()  const { return m_blendMode;  }
    [[nodiscard]] uint32_t           nodeCount()  const { return m_nodeCount;  }
    [[nodiscard]] uint32_t           layerCount() const { return m_layerCount; }
    [[nodiscard]] bool               isLooping()  const { return m_looping;    }
    [[nodiscard]] bool               isOptimized()const { return m_optimized;  }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isRunning()  const { return m_state == AnimBPState::Running;  }
    [[nodiscard]] bool hasError()   const { return m_state == AnimBPState::Error;    }
    [[nodiscard]] bool isReady()    const { return m_state == AnimBPState::Ready;    }
    [[nodiscard]] bool isComplex()  const { return m_nodeCount >= 20; }

private:
    std::string       m_name;
    AnimBPNodeType    m_nodeType   = AnimBPNodeType::StateMachine;
    AnimBPState       m_state      = AnimBPState::Inactive;
    AnimBPBlendMode   m_blendMode  = AnimBPBlendMode::Override;
    uint32_t          m_nodeCount;
    uint32_t          m_layerCount;
    bool              m_looping    = false;
    bool              m_optimized  = false;
    bool              m_dirty      = false;
};

class AnimBlueprintEditor {
public:
    static constexpr size_t MAX_BLUEPRINTS = 256;

    [[nodiscard]] bool addBlueprint(const AnimBlueprintAsset& bp) {
        if (m_blueprints.size() >= MAX_BLUEPRINTS) return false;
        for (auto& b : m_blueprints) if (b.name() == bp.name()) return false;
        m_blueprints.push_back(bp);
        return true;
    }

    [[nodiscard]] bool removeBlueprint(const std::string& name) {
        for (auto it = m_blueprints.begin(); it != m_blueprints.end(); ++it) {
            if (it->name() == name) {
                if (m_activeBlueprint == name) m_activeBlueprint.clear();
                m_blueprints.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AnimBlueprintAsset* findBlueprint(const std::string& name) {
        for (auto& b : m_blueprints) if (b.name() == name) return &b;
        return nullptr;
    }

    [[nodiscard]] bool setActiveBlueprint(const std::string& name) {
        for (auto& b : m_blueprints)
            if (b.name() == name) { m_activeBlueprint = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeBlueprint()   const { return m_activeBlueprint;    }
    [[nodiscard]] size_t blueprintCount()                const { return m_blueprints.size();   }
    [[nodiscard]] size_t dirtyCount()                    const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isDirty())      ++n; return n;
    }
    [[nodiscard]] size_t runningCount()                  const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isRunning())    ++n; return n;
    }
    [[nodiscard]] size_t loopingCount()                  const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isLooping())    ++n; return n;
    }
    [[nodiscard]] size_t optimizedCount()                const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isOptimized())  ++n; return n;
    }
    [[nodiscard]] size_t complexCount()                  const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.isComplex())    ++n; return n;
    }
    [[nodiscard]] size_t countByNodeType(AnimBPNodeType t) const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.nodeType() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByState(AnimBPState s) const {
        size_t n = 0; for (auto& b : m_blueprints) if (b.state() == s)   ++n; return n;
    }

private:
    std::vector<AnimBlueprintAsset> m_blueprints;
    std::string                     m_activeBlueprint;
};

// ── M1-C — NFRenderViewport base class ────────────────────────────

enum class ViewportRenderMode : uint8_t {
    Wireframe, Solid, Lit, Textured, Unlit
};

inline const char* viewportRenderModeName(ViewportRenderMode m) {
    switch (m) {
        case ViewportRenderMode::Wireframe: return "Wireframe";
        case ViewportRenderMode::Solid:     return "Solid";
        case ViewportRenderMode::Lit:       return "Lit";
        case ViewportRenderMode::Textured:  return "Textured";
        case ViewportRenderMode::Unlit:     return "Unlit";
    }
    return "Unknown";
}

enum class ViewportGizmoMode : uint8_t {
    None, Translate, Rotate, Scale
};

inline const char* viewportGizmoModeName(ViewportGizmoMode m) {
    switch (m) {
        case ViewportGizmoMode::None:      return "None";
        case ViewportGizmoMode::Translate: return "Translate";
        case ViewportGizmoMode::Rotate:    return "Rotate";
        case ViewportGizmoMode::Scale:     return "Scale";
    }
    return "Unknown";
}

enum class ViewportCameraMode : uint8_t {
    FPS, Orbit, Flythrough, TopDown, Cinematic
};

inline const char* viewportCameraModeName(ViewportCameraMode m) {
    switch (m) {
        case ViewportCameraMode::FPS:        return "FPS";
        case ViewportCameraMode::Orbit:      return "Orbit";
        case ViewportCameraMode::Flythrough: return "Flythrough";
        case ViewportCameraMode::TopDown:    return "TopDown";
        case ViewportCameraMode::Cinematic:  return "Cinematic";
    }
    return "Unknown";
}

class NFRenderViewport {
public:
    explicit NFRenderViewport(const std::string& name,
                              uint32_t width = 1280, uint32_t height = 720)
        : m_name(name), m_width(width), m_height(height) {}

    virtual ~NFRenderViewport() = default;

    // Camera control
    void setCameraMode(ViewportCameraMode mode)   { m_cameraMode = mode; }
    void setRenderMode(ViewportRenderMode mode)   { m_renderMode = mode; }
    void setGizmoMode(ViewportGizmoMode mode)     { m_gizmoMode = mode; }

    void setCameraPosition(float x, float y, float z) { m_camX = x; m_camY = y; m_camZ = z; }
    void setCameraTarget(float x, float y, float z)    { m_tgtX = x; m_tgtY = y; m_tgtZ = z; }
    void setCameraFOV(float fov)                       { m_fov = fov; }
    void setNearFar(float nearP, float farP)           { m_nearPlane = nearP; m_farPlane = farP; }

    void resize(uint32_t w, uint32_t h) { m_width = w; m_height = h; }

    // FPS camera: right-click WASD, Q/E vertical, Shift sprint
    void setSprintMultiplier(float mult)    { m_sprintMult = mult; }
    void setMoveSpeed(float speed)          { m_moveSpeed = speed; }
    void setLookSensitivity(float sens)     { m_lookSens = sens; }
    void setGridVisible(bool v)             { m_gridVisible = v; }
    void setGridSize(float s)               { m_gridSize = s; }

    [[nodiscard]] const std::string&   name()       const { return m_name; }
    [[nodiscard]] uint32_t             width()      const { return m_width; }
    [[nodiscard]] uint32_t             height()     const { return m_height; }
    [[nodiscard]] ViewportCameraMode   cameraMode() const { return m_cameraMode; }
    [[nodiscard]] ViewportRenderMode   renderMode() const { return m_renderMode; }
    [[nodiscard]] ViewportGizmoMode    gizmoMode()  const { return m_gizmoMode; }
    [[nodiscard]] float                fov()        const { return m_fov; }
    [[nodiscard]] float                nearPlane()  const { return m_nearPlane; }
    [[nodiscard]] float                farPlane()   const { return m_farPlane; }
    [[nodiscard]] float                moveSpeed()  const { return m_moveSpeed; }
    [[nodiscard]] float                lookSensitivity() const { return m_lookSens; }
    [[nodiscard]] float                sprintMultiplier() const { return m_sprintMult; }
    [[nodiscard]] bool                 gridVisible() const { return m_gridVisible; }
    [[nodiscard]] float                gridSize()   const { return m_gridSize; }
    [[nodiscard]] float                camX() const { return m_camX; }
    [[nodiscard]] float                camY() const { return m_camY; }
    [[nodiscard]] float                camZ() const { return m_camZ; }
    [[nodiscard]] float                tgtX() const { return m_tgtX; }
    [[nodiscard]] float                tgtY() const { return m_tgtY; }
    [[nodiscard]] float                tgtZ() const { return m_tgtZ; }

    [[nodiscard]] float aspectRatio() const {
        return m_height > 0 ? static_cast<float>(m_width) / static_cast<float>(m_height) : 1.0f;
    }
    [[nodiscard]] bool isGizmoActive() const { return m_gizmoMode != ViewportGizmoMode::None; }

    // Frame counter for rendering tick
    void tick() { ++m_frameCount; }
    [[nodiscard]] uint64_t frameCount() const { return m_frameCount; }

protected:
    std::string         m_name;
    uint32_t            m_width      = 1280;
    uint32_t            m_height     = 720;
    ViewportCameraMode  m_cameraMode = ViewportCameraMode::FPS;
    ViewportRenderMode  m_renderMode = ViewportRenderMode::Lit;
    ViewportGizmoMode   m_gizmoMode  = ViewportGizmoMode::None;
    float               m_fov        = 60.0f;
    float               m_nearPlane  = 0.1f;
    float               m_farPlane   = 1000.0f;
    float               m_moveSpeed  = 5.0f;
    float               m_lookSens   = 0.15f;
    float               m_sprintMult = 2.5f;
    bool                m_gridVisible = true;
    float               m_gridSize    = 1.0f;
    float               m_camX = 0.0f, m_camY = 5.0f, m_camZ = 10.0f;
    float               m_tgtX = 0.0f, m_tgtY = 0.0f, m_tgtZ = 0.0f;
    uint64_t            m_frameCount = 0;
};

// ── M1-C — MeshViewerPanel ────────────────────────────────────────

enum class MeshDisplayMode : uint8_t {
    Wireframe, Solid, Lit, UV, Normals
};

inline const char* meshDisplayModeName(MeshDisplayMode m) {
    switch (m) {
        case MeshDisplayMode::Wireframe: return "Wireframe";
        case MeshDisplayMode::Solid:     return "Solid";
        case MeshDisplayMode::Lit:       return "Lit";
        case MeshDisplayMode::UV:        return "UV";
        case MeshDisplayMode::Normals:   return "Normals";
    }
    return "Unknown";
}

struct MeshViewerAsset {
    std::string name;
    uint32_t    vertexCount   = 0;
    uint32_t    triangleCount = 0;
    uint32_t    lodCount      = 1;
    bool        hasNormals    = true;
    bool        hasUVs        = true;
    bool        loaded        = false;

    [[nodiscard]] bool isHighPoly()  const { return triangleCount >= 100000; }
    [[nodiscard]] bool isMultiLOD()  const { return lodCount > 1; }
    [[nodiscard]] bool isComplete()  const { return hasNormals && hasUVs && loaded; }
};

class MeshViewerPanel : public NFRenderViewport {
public:
    static constexpr size_t MAX_MESHES = 256;

    MeshViewerPanel() : NFRenderViewport("MeshViewer") {
        m_cameraMode = ViewportCameraMode::Orbit;
    }

    [[nodiscard]] bool addMesh(const MeshViewerAsset& mesh) {
        if (m_meshes.size() >= MAX_MESHES) return false;
        for (auto& m : m_meshes) if (m.name == mesh.name) return false;
        m_meshes.push_back(mesh);
        return true;
    }

    [[nodiscard]] bool removeMesh(const std::string& name) {
        for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it) {
            if (it->name == name) {
                if (m_activeMesh == name) m_activeMesh.clear();
                m_meshes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] MeshViewerAsset* findMesh(const std::string& name) {
        for (auto& m : m_meshes) if (m.name == name) return &m;
        return nullptr;
    }

    [[nodiscard]] bool setActiveMesh(const std::string& name) {
        for (auto& m : m_meshes)
            if (m.name == name) { m_activeMesh = name; return true; }
        return false;
    }

    void setDisplayMode(MeshDisplayMode dm) { m_displayMode = dm; }

    [[nodiscard]] MeshDisplayMode         displayMode()  const { return m_displayMode; }
    [[nodiscard]] const std::string&      activeMesh()   const { return m_activeMesh; }
    [[nodiscard]] size_t                  meshCount()    const { return m_meshes.size(); }

    [[nodiscard]] size_t highPolyCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.isHighPoly()) ++n; return n;
    }
    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.loaded) ++n; return n;
    }
    [[nodiscard]] size_t multiLODCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.isMultiLOD()) ++n; return n;
    }
    [[nodiscard]] size_t completeCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.isComplete()) ++n; return n;
    }

private:
    std::vector<MeshViewerAsset> m_meshes;
    std::string                  m_activeMesh;
    MeshDisplayMode              m_displayMode = MeshDisplayMode::Lit;
};

// ── M1-C — MaterialEditorPanel (enhanced with NFRenderViewport) ──

enum class MaterialPreviewShape : uint8_t {
    Sphere, Cube, Cylinder, Plane, Custom
};

inline const char* materialPreviewShapeName(MaterialPreviewShape s) {
    switch (s) {
        case MaterialPreviewShape::Sphere:   return "Sphere";
        case MaterialPreviewShape::Cube:     return "Cube";
        case MaterialPreviewShape::Cylinder: return "Cylinder";
        case MaterialPreviewShape::Plane:    return "Plane";
        case MaterialPreviewShape::Custom:   return "Custom";
    }
    return "Unknown";
}

class MaterialEditorPanel : public NFRenderViewport {
public:
    static constexpr size_t MAX_MATERIALS = 128;

    MaterialEditorPanel() : NFRenderViewport("MaterialEditor") {}

    [[nodiscard]] bool addMaterial(const MaterialAsset& mat) {
        if (m_materials.size() >= MAX_MATERIALS) return false;
        for (auto& m : m_materials) if (m.name() == mat.name()) return false;
        m_materials.push_back(mat);
        return true;
    }

    [[nodiscard]] bool removeMaterial(const std::string& name) {
        for (auto it = m_materials.begin(); it != m_materials.end(); ++it) {
            if (it->name() == name) {
                if (m_activeMaterial == name) m_activeMaterial.clear();
                m_materials.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] MaterialAsset* findMaterial(const std::string& name) {
        for (auto& m : m_materials) if (m.name() == name) return &m;
        return nullptr;
    }

    [[nodiscard]] bool setActiveMaterial(const std::string& name) {
        for (auto& m : m_materials)
            if (m.name() == name) { m_activeMaterial = name; return true; }
        return false;
    }

    void setPreviewShape(MaterialPreviewShape s) { m_previewShape = s; }
    void setAutoRecompile(bool v)                { m_autoRecompile = v; }
    void setLivePreview(bool v)                  { m_livePreview = v; }

    [[nodiscard]] MaterialPreviewShape  previewShape()   const { return m_previewShape; }
    [[nodiscard]] bool                  autoRecompile()  const { return m_autoRecompile; }
    [[nodiscard]] bool                  livePreview()    const { return m_livePreview; }
    [[nodiscard]] const std::string&    activeMaterial() const { return m_activeMaterial; }
    [[nodiscard]] size_t                materialCount()  const { return m_materials.size(); }

    [[nodiscard]] size_t dirtyCount() const {
        size_t n = 0; for (auto& m : m_materials) if (m.isDirty()) ++n; return n;
    }
    [[nodiscard]] size_t pbrCount() const {
        size_t n = 0; for (auto& m : m_materials)
            if (m.shadingModel() == MaterialShadingModel::PBR) ++n;
        return n;
    }

private:
    std::vector<MaterialAsset>  m_materials;
    std::string                 m_activeMaterial;
    MaterialPreviewShape        m_previewShape  = MaterialPreviewShape::Sphere;
    bool                        m_autoRecompile = true;
    bool                        m_livePreview   = true;
};

// ── M1-C — SkeletalEditorPanel ────────────────────────────────────

enum class BoneDisplayMode : uint8_t {
    Lines, Octahedral, Stick, BBone, Envelope
};

inline const char* boneDisplayModeName(BoneDisplayMode m) {
    switch (m) {
        case BoneDisplayMode::Lines:      return "Lines";
        case BoneDisplayMode::Octahedral: return "Octahedral";
        case BoneDisplayMode::Stick:      return "Stick";
        case BoneDisplayMode::BBone:      return "BBone";
        case BoneDisplayMode::Envelope:   return "Envelope";
    }
    return "Unknown";
}

enum class WeightPaintMode : uint8_t {
    Off, Add, Subtract, Smooth, Replace, Blur
};

inline const char* weightPaintModeName(WeightPaintMode m) {
    switch (m) {
        case WeightPaintMode::Off:       return "Off";
        case WeightPaintMode::Add:       return "Add";
        case WeightPaintMode::Subtract:  return "Subtract";
        case WeightPaintMode::Smooth:    return "Smooth";
        case WeightPaintMode::Replace:   return "Replace";
        case WeightPaintMode::Blur:      return "Blur";
    }
    return "Unknown";
}

struct SkeletalBone {
    std::string name;
    int32_t     parentIndex = -1;
    float       length      = 1.0f;
    bool        selected    = false;
    bool        locked      = false;

    [[nodiscard]] bool isRoot() const { return parentIndex < 0; }
};

struct SkeletalAsset {
    std::string name;
    std::vector<SkeletalBone> bones;
    bool        loaded = false;
    bool        dirty  = false;

    explicit SkeletalAsset(const std::string& n) : name(n) {}

    [[nodiscard]] bool addBone(const SkeletalBone& bone) {
        for (auto& b : bones) if (b.name == bone.name) return false;
        bones.push_back(bone);
        return true;
    }

    [[nodiscard]] bool removeBone(const std::string& n) {
        for (auto it = bones.begin(); it != bones.end(); ++it) {
            if (it->name == n) { bones.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SkeletalBone* findBone(const std::string& n) {
        for (auto& b : bones) if (b.name == n) return &b;
        return nullptr;
    }

    [[nodiscard]] size_t boneCount()     const { return bones.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& b : bones) if (b.selected) ++c; return c;
    }
    [[nodiscard]] size_t rootCount() const {
        size_t c = 0; for (auto& b : bones) if (b.isRoot()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (auto& b : bones) if (b.locked) ++c; return c;
    }
    [[nodiscard]] bool isComplex() const { return bones.size() >= 50; }
};

class SkeletalEditorPanel : public NFRenderViewport {
public:
    static constexpr size_t MAX_SKELETONS = 128;

    SkeletalEditorPanel() : NFRenderViewport("SkeletalEditor") {}

    [[nodiscard]] bool addSkeleton(const SkeletalAsset& skel) {
        if (m_skeletons.size() >= MAX_SKELETONS) return false;
        for (auto& s : m_skeletons) if (s.name == skel.name) return false;
        m_skeletons.push_back(skel);
        return true;
    }

    [[nodiscard]] bool removeSkeleton(const std::string& name) {
        for (auto it = m_skeletons.begin(); it != m_skeletons.end(); ++it) {
            if (it->name == name) {
                if (m_activeSkeleton == name) m_activeSkeleton.clear();
                m_skeletons.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] SkeletalAsset* findSkeleton(const std::string& name) {
        for (auto& s : m_skeletons) if (s.name == name) return &s;
        return nullptr;
    }

    [[nodiscard]] bool setActiveSkeleton(const std::string& name) {
        for (auto& s : m_skeletons)
            if (s.name == name) { m_activeSkeleton = name; return true; }
        return false;
    }

    void openAsset(const std::string& name) { setActiveSkeleton(name); }

    void setBoneDisplayMode(BoneDisplayMode m) { m_boneDisplay = m; }
    void setWeightPaintMode(WeightPaintMode m) { m_weightPaint = m; }
    void setBrushRadius(float r)               { m_brushRadius = r; }
    void setBrushStrength(float s)             { m_brushStrength = s; }

    [[nodiscard]] BoneDisplayMode     boneDisplayMode()  const { return m_boneDisplay; }
    [[nodiscard]] WeightPaintMode     weightPaintMode()  const { return m_weightPaint; }
    [[nodiscard]] float               brushRadius()      const { return m_brushRadius; }
    [[nodiscard]] float               brushStrength()    const { return m_brushStrength; }
    [[nodiscard]] const std::string&  activeSkeleton()   const { return m_activeSkeleton; }
    [[nodiscard]] size_t              skeletonCount()    const { return m_skeletons.size(); }
    [[nodiscard]] bool                isPainting()       const { return m_weightPaint != WeightPaintMode::Off; }

    [[nodiscard]] size_t dirtyCount() const {
        size_t n = 0; for (auto& s : m_skeletons) if (s.dirty) ++n; return n;
    }
    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0; for (auto& s : m_skeletons) if (s.loaded) ++n; return n;
    }
    [[nodiscard]] size_t complexCount() const {
        size_t n = 0; for (auto& s : m_skeletons) if (s.isComplex()) ++n; return n;
    }

private:
    std::vector<SkeletalAsset> m_skeletons;
    std::string                m_activeSkeleton;
    BoneDisplayMode            m_boneDisplay   = BoneDisplayMode::Octahedral;
    WeightPaintMode            m_weightPaint   = WeightPaintMode::Off;
    float                      m_brushRadius   = 5.0f;
    float                      m_brushStrength = 0.5f;
};

// ── M1-C — AnimationEditorPanel ───────────────────────────────────

enum class AnimPlaybackState : uint8_t {
    Stopped, Playing, Paused, Looping, Reverse
};

inline const char* animPlaybackStateName(AnimPlaybackState s) {
    switch (s) {
        case AnimPlaybackState::Stopped: return "Stopped";
        case AnimPlaybackState::Playing: return "Playing";
        case AnimPlaybackState::Paused:  return "Paused";
        case AnimPlaybackState::Looping: return "Looping";
        case AnimPlaybackState::Reverse: return "Reverse";
    }
    return "Unknown";
}

enum class AnimBlendTreeType : uint8_t {
    Simple, Additive, Layered, Override, Masked
};

inline const char* animBlendTreeTypeName(AnimBlendTreeType t) {
    switch (t) {
        case AnimBlendTreeType::Simple:   return "Simple";
        case AnimBlendTreeType::Additive: return "Additive";
        case AnimBlendTreeType::Layered:  return "Layered";
        case AnimBlendTreeType::Override: return "Override";
        case AnimBlendTreeType::Masked:   return "Masked";
    }
    return "Unknown";
}

struct AnimClipAsset {
    std::string name;
    float       duration    = 0.0f;
    float       frameRate   = 30.0f;
    uint32_t    keyframeCount = 0;
    bool        looping     = false;
    bool        dirty       = false;
    bool        loaded      = false;
    AnimBlendTreeType blendType = AnimBlendTreeType::Simple;

    explicit AnimClipAsset(const std::string& n) : name(n) {}

    [[nodiscard]] uint32_t totalFrames() const {
        return static_cast<uint32_t>(duration * frameRate);
    }
    [[nodiscard]] bool isLong()   const { return duration >= 10.0f; }
    [[nodiscard]] bool isDense()  const { return keyframeCount >= 100; }
    [[nodiscard]] bool isReady()  const { return loaded && !dirty; }
};

class AnimationEditorPanel : public NFRenderViewport {
public:
    static constexpr size_t MAX_CLIPS = 256;

    AnimationEditorPanel() : NFRenderViewport("AnimationEditor") {}

    [[nodiscard]] bool addClip(const AnimClipAsset& clip) {
        if (m_clips.size() >= MAX_CLIPS) return false;
        for (auto& c : m_clips) if (c.name == clip.name) return false;
        m_clips.push_back(clip);
        return true;
    }

    [[nodiscard]] bool removeClip(const std::string& name) {
        for (auto it = m_clips.begin(); it != m_clips.end(); ++it) {
            if (it->name == name) {
                if (m_activeClip == name) m_activeClip.clear();
                m_clips.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AnimClipAsset* findClip(const std::string& name) {
        for (auto& c : m_clips) if (c.name == name) return &c;
        return nullptr;
    }

    [[nodiscard]] bool setActiveClip(const std::string& name) {
        for (auto& c : m_clips)
            if (c.name == name) { m_activeClip = name; return true; }
        return false;
    }

    void openAsset(const std::string& name) { setActiveClip(name); }

    void setPlaybackState(AnimPlaybackState s) { m_playback = s; }
    void setPlaybackSpeed(float s)             { m_playbackSpeed = s; }
    void setCurrentFrame(uint32_t f)           { m_currentFrame = f; }
    void setTimelineZoom(float z)              { m_timelineZoom = z; }
    void setOnionSkinning(bool v)              { m_onionSkinning = v; }
    void setShowBlendTree(bool v)              { m_showBlendTree = v; }

    [[nodiscard]] AnimPlaybackState  playbackState()  const { return m_playback; }
    [[nodiscard]] float              playbackSpeed()  const { return m_playbackSpeed; }
    [[nodiscard]] uint32_t           currentFrame()   const { return m_currentFrame; }
    [[nodiscard]] float              timelineZoom()   const { return m_timelineZoom; }
    [[nodiscard]] bool               onionSkinning()  const { return m_onionSkinning; }
    [[nodiscard]] bool               showBlendTree()  const { return m_showBlendTree; }
    [[nodiscard]] const std::string& activeClip()     const { return m_activeClip; }
    [[nodiscard]] size_t             clipCount()      const { return m_clips.size(); }
    [[nodiscard]] bool               isPlaying()      const {
        return m_playback == AnimPlaybackState::Playing || m_playback == AnimPlaybackState::Looping;
    }

    [[nodiscard]] size_t dirtyCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.dirty) ++n; return n;
    }
    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.loaded) ++n; return n;
    }
    [[nodiscard]] size_t loopingClipCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.looping) ++n; return n;
    }
    [[nodiscard]] size_t longClipCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.isLong()) ++n; return n;
    }
    [[nodiscard]] size_t denseClipCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.isDense()) ++n; return n;
    }
    [[nodiscard]] size_t countByBlendType(AnimBlendTreeType t) const {
        size_t n = 0; for (auto& c : m_clips) if (c.blendType == t) ++n; return n;
    }

private:
    std::vector<AnimClipAsset> m_clips;
    std::string                m_activeClip;
    AnimPlaybackState          m_playback      = AnimPlaybackState::Stopped;
    float                      m_playbackSpeed = 1.0f;
    uint32_t                   m_currentFrame  = 0;
    float                      m_timelineZoom  = 1.0f;
    bool                       m_onionSkinning = false;
    bool                       m_showBlendTree = false;
};

// ── M1-C — ShipEditorPanel ────────────────────────────────────────

enum class ShipEditorTool : uint8_t {
    Select, PlaceModule, RotateModule, RemoveModule, WireConnect
};

inline const char* shipEditorToolName(ShipEditorTool t) {
    switch (t) {
        case ShipEditorTool::Select:       return "Select";
        case ShipEditorTool::PlaceModule:  return "PlaceModule";
        case ShipEditorTool::RotateModule: return "RotateModule";
        case ShipEditorTool::RemoveModule: return "RemoveModule";
        case ShipEditorTool::WireConnect:  return "WireConnect";
    }
    return "Unknown";
}

struct ShipModuleEntry {
    std::string name;
    std::string category;
    uint32_t    slotIndex  = 0;
    float       mass       = 1.0f;
    float       powerDraw  = 0.0f;
    bool        placed     = false;
    bool        connected  = false;

    [[nodiscard]] bool isHeavy()      const { return mass >= 100.0f; }
    [[nodiscard]] bool isPowered()    const { return powerDraw > 0.0f; }
    [[nodiscard]] bool isOperational() const { return placed && connected; }
};

class ShipEditorPanel : public NFRenderViewport {
public:
    static constexpr size_t MAX_MODULES = 512;

    ShipEditorPanel() : NFRenderViewport("ShipEditor") {}

    [[nodiscard]] bool addModule(const ShipModuleEntry& mod) {
        if (m_modules.size() >= MAX_MODULES) return false;
        for (auto& m : m_modules) if (m.name == mod.name) return false;
        m_modules.push_back(mod);
        return true;
    }

    [[nodiscard]] bool removeModule(const std::string& name) {
        for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
            if (it->name == name) {
                if (m_activeModule == name) m_activeModule.clear();
                m_modules.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ShipModuleEntry* findModule(const std::string& name) {
        for (auto& m : m_modules) if (m.name == name) return &m;
        return nullptr;
    }

    [[nodiscard]] bool setActiveModule(const std::string& name) {
        for (auto& m : m_modules)
            if (m.name == name) { m_activeModule = name; return true; }
        return false;
    }

    void setTool(ShipEditorTool t)      { m_tool = t; }
    void setShipName(const std::string& n) { m_shipName = n; }
    void setSymmetryEnabled(bool v)     { m_symmetry = v; }

    [[nodiscard]] ShipEditorTool      tool()         const { return m_tool; }
    [[nodiscard]] const std::string&  shipName()     const { return m_shipName; }
    [[nodiscard]] bool                symmetry()     const { return m_symmetry; }
    [[nodiscard]] const std::string&  activeModule() const { return m_activeModule; }
    [[nodiscard]] size_t              moduleCount()  const { return m_modules.size(); }

    [[nodiscard]] size_t placedCount() const {
        size_t n = 0; for (auto& m : m_modules) if (m.placed) ++n; return n;
    }
    [[nodiscard]] size_t connectedCount() const {
        size_t n = 0; for (auto& m : m_modules) if (m.connected) ++n; return n;
    }
    [[nodiscard]] size_t operationalCount() const {
        size_t n = 0; for (auto& m : m_modules) if (m.isOperational()) ++n; return n;
    }
    [[nodiscard]] size_t heavyCount() const {
        size_t n = 0; for (auto& m : m_modules) if (m.isHeavy()) ++n; return n;
    }
    [[nodiscard]] size_t poweredCount() const {
        size_t n = 0; for (auto& m : m_modules) if (m.isPowered()) ++n; return n;
    }
    [[nodiscard]] float totalMass() const {
        float t = 0.0f; for (auto& m : m_modules) t += m.mass; return t;
    }
    [[nodiscard]] float totalPowerDraw() const {
        float t = 0.0f; for (auto& m : m_modules) t += m.powerDraw; return t;
    }

private:
    std::vector<ShipModuleEntry> m_modules;
    std::string                  m_activeModule;
    std::string                  m_shipName;
    ShipEditorTool               m_tool     = ShipEditorTool::Select;
    bool                         m_symmetry = false;
};

// ── M1-C — CharacterEditorPanel ───────────────────────────────────

enum class CharacterEditorTab : uint8_t {
    Appearance, Skeleton, Equipment, Stats, Preview
};

inline const char* characterEditorTabName(CharacterEditorTab t) {
    switch (t) {
        case CharacterEditorTab::Appearance: return "Appearance";
        case CharacterEditorTab::Skeleton:   return "Skeleton";
        case CharacterEditorTab::Equipment:  return "Equipment";
        case CharacterEditorTab::Stats:      return "Stats";
        case CharacterEditorTab::Preview:    return "Preview";
    }
    return "Unknown";
}

struct CharacterSlot {
    std::string slotName;
    std::string equippedItem;
    bool        locked = false;
    bool        visible = true;

    [[nodiscard]] bool isEmpty() const { return equippedItem.empty(); }
};

struct CharacterPreset {
    std::string name;
    std::string meshPath;
    std::string materialPath;
    uint32_t    slotCount    = 0;
    bool        dirty        = false;
    bool        loaded       = false;
    std::vector<CharacterSlot> slots;

    explicit CharacterPreset(const std::string& n) : name(n) {}

    [[nodiscard]] bool addSlot(const CharacterSlot& slot) {
        for (auto& s : slots) if (s.slotName == slot.slotName) return false;
        slots.push_back(slot);
        slotCount = static_cast<uint32_t>(slots.size());
        return true;
    }

    [[nodiscard]] size_t equippedCount() const {
        size_t c = 0; for (auto& s : slots) if (!s.isEmpty()) ++c; return c;
    }
    [[nodiscard]] size_t visibleSlotCount() const {
        size_t c = 0; for (auto& s : slots) if (s.visible) ++c; return c;
    }
    [[nodiscard]] bool isFullyEquipped() const {
        if (slots.empty()) return false;
        for (auto& s : slots) if (s.isEmpty()) return false;
        return true;
    }
};

class CharacterEditorPanel : public NFRenderViewport {
public:
    static constexpr size_t MAX_PRESETS = 128;

    CharacterEditorPanel() : NFRenderViewport("CharacterEditor") {}

    [[nodiscard]] bool addPreset(const CharacterPreset& preset) {
        if (m_presets.size() >= MAX_PRESETS) return false;
        for (auto& p : m_presets) if (p.name == preset.name) return false;
        m_presets.push_back(preset);
        return true;
    }

    [[nodiscard]] bool removePreset(const std::string& name) {
        for (auto it = m_presets.begin(); it != m_presets.end(); ++it) {
            if (it->name == name) {
                if (m_activePreset == name) m_activePreset.clear();
                m_presets.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] CharacterPreset* findPreset(const std::string& name) {
        for (auto& p : m_presets) if (p.name == name) return &p;
        return nullptr;
    }

    [[nodiscard]] bool setActivePreset(const std::string& name) {
        for (auto& p : m_presets)
            if (p.name == name) { m_activePreset = name; return true; }
        return false;
    }

    void setActiveTab(CharacterEditorTab t) { m_activeTab = t; }
    void setAutoRotate(bool v)              { m_autoRotate = v; }

    [[nodiscard]] CharacterEditorTab   activeTab()    const { return m_activeTab; }
    [[nodiscard]] bool                 autoRotate()   const { return m_autoRotate; }
    [[nodiscard]] const std::string&   activePreset() const { return m_activePreset; }
    [[nodiscard]] size_t               presetCount()  const { return m_presets.size(); }

    [[nodiscard]] size_t dirtyCount() const {
        size_t n = 0; for (auto& p : m_presets) if (p.dirty) ++n; return n;
    }
    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0; for (auto& p : m_presets) if (p.loaded) ++n; return n;
    }
    [[nodiscard]] size_t fullyEquippedCount() const {
        size_t n = 0; for (auto& p : m_presets) if (p.isFullyEquipped()) ++n; return n;
    }

private:
    std::vector<CharacterPreset> m_presets;
    std::string                  m_activePreset;
    CharacterEditorTab           m_activeTab  = CharacterEditorTab::Appearance;
    bool                         m_autoRotate = false;
};

// ── M1-C — PrefabEditorPanel ──────────────────────────────────────

enum class PrefabEditMode : uint8_t {
    View, Place, Delete, Transform, Connect
};

inline const char* prefabEditModeName(PrefabEditMode m) {
    switch (m) {
        case PrefabEditMode::View:      return "View";
        case PrefabEditMode::Place:     return "Place";
        case PrefabEditMode::Delete:    return "Delete";
        case PrefabEditMode::Transform: return "Transform";
        case PrefabEditMode::Connect:   return "Connect";
    }
    return "Unknown";
}

struct PrefabInstance {
    std::string name;
    std::string prefabSource;
    float       posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float       scale = 1.0f;
    bool        visible   = true;
    bool        locked    = false;
    bool        overridden = false;

    [[nodiscard]] bool isModified()  const { return overridden; }
    [[nodiscard]] bool isScaled()    const { return scale != 1.0f; }
};

class PrefabEditorPanel : public NFRenderViewport {
public:
    static constexpr size_t MAX_INSTANCES = 512;

    PrefabEditorPanel() : NFRenderViewport("PrefabEditor") {}

    [[nodiscard]] bool addInstance(const PrefabInstance& inst) {
        if (m_instances.size() >= MAX_INSTANCES) return false;
        for (auto& i : m_instances) if (i.name == inst.name) return false;
        m_instances.push_back(inst);
        return true;
    }

    [[nodiscard]] bool removeInstance(const std::string& name) {
        for (auto it = m_instances.begin(); it != m_instances.end(); ++it) {
            if (it->name == name) {
                if (m_activeInstance == name) m_activeInstance.clear();
                m_instances.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] PrefabInstance* findInstance(const std::string& name) {
        for (auto& i : m_instances) if (i.name == name) return &i;
        return nullptr;
    }

    [[nodiscard]] bool setActiveInstance(const std::string& name) {
        for (auto& i : m_instances)
            if (i.name == name) { m_activeInstance = name; return true; }
        return false;
    }

    void setEditMode(PrefabEditMode m)   { m_editMode = m; }
    void setSnapToGrid(bool v)           { m_snapToGrid = v; }
    void setSnapSize(float s)            { m_snapSize = s; }
    void setIsolationMode(bool v)        { m_isolationMode = v; }

    [[nodiscard]] PrefabEditMode       editMode()        const { return m_editMode; }
    [[nodiscard]] bool                 snapToGrid()      const { return m_snapToGrid; }
    [[nodiscard]] float                snapSize()        const { return m_snapSize; }
    [[nodiscard]] bool                 isolationMode()   const { return m_isolationMode; }
    [[nodiscard]] const std::string&   activeInstance()  const { return m_activeInstance; }
    [[nodiscard]] size_t               instanceCount()   const { return m_instances.size(); }

    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.visible) ++n; return n;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.locked) ++n; return n;
    }
    [[nodiscard]] size_t overriddenCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.overridden) ++n; return n;
    }
    [[nodiscard]] size_t scaledCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.isScaled()) ++n; return n;
    }

private:
    std::vector<PrefabInstance> m_instances;
    std::string                m_activeInstance;
    PrefabEditMode             m_editMode      = PrefabEditMode::View;
    bool                       m_snapToGrid    = true;
    float                      m_snapSize      = 0.5f;
    bool                       m_isolationMode = false;
};

} // namespace NF
