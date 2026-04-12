#pragma once
// NovaForge::MissionRulesPanel — gameplay system panel for mission/quest rule authoring.
//
// Manages quest objectives, mission chains, prerequisites, and completion rewards.
// Hosted inside the workspace.project_systems tool via NovaForgeAdapter.

#include "NF/Workspace/IGameProjectAdapter.h"
#include <string>

namespace NovaForge {

class MissionRulesPanel final : public NF::IEditorPanel {
public:
    static constexpr const char* kPanelId = "novaforge.mission_rules";

    MissionRulesPanel() : m_id(kPanelId), m_title("Mission Rules") {}

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
