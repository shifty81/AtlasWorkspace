// NF::Editor — VisualLogicEditorTool implementation.
//
// Sixth real NF::IHostedTool from Phase 3 consolidation.
// Manages visual scripting / blueprint graph authoring within AtlasWorkspace.

#include "NF/Editor/VisualLogicEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <cstdio>
#include <string>

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

    // Stub node palette (fixed names for the demonstration graph)
    static const char* kNodeTypes[] = {
        "On Start", "Branch", "Set Variable", "Spawn Entity",
        "Play Sound", "Delay", "Loop", "Event"
    };
    static constexpr int kPaletteCount = 8;

    // ── Node List panel ───────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, nodesW, ctx.h, "Nodes");
    {
        char nodeBuf[32];
        std::snprintf(nodeBuf, sizeof(nodeBuf), "%u nodes", m_stats.nodeCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, nodeBuf, ctx.kTextSecond);
        char connBuf[32];
        std::snprintf(connBuf, sizeof(connBuf), "%u connections", m_stats.connectionCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 48.f, connBuf, ctx.kTextSecond);

        // Node palette list — click to place / select node type
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 68.f, "Palette:", ctx.kTextMuted);
        float ny = ctx.y + 84.f;
        for (int i = 0; i < kPaletteCount; ++i) {
            if (ny + 18.f > ctx.y + ctx.h - 4.f) break;
            bool sel = (m_viewSelectedNode == i);
            bool hov = ctx.isHovered({ctx.x + 2.f, ny, nodesW - 4.f, 16.f});
            uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0x00000000u);
            if (bg) ctx.ui.drawRect({ctx.x + 2.f, ny, nodesW - 4.f, 16.f}, bg);
            ctx.ui.drawText(ctx.x + 8.f, ny + 1.f, kNodeTypes[i],
                            sel ? ctx.kTextPrimary : ctx.kTextSecond);
            if (ctx.hitRegion({ctx.x + 2.f, ny, nodesW - 4.f, 16.f}, false))
                m_viewSelectedNode = sel ? -1 : i;
            ny += 18.f;
        }
    }

    // ── Graph Canvas panel ────────────────────────────────────────
    ctx.drawPanel(ctx.x + nodesW, ctx.y, graphW, ctx.h, "Graph");
    // Mode pill + compile status
    ctx.drawStatusPill(ctx.x + nodesW + 8.f, ctx.y + 30.f,
                       visualLogicModeName(m_editMode), ctx.kAccentBlue);
    if (m_stats.isCompiling) {
        ctx.drawStatusPill(ctx.x + nodesW + 80.f, ctx.y + 30.f, "Compiling...", ctx.kAccentBlue);
    } else if (m_stats.errorCount > 0) {
        char errBuf[24];
        std::snprintf(errBuf, sizeof(errBuf), "%u errors", m_stats.errorCount);
        ctx.drawStatusPill(ctx.x + nodesW + 80.f, ctx.y + 30.f, errBuf, ctx.kRed);
    } else {
        ctx.drawStatusPill(ctx.x + nodesW + 80.f, ctx.y + 30.f, "OK", ctx.kGreen);
    }

    // Draw node stubs on the canvas — 4 default graph nodes
    struct NodeStub { const char* label; float rx; float ry; };
    static const NodeStub kGraphNodes[] = {
        {"On Start",     0.10f, 0.25f},
        {"Branch",       0.32f, 0.20f},
        {"Set Variable", 0.55f, 0.15f},
        {"Spawn Entity", 0.55f, 0.50f},
    };
    {
        const float gvpx = ctx.x + nodesW;
        const float gvpy = ctx.y + 48.f;
        const float gvph = ctx.h - 48.f;
        for (int i = 0; i < 4; ++i) {
            float nx = gvpx + graphW * kGraphNodes[i].rx;
            float ny = gvpy + gvph * kGraphNodes[i].ry;
            bool hov = ctx.isHovered({nx, ny, 100.f, 44.f});
            uint32_t bg = hov ? 0x2A3A5AFF : ctx.kCardBg;
            ctx.ui.drawRect({nx, ny, 100.f, 44.f}, bg);
            ctx.ui.drawRectOutline({nx, ny, 100.f, 44.f}, ctx.kBorder, 1.f);
            ctx.ui.drawRect({nx, ny, 100.f, 20.f}, ctx.kAccentBlue);
            ctx.ui.drawText(nx + 6.f, ny + 4.f, kGraphNodes[i].label, ctx.kTextPrimary);
            // I/O pins
            ctx.ui.drawRect({nx - 5.f,  ny + 28.f, 8.f, 8.f}, ctx.kAccentBlue);
            ctx.ui.drawRect({nx + 97.f, ny + 28.f, 8.f, 8.f}, ctx.kAccentBlue);
            // Click selects the node type in the palette
            if (ctx.hitRegion({nx, ny, 100.f, 44.f}, false)) {
                // Find the matching palette index
                for (int pi = 0; pi < kPaletteCount; ++pi) {
                    if (kNodeTypes[pi] == std::string(kGraphNodes[i].label)) {
                        m_viewSelectedNode = (m_viewSelectedNode == pi) ? -1 : pi;
                        break;
                    }
                }
            }
        }
        // Wires between nodes (simple horizontal segments)
        float n0x = gvpx + graphW * 0.10f;
        float n0y = gvpy + gvph * 0.25f + 32.f;
        ctx.ui.drawRect({n0x + 105.f, n0y, 55.f, 1.f}, ctx.kTextMuted);
        ctx.ui.drawRect({n0x + 265.f, gvpy + gvph * 0.20f + 32.f, 55.f, 1.f}, ctx.kTextMuted);
        ctx.ui.drawRect({n0x + 265.f, gvpy + gvph * 0.50f + 32.f, 55.f, 1.f}, ctx.kTextMuted);
    }

    // Compile / run buttons
    if (ctx.drawButton(ctx.x + nodesW + graphW - 100.f, ctx.y + 28.f, 46.f, 18.f, "Compile")) {
        if (ctx.shell)
            (void)ctx.shell->commandBus().execute("logic.compile");
    }
    if (ctx.drawButton(ctx.x + nodesW + graphW - 50.f, ctx.y + 28.f, 42.f, 18.f, "Run")) {
        if (ctx.shell)
            (void)ctx.shell->commandBus().execute("logic.run");
    }

    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + nodesW + 8.f, ctx.y + ctx.h - 20.f,
                        "* unsaved", ctx.kRed);
    }

    // ── Properties panel ──────────────────────────────────────────
    ctx.drawPanel(ctx.x + nodesW + graphW, ctx.y, propW, ctx.h, "Properties");
    const float px = ctx.x + nodesW + graphW;
    if (m_viewSelectedNode >= 0 && m_viewSelectedNode < kPaletteCount) {
        ctx.ui.drawText(px + 8.f, ctx.y + 30.f, kNodeTypes[m_viewSelectedNode], ctx.kTextPrimary);
        ctx.ui.drawText(px + 8.f, ctx.y + 50.f, "Type: Logic Node", ctx.kTextSecond);
        ctx.ui.drawText(px + 8.f, ctx.y + 68.f, "Inputs: 1", ctx.kTextMuted);
        ctx.ui.drawText(px + 8.f, ctx.y + 86.f, "Outputs: 2", ctx.kTextMuted);
    } else {
        ctx.ui.drawText(px + 8.f, ctx.y + 30.f, "Node Parameters", ctx.kTextSecond);
        ctx.ui.drawText(px + 8.f, ctx.y + 50.f, "Select a node", ctx.kTextMuted);
    }
}

} // namespace NF
