#pragma once
// NF::Editor — road tool editor
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

enum class RtlRoadSurface : uint8_t { Asphalt, Gravel, Dirt, Cobblestone, Sand };
inline const char* rtlRoadSurfaceName(RtlRoadSurface v) {
    switch (v) {
        case RtlRoadSurface::Asphalt:     return "Asphalt";
        case RtlRoadSurface::Gravel:      return "Gravel";
        case RtlRoadSurface::Dirt:        return "Dirt";
        case RtlRoadSurface::Cobblestone: return "Cobblestone";
        case RtlRoadSurface::Sand:        return "Sand";
    }
    return "Unknown";
}

class RtlSegment {
public:
    explicit RtlSegment(uint32_t id) : m_id(id) {}

    void setSurface(RtlRoadSurface v)   { m_surface = v; }
    void setWidth(float v)              { m_width   = v; }
    void setLength(float v)             { m_length  = v; }
    void setEnabled(bool v)             { m_enabled = v; }

    [[nodiscard]] uint32_t        id()      const { return m_id;      }
    [[nodiscard]] RtlRoadSurface  surface() const { return m_surface; }
    [[nodiscard]] float           width()   const { return m_width;   }
    [[nodiscard]] float           length()  const { return m_length;  }
    [[nodiscard]] bool            enabled() const { return m_enabled; }

private:
    uint32_t       m_id;
    RtlRoadSurface m_surface = RtlRoadSurface::Asphalt;
    float          m_width   = 4.0f;
    float          m_length  = 10.0f;
    bool           m_enabled = true;
};

class RoadToolV1 {
public:
    bool addSegment(const RtlSegment& s) {
        for (auto& x : m_segments) if (x.id() == s.id()) return false;
        m_segments.push_back(s); return true;
    }
    bool removeSegment(uint32_t id) {
        auto it = std::find_if(m_segments.begin(), m_segments.end(),
            [&](const RtlSegment& s){ return s.id() == id; });
        if (it == m_segments.end()) return false;
        m_segments.erase(it); return true;
    }
    [[nodiscard]] RtlSegment* findSegment(uint32_t id) {
        for (auto& s : m_segments) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t segmentCount()   const { return m_segments.size(); }
    void setSnapping(bool v)                    { m_snapping = v; }
    [[nodiscard]] bool snapping()         const { return m_snapping; }

private:
    std::vector<RtlSegment> m_segments;
    bool                    m_snapping = true;
};

} // namespace NF
