#pragma once
// NF::Editor — network sync editor
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

enum class NsvSyncMode : uint8_t { Reliable, Unreliable, ReliableOrdered, UnreliableSequenced };
inline const char* nsvSyncModeName(NsvSyncMode v) {
    switch (v) {
        case NsvSyncMode::Reliable:             return "Reliable";
        case NsvSyncMode::Unreliable:           return "Unreliable";
        case NsvSyncMode::ReliableOrdered:      return "ReliableOrdered";
        case NsvSyncMode::UnreliableSequenced:  return "UnreliableSequenced";
    }
    return "Unknown";
}

class NsvVar {
public:
    explicit NsvVar(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setMode(NsvSyncMode v)         { m_mode      = v; }
    void setRate(float v)               { m_rate      = v; }
    void setEnabled(bool v)             { m_enabled   = v; }
    void setInterpolated(bool v)        { m_interp    = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;       }
    [[nodiscard]] const std::string& name()         const { return m_name;     }
    [[nodiscard]] NsvSyncMode        mode()         const { return m_mode;     }
    [[nodiscard]] float              rate()         const { return m_rate;     }
    [[nodiscard]] bool               enabled()      const { return m_enabled;  }
    [[nodiscard]] bool               interpolated() const { return m_interp;   }

private:
    uint32_t    m_id;
    std::string m_name;
    NsvSyncMode m_mode    = NsvSyncMode::Reliable;
    float       m_rate    = 20.0f;
    bool        m_enabled = true;
    bool        m_interp  = false;
};

class NetworkSyncEditorV1 {
public:
    bool addVar(const NsvVar& v) {
        for (auto& x : m_vars) if (x.id() == v.id()) return false;
        m_vars.push_back(v); return true;
    }
    bool removeVar(uint32_t id) {
        auto it = std::find_if(m_vars.begin(), m_vars.end(),
            [&](const NsvVar& v){ return v.id() == id; });
        if (it == m_vars.end()) return false;
        m_vars.erase(it); return true;
    }
    [[nodiscard]] NsvVar* findVar(uint32_t id) {
        for (auto& v : m_vars) if (v.id() == id) return &v;
        return nullptr;
    }
    [[nodiscard]] size_t varCount()         const { return m_vars.size(); }
    [[nodiscard]] size_t interpolatedCount() const {
        size_t n = 0;
        for (auto& v : m_vars) if (v.interpolated()) ++n;
        return n;
    }

private:
    std::vector<NsvVar> m_vars;
};

} // namespace NF
