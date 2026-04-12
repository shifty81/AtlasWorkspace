#pragma once
// NovaForge::MissionRulesPanel — document panel for mission/quest rule authoring.
//
// Binds to a NovaForgeDocument. Manages quest objectives, chains, rewards,
// and prerequisite conditions.
//
// Phase C.2 — NovaForge Gameplay Panels (Real Content)

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <string>
#include <vector>

namespace NovaForge {

// ── Mission data schema ───────────────────────────────────────────────────

enum class ObjectiveType : uint8_t {
    Kill      = 0,
    Collect   = 1,
    Reach     = 2,
    Interact  = 3,
    Escort    = 4,
    Defend    = 5,
    Survive   = 6,
    Custom    = 7,
};

inline const char* objectiveTypeName(ObjectiveType t) {
    switch (t) {
    case ObjectiveType::Kill:     return "Kill";
    case ObjectiveType::Collect:  return "Collect";
    case ObjectiveType::Reach:    return "Reach";
    case ObjectiveType::Interact: return "Interact";
    case ObjectiveType::Escort:   return "Escort";
    case ObjectiveType::Defend:   return "Defend";
    case ObjectiveType::Survive:  return "Survive";
    case ObjectiveType::Custom:   return "Custom";
    }
    return "Unknown";
}

struct MissionObjective {
    std::string   objectiveId;
    ObjectiveType type        = ObjectiveType::Kill;
    std::string   targetId;
    int           targetCount = 1;
    bool          optional    = false;
};

struct MissionReward {
    std::string currencyId;
    float       amount    = 0.0f;
    std::string itemId;      // optional item reward
    int         itemCount = 0;
};

struct MissionDefinitionData {
    std::string                   missionId;
    std::string                   displayName;
    std::string                   prerequisiteMissionId;  // empty = no prereq
    std::vector<MissionObjective> objectives;
    std::vector<MissionReward>    rewards;
    bool                          repeatable = false;
};

// ── MissionRulesPanel ────────────────────────────────────────────────────

class MissionRulesPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.mission_rules";

    MissionRulesPanel() : m_id(kPanelId), m_title("Mission Rules") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    // ── Schema accessors ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<MissionDefinitionData>& missions() const { return m_missions; }
    [[nodiscard]] int   maxActiveMissions() const { return m_maxActiveMissions; }
    [[nodiscard]] bool  chainEnabled()      const { return m_chainEnabled; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addMission(MissionDefinitionData def) {
        auto old = m_missions;
        m_missions.push_back(std::move(def));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Mission",
            [this, cur = m_missions]() { m_missions = cur; markFieldChanged(); return true; },
            [this, prev = old]()       { m_missions = prev; markFieldChanged(); return true; }
        });
    }

    void removeMission(const std::string& missionId) {
        auto it = std::find_if(m_missions.begin(), m_missions.end(),
                               [&](const auto& m) { return m.missionId == missionId; });
        if (it == m_missions.end()) return;
        auto old = m_missions;
        m_missions.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Mission",
            [this, cur = m_missions]() { m_missions = cur; markFieldChanged(); return true; },
            [this, prev = old]()       { m_missions = prev; markFieldChanged(); return true; }
        });
    }

    void setMaxActiveMissions(int max) {
        pushPropertyEdit("Set Max Active Missions",
                         m_maxActiveMissions, m_maxActiveMissions, max);
    }

    void setChainEnabled(bool enabled) {
        pushPropertyEdit("Toggle Mission Chains", m_chainEnabled, m_chainEnabled, enabled);
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        for (const auto& m : m_missions) {
            if (m.missionId.empty()) {
                msgs.push_back({"missions", "Mission has empty id",
                                DocumentPanelValidationSeverity::Error});
            }
            if (m.objectives.empty()) {
                msgs.push_back({"missions." + m.missionId + ".objectives",
                                "Mission has no objectives",
                                DocumentPanelValidationSeverity::Warning});
            }
        }
        if (m_maxActiveMissions < 1) {
            msgs.push_back({"maxActiveMissions", "Must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        }
        return msgs;
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_missions.empty()) {
            MissionDefinitionData intro;
            intro.missionId   = "intro_mission";
            intro.displayName = "Into the Forge";
            intro.objectives  = {{"obj1", ObjectiveType::Reach, "forge_entrance", 1, false}};
            intro.rewards     = {{"credits", 200.0f, "", 0}};
            intro.repeatable  = false;
            m_missions.push_back(std::move(intro));

            MissionDefinitionData hunt;
            hunt.missionId            = "creature_hunt";
            hunt.displayName          = "First Hunt";
            hunt.prerequisiteMissionId = "intro_mission";
            hunt.objectives           = {{"obj1", ObjectiveType::Kill, "forest_wolf", 5, false}};
            hunt.rewards              = {{"credits", 500.0f, "wolf_pelt", 2}};
            hunt.repeatable           = true;
            m_missions.push_back(std::move(hunt));
        }
    }

    void applyToDocument(NovaForgeDocument& doc) override { doc.markDirty(); }

private:
    std::string m_id;
    std::string m_title;

    std::vector<MissionDefinitionData> m_missions;
    int                                m_maxActiveMissions = 5;
    bool                               m_chainEnabled      = true;
};

} // namespace NovaForge
