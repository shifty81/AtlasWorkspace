#pragma once
// NF::Editor — terrain brush editor
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

enum class TbrBrushMode : uint8_t { Raise, Lower, Smooth, Flatten, Paint, Stamp };
inline const char* tbrBrushModeName(TbrBrushMode v) {
    switch (v) {
        case TbrBrushMode::Raise:   return "Raise";
        case TbrBrushMode::Lower:   return "Lower";
        case TbrBrushMode::Smooth:  return "Smooth";
        case TbrBrushMode::Flatten: return "Flatten";
        case TbrBrushMode::Paint:   return "Paint";
        case TbrBrushMode::Stamp:   return "Stamp";
    }
    return "Unknown";
}

class TbrBrush {
public:
    explicit TbrBrush(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setMode(TbrBrushMode v)     { m_mode     = v; }
    void setRadius(float v)          { m_radius   = v; }
    void setStrength(float v)        { m_strength = v; }
    void setFalloff(float v)         { m_falloff  = v; }
    void setEnabled(bool v)          { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] TbrBrushMode       mode()     const { return m_mode;     }
    [[nodiscard]] float              radius()   const { return m_radius;   }
    [[nodiscard]] float              strength() const { return m_strength; }
    [[nodiscard]] float              falloff()  const { return m_falloff;  }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t    m_id;
    std::string m_name;
    TbrBrushMode m_mode     = TbrBrushMode::Raise;
    float        m_radius   = 5.0f;
    float        m_strength = 1.0f;
    float        m_falloff  = 0.5f;
    bool         m_enabled  = true;
};

class TerrainBrushV1 {
public:
    bool addBrush(const TbrBrush& b) {
        for (auto& x : m_brushes) if (x.id() == b.id()) return false;
        m_brushes.push_back(b); return true;
    }
    bool removeBrush(uint32_t id) {
        auto it = std::find_if(m_brushes.begin(), m_brushes.end(),
            [&](const TbrBrush& b){ return b.id() == id; });
        if (it == m_brushes.end()) return false;
        m_brushes.erase(it); return true;
    }
    [[nodiscard]] TbrBrush* findBrush(uint32_t id) {
        for (auto& b : m_brushes) if (b.id() == id) return &b;
        return nullptr;
    }
    [[nodiscard]] size_t brushCount()   const { return m_brushes.size(); }
    void setActiveBrush(uint32_t id)          { m_activeBrush = id; }
    [[nodiscard]] uint32_t activeBrush() const { return m_activeBrush; }

private:
    std::vector<TbrBrush> m_brushes;
    uint32_t              m_activeBrush = 0;
};

} // namespace NF
