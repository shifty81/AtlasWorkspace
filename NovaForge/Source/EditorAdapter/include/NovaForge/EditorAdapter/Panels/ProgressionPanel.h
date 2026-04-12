#pragma once
// NovaForge::ProgressionPanel — gameplay system panel for XP/leveling/unlock authoring.
//
// Manages experience curves, level thresholds, skill tree unlocks, and achievement paths.
// Hosted inside the workspace.project_systems tool via NovaForgeAdapter.

#include "NF/Workspace/IGameProjectAdapter.h"
#include <string>

namespace NovaForge {

class ProgressionPanel final : public NF::IEditorPanel {
public:
    static constexpr const char* kPanelId = "novaforge.progression";

    ProgressionPanel() : m_id(kPanelId), m_title("Progression") {}

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
