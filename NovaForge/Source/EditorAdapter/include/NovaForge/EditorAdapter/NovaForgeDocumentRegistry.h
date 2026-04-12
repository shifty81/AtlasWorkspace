#pragma once
// NovaForge::NovaForgeDocumentRegistry — maps document type → schema + load/save paths.
//
// Maintains the global set of document type registrations for the NovaForge project.
// Each registration describes:
//   - The document type enum value
//   - The data root subdirectory (relative to Data/)
//   - The file extension used for documents of that type
//   - The schema root subdirectory (relative to Schema/)

#include "NovaForge/EditorAdapter/NovaForgeDocument.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NovaForge {

struct DocumentTypeRegistration {
    NovaForgeDocumentType type;
    std::string           displayName;
    std::string           dataSubdir;
    std::string           fileExtension;
    std::string           schemaSubdir;
    std::function<std::unique_ptr<NovaForgeDocument>(std::string filePath)> factory;
};

class NovaForgeDocumentRegistry {
public:
    void registerBuiltins() {
        registerType({
            NovaForgeDocumentType::ItemDefinition, "Item Definition",
            "Items", ".item.json", "Items",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::ItemDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::StructureArchetype, "Structure Archetype",
            "Structures", ".structure.json", "Structures",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::StructureArchetype, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::BiomeDefinition, "Biome Definition",
            "Biomes", ".biome.json", "Biomes",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::BiomeDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::PlanetArchetype, "Planet Archetype",
            "Planets", ".planet.json", "Planets",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::PlanetArchetype, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::FactionDefinition, "Faction Definition",
            "Factions", ".faction.json", "Factions",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::FactionDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::MissionDefinition, "Mission Definition",
            "Missions", ".mission.json", "Missions",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::MissionDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::ProgressionRules, "Progression Rules",
            "Progression", ".progression.json", "Progression",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::ProgressionRules, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::CharacterRules, "Character Rules",
            "Characters", ".character.json", "Characters",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::CharacterRules, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::EconomyRules, "Economy Rules",
            "Economy", ".economy.json", "Economy",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::EconomyRules, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::CraftingDefinition, "Crafting Definition",
            "Crafting", ".crafting.json", "Crafting",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::CraftingDefinition, std::move(fp)); }
        });
        registerType({
            NovaForgeDocumentType::PCGRuleset, "PCG Ruleset",
            "PCG", ".pcg.json", "PCG",
            [](std::string fp) { return std::make_unique<NovaForgeDocument>(NovaForgeDocumentType::PCGRuleset, std::move(fp)); }
        });
    }

    void registerType(DocumentTypeRegistration reg) {
        m_registrations.emplace(static_cast<uint8_t>(reg.type), std::move(reg));
    }

    [[nodiscard]] const DocumentTypeRegistration* find(NovaForgeDocumentType type) const {
        auto it = m_registrations.find(static_cast<uint8_t>(type));
        return it != m_registrations.end() ? &it->second : nullptr;
    }

    [[nodiscard]] size_t count() const { return m_registrations.size(); }
    [[nodiscard]] bool empty() const { return m_registrations.empty(); }

    [[nodiscard]] std::vector<const DocumentTypeRegistration*> all() const {
        std::vector<const DocumentTypeRegistration*> out;
        for (const auto& [k, v] : m_registrations)
            out.push_back(&v);
        return out;
    }

    [[nodiscard]] std::unique_ptr<NovaForgeDocument> createDocument(
        NovaForgeDocumentType type, std::string filePath) const {
        const auto* reg = find(type);
        if (!reg || !reg->factory) return nullptr;
        return reg->factory(std::move(filePath));
    }

private:
    std::unordered_map<uint8_t, DocumentTypeRegistration> m_registrations;
};

} // namespace NovaForge
