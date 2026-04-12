#pragma once
// NovaForge::InventoryRulesPanel — gameplay system panel for inventory rule authoring.
//
// Manages item slot layouts, storage limits, equipment constraints, and stacking rules.
// Hosted inside the workspace.project_systems tool via NovaForgeAdapter.

#include "NF/Workspace/IGameProjectAdapter.h"
#include <string>

namespace NovaForge {

class InventoryRulesPanel final : public NF::IEditorPanel {
public:
    static constexpr const char* kPanelId = "novaforge.inventory_rules";

    InventoryRulesPanel() : m_id(kPanelId), m_title("Inventory Rules") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    void onProjectLoaded(const std::string& projectRoot) override {
        m_projectRoot = projectRoot;
        m_ready = true;
    }
    void onProjectUnloaded() override {
        m_projectRoot.clear();
        m_ready = false;
    }

    void update(float /*dt*/) override {}

    [[nodiscard]] bool isReady() const override { return m_ready; }

    [[nodiscard]] const std::string& projectRoot() const { return m_projectRoot; }

private:
    std::string m_id;
    std::string m_title;
    std::string m_projectRoot;
    bool        m_ready = false;
};

} // namespace NovaForge
