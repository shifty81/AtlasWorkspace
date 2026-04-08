#pragma once
// NF::Editor — property grid widget
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace NF {

enum class PgPropType : uint8_t { Bool, Int, Float, String, Color, Vector, Enum, Object };
inline const char* pgPropTypeName(PgPropType v) {
    switch (v) {
        case PgPropType::Bool:   return "Bool";
        case PgPropType::Int:    return "Int";
        case PgPropType::Float:  return "Float";
        case PgPropType::String: return "String";
        case PgPropType::Color:  return "Color";
        case PgPropType::Vector: return "Vector";
        case PgPropType::Enum:   return "Enum";
        case PgPropType::Object: return "Object";
    }
    return "Unknown";
}

enum class PgEditMode : uint8_t { ReadOnly, Inline, Popup, Custom };
inline const char* pgEditModeName(PgEditMode v) {
    switch (v) {
        case PgEditMode::ReadOnly: return "ReadOnly";
        case PgEditMode::Inline:   return "Inline";
        case PgEditMode::Popup:    return "Popup";
        case PgEditMode::Custom:   return "Custom";
    }
    return "Unknown";
}

class PgProperty {
public:
    explicit PgProperty(uint32_t id, const std::string& name, PgPropType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setEditMode(PgEditMode v)        { m_editMode = v; }
    void setValue(const std::string& v)   { m_value    = v; }
    void setTooltip(const std::string& v) { m_tooltip  = v; }
    void setVisible(bool v)               { m_visible  = v; }
    void setReadOnly(bool v)              { m_readOnly = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] PgPropType         type()     const { return m_type;     }
    [[nodiscard]] PgEditMode         editMode() const { return m_editMode; }
    [[nodiscard]] const std::string& value()    const { return m_value;    }
    [[nodiscard]] const std::string& tooltip()  const { return m_tooltip;  }
    [[nodiscard]] bool               visible()  const { return m_visible;  }
    [[nodiscard]] bool               readOnly() const { return m_readOnly; }

private:
    uint32_t    m_id;
    std::string m_name;
    PgPropType  m_type;
    PgEditMode  m_editMode = PgEditMode::Inline;
    std::string m_value;
    std::string m_tooltip;
    bool        m_visible  = true;
    bool        m_readOnly = false;
};

class PropertyGridV1 {
public:
    bool addProperty(const PgProperty& p) {
        for (auto& x : m_properties) if (x.id() == p.id()) return false;
        m_properties.push_back(p); return true;
    }
    bool removeProperty(uint32_t id) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const PgProperty& p){ return p.id() == id; });
        if (it == m_properties.end()) return false;
        m_properties.erase(it); return true;
    }
    [[nodiscard]] PgProperty* findProperty(uint32_t id) {
        for (auto& p : m_properties) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t propertyCount() const { return m_properties.size(); }
    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0;
        for (auto& p : m_properties) if (p.visible()) ++n;
        return n;
    }
    bool setReadOnly(uint32_t id, bool v) {
        auto* p = findProperty(id);
        if (!p) return false;
        p->setReadOnly(v);
        return true;
    }

private:
    std::vector<PgProperty> m_properties;
};

} // namespace NF
