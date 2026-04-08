#pragma once
// NF::Editor — animation curve editor
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

enum class AcInterp : uint8_t { Constant, Linear, Bezier, Hermite };
inline const char* acInterpName(AcInterp v) {
    switch (v) {
        case AcInterp::Constant: return "Constant";
        case AcInterp::Linear:   return "Linear";
        case AcInterp::Bezier:   return "Bezier";
        case AcInterp::Hermite:  return "Hermite";
    }
    return "Unknown";
}

enum class AcTangentMode : uint8_t { Auto, Free, Aligned, Flat };
inline const char* acTangentModeName(AcTangentMode v) {
    switch (v) {
        case AcTangentMode::Auto:    return "Auto";
        case AcTangentMode::Free:    return "Free";
        case AcTangentMode::Aligned: return "Aligned";
        case AcTangentMode::Flat:    return "Flat";
    }
    return "Unknown";
}

class AcKeyframe {
public:
    explicit AcKeyframe(uint32_t id, float time, float value)
        : m_id(id), m_time(time), m_value(value) {}

    void setInterp(AcInterp v)       { m_interp      = v; }
    void setTangentMode(AcTangentMode v){ m_tangentMode = v; }
    void setInTangent(float v)       { m_inTangent   = v; }
    void setOutTangent(float v)      { m_outTangent  = v; }
    void setTime(float v)            { m_time        = v; }
    void setValue(float v)           { m_value       = v; }

    [[nodiscard]] uint32_t      id()          const { return m_id;          }
    [[nodiscard]] float         time()        const { return m_time;        }
    [[nodiscard]] float         value()       const { return m_value;       }
    [[nodiscard]] AcInterp      interp()      const { return m_interp;      }
    [[nodiscard]] AcTangentMode tangentMode() const { return m_tangentMode; }
    [[nodiscard]] float         inTangent()   const { return m_inTangent;   }
    [[nodiscard]] float         outTangent()  const { return m_outTangent;  }

private:
    uint32_t      m_id;
    float         m_time        = 0.0f;
    float         m_value       = 0.0f;
    AcInterp      m_interp      = AcInterp::Linear;
    AcTangentMode m_tangentMode = AcTangentMode::Auto;
    float         m_inTangent   = 0.0f;
    float         m_outTangent  = 0.0f;
};

class AnimationCurveEditorV1 {
public:
    bool addKeyframe(const AcKeyframe& k) {
        for (auto& x : m_keyframes) if (x.id() == k.id()) return false;
        m_keyframes.push_back(k); return true;
    }
    bool removeKeyframe(uint32_t id) {
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(),
            [&](const AcKeyframe& k){ return k.id() == id; });
        if (it == m_keyframes.end()) return false;
        m_keyframes.erase(it); return true;
    }
    [[nodiscard]] AcKeyframe* findKeyframe(uint32_t id) {
        for (auto& k : m_keyframes) if (k.id() == id) return &k;
        return nullptr;
    }
    [[nodiscard]] size_t keyframeCount() const { return m_keyframes.size(); }
    [[nodiscard]] std::vector<AcKeyframe> keyframesInRange(float tMin, float tMax) const {
        std::vector<AcKeyframe> result;
        for (auto& k : m_keyframes) if (k.time() >= tMin && k.time() <= tMax) result.push_back(k);
        return result;
    }
    void clearKeyframes() { m_keyframes.clear(); }
    void setAllInterp(AcInterp interp) {
        for (auto& k : m_keyframes) k.setInterp(interp);
    }

private:
    std::vector<AcKeyframe> m_keyframes;
};

} // namespace NF
