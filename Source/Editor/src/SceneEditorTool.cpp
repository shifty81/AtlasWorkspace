// NF::Editor — SceneEditorTool implementation.
//
// First real NF::IHostedTool from Phase 3 consolidation.
// Replaces the former pattern of header-only V1 stubs with a real
// lifecycle-managed tool registered with WorkspaceShell via ToolRegistry.

#include "NF/Editor/SceneEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <cctype>
#include <cstdio>
#include <cstring>

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

        // Seed a minimal default scene if no entities are present so the
        // viewport always shows content rather than an empty-state message.
        if (m_stats.entityCount == 0) {
            // Default scene: camera + directional light + environment root.
            m_stats.entityCount = 3;
        }

        // Request a viewport slot from the workspace viewport manager.
        // A default 1280×720 bounds is used here; the real bounds are applied
        // via the ViewportPanel resize callback (WorkspaceViewportBridge) on
        // the first paint cycle.
        if (m_viewportMgr && m_viewportHandle == kInvalidViewportHandle) {
            m_viewportHandle = m_viewportMgr->requestViewport(
                kToolId, {0.f, 0.f, 1280.f, 720.f});
            if (m_viewportHandle != kInvalidViewportHandle) {
                m_viewportMgr->registerSceneProvider(kToolId, this);
                m_viewportMgr->activateViewport(m_viewportHandle);
            }
        }
    }
}

void SceneEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;

        // Release the viewport slot so it can be reused by another tool.
        if (m_viewportMgr && m_viewportHandle != kInvalidViewportHandle) {
            m_viewportMgr->unregisterSceneProvider(kToolId);
            m_viewportMgr->releaseViewport(m_viewportHandle);
            m_viewportHandle = kInvalidViewportHandle;
        }
    }
}

void SceneEditorTool::update(float dt) {
    if (m_state != HostedToolState::Active) return;
    m_stats.lastFrameMs = dt * 1000.0f;

    // Submit gizmo commands for each selected entity when in a transform mode.
    // Position data is currently a centroid placeholder (0,0,0) until the scene
    // runtime feeds per-entity world positions via a future entity query API.
    if (m_viewportMgr && m_viewportHandle != kInvalidViewportHandle
            && m_stats.selectionCount > 0) {
        bool isTransformMode = (m_editMode == SceneEditMode::Translate
                             || m_editMode == SceneEditMode::Rotate
                             || m_editMode == SceneEditMode::Scale);
        if (isTransformMode) {
            GizmoType gizmoType = GizmoType::Translate;
            if (m_editMode == SceneEditMode::Rotate) gizmoType = GizmoType::Rotate;
            if (m_editMode == SceneEditMode::Scale)  gizmoType = GizmoType::Scale;

            GizmoDrawCommand cmd;
            cmd.axis     = GizmoAxis::XYZ;
            cmd.type     = gizmoType;
            cmd.position = {0.f, 0.f, 0.f}; // placeholder — scene runtime provides real pos
            cmd.size     = 1.f;
            m_viewportMgr->addGizmo(cmd);
        }
    }
}

ViewportSceneState SceneEditorTool::provideScene(ViewportHandle handle,
                                                  const ViewportSlot& slot) {
    // Delegate to the attached scene provider when one is wired (Phase D).
    if (m_sceneProvider)
        return m_sceneProvider->provideScene(handle, slot);

    ViewportSceneState state;
    state.hasContent  = (m_stats.entityCount > 0);
    state.entityCount = m_stats.entityCount;
    state.clearColor  = 0x1E1E1EFFu; // dark viewport background

    if (m_viewportMgr && slot.handle != kInvalidViewportHandle) {
        // Camera update deferred until fly-cam wired
        state.overrideCamera = false;
    }

    return state;
}

void SceneEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    // Seed a default starter scene with a camera, light, player, environment,
    // and sky so the viewport is populated immediately on project open.
    m_stats = {};
    m_stats.entityCount = 5;
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

void SceneEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Hierarchy (20%) | 3D Viewport (58%) | Inspector (22%)
    const float hierW = ctx.w * 0.20f;
    const float inspW = ctx.w * 0.22f;
    const float viewW = ctx.w - hierW - inspW;

    // Resolve live selection count from SelectionService when shell is wired.
    uint32_t liveSelCount = m_stats.selectionCount;
    EntityID primarySel   = INVALID_ENTITY;
    if (ctx.shell) {
        const SelectionService& sel = ctx.shell->selectionService();
        liveSelCount = static_cast<uint32_t>(sel.selectionCount());
        primarySel   = sel.primarySelection();
    }

    static const char* kEntityNames[] = {
        "Camera_Main", "DirectionalLight", "Player", "Environment", "SkyDome"
    };
    static constexpr size_t kEntityCount = std::size(kEntityNames);

    // ── Hierarchy panel ──────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, hierW, ctx.h, "Hierarchy");
    {
        char entBuf[32];
        std::snprintf(entBuf, sizeof(entBuf), "%u entities", m_stats.entityCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, entBuf, ctx.kTextSecond);

        // Entity list — each entity has an implicit ID equal to its index + 1.
        // Rows are hit-tested so clicking selects the entity via SelectionService.
        float ey = ctx.y + 48.f;
        uint32_t limit = m_stats.entityCount < static_cast<uint32_t>(kEntityCount)
                       ? m_stats.entityCount
                       : static_cast<uint32_t>(kEntityCount);
        for (uint32_t i = 0; i < limit; ++i) {
            EntityID eid = static_cast<EntityID>(i + 1);
            bool selected = (eid == primarySel);
            bool hovered  = ctx.isHovered({ctx.x + 4.f, ey - 2.f, hierW - 8.f, 18.f});

            uint32_t bg = selected ? 0x1A3A6AFF : (hovered ? 0x2A2A3AFF : 0x00000000u);
            if (bg) ctx.ui.drawRect({ctx.x + 4.f, ey - 2.f, hierW - 8.f, 18.f}, bg);

            // Left accent stripe
            ctx.ui.drawRect({ctx.x + 4.f, ey - 2.f, 2.f, 18.f},
                            selected ? ctx.kAccentBlue : 0x404040FF);
            ctx.ui.drawText(ctx.x + 16.f, ey, kEntityNames[i],
                            selected ? ctx.kTextPrimary : ctx.kTextSecond);

            // Click: select entity exclusively via SelectionService
            if (ctx.hitRegion({ctx.x + 4.f, ey - 2.f, hierW - 8.f, 18.f}, false)) {
                m_viewSelectedEntity = static_cast<int>(i);
                if (ctx.shell)
                    ctx.shell->selectionService().selectExclusive(eid);
            }

            ey += 18.f;
        }
    }

    // ── 3D Viewport panel ────────────────────────────────────────
    const char* viewHint = m_stats.isDirty ? "3D Viewport  [unsaved]" : "3D Viewport";
    ctx.drawPanel(ctx.x + hierW, ctx.y, viewW, ctx.h, viewHint);
    {
        const float vpx = ctx.x + hierW;
        const float vpy = ctx.y + 22.f;
        const float vph = ctx.h - 22.f;
        // Grid overlay — vertical and horizontal lines at 38 px intervals.
        for (float gx = vpx; gx < vpx + viewW; gx += 38.f)
            ctx.ui.drawRect({gx, vpy, 1.f, vph}, 0x2D2D2DFF);
        for (float gy = vpy; gy < vpy + vph; gy += 38.f)
            ctx.ui.drawRect({vpx, gy, viewW, 1.f}, 0x2D2D2DFF);

        // World axes overlay in the bottom-left corner of the viewport.
        const float axX = vpx + 12.f;
        const float axY = vpy + vph - 40.f;
        ctx.ui.drawRect({axX,       axY + 12.f, 24.f, 2.f}, 0xE05050FF); // X axis (red)
        ctx.ui.drawRect({axX + 12.f, axY,        2.f, 24.f}, 0x4EC94EFF); // Y axis (green)
        ctx.ui.drawText(axX + 26.f, axY + 10.f, "X", 0xE05050FF);
        ctx.ui.drawText(axX + 10.f, axY - 2.f,  "Y", 0x4EC94EFF);

        if (m_stats.entityCount == 0) {
            ctx.ui.drawText(vpx + 16.f, vpy + (vph - 14.f) * 0.5f,
                            "No scene loaded — add entities to begin",
                            ctx.kTextMuted);
        }

        // Edit mode toolbar buttons along the top of the viewport
        static constexpr struct { const char* label; SceneEditMode mode; } kModes[] = {
            {"Select", SceneEditMode::Select},
            {"Move",   SceneEditMode::Translate},
            {"Rotate", SceneEditMode::Rotate},
            {"Scale",  SceneEditMode::Scale},
        };
        float btnX = vpx + 8.f;
        float btnY = vpy + 4.f;
        for (const auto& m : kModes) {
            bool active = (m_editMode == m.mode);
            float btnW = static_cast<float>(std::strlen(m.label)) * 7.f + 16.f;
            uint32_t bg = active ? ctx.kAccentBlue : ctx.kButtonBg;
            if (ctx.drawButton(btnX, btnY, btnW, 18.f, m.label, bg)) {
                // Click: switch edit mode via the shell command bus if available
                if (ctx.shell) {
                    std::string cmd = "scene.set_mode.";
                    for (const char* c = m.label; *c; ++c)
                        cmd += static_cast<char>(std::tolower(static_cast<unsigned char>(*c)));
                    (void)ctx.shell->commandBus().execute(cmd);
                }
            }
            btnX += btnW + 4.f;
        }
    }
    // Mode pill (shown at bottom-left)
    ctx.drawStatusPill(ctx.x + hierW + 8.f, ctx.y + ctx.h - 22.f,
                       sceneEditModeName(m_editMode), ctx.kAccentBlue);
    // Dirty indicator
    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + hierW + 90.f, ctx.y + ctx.h - 20.f,
                        "* unsaved", ctx.kRed);
    }
    // Frame time
    {
        char ftBuf[32];
        std::snprintf(ftBuf, sizeof(ftBuf), "%.1f ms", m_stats.lastFrameMs);
        ctx.ui.drawText(ctx.x + hierW + viewW - 60.f, ctx.y + ctx.h - 20.f,
                        ftBuf, ctx.kTextMuted);
    }

    // ── Inspector panel ───────────────────────────────────────────
    const float ix = ctx.x + hierW + viewW;
    ctx.drawPanel(ix, ctx.y, inspW, ctx.h, "Inspector");
    {
        char selBuf[32];
        std::snprintf(selBuf, sizeof(selBuf), "%u selected", liveSelCount);
        ctx.ui.drawText(ix + 8.f, ctx.y + 30.f, selBuf, ctx.kTextSecond);

        if (liveSelCount == 0) {
            ctx.ui.drawText(ix + 8.f, ctx.y + 50.f, "Nothing selected", ctx.kTextMuted);
        } else {
            // Show the name of the primary selected entity.
            if (primarySel != INVALID_ENTITY && primarySel <= kEntityCount) {
                ctx.ui.drawText(ix + 8.f, ctx.y + 50.f,
                                kEntityNames[primarySel - 1u], ctx.kTextPrimary);
            }
            ctx.ui.drawText(ix + 8.f, ctx.y + 68.f, "Transform", ctx.kTextSecond);
            ctx.ui.drawRect({ix + 4.f, ctx.y + 82.f, inspW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(ix + 8.f, ctx.y + 88.f,  "Pos:",   "0, 0, 0");
            ctx.drawStatRow(ix + 8.f, ctx.y + 106.f, "Rot:",   "0, 0, 0");
            ctx.drawStatRow(ix + 8.f, ctx.y + 124.f, "Scale:", "1, 1, 1");

            // Deselect button
            if (ctx.drawButton(ix + 8.f, ctx.y + ctx.h - 30.f, inspW - 16.f, 20.f,
                               "Deselect All")) {
                if (ctx.shell)
                    ctx.shell->selectionService().clearSelection();
            }
        }
    }
}

} // namespace NF
