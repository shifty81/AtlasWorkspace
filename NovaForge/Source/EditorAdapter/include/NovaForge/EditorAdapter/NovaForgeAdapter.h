#pragma once
// NovaForge::NovaForgeAdapter — Project adapter for the NovaForge game.
//
// This adapter registers NovaForge-specific gameplay system panels with
// the Atlas Workspace. All NovaForge-specific authoring surfaces are
// exposed through this adapter rather than as standalone workspace editors.
//
// See Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md

#include "NF/Editor/IGameProjectAdapter.h"
#include <string>
#include <vector>

namespace NovaForge {

class NovaForgeAdapter final : public NF::IGameProjectAdapter {
public:
    std::string projectId() const override { return "novaforge"; }
    std::string projectDisplayName() const override { return "NovaForge"; }

    bool initialize() override {
        // Register gameplay system panels
        m_panels = buildPanelDescriptors();
        return true;
    }

    void shutdown() override {
        m_panels.clear();
    }

    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        return m_panels;
    }

    std::vector<std::string> contentRoots() const override {
        return { "NovaForge/Content", "NovaForge/Data" };
    }

    std::vector<std::string> customCommands() const override {
        return {
            "novaforge.build_game",
            "novaforge.build_server",
            "novaforge.launch_editor",
            "novaforge.validate_assets"
        };
    }

private:
    std::vector<NF::GameplaySystemPanelDescriptor> m_panels;

    static std::vector<NF::GameplaySystemPanelDescriptor> buildPanelDescriptors() {
        std::vector<NF::GameplaySystemPanelDescriptor> panels;

        // Economy panel — game currency, pricing, economy balancing
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.economy";
            d.displayName = "Economy";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            panels.push_back(std::move(d));
        }

        // Inventory rules panel — item slots, storage, equipment rules
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.inventory_rules";
            d.displayName = "Inventory Rules";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            panels.push_back(std::move(d));
        }

        // Shop panel — item shop configuration
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.shop";
            d.displayName = "Shop";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = false;
            d.enabled = true;
            panels.push_back(std::move(d));
        }

        // Mission rules panel — quest, mission, objective rules
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.mission_rules";
            d.displayName = "Mission Rules";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            panels.push_back(std::move(d));
        }

        // Progression panel — XP, leveling, achievements, unlock paths
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.progression";
            d.displayName = "Progression";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            panels.push_back(std::move(d));
        }

        // Character rules panel — character creation, appearance, classes
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.character_rules";
            d.displayName = "Character Rules";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = false;
            d.enabled = true;
            panels.push_back(std::move(d));
        }

        return panels;
    }
};

} // namespace NovaForge
