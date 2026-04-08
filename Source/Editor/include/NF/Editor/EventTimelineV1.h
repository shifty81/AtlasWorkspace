#pragma once
// NF::Editor — event timeline editor
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

enum class EtlEventType : uint8_t { Trigger, State, Action, Transition, Note };
inline const char* etlEventTypeName(EtlEventType v) {
    switch (v) {
        case EtlEventType::Trigger:    return "Trigger";
        case EtlEventType::State:      return "State";
        case EtlEventType::Action:     return "Action";
        case EtlEventType::Transition: return "Transition";
        case EtlEventType::Note:       return "Note";
    }
    return "Unknown";
}

class EtlEvent {
public:
    explicit EtlEvent(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setType(EtlEventType v)        { m_type      = v; }
    void setTime(float v)               { m_time      = v; }
    void setDuration(float v)           { m_duration  = v; }
    void setEnabled(bool v)             { m_enabled   = v; }
    void setPayload(const std::string& v){ m_payload  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;      }
    [[nodiscard]] const std::string& name()     const { return m_name;    }
    [[nodiscard]] EtlEventType       type()     const { return m_type;    }
    [[nodiscard]] float              time()     const { return m_time;    }
    [[nodiscard]] float              duration() const { return m_duration;}
    [[nodiscard]] bool               enabled()  const { return m_enabled; }
    [[nodiscard]] const std::string& payload()  const { return m_payload; }

private:
    uint32_t      m_id;
    std::string   m_name;
    EtlEventType  m_type     = EtlEventType::Trigger;
    float         m_time     = 0.0f;
    float         m_duration = 0.0f;
    bool          m_enabled  = true;
    std::string   m_payload  = "";
};

class EventTimelineV1 {
public:
    bool addEvent(const EtlEvent& e) {
        for (auto& x : m_events) if (x.id() == e.id()) return false;
        m_events.push_back(e); return true;
    }
    bool removeEvent(uint32_t id) {
        auto it = std::find_if(m_events.begin(), m_events.end(),
            [&](const EtlEvent& e){ return e.id() == id; });
        if (it == m_events.end()) return false;
        m_events.erase(it); return true;
    }
    [[nodiscard]] EtlEvent* findEvent(uint32_t id) {
        for (auto& e : m_events) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t eventCount()   const { return m_events.size(); }
    void setDuration(float v)                 { m_duration = v; }
    [[nodiscard]] float duration()      const { return m_duration; }
    void setPlaying(bool v)                   { m_playing = v; }
    [[nodiscard]] bool playing()        const { return m_playing; }
    [[nodiscard]] std::vector<EtlEvent> eventsInRange(float t0, float t1) const {
        std::vector<EtlEvent> result;
        for (auto& e : m_events) if (e.time() >= t0 && e.time() <= t1) result.push_back(e);
        return result;
    }

private:
    std::vector<EtlEvent> m_events;
    float                 m_duration = 10.0f;
    bool                  m_playing  = false;
};

} // namespace NF
