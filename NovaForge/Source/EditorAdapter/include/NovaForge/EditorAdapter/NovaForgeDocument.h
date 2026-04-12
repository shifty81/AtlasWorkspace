#pragma once
// NovaForge::NovaForgeDocument — base class for all NovaForge authoring documents.
// Provides dirty tracking, validate/save/revert lifecycle, and apply support.

#include <string>
#include <vector>

namespace NovaForge {

// ── Document type taxonomy ────────────────────────────────────────────────

enum class NovaForgeDocumentType : uint8_t {
    ItemDefinition,
    StructureArchetype,
    BiomeDefinition,
    PlanetArchetype,
    FactionDefinition,
    MissionDefinition,
    ProgressionRules,
    CharacterRules,
    EconomyRules,
    CraftingDefinition,
    PCGRuleset,
};

inline const char* documentTypeName(NovaForgeDocumentType t) {
    switch (t) {
    case NovaForgeDocumentType::ItemDefinition:     return "ItemDefinition";
    case NovaForgeDocumentType::StructureArchetype: return "StructureArchetype";
    case NovaForgeDocumentType::BiomeDefinition:    return "BiomeDefinition";
    case NovaForgeDocumentType::PlanetArchetype:    return "PlanetArchetype";
    case NovaForgeDocumentType::FactionDefinition:  return "FactionDefinition";
    case NovaForgeDocumentType::MissionDefinition:  return "MissionDefinition";
    case NovaForgeDocumentType::ProgressionRules:   return "ProgressionRules";
    case NovaForgeDocumentType::CharacterRules:     return "CharacterRules";
    case NovaForgeDocumentType::EconomyRules:       return "EconomyRules";
    case NovaForgeDocumentType::CraftingDefinition: return "CraftingDefinition";
    case NovaForgeDocumentType::PCGRuleset:         return "PCGRuleset";
    }
    return "Unknown";
}

// ── Validation result ────────────────────────────────────────────────────

struct DocumentValidationResult {
    bool valid = true;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

// ── NovaForgeDocument base ────────────────────────────────────────────────
// Subclass per document type. Override validate(), onSave(), onRevert().

class NovaForgeDocument {
public:
    explicit NovaForgeDocument(NovaForgeDocumentType type,
                               std::string filePath)
        : m_type(type), m_filePath(std::move(filePath)) {}

    virtual ~NovaForgeDocument() = default;

    // ── Identity ──────────────────────────────────────────────────
    [[nodiscard]] NovaForgeDocumentType type()     const { return m_type; }
    [[nodiscard]] const std::string&    filePath() const { return m_filePath; }
    [[nodiscard]] const std::string&    displayName() const { return m_displayName; }
    void setDisplayName(const std::string& name) { m_displayName = name; }

    // ── Dirty tracking ────────────────────────────────────────────
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true;  }
    void clearDirty() { m_dirty = false; }

    // ── Lifecycle ─────────────────────────────────────────────────

    virtual DocumentValidationResult validate() const {
        return DocumentValidationResult{};
    }

    bool save() {
        auto v = validate();
        if (!v.valid) return false;
        if (onSave()) {
            clearDirty();
            return true;
        }
        return false;
    }

    bool revert() {
        if (onRevert()) {
            clearDirty();
            return true;
        }
        return false;
    }

    virtual bool apply() { return true; }

protected:
    virtual bool onSave()   { return true; }
    virtual bool onRevert() { return true; }

    NovaForgeDocumentType m_type;
    std::string           m_filePath;
    std::string           m_displayName;
    bool                  m_dirty = false;
};

} // namespace NovaForge
