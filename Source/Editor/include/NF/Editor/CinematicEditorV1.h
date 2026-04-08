#pragma once
// NF::Editor — cinematic editor
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

enum class CedShotType : uint8_t { Wide, Medium, CloseUp, Extreme, Overhead, POV };
inline const char* cedShotTypeName(CedShotType v) {
    switch (v) {
        case CedShotType::Wide:      return "Wide";
        case CedShotType::Medium:    return "Medium";
        case CedShotType::CloseUp:   return "CloseUp";
        case CedShotType::Extreme:   return "Extreme";
        case CedShotType::Overhead:  return "Overhead";
        case CedShotType::POV:       return "POV";
    }
    return "Unknown";
}

class CedShot {
public:
    explicit CedShot(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setShotType(CedShotType v)    { m_shotType = v; }
    void setStartTime(float v)         { m_startTime = v; }
    void setEndTime(float v)           { m_endTime   = v; }
    void setEnabled(bool v)            { m_enabled   = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] CedShotType        shotType()  const { return m_shotType;  }
    [[nodiscard]] float              startTime() const { return m_startTime; }
    [[nodiscard]] float              endTime()   const { return m_endTime;   }
    [[nodiscard]] bool               enabled()   const { return m_enabled;   }
    [[nodiscard]] float              duration()  const { return m_endTime - m_startTime; }

private:
    uint32_t    m_id;
    std::string m_name;
    CedShotType m_shotType  = CedShotType::Wide;
    float       m_startTime = 0.0f;
    float       m_endTime   = 5.0f;
    bool        m_enabled   = true;
};

class CinematicEditorV1 {
public:
    bool addShot(const CedShot& s) {
        for (auto& x : m_shots) if (x.id() == s.id()) return false;
        m_shots.push_back(s); return true;
    }
    bool removeShot(uint32_t id) {
        auto it = std::find_if(m_shots.begin(), m_shots.end(),
            [&](const CedShot& s){ return s.id() == id; });
        if (it == m_shots.end()) return false;
        m_shots.erase(it); return true;
    }
    [[nodiscard]] CedShot* findShot(uint32_t id) {
        for (auto& s : m_shots) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t shotCount()    const { return m_shots.size(); }
    void setDuration(float v)                 { m_duration = v; }
    [[nodiscard]] float duration()      const { return m_duration; }
    void setPlaying(bool v)                   { m_playing = v; }
    [[nodiscard]] bool playing()        const { return m_playing; }

private:
    std::vector<CedShot> m_shots;
    float                m_duration = 30.0f;
    bool                 m_playing  = false;
};

} // namespace NF
