// NF::Editor — AssetEditorTool implementation.
//
// Second real NF::IHostedTool from Phase 3 consolidation.
// Manages the primary content browser and asset import/management
// workflow within the AtlasWorkspace.

#include "NF/Editor/AssetEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/AssetCatalog.h"
#include <cstdio>

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

ViewportSceneState AssetEditorTool::provideScene(ViewportHandle handle,
                                                  const ViewportSlot& slot) {
    if (m_assetPreviewProvider)
        return m_assetPreviewProvider->provideScene(handle, slot);

    ViewportSceneState state;
    state.hasContent  = false;
    state.entityCount = 0;
    state.clearColor  = 0x1A1A1AFFu;
    return state;
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

void AssetEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Content Browser (55%) | Preview (25%) | Inspector (20%)
    const float browserW = ctx.w * 0.55f;
    const float previewW = ctx.w * 0.25f;
    const float inspW    = ctx.w - browserW - previewW;

    // ── Live catalog data ─────────────────────────────────────────
    // Pull counts directly from the shell's AssetCatalog so the browser
    // reflects real disk content discovered during project load.
    uint32_t totalCount    = m_stats.totalAssetCount;
    uint32_t filteredCount = m_stats.filteredAssetCount;

    if (ctx.shell) {
        const AssetCatalog& catalog = ctx.shell->assetCatalog();
        totalCount = static_cast<uint32_t>(catalog.count());

        // Apply filter mode to derive the filtered count.
        if (m_filterMode == AssetFilterMode::All) {
            filteredCount = totalCount;
        } else {
            AssetTypeTag filterTag = AssetTypeTag::Custom;
            switch (m_filterMode) {
                case AssetFilterMode::Textures:  filterTag = AssetTypeTag::Texture;   break;
                case AssetFilterMode::Materials: filterTag = AssetTypeTag::Material;  break;
                case AssetFilterMode::Meshes:    filterTag = AssetTypeTag::Mesh;      break;
                case AssetFilterMode::Audio:     filterTag = AssetTypeTag::Audio;     break;
                case AssetFilterMode::Scripts:   filterTag = AssetTypeTag::Script;    break;
                case AssetFilterMode::Prefabs:   filterTag = AssetTypeTag::Prefab;    break;
                default:                         filterTag = AssetTypeTag::Custom;    break;
            }
            filteredCount = static_cast<uint32_t>(catalog.countByType(filterTag));
        }
    }

    // ── Content Browser ───────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, browserW, ctx.h, "Content Browser");
    // Filter mode pill
    ctx.drawStatusPill(ctx.x + 8.f, ctx.y + 30.f,
                       assetFilterModeName(m_filterMode), ctx.kAccentBlue);
    // Asset counts
    {
        char buf[48];
        std::snprintf(buf, sizeof(buf), "%u / %u assets", filteredCount, totalCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 54.f, buf, ctx.kTextSecond);
    }

    // Asset type summary tiles (one per type that has entries in the catalog)
    if (ctx.shell && totalCount > 0) {
        const AssetCatalog& catalog = ctx.shell->assetCatalog();

        struct TypeEntry { const char* label; AssetTypeTag tag; };
        static constexpr TypeEntry kTypes[] = {
            {"Mesh",      AssetTypeTag::Mesh},
            {"Texture",   AssetTypeTag::Texture},
            {"Material",  AssetTypeTag::Material},
            {"Script",    AssetTypeTag::Script},
            {"Audio",     AssetTypeTag::Audio},
            {"Prefab",    AssetTypeTag::Prefab},
            {"Scene",     AssetTypeTag::Scene},
            {"Shader",    AssetTypeTag::Shader},
            {"Animation", AssetTypeTag::Animation},
        };

        float tx = ctx.x + 8.f;
        float ty = ctx.y + 74.f;
        for (const auto& te : kTypes) {
            size_t cnt = catalog.countByType(te.tag);
            if (cnt == 0) continue;
            if (tx + 72.f > ctx.x + browserW - 4.f) { tx = ctx.x + 8.f; ty += 64.f; }
            if (ty + 60.f > ctx.y + ctx.h - 4.f) break;

            bool sel = (m_filterMode != AssetFilterMode::All &&
                        assetFilterModeName(m_filterMode) ==
                        assetTypeTagName(te.tag));
            ctx.ui.drawRect({tx, ty, 68.f, 58.f},
                            sel ? 0x2A3A5AFF : 0x333333FF);
            ctx.ui.drawRectOutline({tx, ty, 68.f, 58.f},
                                   sel ? ctx.kAccentBlue : ctx.kBorder, 1.f);
            // Type label
            ctx.ui.drawText(tx + 4.f, ty + 20.f, te.label, ctx.kTextSecond);
            // Count badge
            char countBuf[16];
            std::snprintf(countBuf, sizeof(countBuf), "%zu", cnt);
            ctx.ui.drawText(tx + 4.f, ty + 38.f, countBuf, ctx.kTextPrimary);
            tx += 76.f;
        }
    } else if (totalCount == 0) {
        const char* hint = ctx.shell && ctx.shell->hasProject()
                         ? "No assets found in project content roots."
                         : "Open a project to browse assets.";
        float hx = ctx.x + (browserW - static_cast<float>(std::strlen(hint)) * 7.f) * 0.5f;
        float hy = ctx.y + 22.f + (ctx.h - 22.f - 14.f) * 0.5f;
        ctx.ui.drawText(hx > ctx.x + 8.f ? hx : ctx.x + 8.f, hy, hint, ctx.kTextMuted);
    }

    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + ctx.h - 20.f,
                        "* pending import", ctx.kRed);
    }

    // ── Preview panel ─────────────────────────────────────────────
    const char* previewHint = totalCount > 0 ? nullptr : "No asset selected";
    ctx.drawPanel(ctx.x + browserW, ctx.y, previewW, ctx.h, "Preview", previewHint);

    // ── Inspector panel ───────────────────────────────────────────
    ctx.drawPanel(ctx.x + browserW + previewW, ctx.y, inspW, ctx.h, "Inspector");
    ctx.ui.drawText(ctx.x + browserW + previewW + 8.f, ctx.y + 30.f,
                    "Asset Properties", ctx.kTextSecond);
    if (totalCount > 0) {
        char buf[48];
        std::snprintf(buf, sizeof(buf), "%u total assets", totalCount);
        ctx.ui.drawText(ctx.x + browserW + previewW + 8.f, ctx.y + 48.f,
                        buf, ctx.kTextMuted);
    }
}

} // namespace NF
