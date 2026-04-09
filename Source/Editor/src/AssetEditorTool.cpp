// NF::Editor — AssetEditorTool implementation.
//
// Second real NF::IHostedTool from Phase 3 consolidation.
// Manages the primary content browser and asset import/management
// workflow within the AtlasWorkspace.

#include "NF/Editor/AssetEditorTool.h"

namespace NF {

AssetEditorTool::AssetEditorTool() {
    buildDescriptor();
}

void AssetEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Asset Editor";
    m_descriptor.category    = HostedToolCategory::ProjectBrowser;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    // Shared panels declared by this tool.
    // These are registered with PanelRegistry by WorkspaceShell.
    m_descriptor.supportedPanels = {
        "panel.content_browser",
        "panel.inspector",
        "panel.asset_preview",
        "panel.console",
    };

    // Commands contributed by this tool.
    m_descriptor.commands = {
        "asset.import",
        "asset.delete",
        "asset.rename",
        "asset.duplicate",
        "asset.refresh",
        "asset.open",
        "asset.create_folder",
    };
}

bool AssetEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_filterMode  = AssetFilterMode::All;
    m_searchQuery = {};
    m_stats       = {};
    m_state       = HostedToolState::Ready;
    return true;
}

void AssetEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_filterMode      = AssetFilterMode::All;
    m_searchQuery     = {};
    m_stats           = {};
    m_activeProjectId = {};
}

void AssetEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void AssetEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void AssetEditorTool::update(float /*dt*/) {
    // No per-frame work at this layer; content refresh is event-driven.
}

void AssetEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
    m_filterMode      = AssetFilterMode::All;
    m_searchQuery     = {};
}

void AssetEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
    m_filterMode      = AssetFilterMode::All;
    m_searchQuery     = {};
}

void AssetEditorTool::setFilterMode(AssetFilterMode mode) {
    m_filterMode = mode;
}

void AssetEditorTool::setSearchQuery(const std::string& query) {
    m_searchQuery = query;
}

void AssetEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void AssetEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void AssetEditorTool::setSelectionCount(uint32_t count) {
    m_stats.selectionCount = count;
}

void AssetEditorTool::setTotalAssetCount(uint32_t count) {
    m_stats.totalAssetCount = count;
}

void AssetEditorTool::setFilteredAssetCount(uint32_t count) {
    m_stats.filteredAssetCount = count;
}

} // namespace NF
