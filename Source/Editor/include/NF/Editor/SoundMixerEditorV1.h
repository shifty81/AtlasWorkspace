#pragma once
// NF::Editor — sound mixer editor
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

enum class SmChannelType : uint8_t { Master, Music, SFX, Voice, Ambient, UI };
inline const char* smChannelTypeName(SmChannelType v) {
    switch (v) {
        case SmChannelType::Master:  return "Master";
        case SmChannelType::Music:   return "Music";
        case SmChannelType::SFX:     return "SFX";
        case SmChannelType::Voice:   return "Voice";
        case SmChannelType::Ambient: return "Ambient";
        case SmChannelType::UI:      return "UI";
    }
    return "Unknown";
}

enum class SmFaderCurve : uint8_t { Linear, Logarithmic, Exponential, SCurve };
inline const char* smFaderCurveName(SmFaderCurve v) {
    switch (v) {
        case SmFaderCurve::Linear:      return "Linear";
        case SmFaderCurve::Logarithmic: return "Logarithmic";
        case SmFaderCurve::Exponential: return "Exponential";
        case SmFaderCurve::SCurve:      return "SCurve";
    }
    return "Unknown";
}

class SmChannel {
public:
    explicit SmChannel(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setType(SmChannelType v) { m_type   = v; }
    void setVolume(float v)       { m_volume = v; }
    void setPan(float v)          { m_pan    = v; }
    void setMuted(bool v)         { m_muted  = v; }
    void setSolo(bool v)          { m_solo   = v; }

    [[nodiscard]] uint32_t           id()     const { return m_id;     }
    [[nodiscard]] const std::string& name()   const { return m_name;   }
    [[nodiscard]] SmChannelType      type()   const { return m_type;   }
    [[nodiscard]] float              volume() const { return m_volume; }
    [[nodiscard]] float              pan()    const { return m_pan;    }
    [[nodiscard]] bool               muted()  const { return m_muted;  }
    [[nodiscard]] bool               solo()   const { return m_solo;   }

private:
    uint32_t       m_id;
    std::string    m_name;
    SmChannelType  m_type   = SmChannelType::Music;
    float          m_volume = 1.0f;
    float          m_pan    = 0.0f;
    bool           m_muted  = false;
    bool           m_solo   = false;
};

class SoundMixerEditorV1 {
public:
    bool addChannel(const SmChannel& c) {
        for (auto& x : m_channels) if (x.id() == c.id()) return false;
        m_channels.push_back(c); return true;
    }
    bool removeChannel(uint32_t id) {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
            [&](const SmChannel& c){ return c.id() == id; });
        if (it == m_channels.end()) return false;
        m_channels.erase(it); return true;
    }
    [[nodiscard]] SmChannel* findChannel(uint32_t id) {
        for (auto& c : m_channels) if (c.id() == id) return &c;
        return nullptr;
    }
    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }
    [[nodiscard]] size_t mutedCount() const {
        size_t n = 0;
        for (auto& c : m_channels) if (c.muted()) ++n;
        return n;
    }
    [[nodiscard]] size_t soloCount() const {
        size_t n = 0;
        for (auto& c : m_channels) if (c.solo()) ++n;
        return n;
    }
    void setFaderCurve(SmFaderCurve v) { m_faderCurve = v; }
    [[nodiscard]] SmFaderCurve faderCurve() const { return m_faderCurve; }

private:
    std::vector<SmChannel> m_channels;
    SmFaderCurve           m_faderCurve = SmFaderCurve::Linear;
};

} // namespace NF
