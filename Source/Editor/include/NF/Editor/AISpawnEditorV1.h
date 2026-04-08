#pragma once
// NF::Editor — AI spawn editor
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

enum class AspSpawnMode : uint8_t { OnDemand, Wave, Continuous, Scripted };
inline const char* aspSpawnModeName(AspSpawnMode v) {
    switch (v) {
        case AspSpawnMode::OnDemand:   return "OnDemand";
        case AspSpawnMode::Wave:       return "Wave";
        case AspSpawnMode::Continuous: return "Continuous";
        case AspSpawnMode::Scripted:   return "Scripted";
    }
    return "Unknown";
}

class AspSpawnPoint {
public:
    explicit AspSpawnPoint(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setMode(AspSpawnMode v)      { m_mode     = v; }
    void setMaxCount(uint32_t v)      { m_maxCount = v; }
    void setInterval(float v)         { m_interval = v; }
    void setEnabled(bool v)           { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] AspSpawnMode       mode()     const { return m_mode;     }
    [[nodiscard]] uint32_t           maxCount() const { return m_maxCount; }
    [[nodiscard]] float              interval() const { return m_interval; }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t    m_id;
    std::string m_name;
    AspSpawnMode m_mode     = AspSpawnMode::OnDemand;
    uint32_t     m_maxCount = 10;
    float        m_interval = 5.0f;
    bool         m_enabled  = true;
};

class AISpawnEditorV1 {
public:
    bool addPoint(const AspSpawnPoint& p) {
        for (auto& x : m_points) if (x.id() == p.id()) return false;
        m_points.push_back(p); return true;
    }
    bool removePoint(uint32_t id) {
        auto it = std::find_if(m_points.begin(), m_points.end(),
            [&](const AspSpawnPoint& p){ return p.id() == id; });
        if (it == m_points.end()) return false;
        m_points.erase(it); return true;
    }
    [[nodiscard]] AspSpawnPoint* findPoint(uint32_t id) {
        for (auto& p : m_points) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t pointCount()   const { return m_points.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& p : m_points) if (p.enabled()) ++n;
        return n;
    }

private:
    std::vector<AspSpawnPoint> m_points;
};

} // namespace NF
