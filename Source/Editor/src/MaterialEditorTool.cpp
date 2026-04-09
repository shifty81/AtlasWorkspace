// NF::Editor — MaterialEditorTool implementation.
//
// Third real NF::IHostedTool from Phase 3 consolidation.
// Manages material and shader-graph authoring within AtlasWorkspace.

#include "NF/Editor/MaterialEditorTool.h"

namespace NF {

MaterialEditorTool::MaterialEditorTool() {
    buildDescriptor();
}

void MaterialEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Material Editor";
    m_descriptor.category    = HostedToolCategory::AssetAuthoring;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.viewport_material",
        "panel.inspector",
        "panel.asset_preview",
        "panel.console",
    };

    m_descriptor.commands = {
        "material.create",
        "material.save",
        "material.set_shader",
        "material.add_texture",
        "material.duplicate",
        "material.open",
    };
}

bool MaterialEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode     = MaterialEditMode::Properties;
    m_stats        = {};
    m_openAssetPath = {};
    m_state        = HostedToolState::Ready;
    return true;
}

void MaterialEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = MaterialEditMode::Properties;
    m_stats           = {};
    m_openAssetPath   = {};
    m_activeProjectId = {};
}

void MaterialEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void MaterialEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void MaterialEditorTool::update(float /*dt*/) {
    // Material editor updates are event-driven (shader recompile, live preview).
}

void MaterialEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
    m_openAssetPath   = {};
}

void MaterialEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
    m_openAssetPath   = {};
}

void MaterialEditorTool::setEditMode(MaterialEditMode mode) {
    m_editMode = mode;
}

void MaterialEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void MaterialEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void MaterialEditorTool::setNodeCount(uint32_t count) {
    m_stats.nodeCount = count;
}

void MaterialEditorTool::setTextureSlotCount(uint32_t count) {
    m_stats.textureSlotCount = count;
}

void MaterialEditorTool::setOpenAssetPath(const std::string& path) {
    m_openAssetPath = path;
}

} // namespace NF
