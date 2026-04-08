#pragma once
// NF::Editor — asset tag editor
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

enum class AtgTagCategory : uint8_t { Type, Status, Platform, Pipeline, Custom };
inline const char* atgTagCategoryName(AtgTagCategory v) {
    switch (v) {
        case AtgTagCategory::Type:     return "Type";
        case AtgTagCategory::Status:   return "Status";
        case AtgTagCategory::Platform: return "Platform";
        case AtgTagCategory::Pipeline: return "Pipeline";
        case AtgTagCategory::Custom:   return "Custom";
    }
    return "Unknown";
}

class AtgTag {
public:
    explicit AtgTag(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setCategory(AtgTagCategory v)    { m_category = v; }
    void setColor(uint32_t v)             { m_color    = v; }
    void setEnabled(bool v)               { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] AtgTagCategory     category() const { return m_category; }
    [[nodiscard]] uint32_t           color()    const { return m_color;    }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t       m_id;
    std::string    m_name;
    AtgTagCategory m_category = AtgTagCategory::Custom;
    uint32_t       m_color    = 0xFFFFFF;
    bool           m_enabled  = true;
};

class AssetTagEditorV1 {
public:
    bool addTag(const AtgTag& t) {
        for (auto& x : m_tags) if (x.id() == t.id()) return false;
        m_tags.push_back(t); return true;
    }
    bool removeTag(uint32_t id) {
        auto it = std::find_if(m_tags.begin(), m_tags.end(),
            [&](const AtgTag& t){ return t.id() == id; });
        if (it == m_tags.end()) return false;
        m_tags.erase(it); return true;
    }
    [[nodiscard]] AtgTag* findTag(uint32_t id) {
        for (auto& t : m_tags) if (t.id() == id) return &t;
        return nullptr;
    }
    [[nodiscard]] size_t tagCount()    const { return m_tags.size(); }
    [[nodiscard]] std::vector<AtgTag> filterByCategory(AtgTagCategory cat) const {
        std::vector<AtgTag> result;
        for (auto& t : m_tags) if (t.category() == cat) result.push_back(t);
        return result;
    }

private:
    std::vector<AtgTag> m_tags;
};

} // namespace NF
