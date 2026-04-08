#pragma once
// NF::Editor — spline editor
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

enum class SplInterp : uint8_t { CatmullRom, Bezier, Linear, BSpline };
inline const char* splInterpName(SplInterp v) {
    switch (v) {
        case SplInterp::CatmullRom: return "CatmullRom";
        case SplInterp::Bezier:     return "Bezier";
        case SplInterp::Linear:     return "Linear";
        case SplInterp::BSpline:    return "BSpline";
    }
    return "Unknown";
}

class SplPoint {
public:
    explicit SplPoint(uint32_t id, float x, float y, float z)
        : m_id(id), m_x(x), m_y(y), m_z(z) {}

    void setTension(float v)   { m_tension   = v; }
    void setBias(float v)      { m_bias      = v; }
    void setContinuity(float v){ m_continuity= v; }

    [[nodiscard]] uint32_t id()         const { return m_id;         }
    [[nodiscard]] float    x()          const { return m_x;          }
    [[nodiscard]] float    y()          const { return m_y;          }
    [[nodiscard]] float    z()          const { return m_z;          }
    [[nodiscard]] float    tension()    const { return m_tension;    }
    [[nodiscard]] float    bias()       const { return m_bias;       }
    [[nodiscard]] float    continuity() const { return m_continuity; }

private:
    uint32_t m_id;
    float    m_x          = 0.0f;
    float    m_y          = 0.0f;
    float    m_z          = 0.0f;
    float    m_tension    = 0.0f;
    float    m_bias       = 0.0f;
    float    m_continuity = 0.0f;
};

class SplineEditorV1 {
public:
    bool addPoint(const SplPoint& p) {
        for (auto& x : m_points) if (x.id() == p.id()) return false;
        m_points.push_back(p); return true;
    }
    bool removePoint(uint32_t id) {
        auto it = std::find_if(m_points.begin(), m_points.end(),
            [&](const SplPoint& p){ return p.id() == id; });
        if (it == m_points.end()) return false;
        m_points.erase(it); return true;
    }
    [[nodiscard]] SplPoint* findPoint(uint32_t id) {
        for (auto& p : m_points) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t pointCount()     const { return m_points.size(); }
    void setInterp(SplInterp v)                 { m_interp = v; }
    [[nodiscard]] SplInterp interp()      const { return m_interp; }
    void setClosed(bool v)                      { m_closed = v; }
    [[nodiscard]] bool closed()           const { return m_closed; }

private:
    std::vector<SplPoint> m_points;
    SplInterp             m_interp = SplInterp::CatmullRom;
    bool                  m_closed = false;
};

} // namespace NF
