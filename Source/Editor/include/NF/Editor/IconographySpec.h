#pragma once
// NF::Editor — icon set and specification management
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

enum class IconSpecSize : uint8_t { XS, SM, MD, LG, XL };
inline int iconSpecSizePx(IconSpecSize v) {
    switch (v) {
        case IconSpecSize::XS: return 16;
        case IconSpecSize::SM: return 24;
        case IconSpecSize::MD: return 32;
        case IconSpecSize::LG: return 48;
        case IconSpecSize::XL: return 64;
    }
    return 0;
}

enum class IconStyle : uint8_t { Filled, Outlined, TwoTone, Rounded, Sharp };
inline const char* iconStyleName(IconStyle v) {
    switch (v) {
        case IconStyle::Filled:   return "Filled";
        case IconStyle::Outlined: return "Outlined";
        case IconStyle::TwoTone:  return "TwoTone";
        case IconStyle::Rounded:  return "Rounded";
        case IconStyle::Sharp:    return "Sharp";
    }
    return "Unknown";
}

class IconEntry {
public:
    explicit IconEntry(uint32_t id, const std::string& name, const std::string& category,
                       IconSpecSize size, IconStyle style, uint32_t unicode)
        : m_id(id), m_name(name), m_category(category),
          m_size(size), m_style(style), m_unicode(unicode) {}

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] const std::string& category() const { return m_category; }
    [[nodiscard]] IconSpecSize       size()     const { return m_size;     }
    [[nodiscard]] IconStyle          style()    const { return m_style;    }
    [[nodiscard]] uint32_t           unicode()  const { return m_unicode;  }

private:
    uint32_t     m_id;
    std::string  m_name;
    std::string  m_category;
    IconSpecSize m_size;
    IconStyle    m_style;
    uint32_t     m_unicode;
};

class IconographySpec {
public:
    bool addIcon(const IconEntry& e) {
        for (auto& x : m_icons) if (x.id() == e.id()) return false;
        m_icons.push_back(e); return true;
    }
    bool removeIcon(uint32_t id) {
        auto it = std::find_if(m_icons.begin(), m_icons.end(),
            [&](const IconEntry& e){ return e.id() == id; });
        if (it == m_icons.end()) return false;
        m_icons.erase(it); return true;
    }
    [[nodiscard]] IconEntry* findIcon(uint32_t id) {
        for (auto& e : m_icons) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t iconCount() const { return m_icons.size(); }
    [[nodiscard]] std::vector<IconEntry> filterBySize(IconSpecSize s) const {
        std::vector<IconEntry> result;
        for (auto& e : m_icons) if (e.size() == s) result.push_back(e);
        return result;
    }

private:
    std::vector<IconEntry> m_icons;
};

} // namespace NF

