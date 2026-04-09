// NF::Editor — SceneEditorTool implementation.
//
// First real NF::IHostedTool from Phase 3 consolidation.
// Replaces the former pattern of header-only V1 stubs with a real
// lifecycle-managed tool registered with WorkspaceShell via ToolRegistry.

#include "NF/Editor/SceneEditorTool.h"

namespace NF {

SceneEditorTool::SceneEditorTool() {
    buildDescriptor();
}

void SceneEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Scene Editor";
    m_descriptor.category    = HostedToolCategory::SceneEditing;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    // Shared panels declared by this tool.
    // These are registered with PanelRegistry by WorkspaceShell.
    m_descriptor.supportedPanels = {
        "panel.viewport",
        "panel.outliner",
        "panel.inspector",
        "panel.console",
    };

    // Commands contributed by this tool.
    m_descriptor.commands = {
        "scene.create_entity",
        "scene.delete_entity",
        "scene.duplicate_entity",
        "scene.save_scene",
        "scene.enter_play",
        "scene.exit_play",
        "scene.set_mode.select",
        "scene.set_mode.translate",
        "scene.set_mode.rotate",
        "scene.set_mode.scale",
    };
}

bool SceneEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode = SceneEditMode::Select;
    m_stats    = {};
    m_state    = HostedToolState::Ready;
    return true;
}

void SceneEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = SceneEditMode::Select;
    m_stats           = {};
    m_activeProjectId = {};
}

void SceneEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void SceneEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void SceneEditorTool::update(float dt) {
    if (m_state != HostedToolState::Active) return;
    m_stats.lastFrameMs = dt * 1000.0f;
}

void SceneEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats = {};
}

void SceneEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats = {};
}

void SceneEditorTool::setEditMode(SceneEditMode mode) {
    m_editMode = mode;
}

void SceneEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void SceneEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void SceneEditorTool::setSelectionCount(uint32_t count) {
    m_stats.selectionCount = count;
}

void SceneEditorTool::setEntityCount(uint32_t count) {
    m_stats.entityCount = count;
}

} // namespace NF
