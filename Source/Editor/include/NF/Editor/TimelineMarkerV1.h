#pragma once
// NF::Editor — timeline marker editor
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

enum class TlmMarkerKind : uint8_t { Cue, Loop, Cut, Sync, Custom };
inline const char* tlmMarkerKindName(TlmMarkerKind v) {
    switch (v) {
        case TlmMarkerKind::Cue:    return "Cue";
        case TlmMarkerKind::Loop:   return "Loop";
        case TlmMarkerKind::Cut:    return "Cut";
        case TlmMarkerKind::Sync:   return "Sync";
        case TlmMarkerKind::Custom: return "Custom";
    }
    return "Unknown";
}

class TlmMarker {
public:
    explicit TlmMarker(uint32_t id, const std::string& label) : m_id(id), m_label(label) {}

    void setKind(TlmMarkerKind v)        { m_kind    = v; }
    void setTime(float v)                { m_time    = v; }
    void setColor(uint32_t v)            { m_color   = v; }
    void setEnabled(bool v)              { m_enabled = v; }

    [[nodiscard]] uint32_t           id()      const { return m_id;      }
    [[nodiscard]] const std::string& label()   const { return m_label;   }
    [[nodiscard]] TlmMarkerKind      kind()    const { return m_kind;    }
    [[nodiscard]] float              time()    const { return m_time;    }
    [[nodiscard]] uint32_t           color()   const { return m_color;   }
    [[nodiscard]] bool               enabled() const { return m_enabled; }

private:
    uint32_t      m_id;
    std::string   m_label;
    TlmMarkerKind m_kind    = TlmMarkerKind::Cue;
    float         m_time    = 0.0f;
    uint32_t      m_color   = 0xFFFFFFFF;
    bool          m_enabled = true;
};

class TimelineMarkerV1 {
public:
    bool addMarker(const TlmMarker& m) {
        for (auto& x : m_markers) if (x.id() == m.id()) return false;
        m_markers.push_back(m); return true;
    }
    bool removeMarker(uint32_t id) {
        auto it = std::find_if(m_markers.begin(), m_markers.end(),
            [&](const TlmMarker& m){ return m.id() == id; });
        if (it == m_markers.end()) return false;
        m_markers.erase(it); return true;
    }
    [[nodiscard]] TlmMarker* findMarker(uint32_t id) {
        for (auto& m : m_markers) if (m.id() == id) return &m;
        return nullptr;
    }
    [[nodiscard]] size_t markerCount()   const { return m_markers.size(); }
    [[nodiscard]] std::vector<TlmMarker> markersInRange(float t0, float t1) const {
        std::vector<TlmMarker> result;
        for (auto& m : m_markers) if (m.time() >= t0 && m.time() <= t1) result.push_back(m);
        return result;
    }

private:
    std::vector<TlmMarker> m_markers;
};

} // namespace NF
