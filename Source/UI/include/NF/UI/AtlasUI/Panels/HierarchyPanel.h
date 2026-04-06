#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI HierarchyPanel — displays entity tree with search filter.
/// Replaces the legacy NF::Editor::HierarchyPanel for the AtlasUI framework.
class HierarchyPanel final : public PanelBase {
public:
    HierarchyPanel()
        : PanelBase("atlas.hierarchy", "Hierarchy") {}

    void paint(IPaintContext& context) override;

    void setSearchFilter(const std::string& filter) { m_searchFilter = filter; }
    [[nodiscard]] const std::string& searchFilter() const { return m_searchFilter; }

    struct EntityEntry {
        int id = 0;
        std::string name;
        bool selected = false;
        int depth = 0;
    };

    void clearEntities() { m_entities.clear(); }
    void addEntity(int id, const std::string& name, bool selected = false, int depth = 0) {
        m_entities.push_back({id, name, selected, depth});
    }
    [[nodiscard]] const std::vector<EntityEntry>& entities() const { return m_entities; }
    [[nodiscard]] size_t entityCount() const { return m_entities.size(); }

private:
    std::string m_searchFilter;
    std::vector<EntityEntry> m_entities;
};

} // namespace NF::UI::AtlasUI
