// NF::Editor — VisualLogicEditorTool implementation.
//
// Sixth real NF::IHostedTool from Phase 3 consolidation.
// Manages visual scripting / blueprint graph authoring within AtlasWorkspace.

#include "NF/Editor/VisualLogicEditorTool.h"

namespace NF {

VisualLogicEditorTool::VisualLogicEditorTool() {
    buildDescriptor();
}

void VisualLogicEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Visual Logic Editor";
    m_descriptor.category    = HostedToolCategory::LogicAuthoring;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.node_graph",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "vl.add_node",
        "vl.delete_node",
        "vl.connect_pins",
        "vl.disconnect_pins",
        "vl.compile",
        "vl.save",
    };
}

bool VisualLogicEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode = VisualLogicMode::Graph;
    m_stats    = {};
    m_state    = HostedToolState::Ready;
    return true;
}

void VisualLogicEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = VisualLogicMode::Graph;
    m_stats           = {};
    m_activeProjectId = {};
}

void VisualLogicEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void VisualLogicEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_stats.isCompiling = false;
        m_state = HostedToolState::Suspended;
    }
}

void VisualLogicEditorTool::update(float /*dt*/) {
    // Compilation and graph evaluation are async; no per-frame polling needed here.
}

void VisualLogicEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
}

void VisualLogicEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
}

void VisualLogicEditorTool::setEditMode(VisualLogicMode mode) {
    m_editMode = mode;
}

void VisualLogicEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void VisualLogicEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void VisualLogicEditorTool::setNodeCount(uint32_t count) {
    m_stats.nodeCount = count;
}

void VisualLogicEditorTool::setConnectionCount(uint32_t count) {
    m_stats.connectionCount = count;
}

void VisualLogicEditorTool::setCompiling(bool compiling) {
    m_stats.isCompiling = compiling;
}

void VisualLogicEditorTool::setErrorCount(uint32_t count) {
    m_stats.errorCount = count;
}

} // namespace NF
