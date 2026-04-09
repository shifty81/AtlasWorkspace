#pragma once
// NF::ProjectSystemsTool — Host tool for project-specific gameplay panels.
//
// This tool provides a generic host surface where project adapters can
// inject gameplay system panels (economy, inventory, progression, etc.).
// It is the "workspace.project_systems" tool in the host-tool ID scheme.
//
// The workspace asks the loaded project adapter for panel descriptors,
// and this tool hosts them as tabs/sub-panels within a unified surface.
//
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for consolidation plan.
// See Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md for boundary rules.

#include "NF/Editor/IGameProjectAdapter.h"
#include <string>
#include <vector>
#include <memory>

namespace NF {

// ── Host tool IDs ───────────────────────────────────────────────
// Stable identifiers for workspace host tools.
// Project adapters reference these when specifying panel placement.

namespace HostToolId {
    inline constexpr const char* ProjectBrowser     = "workspace.project_browser";
    inline constexpr const char* SceneEditor        = "workspace.scene_editor";
    inline constexpr const char* AssetEditor         = "workspace.asset_editor";
    inline constexpr const char* MaterialEditor      = "workspace.material_editor";
    inline constexpr const char* VisualLogicEditor   = "workspace.visual_logic_editor";
    inline constexpr const char* UIEditor            = "workspace.ui_editor";
    inline constexpr const char* AnimationEditor     = "workspace.animation_editor";
    inline constexpr const char* DataEditor          = "workspace.data_editor";
    inline constexpr const char* BuildTool           = "workspace.build_tool";
    inline constexpr const char* ProjectSystems      = "workspace.project_systems";
    inline constexpr const char* AtlasAI             = "workspace.atlasai";
} // namespace HostToolId

// ── ProjectSystemsTool ──────────────────────────────────────────
// The host tool that receives and manages project-specific panels.

class ProjectSystemsTool {
public:
    void loadFromAdapter(const IGameProjectAdapter& adapter) {
        m_panels = adapter.panelDescriptors();
    }

    const std::vector<GameplaySystemPanelDescriptor>& panels() const {
        return m_panels;
    }

    size_t panelCount() const { return m_panels.size(); }

    const GameplaySystemPanelDescriptor* findPanel(const std::string& id) const {
        for (const auto& p : m_panels) {
            if (p.id == id) return &p;
        }
        return nullptr;
    }

    // Filter panels by category
    std::vector<const GameplaySystemPanelDescriptor*> panelsByCategory(
        const std::string& category) const
    {
        std::vector<const GameplaySystemPanelDescriptor*> result;
        for (const auto& p : m_panels) {
            if (p.category == category && p.enabled) {
                result.push_back(&p);
            }
        }
        return result;
    }

    // Filter visible panels
    std::vector<const GameplaySystemPanelDescriptor*> visiblePanels() const {
        std::vector<const GameplaySystemPanelDescriptor*> result;
        for (const auto& p : m_panels) {
            if (p.defaultVisible && p.enabled) {
                result.push_back(&p);
            }
        }
        return result;
    }

private:
    std::vector<GameplaySystemPanelDescriptor> m_panels;
};

} // namespace NF
