#pragma once
// NF::Editor — gesture recognizer
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

enum class GestureType : uint8_t { Tap, DoubleTap, LongPress, Swipe, Pinch, Rotate, Pan };
inline const char* gestureTypeName(GestureType v) {
    switch (v) {
        case GestureType::Tap:       return "Tap";
        case GestureType::DoubleTap: return "DoubleTap";
        case GestureType::LongPress: return "LongPress";
        case GestureType::Swipe:     return "Swipe";
        case GestureType::Pinch:     return "Pinch";
        case GestureType::Rotate:    return "Rotate";
        case GestureType::Pan:       return "Pan";
    }
    return "Unknown";
}

enum class GestureState : uint8_t { Possible, Began, Changed, Ended, Cancelled, Failed };
inline const char* gestureStateName(GestureState v) {
    switch (v) {
        case GestureState::Possible:  return "Possible";
        case GestureState::Began:     return "Began";
        case GestureState::Changed:   return "Changed";
        case GestureState::Ended:     return "Ended";
        case GestureState::Cancelled: return "Cancelled";
        case GestureState::Failed:    return "Failed";
    }
    return "Unknown";
}

class GestureConfig {
public:
    explicit GestureConfig(uint32_t id, GestureType type) : m_id(id), m_type(type) {}

    void setState(GestureState v)  { m_state      = v; }
    void setMinFingers(int v)      { m_minFingers = v; }
    void setMaxFingers(int v)      { m_maxFingers = v; }
    void setThreshold(float v)     { m_threshold  = v; }
    void setEnabled(bool v)        { m_enabled    = v; }

    [[nodiscard]] uint32_t      id()         const { return m_id;         }
    [[nodiscard]] GestureType   type()       const { return m_type;       }
    [[nodiscard]] GestureState  state()      const { return m_state;      }
    [[nodiscard]] int           minFingers() const { return m_minFingers; }
    [[nodiscard]] int           maxFingers() const { return m_maxFingers; }
    [[nodiscard]] float         threshold()  const { return m_threshold;  }
    [[nodiscard]] bool          enabled()    const { return m_enabled;    }

private:
    uint32_t     m_id;
    GestureType  m_type;
    GestureState m_state      = GestureState::Possible;
    int          m_minFingers = 1;
    int          m_maxFingers = 1;
    float        m_threshold  = 10.0f;
    bool         m_enabled    = true;
};

class GestureRecognizerV1 {
public:
    bool addGesture(const GestureConfig& g) {
        for (auto& x : m_configs) if (x.id() == g.id()) return false;
        m_configs.push_back(g); return true;
    }
    bool removeGesture(uint32_t id) {
        auto it = std::find_if(m_configs.begin(), m_configs.end(),
            [&](const GestureConfig& g){ return g.id() == id; });
        if (it == m_configs.end()) return false;
        m_configs.erase(it); return true;
    }
    [[nodiscard]] GestureConfig* findGesture(uint32_t id) {
        for (auto& g : m_configs) if (g.id() == id) return &g;
        return nullptr;
    }
    [[nodiscard]] size_t gestureCount() const { return m_configs.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& g : m_configs) if (g.enabled()) ++n;
        return n;
    }
    bool setState(uint32_t id, GestureState s) {
        auto* g = findGesture(id);
        if (!g) return false;
        g->setState(s); return true;
    }
    [[nodiscard]] std::vector<GestureConfig> filterByType(GestureType t) const {
        std::vector<GestureConfig> result;
        for (auto& g : m_configs) if (g.type() == t) result.push_back(g);
        return result;
    }

private:
    std::vector<GestureConfig> m_configs;
};

} // namespace NF
