// NF::Editor — DataEditorTool implementation.
//
// Fifth real NF::IHostedTool from Phase 3 consolidation.
// Manages data tables, game configs, and localization keys within AtlasWorkspace.

#include "NF/Editor/DataEditorTool.h"

namespace NF {

DataEditorTool::DataEditorTool() {
    buildDescriptor();
}

void DataEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Data Editor";
    m_descriptor.category    = HostedToolCategory::DataEditing;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    m_descriptor.supportedPanels = {
        "panel.data_table",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "data.new_row",
        "data.delete_row",
        "data.duplicate_row",
        "data.import_csv",
        "data.export_csv",
        "data.save",
    };
}

bool DataEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode      = DataEditMode::Table;
    m_stats         = {};
    m_openTablePath = {};
    m_state         = HostedToolState::Ready;
    return true;
}

void DataEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = DataEditMode::Table;
    m_stats           = {};
    m_openTablePath   = {};
    m_activeProjectId = {};
}

void DataEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void DataEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void DataEditorTool::update(float /*dt*/) {
    // Data editor updates are event-driven (cell edits, import/export).
}

void DataEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
    m_openTablePath   = {};
}

void DataEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
    m_openTablePath   = {};
}

void DataEditorTool::setEditMode(DataEditMode mode) {
    m_editMode = mode;
}

void DataEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void DataEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void DataEditorTool::setRowCount(uint32_t count) {
    m_stats.rowCount = count;
}

void DataEditorTool::setSelectedRowCount(uint32_t count) {
    m_stats.selectedRowCount = count;
}

void DataEditorTool::setColumnCount(uint32_t count) {
    m_stats.columnCount = count;
}

void DataEditorTool::setOpenTablePath(const std::string& path) {
    m_openTablePath = path;
}

} // namespace NF
