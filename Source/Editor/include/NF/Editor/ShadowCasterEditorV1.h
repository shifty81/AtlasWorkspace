#pragma once
// NF::Editor — shadow caster editor
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

enum class ScvShadowQuality : uint8_t { Low, Medium, High, Ultra };
inline const char* scvShadowQualityName(ScvShadowQuality v) {
    switch (v) {
        case ScvShadowQuality::Low:    return "Low";
        case ScvShadowQuality::Medium: return "Medium";
        case ScvShadowQuality::High:   return "High";
        case ScvShadowQuality::Ultra:  return "Ultra";
    }
    return "Unknown";
}

class ScvCaster {
public:
    explicit ScvCaster(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setQuality(ScvShadowQuality v) { m_quality   = v; }
    void setBias(float v)               { m_bias      = v; }
    void setDistance(float v)           { m_distance  = v; }
    void setCastShadow(bool v)          { m_cast      = v; }
    void setReceiveShadow(bool v)       { m_receive   = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;       }
    [[nodiscard]] const std::string& name()          const { return m_name;     }
    [[nodiscard]] ScvShadowQuality   quality()       const { return m_quality;  }
    [[nodiscard]] float              bias()          const { return m_bias;     }
    [[nodiscard]] float              distance()      const { return m_distance; }
    [[nodiscard]] bool               castShadow()    const { return m_cast;     }
    [[nodiscard]] bool               receiveShadow() const { return m_receive;  }

private:
    uint32_t          m_id;
    std::string       m_name;
    ScvShadowQuality  m_quality  = ScvShadowQuality::Medium;
    float             m_bias     = 0.05f;
    float             m_distance = 50.0f;
    bool              m_cast     = true;
    bool              m_receive  = true;
};

class ShadowCasterEditorV1 {
public:
    bool addCaster(const ScvCaster& c) {
        for (auto& x : m_casters) if (x.id() == c.id()) return false;
        m_casters.push_back(c); return true;
    }
    bool removeCaster(uint32_t id) {
        auto it = std::find_if(m_casters.begin(), m_casters.end(),
            [&](const ScvCaster& c){ return c.id() == id; });
        if (it == m_casters.end()) return false;
        m_casters.erase(it); return true;
    }
    [[nodiscard]] ScvCaster* findCaster(uint32_t id) {
        for (auto& c : m_casters) if (c.id() == id) return &c;
        return nullptr;
    }
    [[nodiscard]] size_t casterCount()       const { return m_casters.size(); }
    [[nodiscard]] size_t castingShadowCount() const {
        size_t n = 0;
        for (auto& c : m_casters) if (c.castShadow()) ++n;
        return n;
    }

private:
    std::vector<ScvCaster> m_casters;
};

} // namespace NF
