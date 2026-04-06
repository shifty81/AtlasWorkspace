#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <cstdio>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI InspectorPanel — displays selected entity properties.
/// Replaces the legacy NF::Editor::InspectorPanel for the AtlasUI framework.
class InspectorPanel final : public PanelBase {
public:
    InspectorPanel()
        : PanelBase("atlas.inspector", "Inspector") {}

    void paint(IPaintContext& context) override;

    void setSelectedEntityId(int id) { m_selectedEntityId = id; }
    [[nodiscard]] int selectedEntityId() const { return m_selectedEntityId; }

    void setTransform(float x, float y, float z) {
        m_transformX = x;
        m_transformY = y;
        m_transformZ = z;
    }

    [[nodiscard]] float transformX() const { return m_transformX; }
    [[nodiscard]] float transformY() const { return m_transformY; }
    [[nodiscard]] float transformZ() const { return m_transformZ; }

    struct PropertyEntry {
        std::string label;
        std::string value;
    };

    void clearProperties() { m_properties.clear(); }
    void addProperty(const std::string& label, const std::string& value) {
        m_properties.push_back({label, value});
    }
    [[nodiscard]] const std::vector<PropertyEntry>& properties() const { return m_properties; }

private:
    int m_selectedEntityId = -1;
    float m_transformX = 0.f;
    float m_transformY = 0.f;
    float m_transformZ = 0.f;
    std::vector<PropertyEntry> m_properties;
};

} // namespace NF::UI::AtlasUI
