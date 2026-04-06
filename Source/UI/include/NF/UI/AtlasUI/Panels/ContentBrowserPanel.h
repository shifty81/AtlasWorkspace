#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI ContentBrowserPanel — displays project file system for asset browsing.
/// Replaces the legacy NF::Editor::ContentBrowserPanel for the AtlasUI framework.
class ContentBrowserPanel final : public PanelBase {
public:
    ContentBrowserPanel()
        : PanelBase("atlas.content_browser", "Content Browser") {}

    void paint(IPaintContext& context) override;

    void setCurrentPath(const std::string& path) { m_currentPath = path; }
    [[nodiscard]] const std::string& currentPath() const { return m_currentPath; }

    struct FileEntry {
        std::string name;
        bool isDirectory = false;
    };

    void clearEntries() { m_entries.clear(); }
    void addEntry(const std::string& name, bool isDirectory) {
        m_entries.push_back({name, isDirectory});
    }
    [[nodiscard]] const std::vector<FileEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

private:
    std::string m_currentPath = "/";
    std::vector<FileEntry> m_entries;
};

} // namespace NF::UI::AtlasUI
