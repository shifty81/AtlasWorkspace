#pragma once
// NF::Editor — lightmap editor
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

enum class LmpBakeMode : uint8_t { Baked, Mixed, Realtime };
inline const char* lmpBakeModeName(LmpBakeMode v) {
    switch (v) {
        case LmpBakeMode::Baked:    return "Baked";
        case LmpBakeMode::Mixed:    return "Mixed";
        case LmpBakeMode::Realtime: return "Realtime";
    }
    return "Unknown";
}

enum class LmpResolution : uint8_t { R64, R128, R256, R512, R1024, R2048 };
inline const char* lmpResolutionName(LmpResolution v) {
    switch (v) {
        case LmpResolution::R64:   return "64";
        case LmpResolution::R128:  return "128";
        case LmpResolution::R256:  return "256";
        case LmpResolution::R512:  return "512";
        case LmpResolution::R1024: return "1024";
        case LmpResolution::R2048: return "2048";
    }
    return "Unknown";
}

class LmpObject {
public:
    explicit LmpObject(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setBakeMode(LmpBakeMode v)    { m_bakeMode   = v; }
    void setResolution(LmpResolution v){ m_resolution = v; }
    void setScale(float v)             { m_scale      = v; }
    void setContribute(bool v)         { m_contribute = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] LmpBakeMode        bakeMode()    const { return m_bakeMode;    }
    [[nodiscard]] LmpResolution      resolution()  const { return m_resolution;  }
    [[nodiscard]] float              scale()       const { return m_scale;       }
    [[nodiscard]] bool               contribute()  const { return m_contribute;  }

private:
    uint32_t      m_id;
    std::string   m_name;
    LmpBakeMode   m_bakeMode   = LmpBakeMode::Baked;
    LmpResolution m_resolution = LmpResolution::R256;
    float         m_scale      = 1.0f;
    bool          m_contribute = true;
};

class LightmapEditorV1 {
public:
    bool addObject(const LmpObject& o) {
        for (auto& x : m_objects) if (x.id() == o.id()) return false;
        m_objects.push_back(o); return true;
    }
    bool removeObject(uint32_t id) {
        auto it = std::find_if(m_objects.begin(), m_objects.end(),
            [&](const LmpObject& o){ return o.id() == id; });
        if (it == m_objects.end()) return false;
        m_objects.erase(it); return true;
    }
    [[nodiscard]] LmpObject* findObject(uint32_t id) {
        for (auto& o : m_objects) if (o.id() == id) return &o;
        return nullptr;
    }
    [[nodiscard]] size_t objectCount()      const { return m_objects.size(); }
    [[nodiscard]] size_t contributeCount()  const {
        size_t n = 0;
        for (auto& o : m_objects) if (o.contribute()) ++n;
        return n;
    }
    void setBaked(bool v)                         { m_baked = v; }
    [[nodiscard]] bool baked()              const { return m_baked; }

private:
    std::vector<LmpObject> m_objects;
    bool                   m_baked = false;
};

} // namespace NF
