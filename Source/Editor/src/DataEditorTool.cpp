// NF::Editor — DataEditorTool implementation.
//
// Fifth real NF::IHostedTool from Phase 3 consolidation.
// Manages data tables, game configs, and localization keys within AtlasWorkspace.

#include "NF/Editor/DataEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <cstdio>

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

void DataEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Tables (25%) | Data Grid (50%) | Inspector (25%)
    const float tablesW = ctx.w * 0.25f;
    const float gridW   = ctx.w * 0.50f;
    const float inspW   = ctx.w - tablesW - gridW;

    // ── Tables panel ──────────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, tablesW, ctx.h, "Tables");
    if (!m_openTablePath.empty()) {
        std::string label = m_openTablePath;
        if (label.size() > 22) label = label.substr(0, 19) + "...";
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, label, ctx.kTextPrimary);
    } else {
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, "No table open", ctx.kTextMuted);
    }

    // ── Data Grid ─────────────────────────────────────────────────
    ctx.drawPanel(ctx.x + tablesW, ctx.y, gridW, ctx.h, "Data Grid");
    {
        char gridBuf[64];
        std::snprintf(gridBuf, sizeof(gridBuf), "%u rows  x  %u cols",
                      m_stats.rowCount, m_stats.columnCount);
        ctx.ui.drawText(ctx.x + tablesW + 8.f, ctx.y + 30.f, gridBuf, ctx.kTextSecond);
        if (m_stats.selectedRowCount > 0) {
            char selBuf[32];
            std::snprintf(selBuf, sizeof(selBuf), "%u selected", m_stats.selectedRowCount);
            ctx.drawStatusPill(ctx.x + tablesW + 8.f, ctx.y + 50.f, selBuf, ctx.kAccentBlue);
        }
    }
    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + tablesW + 8.f, ctx.y + ctx.h - 20.f,
                        "* unsaved changes", ctx.kRed);
    }

    // ── Inspector ─────────────────────────────────────────────────
    ctx.drawPanel(ctx.x + tablesW + gridW, ctx.y, inspW, ctx.h, "Inspector");
    ctx.ui.drawText(ctx.x + tablesW + gridW + 8.f, ctx.y + 30.f,
                    "Row Properties", ctx.kTextSecond);
}

} // namespace NF
