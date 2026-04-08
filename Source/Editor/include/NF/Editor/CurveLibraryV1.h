#pragma once
// NF::Editor — curve library editor
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

enum class ClbCurveType : uint8_t { Linear, Bezier, Hermite, BSpline, Step };
inline const char* clbCurveTypeName(ClbCurveType v) {
    switch (v) {
        case ClbCurveType::Linear:  return "Linear";
        case ClbCurveType::Bezier:  return "Bezier";
        case ClbCurveType::Hermite: return "Hermite";
        case ClbCurveType::BSpline: return "BSpline";
        case ClbCurveType::Step:    return "Step";
    }
    return "Unknown";
}

class ClbKeyframe {
public:
    explicit ClbKeyframe(uint32_t id, float t, float v) : m_id(id), m_t(t), m_v(v) {}

    void setTangentIn(float v)  { m_tangentIn  = v; }
    void setTangentOut(float v) { m_tangentOut = v; }

    [[nodiscard]] uint32_t id()         const { return m_id;         }
    [[nodiscard]] float    t()          const { return m_t;          }
    [[nodiscard]] float    v()          const { return m_v;          }
    [[nodiscard]] float    tangentIn()  const { return m_tangentIn;  }
    [[nodiscard]] float    tangentOut() const { return m_tangentOut; }

private:
    uint32_t m_id;
    float    m_t          = 0.0f;
    float    m_v          = 0.0f;
    float    m_tangentIn  = 0.0f;
    float    m_tangentOut = 0.0f;
};

class CurveLibraryV1 {
public:
    bool addKeyframe(const ClbKeyframe& k) {
        for (auto& x : m_keyframes) if (x.id() == k.id()) return false;
        m_keyframes.push_back(k); return true;
    }
    bool removeKeyframe(uint32_t id) {
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(),
            [&](const ClbKeyframe& k){ return k.id() == id; });
        if (it == m_keyframes.end()) return false;
        m_keyframes.erase(it); return true;
    }
    [[nodiscard]] ClbKeyframe* findKeyframe(uint32_t id) {
        for (auto& k : m_keyframes) if (k.id() == id) return &k;
        return nullptr;
    }
    [[nodiscard]] size_t keyframeCount()  const { return m_keyframes.size(); }
    void setCurveType(ClbCurveType v)           { m_curveType = v; }
    [[nodiscard]] ClbCurveType curveType() const { return m_curveType; }
    void setLooping(bool v)                     { m_looping = v; }
    [[nodiscard]] bool looping()          const { return m_looping; }

private:
    std::vector<ClbKeyframe> m_keyframes;
    ClbCurveType             m_curveType = ClbCurveType::Linear;
    bool                     m_looping   = false;
};

} // namespace NF
