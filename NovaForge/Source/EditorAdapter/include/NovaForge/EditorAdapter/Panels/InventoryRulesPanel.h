#pragma once
// NovaForge::InventoryRulesPanel — document panel for inventory rule authoring.
//
// Binds to a NovaForgeDocument. Manages slot layouts, storage rules, stacking config.
//
// Phase C.2 — NovaForge Gameplay Panels (Real Content)

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <string>
#include <vector>

namespace NovaForge {

// ── Inventory data schema ────────────────────────────────────────────────

struct InventorySlotConfig {
    std::string slotId;
    std::string acceptedCategory; // item category allowed in this slot
    int         maxStack = 1;
    bool        locked   = false;  // cannot be moved by player
};

struct StorageRule {
    std::string containerId;      // container type id
    int         maxSlots    = 20;
    int         maxWeight   = 0;  // 0 = unlimited
    bool        autoSort    = false;
};

// ── InventoryRulesPanel ──────────────────────────────────────────────────

class InventoryRulesPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.inventory_rules";

    InventoryRulesPanel() : m_id(kPanelId), m_title("Inventory Rules") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    // ── Schema accessors ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<InventorySlotConfig>& slots() const { return m_slots; }
    [[nodiscard]] const std::vector<StorageRule>& storageRules() const { return m_storageRules; }
    [[nodiscard]] int   defaultMaxSlots()  const { return m_defaultMaxSlots; }
    [[nodiscard]] bool  globalStackable()  const { return m_globalStackable; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addSlot(InventorySlotConfig slot) {
        auto old = m_slots;
        m_slots.push_back(std::move(slot));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Slot",
            [this, cur = m_slots]() { m_slots = cur; markFieldChanged(); return true; },
            [this, prev = old]()    { m_slots = prev; markFieldChanged(); return true; }
        });
    }

    void setDefaultMaxSlots(int max) {
        pushPropertyEdit("Set Max Slots", m_defaultMaxSlots, m_defaultMaxSlots, max);
    }

    void setGlobalStackable(bool stackable) {
        pushPropertyEdit("Toggle Stackable", m_globalStackable, m_globalStackable, stackable);
    }

    void addStorageRule(StorageRule rule) {
        auto old = m_storageRules;
        m_storageRules.push_back(std::move(rule));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Storage Rule",
            [this, cur = m_storageRules]() { m_storageRules = cur; markFieldChanged(); return true; },
            [this, prev = old]()           { m_storageRules = prev; markFieldChanged(); return true; }
        });
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        for (const auto& s : m_slots) {
            if (s.slotId.empty()) {
                msgs.push_back({"slots", "Slot has empty id",
                                DocumentPanelValidationSeverity::Error});
            }
            if (s.maxStack < 1) {
                msgs.push_back({"slots." + s.slotId + ".maxStack",
                                "maxStack must be >= 1",
                                DocumentPanelValidationSeverity::Error});
            }
        }
        if (m_defaultMaxSlots < 1) {
            msgs.push_back({"defaultMaxSlots", "Max slots must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        }
        return msgs;
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_slots.empty()) {
            m_slots.push_back({"head",     "armor",   1, false});
            m_slots.push_back({"chest",    "armor",   1, false});
            m_slots.push_back({"primary",  "weapon",  1, false});
            m_slots.push_back({"backpack", "any",    20, false});
        }
        if (m_storageRules.empty()) {
            m_storageRules.push_back({"player_inventory", 40, 0, true});
            m_storageRules.push_back({"chest_container",  20, 100, false});
        }
    }

    void applyToDocument(NovaForgeDocument& doc) override { doc.markDirty(); }

private:
    std::string m_id;
    std::string m_title;

    std::vector<InventorySlotConfig> m_slots;
    std::vector<StorageRule>         m_storageRules;
    int                              m_defaultMaxSlots = 40;
    bool                             m_globalStackable = true;
};

} // namespace NovaForge
