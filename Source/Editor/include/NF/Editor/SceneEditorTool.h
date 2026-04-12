#pragma once
// NF::Editor — SceneEditorTool: primary scene/world editing tool.
//
// Implements NF::IHostedTool for the Scene Editor, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Scene Editor hosts:
//   Shared panels: viewport, outliner/hierarchy, inspector, console
//   Commands:      scene.create_entity, scene.delete_entity,
//                  scene.duplicate_entity, scene.save_scene,
//                  scene.enter_play, scene.exit_play
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include <string>

namespace NF {

// ── Scene editing mode ────────────────────────────────────────────

enum class SceneEditMode : uint8_t {
    Select,    // selection / inspection
    Translate, // gizmo: move
    Rotate,    // gizmo: rotate
    Scale,     // gizmo: scale
    Paint,     // foliage / terrain paint
    Play,      // PIE — play-in-editor
};

inline const char* sceneEditModeName(SceneEditMode m) {
    switch (m) {
        case SceneEditMode::Select:    return "Select";
        case SceneEditMode::Translate: return "Translate";
        case SceneEditMode::Rotate:    return "Rotate";
        case SceneEditMode::Scale:     return "Scale";
        case SceneEditMode::Paint:     return "Paint";
        case SceneEditMode::Play:      return "Play";
    }
    return "Unknown";
}

// ── Scene Editor statistics ───────────────────────────────────────

struct SceneEditorStats {
    uint32_t entityCount      = 0; // entities in the active scene
    uint32_t selectionCount   = 0; // currently selected entities
    float    lastFrameMs      = 0.0f;
    bool     isDirty          = false; // unsaved changes
};

// ── SceneEditorTool ───────────────────────────────────────────────

class SceneEditorTool final : public IHostedTool, public IViewportSceneProvider {
public:
    static constexpr const char* kToolId = "workspace.scene_editor";

    SceneEditorTool();
    ~SceneEditorTool() override = default;

    // ── IHostedTool identity ──────────────────────────────────────
    [[nodiscard]] const HostedToolDescriptor& descriptor() const override { return m_descriptor; }
    [[nodiscard]] const std::string& toolId()              const override { return m_descriptor.toolId; }

    // ── IHostedTool lifecycle ─────────────────────────────────────
    bool initialize() override;
    void shutdown()   override;
    void activate()   override;
    void suspend()    override;
    void update(float dt) override;

    [[nodiscard]] HostedToolState state() const override { return m_state; }

    // ── IHostedTool input hook ────────────────────────────────────
    void onAttachInput(const InputSystem* input) override { m_input = input; }
    void onDetachInput() override                        { m_input = nullptr; }

    // ── IViewportSceneProvider ────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle handle,
                                    const ViewportSlot& slot) override;

    // ── Viewport slot access ──────────────────────────────────────
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

    /// Attach the workspace viewport manager so this tool can request/release slots.
    /// Call before activate() (e.g. from WorkspaceShell::initialize()).
    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }

    /// Attach a scene provider so provideScene() delegates to it (Phase D wiring).
    /// Pass nullptr to detach and revert to the default stub scene state.
    /// NovaForgePreviewRuntime implements IViewportSceneProvider — pass it directly.
    void attachSceneProvider(IViewportSceneProvider* provider) {
        m_sceneProvider = provider;
    }

    [[nodiscard]] IViewportSceneProvider* sceneProvider() const {
        return m_sceneProvider;
    }

    // ── Project adapter hooks ─────────────────────────────────────
    void onProjectLoaded(const std::string& projectId) override;
    void onProjectUnloaded() override;

    // ── Scene Editor interface ────────────────────────────────────

    [[nodiscard]] SceneEditMode    editMode() const { return m_editMode; }
    void                           setEditMode(SceneEditMode mode);

    [[nodiscard]] const SceneEditorStats& stats()   const { return m_stats; }
    [[nodiscard]] bool                    isDirty() const { return m_stats.isDirty; }
    void                                  markDirty();
    void                                  clearDirty();

    // Entity selection (counts only — scene data lives in the runtime)
    [[nodiscard]] uint32_t selectionCount() const { return m_stats.selectionCount; }
    void setSelectionCount(uint32_t count);

    // Scene entity count (updated each frame from scene runtime)
    void setEntityCount(uint32_t count);

    // ── Render contract ───────────────────────────────────────────
    // Renders: Hierarchy | 3D Viewport | Inspector — three-column layout.
    // Status bar row shows mode, dirty flag, entity/selection counts.
    void renderToolView(const ToolViewRenderContext& ctx) const override;

private:
    HostedToolDescriptor     m_descriptor;
    HostedToolState          m_state      = HostedToolState::Unloaded;
    SceneEditMode            m_editMode   = SceneEditMode::Select;
    SceneEditorStats         m_stats;
    std::string              m_activeProjectId;

    // Viewport slot owned by this tool while Active
    ViewportHandle           m_viewportHandle = kInvalidViewportHandle;
    WorkspaceViewportManager* m_viewportMgr  = nullptr;

    // Optional scene provider — when set, provideScene() delegates to it (Phase D)
    IViewportSceneProvider*   m_sceneProvider = nullptr;

    // Input pointer injected via onAttachInput()
    const InputSystem*       m_input          = nullptr;

    void buildDescriptor();
};

} // namespace NF
