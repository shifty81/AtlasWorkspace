#pragma once
// NF::Editor — sequencer track editor
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

enum class SqtTrackType : uint8_t { Animation, Audio, Event, Camera, Effect, Script };
inline const char* sqtTrackTypeName(SqtTrackType v) {
    switch (v) {
        case SqtTrackType::Animation: return "Animation";
        case SqtTrackType::Audio:     return "Audio";
        case SqtTrackType::Event:     return "Event";
        case SqtTrackType::Camera:    return "Camera";
        case SqtTrackType::Effect:    return "Effect";
        case SqtTrackType::Script:    return "Script";
    }
    return "Unknown";
}

class SqtClip {
public:
    explicit SqtClip(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setStartTime(float v)  { m_start = v; }
    void setEndTime(float v)    { m_end   = v; }
    void setEnabled(bool v)     { m_enabled = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;      }
    [[nodiscard]] const std::string& name()      const { return m_name;    }
    [[nodiscard]] float              startTime() const { return m_start;   }
    [[nodiscard]] float              endTime()   const { return m_end;     }
    [[nodiscard]] bool               enabled()   const { return m_enabled; }
    [[nodiscard]] float              duration()  const { return m_end - m_start; }

private:
    uint32_t    m_id;
    std::string m_name;
    float       m_start   = 0.0f;
    float       m_end     = 1.0f;
    bool        m_enabled = true;
};

class SequencerTrackV1 {
public:
    bool addClip(const SqtClip& c) {
        for (auto& x : m_clips) if (x.id() == c.id()) return false;
        m_clips.push_back(c); return true;
    }
    bool removeClip(uint32_t id) {
        auto it = std::find_if(m_clips.begin(), m_clips.end(),
            [&](const SqtClip& c){ return c.id() == id; });
        if (it == m_clips.end()) return false;
        m_clips.erase(it); return true;
    }
    [[nodiscard]] SqtClip* findClip(uint32_t id) {
        for (auto& c : m_clips) if (c.id() == id) return &c;
        return nullptr;
    }
    [[nodiscard]] size_t clipCount()    const { return m_clips.size(); }
    void setTrackType(SqtTrackType v)         { m_trackType = v; }
    [[nodiscard]] SqtTrackType trackType() const { return m_trackType; }
    void setMuted(bool v)                     { m_muted = v; }
    [[nodiscard]] bool muted()          const { return m_muted; }
    void setLocked(bool v)                    { m_locked = v; }
    [[nodiscard]] bool locked()         const { return m_locked; }

private:
    std::vector<SqtClip> m_clips;
    SqtTrackType         m_trackType = SqtTrackType::Animation;
    bool                 m_muted     = false;
    bool                 m_locked    = false;
};

} // namespace NF
