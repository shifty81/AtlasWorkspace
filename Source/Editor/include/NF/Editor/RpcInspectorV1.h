#pragma once
// NF::Editor — RPC inspector
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

enum class RpiCallTarget : uint8_t { Server, AllClients, Owner, Others };
inline const char* rpiCallTargetName(RpiCallTarget v) {
    switch (v) {
        case RpiCallTarget::Server:     return "Server";
        case RpiCallTarget::AllClients: return "AllClients";
        case RpiCallTarget::Owner:      return "Owner";
        case RpiCallTarget::Others:     return "Others";
    }
    return "Unknown";
}

class RpiRpc {
public:
    explicit RpiRpc(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setTarget(RpiCallTarget v)      { m_target    = v; }
    void setBuffered(bool v)             { m_buffered  = v; }
    void setEnabled(bool v)             { m_enabled   = v; }
    void setPayloadSize(uint32_t v)      { m_payloadSz = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;         }
    [[nodiscard]] const std::string& name()        const { return m_name;       }
    [[nodiscard]] RpiCallTarget      target()      const { return m_target;     }
    [[nodiscard]] bool               buffered()    const { return m_buffered;   }
    [[nodiscard]] bool               enabled()     const { return m_enabled;    }
    [[nodiscard]] uint32_t           payloadSize() const { return m_payloadSz;  }

private:
    uint32_t      m_id;
    std::string   m_name;
    RpiCallTarget m_target    = RpiCallTarget::Server;
    bool          m_buffered  = false;
    bool          m_enabled   = true;
    uint32_t      m_payloadSz = 0;
};

class RpcInspectorV1 {
public:
    bool addRpc(const RpiRpc& r) {
        for (auto& x : m_rpcs) if (x.id() == r.id()) return false;
        m_rpcs.push_back(r); return true;
    }
    bool removeRpc(uint32_t id) {
        auto it = std::find_if(m_rpcs.begin(), m_rpcs.end(),
            [&](const RpiRpc& r){ return r.id() == id; });
        if (it == m_rpcs.end()) return false;
        m_rpcs.erase(it); return true;
    }
    [[nodiscard]] RpiRpc* findRpc(uint32_t id) {
        for (auto& r : m_rpcs) if (r.id() == id) return &r;
        return nullptr;
    }
    [[nodiscard]] size_t rpcCount()      const { return m_rpcs.size(); }
    [[nodiscard]] size_t bufferedCount() const {
        size_t n = 0;
        for (auto& r : m_rpcs) if (r.buffered()) ++n;
        return n;
    }

private:
    std::vector<RpiRpc> m_rpcs;
};

} // namespace NF
