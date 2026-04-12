// NF::Editor — VisualLogicEditorTool implementation.
//
// Sixth real NF::IHostedTool from Phase 3 consolidation.
// Manages visual scripting / blueprint graph authoring within AtlasWorkspace.

#include "NF/Editor/VisualLogicEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <cstdio>

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

void VisualLogicEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Node List (20%) | Graph Canvas (60%) | Properties (20%)
    const float nodesW  = ctx.w * 0.20f;
    const float graphW  = ctx.w * 0.60f;
    const float propW   = ctx.w - nodesW - graphW;

    // ── Node List panel ───────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, nodesW, ctx.h, "Nodes");
    {
        char nodeBuf[32];
        std::snprintf(nodeBuf, sizeof(nodeBuf), "%u nodes", m_stats.nodeCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, nodeBuf, ctx.kTextSecond);
        char connBuf[32];
        std::snprintf(connBuf, sizeof(connBuf), "%u connections", m_stats.connectionCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 48.f, connBuf, ctx.kTextSecond);
    }

    // ── Graph Canvas panel ────────────────────────────────────────
    ctx.drawPanel(ctx.x + nodesW, ctx.y, graphW, ctx.h, "Graph");
    // Mode pill + compile status
    ctx.drawStatusPill(ctx.x + nodesW + 8.f, ctx.y + 30.f,
                       visualLogicModeName(m_editMode), ctx.kAccentBlue);
    if (m_stats.isCompiling) {
        ctx.drawStatusPill(ctx.x + nodesW + 70.f, ctx.y + 30.f, "Compiling...", ctx.kAccentBlue);
    } else if (m_stats.errorCount > 0) {
        char errBuf[24];
        std::snprintf(errBuf, sizeof(errBuf), "%u errors", m_stats.errorCount);
        ctx.drawStatusPill(ctx.x + nodesW + 70.f, ctx.y + 30.f, errBuf, ctx.kRed);
    } else {
        ctx.drawStatusPill(ctx.x + nodesW + 70.f, ctx.y + 30.f, "OK", ctx.kGreen);
    }
    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + nodesW + 8.f, ctx.y + ctx.h - 20.f,
                        "* unsaved", ctx.kRed);
    }

    // ── Properties panel ──────────────────────────────────────────
    ctx.drawPanel(ctx.x + nodesW + graphW, ctx.y, propW, ctx.h, "Properties");
    ctx.ui.drawText(ctx.x + nodesW + graphW + 8.f, ctx.y + 30.f,
                    "Node Parameters", ctx.kTextSecond);
}

} // namespace NF
