#pragma once
// NF::Editor — foliage painter editor
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

enum class FpvPlacementMode : uint8_t { Scatter, Align, Cluster, Grid };
inline const char* fpvPlacementModeName(FpvPlacementMode v) {
    switch (v) {
        case FpvPlacementMode::Scatter: return "Scatter";
        case FpvPlacementMode::Align:   return "Align";
        case FpvPlacementMode::Cluster: return "Cluster";
        case FpvPlacementMode::Grid:    return "Grid";
    }
    return "Unknown";
}

class FpvFoliageType {
public:
    explicit FpvFoliageType(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setMode(FpvPlacementMode v)  { m_mode    = v; }
    void setDensity(float v)          { m_density = v; }
    void setScaleMin(float v)         { m_scaleMin = v; }
    void setScaleMax(float v)         { m_scaleMax = v; }
    void setEnabled(bool v)           { m_enabled = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] FpvPlacementMode   mode()     const { return m_mode;     }
    [[nodiscard]] float              density()  const { return m_density;  }
    [[nodiscard]] float              scaleMin() const { return m_scaleMin; }
    [[nodiscard]] float              scaleMax() const { return m_scaleMax; }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t          m_id;
    std::string       m_name;
    FpvPlacementMode  m_mode     = FpvPlacementMode::Scatter;
    float             m_density  = 1.0f;
    float             m_scaleMin = 0.8f;
    float             m_scaleMax = 1.2f;
    bool              m_enabled  = true;
};

class FoliagePainterV1 {
public:
    bool addType(const FpvFoliageType& t) {
        for (auto& x : m_types) if (x.id() == t.id()) return false;
        m_types.push_back(t); return true;
    }
    bool removeType(uint32_t id) {
        auto it = std::find_if(m_types.begin(), m_types.end(),
            [&](const FpvFoliageType& t){ return t.id() == id; });
        if (it == m_types.end()) return false;
        m_types.erase(it); return true;
    }
    [[nodiscard]] FpvFoliageType* findType(uint32_t id) {
        for (auto& t : m_types) if (t.id() == id) return &t;
        return nullptr;
    }
    [[nodiscard]] size_t typeCount()    const { return m_types.size(); }
    void setBrushRadius(float v)              { m_brushRadius = v; }
    [[nodiscard]] float brushRadius()   const { return m_brushRadius; }

private:
    std::vector<FpvFoliageType> m_types;
    float                       m_brushRadius = 5.0f;
};

} // namespace NF
