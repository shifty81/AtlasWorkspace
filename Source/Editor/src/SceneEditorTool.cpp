// NF::Editor — SceneEditorTool implementation.
//
// First real NF::IHostedTool from Phase 3 consolidation.
// Replaces the former pattern of header-only V1 stubs with a real
// lifecycle-managed tool registered with WorkspaceShell via ToolRegistry.

#include "NF/Editor/SceneEditorTool.h"

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

ViewportSceneState SceneEditorTool::provideScene(ViewportHandle /*handle*/,
                                                  const ViewportSlot& slot) {
    ViewportSceneState state;
    state.hasContent  = (m_stats.entityCount > 0);
    state.entityCount = m_stats.entityCount;
    state.clearColor  = 0x1E1E1EFFu; // dark viewport background

    // Push the tool camera into the slot camera descriptor.
    // ViewportHostRegistry::setCamera() is called via the manager so the frame
    // loop picks up the updated camera on the same tick.
    if (m_viewportMgr && slot.handle != kInvalidViewportHandle) {
        ViewportCameraDescriptor cam = slot.camera; // start from existing slot camera
        // Camera position / orientation are driven by m_input (if attached).
        // For now the descriptor is passed through unmodified; the fly-cam
        // update in update() will write to it in a future integration step.
        state.overrideCamera = false; // camera update deferred until fly-cam wired
    }

    return state;
}

void SceneEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats = {};
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

} // namespace NF
