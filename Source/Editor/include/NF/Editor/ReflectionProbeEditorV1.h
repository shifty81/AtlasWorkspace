#pragma once
// NF::Editor — reflection probe editor
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

enum class RpvRefreshMode : uint8_t { Baked, OnAwake, EveryFrame, Custom };
inline const char* rpvRefreshModeName(RpvRefreshMode v) {
    switch (v) {
        case RpvRefreshMode::Baked:      return "Baked";
        case RpvRefreshMode::OnAwake:    return "OnAwake";
        case RpvRefreshMode::EveryFrame: return "EveryFrame";
        case RpvRefreshMode::Custom:     return "Custom";
    }
    return "Unknown";
}

class RpvProbe {
public:
    explicit RpvProbe(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setRefreshMode(RpvRefreshMode v) { m_refresh    = v; }
    void setResolution(uint32_t v)        { m_resolution = v; }
    void setIntensity(float v)            { m_intensity  = v; }
    void setEnabled(bool v)               { m_enabled    = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] RpvRefreshMode     refreshMode()const { return m_refresh;    }
    [[nodiscard]] uint32_t           resolution() const { return m_resolution; }
    [[nodiscard]] float              intensity()  const { return m_intensity;  }
    [[nodiscard]] bool               enabled()    const { return m_enabled;    }

private:
    uint32_t       m_id;
    std::string    m_name;
    RpvRefreshMode m_refresh    = RpvRefreshMode::Baked;
    uint32_t       m_resolution = 128;
    float          m_intensity  = 1.0f;
    bool           m_enabled    = true;
};

class ReflectionProbeEditorV1 {
public:
    bool addProbe(const RpvProbe& p) {
        for (auto& x : m_probes) if (x.id() == p.id()) return false;
        m_probes.push_back(p); return true;
    }
    bool removeProbe(uint32_t id) {
        auto it = std::find_if(m_probes.begin(), m_probes.end(),
            [&](const RpvProbe& p){ return p.id() == id; });
        if (it == m_probes.end()) return false;
        m_probes.erase(it); return true;
    }
    [[nodiscard]] RpvProbe* findProbe(uint32_t id) {
        for (auto& p : m_probes) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t probeCount()   const { return m_probes.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& p : m_probes) if (p.enabled()) ++n;
        return n;
    }

private:
    std::vector<RpvProbe> m_probes;
};

} // namespace NF
