// NF::Editor — MaterialEditorTool implementation.
//
// Third real NF::IHostedTool from Phase 3 consolidation.
// Manages material and shader-graph authoring within AtlasWorkspace.

#include "NF/Editor/MaterialEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <cstdio>

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

ViewportSceneState MaterialEditorTool::provideScene(ViewportHandle handle,
                                                     const ViewportSlot& slot) {
    if (m_materialPreviewProvider)
        return m_materialPreviewProvider->provideScene(handle, slot);

    ViewportSceneState state;
    state.hasContent  = false;
    state.entityCount = 0;
    state.clearColor  = 0x222222FFu;
    return state;
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

void MaterialEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Material Graph (35%) | Viewport Preview (40%) | Properties (25%)
    const float graphW   = ctx.w * 0.35f;
    const float previewW = ctx.w * 0.40f;
    const float propW    = ctx.w - graphW - previewW;

    // ── Material Graph ────────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, graphW, ctx.h, "Material Graph");
    {
        char nodeBuf[32];
        std::snprintf(nodeBuf, sizeof(nodeBuf), "%u nodes", m_stats.nodeCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, nodeBuf, ctx.kTextSecond);
        char texBuf[32];
        std::snprintf(texBuf, sizeof(texBuf), "%u texture slots", m_stats.textureSlotCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 48.f, texBuf, ctx.kTextSecond);
        if (m_stats.isDirty) {
            ctx.drawStatusPill(ctx.x + 8.f, ctx.y + 70.f, "unsaved", ctx.kRed);
        }
    }

    // ── Viewport Preview ──────────────────────────────────────────
    const char* assetLabel = m_openAssetPath.empty() ? "No material open" : nullptr;
    ctx.drawPanel(ctx.x + graphW, ctx.y, previewW, ctx.h, "Viewport Preview", assetLabel);
    if (!m_openAssetPath.empty()) {
        std::string label = "Asset: " + m_openAssetPath;
        if (label.size() > 40) label = label.substr(0, 37) + "...";
        ctx.ui.drawText(ctx.x + graphW + 8.f, ctx.y + 30.f, label, ctx.kTextSecond);
    }

    // ── Properties panel ──────────────────────────────────────────
    ctx.drawPanel(ctx.x + graphW + previewW, ctx.y, propW, ctx.h, "Properties");
    ctx.ui.drawText(ctx.x + graphW + previewW + 8.f, ctx.y + 30.f,
                    "Material Parameters", ctx.kTextSecond);
    ctx.drawStatRow(ctx.x + graphW + previewW + 8.f, ctx.y + 50.f, "Nodes:", "");
    {
        char nb[16]; std::snprintf(nb, sizeof(nb), "%u", m_stats.nodeCount);
        ctx.ui.drawText(ctx.x + graphW + previewW + 118.f, ctx.y + 50.f, nb, ctx.kTextPrimary);
    }
}

} // namespace NF
