#pragma once
// NovaForge::ProgressionPanel — document panel for XP/leveling/unlock authoring.
//
// Binds to a NovaForgeDocument of type ProgressionRules. Manages experience curves,
// level thresholds, skill tree unlock paths, and achievement gates.
//
// Phase C.2 — NovaForge Gameplay Panels (Real Content)

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <algorithm>
#include <string>
#include <vector>

namespace NovaForge {

// ── Progression data schema ───────────────────────────────────────────────

struct LevelThreshold {
    int   level          = 1;
    float xpRequired     = 100.0f;  // cumulative XP to reach this level
    float xpMultiplier   = 1.0f;   // scaling applied at this level
};

struct SkillUnlock {
    std::string skillId;
    std::string displayName;
    int         requiredLevel  = 1;
    std::string prerequisiteId; // empty = no prereq skill
    int         skillPointCost = 1;
};

// ── ProgressionPanel ─────────────────────────────────────────────────────

class ProgressionPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.progression";

    ProgressionPanel() : m_id(kPanelId), m_title("Progression") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    // ── Schema accessors ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<LevelThreshold>& levelThresholds() const { return m_levels; }
    [[nodiscard]] const std::vector<SkillUnlock>&    skillUnlocks()    const { return m_skills; }
    [[nodiscard]] int   maxLevel()             const { return m_maxLevel; }
    [[nodiscard]] float xpGlobalMultiplier()   const { return m_xpGlobalMultiplier; }
    [[nodiscard]] int   skillPointsPerLevel()  const { return m_skillPointsPerLevel; }

    // ── Editing API ───────────────────────────────────────────────────────
    void setMaxLevel(int max) {
        pushPropertyEdit("Set Max Level", m_maxLevel, m_maxLevel, max);
    }

    void setXpGlobalMultiplier(float mult) {
        pushPropertyEdit("Set XP Multiplier",
                         m_xpGlobalMultiplier, m_xpGlobalMultiplier, mult);
    }

    void setSkillPointsPerLevel(int pts) {
        pushPropertyEdit("Set Skill Points Per Level",
                         m_skillPointsPerLevel, m_skillPointsPerLevel, pts);
    }

    void addLevelThreshold(LevelThreshold thresh) {
        auto old = m_levels;
        m_levels.push_back(std::move(thresh));
        std::sort(m_levels.begin(), m_levels.end(),
                  [](const auto& a, const auto& b) { return a.level < b.level; });
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Level Threshold",
            [this, cur = m_levels]() { m_levels = cur; markFieldChanged(); return true; },
            [this, prev = old]()     { m_levels = prev; markFieldChanged(); return true; }
        });
    }

    void addSkillUnlock(SkillUnlock skill) {
        auto old = m_skills;
        m_skills.push_back(std::move(skill));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Skill",
            [this, cur = m_skills]() { m_skills = cur; markFieldChanged(); return true; },
            [this, prev = old]()     { m_skills = prev; markFieldChanged(); return true; }
        });
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        if (m_maxLevel < 1) {
            msgs.push_back({"maxLevel", "Max level must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        }
        if (m_xpGlobalMultiplier <= 0.0f) {
            msgs.push_back({"xpGlobalMultiplier", "XP multiplier must be > 0",
                            DocumentPanelValidationSeverity::Error});
        }
        if (m_skillPointsPerLevel < 0) {
            msgs.push_back({"skillPointsPerLevel", "Must be >= 0",
                            DocumentPanelValidationSeverity::Warning});
        }
        for (const auto& s : m_skills) {
            if (s.skillId.empty()) {
                msgs.push_back({"skills", "Skill has empty id",
                                DocumentPanelValidationSeverity::Error});
            }
        }
        return msgs;
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_levels.empty()) {
            // Generate 10 default levels with exponential XP scaling
            float xp = 100.0f;
            for (int i = 1; i <= 10; ++i) {
                m_levels.push_back({i, xp, 1.0f + (i - 1) * 0.1f});
                xp *= 1.5f;
            }
        }
        if (m_skills.empty()) {
            m_skills.push_back({"sprint",      "Sprint",      1, "",       1});
            m_skills.push_back({"double_jump", "Double Jump", 5, "sprint", 2});
            m_skills.push_back({"crafting_1",  "Basic Craft", 3, "",       1});
            m_skills.push_back({"crafting_2",  "Adv. Craft",  8, "crafting_1", 2});
        }
    }

    void applyToDocument(NovaForgeDocument& doc) override { doc.markDirty(); }

private:
    std::string m_id;
    std::string m_title;

    std::vector<LevelThreshold> m_levels;
    std::vector<SkillUnlock>    m_skills;
    int                         m_maxLevel            = 50;
    float                       m_xpGlobalMultiplier  = 1.0f;
    int                         m_skillPointsPerLevel = 1;
};

} // namespace NovaForge
