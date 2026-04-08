#pragma once
// NF::Editor — texture viewer
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

enum class TvFormat : uint8_t { R8, RG8, RGB8, RGBA8, R16F, RG16F, RGB16F, RGBA16F, BC1, BC3, BC5, BC7 };
inline const char* tvFormatName(TvFormat v) {
    switch (v) {
        case TvFormat::R8:     return "R8";
        case TvFormat::RG8:    return "RG8";
        case TvFormat::RGB8:   return "RGB8";
        case TvFormat::RGBA8:  return "RGBA8";
        case TvFormat::R16F:   return "R16F";
        case TvFormat::RG16F:  return "RG16F";
        case TvFormat::RGB16F: return "RGB16F";
        case TvFormat::RGBA16F:return "RGBA16F";
        case TvFormat::BC1:    return "BC1";
        case TvFormat::BC3:    return "BC3";
        case TvFormat::BC5:    return "BC5";
        case TvFormat::BC7:    return "BC7";
    }
    return "Unknown";
}

enum class TvChannel : uint8_t { RGBA, R, G, B, A };
inline const char* tvChannelName(TvChannel v) {
    switch (v) {
        case TvChannel::RGBA: return "RGBA";
        case TvChannel::R:    return "R";
        case TvChannel::G:    return "G";
        case TvChannel::B:    return "B";
        case TvChannel::A:    return "A";
    }
    return "Unknown";
}

class TvMip {
public:
    explicit TvMip(uint32_t level, uint32_t w, uint32_t h)
        : m_level(level), m_width(w), m_height(h) {}

    void setSizeBytes(uint32_t v) { m_sizeBytes = v; }

    [[nodiscard]] uint32_t level()     const { return m_level;     }
    [[nodiscard]] uint32_t width()     const { return m_width;     }
    [[nodiscard]] uint32_t height()    const { return m_height;    }
    [[nodiscard]] uint32_t sizeBytes() const { return m_sizeBytes; }

private:
    uint32_t m_level     = 0;
    uint32_t m_width     = 1;
    uint32_t m_height    = 1;
    uint32_t m_sizeBytes = 0;
};

class TextureViewerV1 {
public:
    void setTextureId(uint32_t v)    { m_textureId  = v; }
    void setFormat(TvFormat v)       { m_format     = v; }
    void setChannel(TvChannel v)     { m_channel    = v; }
    void setZoom(float v)            { m_zoom       = v; }
    void setShowAlpha(bool v)        { m_showAlpha  = v; }
    void setActiveMip(uint32_t v)    { m_activeMip  = v; }

    [[nodiscard]] uint32_t   textureId()  const { return m_textureId;  }
    [[nodiscard]] TvFormat   format()     const { return m_format;     }
    [[nodiscard]] TvChannel  channel()    const { return m_channel;    }
    [[nodiscard]] float      zoom()       const { return m_zoom;       }
    [[nodiscard]] bool       showAlpha()  const { return m_showAlpha;  }
    [[nodiscard]] uint32_t   activeMip()  const { return m_activeMip;  }
    [[nodiscard]] size_t     mipCount()   const { return m_mips.size(); }

    bool addMip(const TvMip& m) {
        for (auto& x : m_mips) if (x.level() == m.level()) return false;
        m_mips.push_back(m); return true;
    }
    [[nodiscard]] TvMip* findMip(uint32_t level) {
        for (auto& m : m_mips) if (m.level() == level) return &m;
        return nullptr;
    }

private:
    uint32_t           m_textureId = 0;
    TvFormat           m_format    = TvFormat::RGBA8;
    TvChannel          m_channel   = TvChannel::RGBA;
    float              m_zoom      = 1.0f;
    bool               m_showAlpha = false;
    uint32_t           m_activeMip = 0;
    std::vector<TvMip> m_mips;
};

} // namespace NF
