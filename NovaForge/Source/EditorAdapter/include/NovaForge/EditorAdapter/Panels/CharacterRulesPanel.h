#pragma once
// NovaForge::CharacterRulesPanel — document panel for character rule authoring.
//
// Binds to a NovaForgeDocument of type CharacterRules. Manages creation presets,
// class editor, stat caps, and appearance configuration.
//
// Phase C.2 — NovaForge Gameplay Panels (Real Content)

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <string>
#include <vector>

namespace NovaForge {

// ── Character data schema ─────────────────────────────────────────────────

struct StatCapEntry {
    std::string statId;
    float       baseValue = 0.0f;
    float       minValue  = 0.0f;
    float       maxValue  = 100.0f;
};

struct CharacterClassPreset {
    std::string               classId;
    std::string               displayName;
    std::string               description;
    std::vector<StatCapEntry> baseStats;
    std::vector<std::string>  startingSkills;
    bool                      selectable = true;
};

struct AppearanceConfig {
    int   numBodyPresets   = 5;
    int   numFacePresets   = 10;
    bool  customColorEnabled = true;
    bool  heightSlider       = true;
};

// ── CharacterRulesPanel ──────────────────────────────────────────────────

class CharacterRulesPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.character_rules";

    CharacterRulesPanel() : m_id(kPanelId), m_title("Character Rules") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    // ── Schema accessors ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<CharacterClassPreset>& classPresets() const { return m_classes; }
    [[nodiscard]] const AppearanceConfig& appearanceConfig() const { return m_appearance; }
    [[nodiscard]] int  maxCharactersPerAccount() const { return m_maxCharactersPerAccount; }
    [[nodiscard]] bool customClassEnabled()      const { return m_customClassEnabled; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addClass(CharacterClassPreset preset) {
        auto old = m_classes;
        m_classes.push_back(std::move(preset));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Class",
            [this, cur = m_classes]() { m_classes = cur; markFieldChanged(); return true; },
            [this, prev = old]()      { m_classes = prev; markFieldChanged(); return true; }
        });
    }

    void removeClass(const std::string& classId) {
        auto it = std::find_if(m_classes.begin(), m_classes.end(),
                               [&](const auto& c) { return c.classId == classId; });
        if (it == m_classes.end()) return;
        auto old = m_classes;
        m_classes.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Class",
            [this, cur = m_classes]() { m_classes = cur; markFieldChanged(); return true; },
            [this, prev = old]()      { m_classes = prev; markFieldChanged(); return true; }
        });
    }

    void setMaxCharactersPerAccount(int max) {
        pushPropertyEdit("Set Max Characters",
                         m_maxCharactersPerAccount, m_maxCharactersPerAccount, max);
    }

    void setCustomClassEnabled(bool enabled) {
        pushPropertyEdit("Toggle Custom Class",
                         m_customClassEnabled, m_customClassEnabled, enabled);
    }

    void setAppearanceConfig(AppearanceConfig cfg) {
        auto old = m_appearance;
        m_appearance = std::move(cfg);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Update Appearance Config",
            [this, cur = m_appearance]() { m_appearance = cur; markFieldChanged(); return true; },
            [this, prev = old]()         { m_appearance = prev; markFieldChanged(); return true; }
        });
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        if (m_classes.empty()) {
            msgs.push_back({"classes", "At least one class preset is required",
                            DocumentPanelValidationSeverity::Error});
        }
        for (const auto& c : m_classes) {
            if (c.classId.empty()) {
                msgs.push_back({"classes", "Class has empty id",
                                DocumentPanelValidationSeverity::Error});
            }
        }
        if (m_maxCharactersPerAccount < 1) {
            msgs.push_back({"maxCharactersPerAccount", "Must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        }
        if (m_appearance.numBodyPresets < 1) {
            msgs.push_back({"appearance.numBodyPresets", "Must be >= 1",
                            DocumentPanelValidationSeverity::Warning});
        }
        return msgs;
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_classes.empty()) {
            CharacterClassPreset warrior;
            warrior.classId    = "warrior";
            warrior.displayName = "Warrior";
            warrior.description = "Melee-focused tank class";
            warrior.baseStats  = {
                {"health",  150.0f, 0.0f, 500.0f},
                {"strength", 20.0f, 0.0f, 100.0f},
                {"agility",  10.0f, 0.0f, 100.0f},
            };
            warrior.startingSkills = {"sprint", "shield_bash"};
            warrior.selectable     = true;
            m_classes.push_back(std::move(warrior));

            CharacterClassPreset ranger;
            ranger.classId     = "ranger";
            ranger.displayName = "Ranger";
            ranger.description = "Ranged DPS class with stealth";
            ranger.baseStats   = {
                {"health",   100.0f, 0.0f, 500.0f},
                {"strength",  10.0f, 0.0f, 100.0f},
                {"agility",   20.0f, 0.0f, 100.0f},
            };
            ranger.startingSkills = {"sprint", "eagle_eye"};
            ranger.selectable     = true;
            m_classes.push_back(std::move(ranger));
        }
        if (m_appearance.numBodyPresets == 0) {
            m_appearance = {5, 10, true, true};
        }
    }

    void applyToDocument(NovaForgeDocument& doc) override { doc.markDirty(); }

private:
    std::string m_id;
    std::string m_title;

    std::vector<CharacterClassPreset> m_classes;
    AppearanceConfig                  m_appearance;
    int                               m_maxCharactersPerAccount = 3;
    bool                              m_customClassEnabled      = false;
};

} // namespace NovaForge
