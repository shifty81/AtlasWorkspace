#pragma once
// NF::Editor — gradient editor
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

enum class GrdBlendMode : uint8_t { Linear, Smooth, Constant, Ease };
inline const char* grdBlendModeName(GrdBlendMode v) {
    switch (v) {
        case GrdBlendMode::Linear:   return "Linear";
        case GrdBlendMode::Smooth:   return "Smooth";
        case GrdBlendMode::Constant: return "Constant";
        case GrdBlendMode::Ease:     return "Ease";
    }
    return "Unknown";
}

class GrdStop {
public:
    explicit GrdStop(uint32_t id, float t) : m_id(id), m_t(t) {}

    void setR(float v)    { m_r = v; }
    void setG(float v)    { m_g = v; }
    void setB(float v)    { m_b = v; }
    void setA(float v)    { m_a = v; }

    [[nodiscard]] uint32_t id() const { return m_id; }
    [[nodiscard]] float    t()  const { return m_t;  }
    [[nodiscard]] float    r()  const { return m_r;  }
    [[nodiscard]] float    g()  const { return m_g;  }
    [[nodiscard]] float    b()  const { return m_b;  }
    [[nodiscard]] float    a()  const { return m_a;  }

private:
    uint32_t m_id;
    float    m_t = 0.0f;
    float    m_r = 1.0f;
    float    m_g = 1.0f;
    float    m_b = 1.0f;
    float    m_a = 1.0f;
};

class GradientEditorV1 {
public:
    bool addStop(const GrdStop& s) {
        for (auto& x : m_stops) if (x.id() == s.id()) return false;
        m_stops.push_back(s); return true;
    }
    bool removeStop(uint32_t id) {
        auto it = std::find_if(m_stops.begin(), m_stops.end(),
            [&](const GrdStop& s){ return s.id() == id; });
        if (it == m_stops.end()) return false;
        m_stops.erase(it); return true;
    }
    [[nodiscard]] GrdStop* findStop(uint32_t id) {
        for (auto& s : m_stops) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t stopCount()        const { return m_stops.size(); }
    void setBlendMode(GrdBlendMode v)             { m_blendMode = v; }
    [[nodiscard]] GrdBlendMode blendMode()  const { return m_blendMode; }
    void setHdr(bool v)                           { m_hdr = v; }
    [[nodiscard]] bool hdr()                const { return m_hdr; }

private:
    std::vector<GrdStop> m_stops;
    GrdBlendMode         m_blendMode = GrdBlendMode::Linear;
    bool                 m_hdr       = false;
};

} // namespace NF
