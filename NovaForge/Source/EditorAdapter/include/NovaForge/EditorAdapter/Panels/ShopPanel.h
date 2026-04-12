#pragma once
// NovaForge::ShopPanel — document panel for in-game shop authoring.
//
// Binds to a NovaForgeDocument. Manages store listings, purchase conditions,
// availability windows, and promotional pricing.
//
// Phase C.2 — NovaForge Gameplay Panels (Real Content)

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <string>
#include <vector>

namespace NovaForge {

// ── Shop data schema ─────────────────────────────────────────────────────

struct ShopListing {
    std::string itemId;
    std::string currencyId;
    float       basePrice      = 1.0f;
    int         stockLimit     = 0;    // 0 = unlimited
    bool        requiresUnlock = false;
    std::string unlockCondition;
};

struct ShopConfig {
    std::string shopId;
    std::string displayName;
    std::string merchantFaction;       // faction that owns this shop
    bool        globallyAvailable = true;
};

// ── ShopPanel ────────────────────────────────────────────────────────────

class ShopPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.shop";

    ShopPanel() : m_id(kPanelId), m_title("Shop") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    // ── Schema accessors ──────────────────────────────────────────────────
    [[nodiscard]] const ShopConfig& shopConfig() const { return m_config; }
    [[nodiscard]] const std::vector<ShopListing>& listings() const { return m_listings; }
    [[nodiscard]] float globalDiscount() const { return m_globalDiscount; }

    // ── Editing API ───────────────────────────────────────────────────────
    void setShopConfig(ShopConfig config) {
        auto old = m_config;
        m_config = std::move(config);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Update Shop Config",
            [this, cur = m_config]() { m_config = cur; markFieldChanged(); return true; },
            [this, prev = old]()     { m_config = prev; markFieldChanged(); return true; }
        });
    }

    void addListing(ShopListing listing) {
        auto old = m_listings;
        m_listings.push_back(std::move(listing));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Listing",
            [this, cur = m_listings]() { m_listings = cur; markFieldChanged(); return true; },
            [this, prev = old]()       { m_listings = prev; markFieldChanged(); return true; }
        });
    }

    void removeListing(const std::string& itemId) {
        auto it = std::find_if(m_listings.begin(), m_listings.end(),
                               [&](const auto& l) { return l.itemId == itemId; });
        if (it == m_listings.end()) return;
        auto old = m_listings;
        m_listings.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Listing",
            [this, cur = m_listings]() { m_listings = cur; markFieldChanged(); return true; },
            [this, prev = old]()       { m_listings = prev; markFieldChanged(); return true; }
        });
    }

    void setGlobalDiscount(float pct) {
        pushPropertyEdit("Set Global Discount", m_globalDiscount, m_globalDiscount, pct);
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        if (m_config.shopId.empty()) {
            msgs.push_back({"config.shopId", "Shop id must not be empty",
                            DocumentPanelValidationSeverity::Error});
        }
        for (const auto& l : m_listings) {
            if (l.itemId.empty()) {
                msgs.push_back({"listings", "Listing has empty itemId",
                                DocumentPanelValidationSeverity::Error});
            }
            if (l.basePrice < 0.0f) {
                msgs.push_back({"listings." + l.itemId + ".basePrice",
                                "Price must be >= 0",
                                DocumentPanelValidationSeverity::Error});
            }
        }
        if (m_globalDiscount < 0.0f || m_globalDiscount > 1.0f) {
            msgs.push_back({"globalDiscount",
                            "Discount must be in [0, 1]",
                            DocumentPanelValidationSeverity::Warning});
        }
        return msgs;
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_config.shopId.empty()) {
            m_config = {"general_store", "General Store", "traders_guild", true};
        }
        if (m_listings.empty()) {
            m_listings.push_back({"health_potion",  "credits", 25.0f, 0, false, ""});
            m_listings.push_back({"iron_sword",     "credits", 150.0f, 10, false, ""});
            m_listings.push_back({"rare_blueprint", "premium", 5.0f,   1, true,  "faction_rank_3"});
        }
    }

    void applyToDocument(NovaForgeDocument& doc) override { doc.markDirty(); }

private:
    std::string m_id;
    std::string m_title;

    ShopConfig                m_config;
    std::vector<ShopListing>  m_listings;
    float                     m_globalDiscount = 0.0f;
};

} // namespace NovaForge
