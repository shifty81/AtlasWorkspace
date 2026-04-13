// NF::Editor — DataEditorTool implementation.
//
// Fifth real NF::IHostedTool from Phase 3 consolidation.
// Manages data tables, game configs, and localization keys within AtlasWorkspace.

#include "NF/Editor/DataEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <cstdio>
#include <cstring>

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

    // Stub table list (reflects open table + project-catalog tables when shell wired)
    static const char* kTableNames[] = {
        "PlayerStats", "EnemyConfig", "ItemTable", "QuestData", "SkillTree"
    };
    static constexpr int kTableCount = 5;

    // ── Tables panel ──────────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, tablesW, ctx.h, "Tables");
    {
        // Show the externally-set table path if available, otherwise the view selection
        const std::string& displayPath = !m_openTablePath.empty() ? m_openTablePath : m_viewOpenTable;
        float ty2 = ctx.y + 28.f;
        for (int i = 0; i < kTableCount; ++i) {
            if (ty2 + 20.f > ctx.y + ctx.h - 4.f) break;
            bool active = (displayPath == kTableNames[i]);
            bool hov    = ctx.isHovered({ctx.x + 2.f, ty2, tablesW - 4.f, 18.f});
            uint32_t bg = active ? ctx.kAccentBlue
                        : (hov ? 0x2A2A3AFF : 0x00000000u);
            if (bg) ctx.ui.drawRect({ctx.x + 2.f, ty2, tablesW - 4.f, 18.f}, bg);
            ctx.ui.drawText(ctx.x + 8.f, ty2 + 2.f, kTableNames[i],
                            active ? ctx.kTextPrimary : ctx.kTextSecond);
            if (ctx.hitRegion({ctx.x + 2.f, ty2, tablesW - 4.f, 18.f}, false))
                m_viewOpenTable = active ? "" : kTableNames[i];
            ty2 += 20.f;
        }
    }

    // ── Data Grid ─────────────────────────────────────────────────
    ctx.drawPanel(ctx.x + tablesW, ctx.y, gridW, ctx.h, "Data Grid");
    {
        char gridBuf[64];
        std::snprintf(gridBuf, sizeof(gridBuf), "%u rows  x  %u cols",
                      m_stats.rowCount, m_stats.columnCount);
        ctx.ui.drawText(ctx.x + tablesW + 8.f, ctx.y + 30.f, gridBuf, ctx.kTextSecond);

        // Header row
        float hdrY = ctx.y + 48.f;
        ctx.ui.drawRect({ctx.x + tablesW, hdrY, gridW, 20.f}, 0x303030FF);
        ctx.ui.drawText(ctx.x + tablesW + 8.f,   hdrY + 3.f, "ID",    ctx.kTextSecond);
        ctx.ui.drawText(ctx.x + tablesW + 60.f,  hdrY + 3.f, "Name",  ctx.kTextSecond);
        ctx.ui.drawText(ctx.x + tablesW + 180.f, hdrY + 3.f, "Value", ctx.kTextSecond);
        ctx.ui.drawRect({ctx.x + tablesW, hdrY + 20.f, gridW, 1.f}, ctx.kBorder);

        // Data rows (show up to 10 stub rows derived from row count)
        static const char* kRowIds[]    = {"001","002","003","004","005","006","007","008","009","010"};
        static const char* kRowValues[] = {"100","50","12","1","0","42","7","99","3","18"};
        uint32_t rowsToShow = m_stats.rowCount < 10u ? m_stats.rowCount : 10u;
        float ry2 = hdrY + 22.f;
        for (uint32_t i = 0; i < rowsToShow; ++i) {
            if (ry2 + 20.f > ctx.y + ctx.h - 4.f) break;
            bool sel = (m_viewSelectedRow == static_cast<int>(i));
            bool hov = ctx.isHovered({ctx.x + tablesW + 2.f, ry2, gridW - 4.f, 18.f});
            uint32_t bg = sel ? 0x1A3A6AFF : (hov ? 0x2A2A3AFF : 0x00000000u);
            if (bg) ctx.ui.drawRect({ctx.x + tablesW + 2.f, ry2, gridW - 4.f, 18.f}, bg);
            ctx.ui.drawRect({ctx.x + tablesW + 2.f, ry2, 2.f, 18.f},
                            sel ? ctx.kAccentBlue : 0x404040FF);
            ctx.ui.drawText(ctx.x + tablesW + 8.f,   ry2 + 2.f, kRowIds[i],    ctx.kTextSecond);
            ctx.ui.drawText(ctx.x + tablesW + 60.f,  ry2 + 2.f, kTableNames[i % kTableCount], ctx.kTextSecond);
            ctx.ui.drawText(ctx.x + tablesW + 180.f, ry2 + 2.f, kRowValues[i], ctx.kTextMuted);
            if (ctx.hitRegion({ctx.x + tablesW + 2.f, ry2, gridW - 4.f, 18.f}, false))
                m_viewSelectedRow = sel ? -1 : static_cast<int>(i);
            ry2 += 20.f;
        }
        if (m_stats.selectedRowCount > 0) {
            char selBuf[32];
            std::snprintf(selBuf, sizeof(selBuf), "%u selected", m_stats.selectedRowCount);
            ctx.drawStatusPill(ctx.x + tablesW + 8.f, ctx.y + ctx.h - 22.f,
                               selBuf, ctx.kAccentBlue);
        }
    }
    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + tablesW + 8.f, ctx.y + ctx.h - 20.f,
                        "* unsaved changes", ctx.kRed);
    }

    // ── Inspector ─────────────────────────────────────────────────
    ctx.drawPanel(ctx.x + tablesW + gridW, ctx.y, inspW, ctx.h, "Inspector");
    const float ix = ctx.x + tablesW + gridW;
    if (m_viewSelectedRow >= 0) {
        static const char* kRowIds[]    = {"001","002","003","004","005","006","007","008","009","010"};
        static const char* kRowValues[] = {"100","50","12","1","0","42","7","99","3","18"};
        ctx.ui.drawText(ix + 8.f, ctx.y + 30.f, "Row Details", ctx.kTextPrimary);
        ctx.ui.drawRect({ix + 4.f, ctx.y + 44.f, inspW - 8.f, 1.f}, ctx.kBorder);
        int ri = m_viewSelectedRow;
        ctx.drawStatRow(ix + 8.f, ctx.y + 50.f, "ID:",    kRowIds[ri % 10]);
        ctx.drawStatRow(ix + 8.f, ctx.y + 68.f, "Name:",  kTableNames[ri % kTableCount]);
        ctx.drawStatRow(ix + 8.f, ctx.y + 86.f, "Value:", kRowValues[ri % 10]);

        // Save row button
        if (ctx.drawButton(ix + 8.f, ctx.y + ctx.h - 30.f, inspW - 16.f, 20.f, "Save Row")) {
            if (ctx.shell)
                (void)ctx.shell->commandBus().execute("data.save_row");
        }
    } else {
        ctx.ui.drawText(ix + 8.f, ctx.y + 30.f, "Row Properties", ctx.kTextSecond);
        ctx.ui.drawText(ix + 8.f, ctx.y + 50.f, "Select a row", ctx.kTextMuted);
    }
}

} // namespace NF
