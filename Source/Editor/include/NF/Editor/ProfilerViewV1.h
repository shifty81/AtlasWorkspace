#pragma once
// NF::Editor — profiler view
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

enum class PrvSampleType : uint8_t { CPU, GPU, Memory, IO, Network };
inline const char* prvSampleTypeName(PrvSampleType v) {
    switch (v) {
        case PrvSampleType::CPU:     return "CPU";
        case PrvSampleType::GPU:     return "GPU";
        case PrvSampleType::Memory:  return "Memory";
        case PrvSampleType::IO:      return "IO";
        case PrvSampleType::Network: return "Network";
    }
    return "Unknown";
}

enum class PrvDisplayMode : uint8_t { Timeline, Flamegraph, Bar, Table };
inline const char* prvDisplayModeName(PrvDisplayMode v) {
    switch (v) {
        case PrvDisplayMode::Timeline:   return "Timeline";
        case PrvDisplayMode::Flamegraph: return "Flamegraph";
        case PrvDisplayMode::Bar:        return "Bar";
        case PrvDisplayMode::Table:      return "Table";
    }
    return "Unknown";
}

class PrvSample {
public:
    explicit PrvSample(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setType(PrvSampleType v)  { m_type      = v; }
    void setDuration(float v)      { m_duration  = v; }
    void setTimestamp(uint64_t v)  { m_timestamp = v; }
    void setDepth(uint32_t v)      { m_depth     = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] PrvSampleType      type()      const { return m_type;      }
    [[nodiscard]] float              duration()  const { return m_duration;  }
    [[nodiscard]] uint64_t           timestamp() const { return m_timestamp; }
    [[nodiscard]] uint32_t           depth()     const { return m_depth;     }

private:
    uint32_t      m_id;
    std::string   m_name;
    PrvSampleType m_type      = PrvSampleType::CPU;
    float         m_duration  = 0.0f;
    uint64_t      m_timestamp = 0;
    uint32_t      m_depth     = 0;
};

class ProfilerViewV1 {
public:
    bool addSample(const PrvSample& s) {
        for (auto& x : m_samples) if (x.id() == s.id()) return false;
        m_samples.push_back(s); return true;
    }
    bool removeSample(uint32_t id) {
        auto it = std::find_if(m_samples.begin(), m_samples.end(),
            [&](const PrvSample& s){ return s.id() == id; });
        if (it == m_samples.end()) return false;
        m_samples.erase(it); return true;
    }
    [[nodiscard]] PrvSample* findSample(uint32_t id) {
        for (auto& s : m_samples) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t sampleCount() const { return m_samples.size(); }
    void setDisplayMode(PrvDisplayMode v) { m_displayMode = v; }
    [[nodiscard]] PrvDisplayMode displayMode() const { return m_displayMode; }
    void setRecording(bool v) { m_recording = v; }
    [[nodiscard]] bool recording() const { return m_recording; }
    [[nodiscard]] std::vector<PrvSample> filterByType(PrvSampleType t) const {
        std::vector<PrvSample> result;
        for (auto& s : m_samples) if (s.type() == t) result.push_back(s);
        return result;
    }
    void clearSamples() { m_samples.clear(); }

private:
    std::vector<PrvSample> m_samples;
    PrvDisplayMode         m_displayMode = PrvDisplayMode::Timeline;
    bool                   m_recording   = false;
};

} // namespace NF
