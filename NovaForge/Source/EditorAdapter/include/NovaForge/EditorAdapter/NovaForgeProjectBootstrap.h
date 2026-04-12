#pragma once
// NovaForge::NovaForgeProjectBootstrap — validates and loads a NovaForge project
// structure from its .atlas manifest path.
//
// Responsibilities:
//   1. Parse the .atlas manifest via AtlasProjectFileLoader
//   2. Validate the NovaForge project directory structure:
//      - Content/ directory (asset root)
//      - Data/ directory (gameplay data root)
//      - Schema/ directory (optional, warned if missing)
//   3. Populate AssetCatalog from the Content/ root
//   4. Record structured validation errors in ProjectLoadContract
//
// Usage:
//   NovaForgeProjectBootstrap bs(projectRoot, catalog);
//   bool ok = bs.run(contract);

#include "NF/Workspace/AtlasProjectFileLoader.h"
#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/AssetCatalogPopulator.h"
#include "NF/Workspace/ProjectLoadContract.h"
#include <filesystem>
#include <string>

namespace NovaForge {

class NovaForgeProjectBootstrap {
public:
    explicit NovaForgeProjectBootstrap(std::string projectRoot,
                                       NF::AssetCatalog* catalog = nullptr)
        : m_projectRoot(std::move(projectRoot)), m_catalog(catalog) {}

    // Run bootstrap. Returns true if no fatal/error entries were produced.
    bool run(NF::ProjectLoadContract& contract) {
        namespace fs = std::filesystem;
        fs::path root(m_projectRoot);

        // Check Content/ directory
        fs::path contentDir = root / "Content";
        if (!fs::exists(contentDir) || !fs::is_directory(contentDir)) {
            contract.addError("missing_content_dir",
                "NovaForge Content/ directory not found at: " + contentDir.string());
        } else {
            contract.addInfo("content_dir_ok",
                "Content/ directory validated: " + contentDir.string());
            if (m_catalog) {
                populateCatalog(contentDir, contract);
            }
        }

        // Check Data/ directory
        fs::path dataDir = root / "Data";
        if (!fs::exists(dataDir) || !fs::is_directory(dataDir)) {
            contract.addWarning("missing_data_dir",
                "NovaForge Data/ directory not found at: " + dataDir.string());
        } else {
            contract.addInfo("data_dir_ok",
                "Data/ directory validated: " + dataDir.string());
        }

        // Check Schema/ directory (optional)
        fs::path schemaDir = root / "Schema";
        if (!fs::exists(schemaDir) || !fs::is_directory(schemaDir)) {
            contract.addWarning("missing_schema_dir",
                "NovaForge Schema/ directory not found (optional): " + schemaDir.string());
        } else {
            contract.addInfo("schema_dir_ok",
                "Schema/ directory validated: " + schemaDir.string());
        }

        return contract.isValid();
    }

    [[nodiscard]] const std::string& projectRoot() const { return m_projectRoot; }
    [[nodiscard]] NF::AssetCatalog* catalog() const { return m_catalog; }
    void setCatalog(NF::AssetCatalog* cat) { m_catalog = cat; }

    [[nodiscard]] size_t assetsPopulated() const { return m_assetsPopulated; }

private:
    void populateCatalog(const std::filesystem::path& contentDir,
                         NF::ProjectLoadContract& contract) {
        NF::AssetCatalogPopulator populator;
        auto result = populator.populateFromDirectory(*m_catalog, contentDir.string());
        m_assetsPopulated = result.assetsAdded;
        if (result.errors > 0) {
            contract.addWarning("catalog_populate_errors",
                "Failed to register " + std::to_string(result.errors) + " asset(s)");
        }
    }

    std::string m_projectRoot;
    NF::AssetCatalog* m_catalog = nullptr;
    size_t m_assetsPopulated = 0;
};

} // namespace NovaForge
