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

void SceneEditorTool::markDirty() const {
    m_stats.isDirty = true;
}

void SceneEditorTool::clearDirty() const {
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
        uint32_t visibleEntityCount = m_stats.entityCount < static_cast<uint32_t>(kEntityCount)
                       ? m_stats.entityCount
                       : static_cast<uint32_t>(kEntityCount);
        for (uint32_t i = 0; i < visibleEntityCount; ++i) {
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

        // Darker interior background so the viewport area is visually distinct
        // from the panel chrome and surrounding panels.
        ctx.ui.drawRect({vpx + 1.f, vpy + 1.f, viewW - 2.f, vph - 2.f}, 0x171717FF);

        // Grid overlay — visible lines at 38px intervals.
        for (float gx = vpx; gx < vpx + viewW; gx += 38.f)
            ctx.ui.drawRect({gx, vpy, 1.f, vph}, 0x363636FF);
        for (float gy = vpy; gy < vpy + vph; gy += 38.f)
            ctx.ui.drawRect({vpx, gy, viewW, 1.f}, 0x363636FF);

        // Emphasise the centre cross-lines
        const float vcx = vpx + viewW * 0.5f;
        const float vcy = vpy + vph * 0.5f;
        ctx.ui.drawRect({vcx, vpy, 1.f, vph}, 0x444444FF);
        ctx.ui.drawRect({vpx, vcy, viewW, 1.f}, 0x444444FF);

        // World axes overlay in the bottom-left corner of the viewport.
        const float axX = vpx + 12.f;
        const float axY = vpy + vph - 40.f;
        ctx.ui.drawRect({axX,        axY + 12.f, 30.f, 2.f}, 0xE05050FF); // X axis (red)
        ctx.ui.drawRect({axX + 12.f, axY,         2.f, 30.f}, 0x4EC94EFF); // Y axis (green)
        ctx.ui.drawText(axX + 34.f, axY + 10.f, "X", 0xE05050FF);
        ctx.ui.drawText(axX + 10.f, axY - 4.f,  "Y", 0x4EC94EFF);

        // ── Entity dot markers ────────────────────────────────────
        // Pseudo top-down positions spread around the viewport centre.
        // Each entity is represented as a coloured square with its name.
        struct EntityLayout { float rx; float ry; uint32_t color; };
        static constexpr EntityLayout kEntityLayout[] = {
            { 0.50f, 0.40f, 0xFFCC44FF }, // Camera_Main     — gold
            { 0.55f, 0.30f, 0xAADDFFFF }, // DirectionalLight — light-blue
            { 0.50f, 0.50f, 0x44DD88FF }, // Player          — green
            { 0.38f, 0.55f, 0x8888AAFF }, // Environment     — grey-blue
            { 0.65f, 0.35f, 0xDDAAFFFF }, // SkyDome         — lavender
        };
        uint32_t visibleEntityCount = m_stats.entityCount < static_cast<uint32_t>(kEntityCount)
                       ? m_stats.entityCount
                       : static_cast<uint32_t>(kEntityCount);
        for (uint32_t i = 0; i < visibleEntityCount; ++i) {
            EntityID eid = static_cast<EntityID>(i + 1);
            bool selected = (eid == primarySel);

            float ex = vpx + kEntityLayout[i].rx * viewW;
            float ey = vpy + kEntityLayout[i].ry * vph;
            float sz = selected ? 10.f : 7.f;

            // Selection halo
            if (selected)
                ctx.ui.drawRect({ex - sz - 2.f, ey - sz - 2.f,
                                 sz * 2.f + 4.f, sz * 2.f + 4.f}, ctx.kAccentBlue);

            ctx.ui.drawRect({ex - sz, ey - sz, sz * 2.f, sz * 2.f}, kEntityLayout[i].color);
            ctx.ui.drawText(ex + sz + 4.f, ey - 6.f, kEntityNames[i], ctx.kTextSecond);
        }

        // ── 2D gizmo overlay for the selected entity ─────────────
        // Drawn when a transform mode is active and an entity is selected.
        if (liveSelCount > 0 && primarySel != INVALID_ENTITY
                && primarySel <= static_cast<EntityID>(kEntityCount)) {
            uint32_t pidx = primarySel - 1u;
            float gx2 = vpx + kEntityLayout[pidx].rx * viewW;
            float gy2 = vpy + kEntityLayout[pidx].ry * vph;
            constexpr float kArm = 32.f;

            switch (m_editMode) {
            case SceneEditMode::Translate:
                // Move gizmo — red (X) and green (Y) arrows
                ctx.ui.drawRect({gx2,        gy2 - 1.f, kArm,  3.f}, 0xE05050FF); // +X
                ctx.ui.drawRect({gx2 - 3.f,  gy2 - kArm, 3.f, kArm}, 0x4EC94EFF); // +Y
                ctx.ui.drawText(gx2 + kArm + 2.f, gy2 - 8.f,  "X", 0xE05050FF);
                ctx.ui.drawText(gx2 + 2.f,  gy2 - kArm - 14.f, "Y", 0x4EC94EFF);
                break;
            case SceneEditMode::Rotate:
                // Rotate gizmo — arc approximated with thin rect ring
                ctx.ui.drawRectOutline({gx2 - kArm, gy2 - kArm,
                                        kArm * 2.f, kArm * 2.f}, 0xDDAA44FF, 2.f);
                ctx.ui.drawText(gx2 - 10.f, gy2 - kArm - 14.f, "Rot", 0xDDAA44FF);
                break;
            case SceneEditMode::Scale:
                // Scale gizmo — square handles at axis ends
                ctx.ui.drawRect({gx2,           gy2 - 1.f, kArm - 4.f, 3.f}, 0xE05050FF);
                ctx.ui.drawRect({gx2 + kArm - 4.f, gy2 - 4.f, 8.f, 8.f}, 0xE05050FF);
                ctx.ui.drawRect({gx2 - 3.f, gy2 - kArm, 3.f, kArm - 4.f}, 0x4EC94EFF);
                ctx.ui.drawRect({gx2 - 4.f, gy2 - kArm - 4.f, 8.f, 8.f}, 0x4EC94EFF);
                ctx.ui.drawText(gx2 + kArm + 2.f, gy2 - 8.f, "X", 0xE05050FF);
                ctx.ui.drawText(gx2 + 2.f, gy2 - kArm - 14.f, "Y", 0x4EC94EFF);
                break;
            default:
                break;
            }
        }

        if (m_stats.entityCount == 0) {
            ctx.ui.drawText(vpx + 16.f, vpy + (vph - 14.f) * 0.5f,
                            "No scene loaded — add entities to begin",
                            ctx.kTextMuted);
        }

        // Edit mode toolbar buttons along the top of the viewport.
        // cmdSuffix is the exact scene.set_mode.<suffix> command registered
        // in WorkspaceShell.  "Move" maps to "translate" (not "move").
        static constexpr struct {
            const char* label;
            SceneEditMode mode;
            const char* cmdSuffix;
        } kModes[] = {
            {"Select", SceneEditMode::Select,    "select"},
            {"Move",   SceneEditMode::Translate, "translate"},
            {"Rotate", SceneEditMode::Rotate,    "rotate"},
            {"Scale",  SceneEditMode::Scale,     "scale"},
        };
        float btnX = vpx + 8.f;
        float btnY = vpy + 4.f;
        for (const auto& m : kModes) {
            bool active = (m_editMode == m.mode);
            float btnW = static_cast<float>(std::strlen(m.label)) * 7.f + 16.f;
            uint32_t bg = active ? ctx.kAccentBlue : ctx.kButtonBg;
            if (ctx.drawButton(btnX, btnY, btnW, 18.f, m.label, bg)) {
                // Set edit mode directly so the button highlights immediately.
                m_editMode = m.mode;
                // Also notify the command bus for keybinding / palette purposes.
                if (ctx.shell) {
                    std::string cmd = std::string("scene.set_mode.") + m.cmdSuffix;
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
            // Show the name and entity-type-specific components.
            const char* selName = (primarySel != INVALID_ENTITY && primarySel <= kEntityCount)
                                  ? kEntityNames[primarySel - 1u] : "Entity";
            ctx.ui.drawText(ix + 8.f, ctx.y + 50.f, selName, ctx.kTextPrimary);
            ctx.ui.drawRect({ix + 4.f, ctx.y + 64.f, inspW - 8.f, 1.f}, ctx.kBorder);

            // Resolve entity index for mutable state access — guard against
            // INVALID_ENTITY or out-of-range values to avoid modifying entity 0.
            const bool validEntity = (primarySel != INVALID_ENTITY && primarySel >= 1u
                                      && primarySel <= kMaxEntities);
            const uint32_t eidx = validEntity ? (primarySel - 1u) : 0u;
            auto& xform = m_entityTransforms[eidx];
            const float sliderW = inspW - 16.f;

            // Transform component (all entities share this) — interactive sliders
            ctx.ui.drawText(ix + 8.f, ctx.y + 70.f, "Transform", ctx.kTextSecond);
            ctx.ui.drawRect({ix + 4.f, ctx.y + 82.f, inspW - 8.f, 1.f}, 0x303030FF);

            float iy = ctx.y + 88.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Pos X", xform.pos[0], -100.f, 100.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Pos Y", xform.pos[1], -100.f, 100.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Pos Z", xform.pos[2], -100.f, 100.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Rot X", xform.rot[0], -180.f, 180.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Rot Y", xform.rot[1], -180.f, 180.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Rot Z", xform.rot[2], -180.f, 180.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Scl X", xform.scale[0], 0.01f, 10.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Scl Y", xform.scale[1], 0.01f, 10.f))
                markDirty();
            iy += 18.f;
            if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Scl Z", xform.scale[2], 0.01f, 10.f))
                markDirty();
            iy += 22.f;

            // Per-entity component sections (entity-type-aware) — interactive sliders
            if (primarySel == 1) {
                // Camera_Main — Camera component
                ctx.ui.drawRect({ix + 4.f, iy, inspW - 8.f, 1.f}, ctx.kBorder);
                iy += 4.f;
                ctx.ui.drawText(ix + 8.f, iy, "Camera", ctx.kTextSecond);
                ctx.ui.drawRect({ix + 4.f, iy + 14.f, inspW - 8.f, 1.f}, 0x303030FF);
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "FOV",  m_cameraFov, 10.f, 120.f))
                    markDirty();
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Near", m_cameraNear, 0.01f, 10.f))
                    markDirty();
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Far",  m_cameraFar, 10.f, 10000.f))
                    markDirty();
                iy += 18.f;
                ctx.drawStatRow(ix + 8.f, iy, "Projection:", "Persp.");
                iy += 22.f;
            } else if (primarySel == 2) {
                // DirectionalLight — Light component
                ctx.ui.drawRect({ix + 4.f, iy, inspW - 8.f, 1.f}, ctx.kBorder);
                iy += 4.f;
                ctx.ui.drawText(ix + 8.f, iy, "Directional Light", ctx.kTextSecond);
                ctx.ui.drawRect({ix + 4.f, iy + 14.f, inspW - 8.f, 1.f}, 0x303030FF);
                iy += 18.f;
                // Color swatch
                ctx.ui.drawRect({ix + 8.f, iy + 2.f, 16.f, 12.f}, 0xFFFAE0FFu);
                ctx.ui.drawRectOutline({ix + 8.f, iy + 2.f, 16.f, 12.f}, ctx.kBorder, 1.f);
                ctx.ui.drawText(ix + 28.f, iy, "Color", ctx.kTextSecond);
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Inten", m_lightIntensity, 0.f, 10.f))
                    markDirty();
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Angle", m_lightAngle, 0.f, 90.f))
                    markDirty();
                iy += 18.f;
                ctx.drawStatRow(ix + 8.f, iy, "Shadows:", "Hard");
                iy += 22.f;
            } else if (primarySel == 3) {
                // Player — RigidBody + Collider + CharacterController
                ctx.ui.drawRect({ix + 4.f, iy, inspW - 8.f, 1.f}, ctx.kBorder);
                iy += 4.f;
                ctx.ui.drawText(ix + 8.f, iy, "Rigidbody", ctx.kTextSecond);
                ctx.ui.drawRect({ix + 4.f, iy + 14.f, inspW - 8.f, 1.f}, 0x303030FF);
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Mass", m_playerMass, 0.1f, 500.f))
                    markDirty();
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Drag", m_playerDrag, 0.f, 1.f))
                    markDirty();
                iy += 18.f;
                ctx.drawStatRow(ix + 8.f, iy, "Kinematic:", "No");
                iy += 20.f;
                ctx.ui.drawRect({ix + 4.f, iy, inspW - 8.f, 1.f}, ctx.kBorder);
                iy += 4.f;
                ctx.ui.drawText(ix + 8.f, iy, "Capsule Collider", ctx.kTextSecond);
                ctx.ui.drawRect({ix + 4.f, iy + 14.f, inspW - 8.f, 1.f}, 0x303030FF);
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Rad.", m_colliderRadius, 0.1f, 5.f))
                    markDirty();
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Hght", m_colliderHeight, 0.5f, 5.f))
                    markDirty();
                iy += 18.f;
                ctx.drawStatRow(ix + 8.f, iy, "Material:", "Default");
                iy += 22.f;
            } else if (primarySel == 4) {
                // Environment
                ctx.ui.drawRect({ix + 4.f, iy, inspW - 8.f, 1.f}, ctx.kBorder);
                iy += 4.f;
                ctx.ui.drawText(ix + 8.f, iy, "Environment Volume", ctx.kTextSecond);
                ctx.ui.drawRect({ix + 4.f, iy + 14.f, inspW - 8.f, 1.f}, 0x303030FF);
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Ambi", m_envAmbient, 0.f, 1.f))
                    markDirty();
                iy += 18.f;
                ctx.drawStatRow(ix + 8.f, iy, "Fog:", "Linear");
                iy += 18.f;
                if (ctx.drawSlider(ix + 8.f, iy, sliderW, "Start", m_envFogStart, 0.f, 500.f))
                    markDirty();
                iy += 22.f;
            } else if (primarySel == 5) {
                // SkyDome — Sky + Atmosphere component
                ctx.ui.drawRect({ix + 4.f, iy, inspW - 8.f, 1.f}, ctx.kBorder);
                iy += 4.f;
                ctx.ui.drawText(ix + 8.f, iy, "Sky & Atmosphere", ctx.kTextSecond);
                ctx.ui.drawRect({ix + 4.f, iy + 14.f, inspW - 8.f, 1.f}, 0x303030FF);
                iy += 18.f;
                ctx.drawStatRow(ix + 8.f, iy,      "Preset:",    "Clear");
                ctx.drawStatRow(ix + 8.f, iy + 18.f, "TimeOfDay:", "12:00");
                ctx.drawStatRow(ix + 8.f, iy + 36.f, "Clouds:",    "None");
                iy += 56.f;
            }

            // Deselect button
            if (iy + 22.f < ctx.y + ctx.h - 4.f) {
                if (ctx.drawButton(ix + 8.f, ctx.y + ctx.h - 30.f, inspW - 16.f, 20.f,
                                   "Deselect All")) {
                    if (ctx.shell)
                        ctx.shell->selectionService().clearSelection();
                }
            }
        }
    }
}

} // namespace NF
