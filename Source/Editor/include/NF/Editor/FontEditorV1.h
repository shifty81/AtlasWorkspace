#pragma once
// NF::Editor — font editor
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

enum class FeStyle : uint8_t { Regular, Bold, Italic, BoldItalic };
inline const char* feStyleName(FeStyle v) {
    switch (v) {
        case FeStyle::Regular:    return "Regular";
        case FeStyle::Bold:       return "Bold";
        case FeStyle::Italic:     return "Italic";
        case FeStyle::BoldItalic: return "BoldItalic";
    }
    return "Unknown";
}

enum class FeHinting : uint8_t { None, Slight, Medium, Full };
inline const char* feHintingName(FeHinting v) {
    switch (v) {
        case FeHinting::None:   return "None";
        case FeHinting::Slight: return "Slight";
        case FeHinting::Medium: return "Medium";
        case FeHinting::Full:   return "Full";
    }
    return "Unknown";
}

class FeGlyph {
public:
    explicit FeGlyph(uint32_t codepoint) : m_codepoint(codepoint) {}

    void setAdvance(float v)   { m_advance  = v; }
    void setBearingX(float v)  { m_bearingX = v; }
    void setBearingY(float v)  { m_bearingY = v; }
    void setWidth(uint32_t v)  { m_width    = v; }
    void setHeight(uint32_t v) { m_height   = v; }

    [[nodiscard]] uint32_t codepoint() const { return m_codepoint; }
    [[nodiscard]] float    advance()   const { return m_advance;   }
    [[nodiscard]] float    bearingX()  const { return m_bearingX;  }
    [[nodiscard]] float    bearingY()  const { return m_bearingY;  }
    [[nodiscard]] uint32_t width()     const { return m_width;     }
    [[nodiscard]] uint32_t height()    const { return m_height;    }

private:
    uint32_t m_codepoint;
    float    m_advance  = 0.0f;
    float    m_bearingX = 0.0f;
    float    m_bearingY = 0.0f;
    uint32_t m_width    = 0;
    uint32_t m_height   = 0;
};

class FontEditorV1 {
public:
    explicit FontEditorV1(uint32_t id, const std::string& familyName)
        : m_id(id), m_familyName(familyName) {}

    void setStyle(FeStyle v)          { m_style      = v; }
    void setHinting(FeHinting v)      { m_hinting    = v; }
    void setSize(uint32_t v)          { m_size       = v; }
    void setAntiAlias(bool v)         { m_antiAlias  = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& familyName() const { return m_familyName; }
    [[nodiscard]] FeStyle            style()      const { return m_style;      }
    [[nodiscard]] FeHinting          hinting()    const { return m_hinting;    }
    [[nodiscard]] uint32_t           size()       const { return m_size;       }
    [[nodiscard]] bool               antiAlias()  const { return m_antiAlias;  }
    [[nodiscard]] size_t             glyphCount() const { return m_glyphs.size(); }

    bool addGlyph(const FeGlyph& g) {
        for (auto& x : m_glyphs) if (x.codepoint() == g.codepoint()) return false;
        m_glyphs.push_back(g); return true;
    }
    [[nodiscard]] FeGlyph* findGlyph(uint32_t codepoint) {
        for (auto& g : m_glyphs) if (g.codepoint() == codepoint) return &g;
        return nullptr;
    }

private:
    uint32_t           m_id;
    std::string        m_familyName;
    FeStyle            m_style     = FeStyle::Regular;
    FeHinting          m_hinting   = FeHinting::Slight;
    uint32_t           m_size      = 12;
    bool               m_antiAlias = true;
    std::vector<FeGlyph> m_glyphs;
};

} // namespace NF
