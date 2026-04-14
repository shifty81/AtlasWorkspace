#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include "NF/UI/AtlasUI/Widgets/PropertyGrid.h"
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI InspectorPanel — displays and edits selected entity properties.
/// Hosts a PropertyGrid child so property rows are interactively editable.
class InspectorPanel final : public PanelBase {
public:
    using ChangeCallback = std::function<void(const std::string& name, const PropertyValue& value)>;

    InspectorPanel()
        : PanelBase("atlas.inspector", "Inspector")
        , m_grid(std::make_unique<PropertyGrid>()) {
        m_grid->setOnChange([this](const std::string& name, const PropertyValue& val) {
            if (m_onChange) m_onChange(name, val);
        });
    }

    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    void setSelectedEntityId(int id) {
        if (m_selectedEntityId == id) return;
        m_selectedEntityId = id;
        rebuildGrid();
    }
    [[nodiscard]] int selectedEntityId() const { return m_selectedEntityId; }

    void setTransform(float x, float y, float z) {
        m_transformX = x;
        m_transformY = y;
        m_transformZ = z;
        rebuildGrid();
    }

    [[nodiscard]] float transformX() const { return m_transformX; }
    [[nodiscard]] float transformY() const { return m_transformY; }
    [[nodiscard]] float transformZ() const { return m_transformZ; }

    struct PropertyEntry {
        std::string label;
        std::string value;
    };

    void clearProperties() { m_properties.clear(); rebuildGrid(); }
    void addProperty(const std::string& label, const std::string& value) {
        m_properties.push_back({label, value});
        rebuildGrid();
    }
    [[nodiscard]] const std::vector<PropertyEntry>& properties() const { return m_properties; }

    /// Register a callback that is invoked whenever the user edits a property.
    void setOnChange(ChangeCallback cb) { m_onChange = std::move(cb); }

    void arrange(const NF::Rect& bounds) override {
        PanelBase::arrange(bounds);
        // Title bar (22 px) + entity-ID row (20 px) + separator/header (26 px) = 68 px.
        // The grid fills the remaining height with a 4 px horizontal inset each side.
        static constexpr float kGridTopOffset  = 68.f; // pixels below panel top
        static constexpr float kGridSideInset  =  4.f; // pixels inset on each side
        NF::Rect gridBounds = {bounds.x + kGridSideInset,
                               bounds.y + kGridTopOffset,
                               std::max(0.f, bounds.w - kGridSideInset * 2.f),
                               std::max(0.f, bounds.h - kGridTopOffset - kGridSideInset)};
        m_grid->arrange(gridBounds);
    }

private:
    int m_selectedEntityId = -1;
    float m_transformX = 0.f;
    float m_transformY = 0.f;
    float m_transformZ = 0.f;
    std::vector<PropertyEntry> m_properties;
    std::unique_ptr<PropertyGrid> m_grid;
    ChangeCallback m_onChange;

    /// Rebuild the PropertyGrid items from the current transform and properties.
    void rebuildGrid() {
        m_grid->clear();
        if (m_selectedEntityId < 0) return;

        // Transform group
        PropertyItem xformGroup;
        xformGroup.name = "Transform";
        xformGroup.expanded = true;
        char buf[32];
        auto makeFloat = [&](const char* n, float v) {
            std::snprintf(buf, sizeof(buf), "%.3f", v);
            PropertyItem item;
            item.name = n;
            item.value = PropertyValue(std::string(buf));
            item.readOnly = false;
            return item;
        };
        xformGroup.children.push_back(makeFloat("X", m_transformX));
        xformGroup.children.push_back(makeFloat("Y", m_transformY));
        xformGroup.children.push_back(makeFloat("Z", m_transformZ));
        m_grid->addItem(std::move(xformGroup));

        // Custom properties group
        if (!m_properties.empty()) {
            PropertyItem propsGroup;
            propsGroup.name = "Properties";
            propsGroup.expanded = true;
            for (const auto& p : m_properties) {
                PropertyItem item;
                item.name = p.label;
                item.value = PropertyValue(p.value);
                item.readOnly = false;
                propsGroup.children.push_back(std::move(item));
            }
            m_grid->addItem(std::move(propsGroup));
        }
    }
};

} // namespace NF::UI::AtlasUI
