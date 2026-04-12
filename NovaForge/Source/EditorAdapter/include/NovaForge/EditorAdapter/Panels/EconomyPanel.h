#pragma once
// NovaForge::EconomyPanel — document panel for economy authoring.
//
// Binds to a NovaForgeDocument of type EconomyRules.
// Manages in-game currency definitions, pricing multipliers, reward rates,
// and economy balance parameters.
//
// Phase C.2 — NovaForge Gameplay Panels (Real Content)

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <string>
#include <vector>

namespace NovaForge {

// ── Economy data schema ───────────────────────────────────────────────────

struct CurrencyDefinition {
    std::string id;
    std::string displayName;
    float       earnRate    = 1.0f;   // base earn rate per in-game minute
    float       spendCap    = 0.0f;   // 0 = uncapped
    bool        tradeable   = true;
};

struct EconomyPricingRule {
    std::string itemCategory;     // item category this rule applies to
    float       basePriceScale   = 1.0f;
    float       demandMultiplier = 1.0f;
    float       supplyMultiplier = 1.0f;
};

// ── EconomyPanel ─────────────────────────────────────────────────────────

class EconomyPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.economy";

    EconomyPanel() : m_id(kPanelId), m_title("Economy") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    // ── Schema accessors ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<CurrencyDefinition>& currencies() const { return m_currencies; }
    [[nodiscard]] const std::vector<EconomyPricingRule>& pricingRules() const { return m_pricingRules; }
    [[nodiscard]] float globalInflationRate() const { return m_globalInflationRate; }
    [[nodiscard]] bool  economyEnabled() const { return m_economyEnabled; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addCurrency(CurrencyDefinition def) {
        auto old = m_currencies;
        m_currencies.push_back(std::move(def));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Currency",
            [this, cur = m_currencies]() { m_currencies = cur; markFieldChanged(); return true; },
            [this, prev = old]()         { m_currencies = prev; markFieldChanged(); return true; }
        });
    }

    void removeCurrency(const std::string& id) {
        auto it = std::find_if(m_currencies.begin(), m_currencies.end(),
                               [&](const auto& c) { return c.id == id; });
        if (it == m_currencies.end()) return;
        auto old = m_currencies;
        m_currencies.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Currency",
            [this, cur = m_currencies]() { m_currencies = cur; markFieldChanged(); return true; },
            [this, prev = old]()         { m_currencies = prev; markFieldChanged(); return true; }
        });
    }

    void setGlobalInflationRate(float rate) {
        pushPropertyEdit("Set Inflation Rate", m_globalInflationRate,
                         m_globalInflationRate, rate);
    }

    void setEconomyEnabled(bool enabled) {
        pushPropertyEdit("Toggle Economy", m_economyEnabled, m_economyEnabled, enabled);
    }

    void addPricingRule(EconomyPricingRule rule) {
        auto old = m_pricingRules;
        m_pricingRules.push_back(std::move(rule));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Pricing Rule",
            [this, cur = m_pricingRules]() { m_pricingRules = cur; markFieldChanged(); return true; },
            [this, prev = old]()           { m_pricingRules = prev; markFieldChanged(); return true; }
        });
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        for (const auto& c : m_currencies) {
            if (c.id.empty()) {
                msgs.push_back({"currencies", "Currency has empty id",
                                DocumentPanelValidationSeverity::Error});
            }
            if (c.earnRate < 0.0f) {
                msgs.push_back({"currencies." + c.id + ".earnRate",
                                "Earn rate must be >= 0",
                                DocumentPanelValidationSeverity::Error});
            }
        }
        if (m_globalInflationRate < 0.0f) {
            msgs.push_back({"globalInflationRate",
                            "Inflation rate must be >= 0",
                            DocumentPanelValidationSeverity::Warning});
        }
        return msgs;
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        // Phase D will load from real JSON; initialize with defaults for now.
        if (m_currencies.empty()) {
            m_currencies.push_back({"credits",   "Credits",   1.0f,  0.0f,  true});
            m_currencies.push_back({"premium",   "Premium",   0.1f,  500.0f, false});
        }
        if (m_pricingRules.empty()) {
            m_pricingRules.push_back({"weapon",    1.0f, 1.2f, 0.8f});
            m_pricingRules.push_back({"consumable",0.5f, 1.0f, 1.0f});
        }
    }

    void applyToDocument(NovaForgeDocument& doc) override {
        doc.markDirty();
    }

private:
    std::string m_id;
    std::string m_title;

    std::vector<CurrencyDefinition> m_currencies;
    std::vector<EconomyPricingRule> m_pricingRules;
    float                           m_globalInflationRate = 0.02f;
    bool                            m_economyEnabled      = true;
};

} // namespace NovaForge
