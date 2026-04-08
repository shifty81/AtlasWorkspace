#pragma once
// NF::Editor — lag compensation editor
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

enum class LcvCompStrategy : uint8_t { Rewind, Extrapolate, Interpolate, Predictive };
inline const char* lcvCompStrategyName(LcvCompStrategy v) {
    switch (v) {
        case LcvCompStrategy::Rewind:      return "Rewind";
        case LcvCompStrategy::Extrapolate: return "Extrapolate";
        case LcvCompStrategy::Interpolate: return "Interpolate";
        case LcvCompStrategy::Predictive:  return "Predictive";
    }
    return "Unknown";
}

class LcvEntry {
public:
    explicit LcvEntry(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setStrategy(LcvCompStrategy v) { m_strategy  = v; }
    void setMaxLag(float v)             { m_maxLag    = v; }
    void setEnabled(bool v)             { m_enabled   = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] LcvCompStrategy    strategy() const { return m_strategy; }
    [[nodiscard]] float              maxLag()   const { return m_maxLag;   }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t        m_id;
    std::string     m_name;
    LcvCompStrategy m_strategy = LcvCompStrategy::Rewind;
    float           m_maxLag   = 0.2f;
    bool            m_enabled  = true;
};

class LagCompEditorV1 {
public:
    bool addEntry(const LcvEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const LcvEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] LcvEntry* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t entryCount()   const { return m_entries.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& e : m_entries) if (e.enabled()) ++n;
        return n;
    }

private:
    std::vector<LcvEntry> m_entries;
};

} // namespace NF
