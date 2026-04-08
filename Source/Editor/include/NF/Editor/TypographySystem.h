#pragma once
// NF::Editor — text style and typography system
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

enum class TypoScale : uint8_t { Caption, Body, Subtitle, Title, Display };
inline const char* typoScaleName(TypoScale v) {
    switch (v) {
        case TypoScale::Caption:  return "Caption";
        case TypoScale::Body:     return "Body";
        case TypoScale::Subtitle: return "Subtitle";
        case TypoScale::Title:    return "Title";
        case TypoScale::Display:  return "Display";
    }
    return "Unknown";
}

enum class TypoWeight : uint8_t { Thin, Light, Regular, Medium, Bold, Black };
inline const char* typoWeightName(TypoWeight v) {
    switch (v) {
        case TypoWeight::Thin:    return "Thin";
        case TypoWeight::Light:   return "Light";
        case TypoWeight::Regular: return "Regular";
        case TypoWeight::Medium:  return "Medium";
        case TypoWeight::Bold:    return "Bold";
        case TypoWeight::Black:   return "Black";
    }
    return "Unknown";
}

class TypoStyle {
public:
    explicit TypoStyle(uint32_t id, const std::string& name, TypoScale scale, TypoWeight weight)
        : m_id(id), m_name(name), m_scale(scale), m_weight(weight) {}

    void setSizeOverride(float v)    { m_sizeOverride  = v; }
    void setLineHeight(float v)      { m_lineHeight     = v; }
    void setLetterSpacing(float v)   { m_letterSpacing  = v; }
    void setEnabled(bool v)          { m_enabled        = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;           }
    [[nodiscard]] const std::string& name()          const { return m_name;         }
    [[nodiscard]] TypoScale          scale()         const { return m_scale;        }
    [[nodiscard]] TypoWeight         weight()        const { return m_weight;       }
    [[nodiscard]] float              sizeOverride()  const { return m_sizeOverride; }
    [[nodiscard]] float              lineHeight()    const { return m_lineHeight;   }
    [[nodiscard]] float              letterSpacing() const { return m_letterSpacing;}
    [[nodiscard]] bool               enabled()       const { return m_enabled;      }

private:
    uint32_t    m_id;
    std::string m_name;
    TypoScale   m_scale;
    TypoWeight  m_weight;
    float       m_sizeOverride  = 0.0f;
    float       m_lineHeight    = 1.2f;
    float       m_letterSpacing = 0.0f;
    bool        m_enabled       = true;
};

class TypographySystem {
public:
    bool addStyle(const TypoStyle& s) {
        for (auto& x : m_styles) if (x.id() == s.id()) return false;
        m_styles.push_back(s); return true;
    }
    bool removeStyle(uint32_t id) {
        auto it = std::find_if(m_styles.begin(), m_styles.end(),
            [&](const TypoStyle& s){ return s.id() == id; });
        if (it == m_styles.end()) return false;
        m_styles.erase(it); return true;
    }
    [[nodiscard]] TypoStyle* findStyle(uint32_t id) {
        for (auto& s : m_styles) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t styleCount() const { return m_styles.size(); }

private:
    std::vector<TypoStyle> m_styles;
};

} // namespace NF
