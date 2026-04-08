#pragma once
// NF::Editor — log message routing
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

// LogLevel is defined in NF/Core/Core.h as:
// enum class LogLevel : uint8_t { Trace, Debug, Info, Warn, Error, Fatal }

enum class LrDestination : uint8_t { Console, File, Network, Memory };
inline const char* lrDestinationName(LrDestination v) {
    switch (v) {
        case LrDestination::Console: return "Console";
        case LrDestination::File:    return "File";
        case LrDestination::Network: return "Network";
        case LrDestination::Memory:  return "Memory";
    }
    return "Unknown";
}

enum class LrFilterMode : uint8_t { Include, Exclude, Passthrough };
inline const char* lrFilterModeName(LrFilterMode v) {
    switch (v) {
        case LrFilterMode::Include:     return "Include";
        case LrFilterMode::Exclude:     return "Exclude";
        case LrFilterMode::Passthrough: return "Passthrough";
    }
    return "Unknown";
}

class LrRoute {
public:
    explicit LrRoute(uint32_t id, const std::string& name, LrDestination destination)
        : m_id(id), m_name(name), m_destination(destination) {}

    void setFilterMode(LrFilterMode v) { m_filterMode = v; }
    void setMinLevel(LogLevel v)       { m_minLevel   = v; }
    void setEnabled(bool v)            { m_enabled    = v; }
    void incrementMessageCount()       { ++m_messageCount; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] LrDestination      destination()  const { return m_destination;  }
    [[nodiscard]] LrFilterMode       filterMode()   const { return m_filterMode;   }
    [[nodiscard]] LogLevel           minLevel()     const { return m_minLevel;     }
    [[nodiscard]] bool               enabled()      const { return m_enabled;      }
    [[nodiscard]] int                messageCount() const { return m_messageCount; }

private:
    uint32_t      m_id;
    std::string   m_name;
    LrDestination m_destination;
    LrFilterMode  m_filterMode   = LrFilterMode::Passthrough;
    LogLevel      m_minLevel     = LogLevel::Trace;
    bool          m_enabled      = true;
    int           m_messageCount = 0;
};

class LoggingRouteV1 {
public:
    bool addRoute(const LrRoute& r) {
        for (auto& x : m_routes) if (x.id() == r.id()) return false;
        m_routes.push_back(r); return true;
    }
    bool removeRoute(uint32_t id) {
        auto it = std::find_if(m_routes.begin(), m_routes.end(),
            [&](const LrRoute& r){ return r.id() == id; });
        if (it == m_routes.end()) return false;
        m_routes.erase(it); return true;
    }
    [[nodiscard]] LrRoute* findRoute(uint32_t id) {
        for (auto& r : m_routes) if (r.id() == id) return &r;
        return nullptr;
    }
    [[nodiscard]] size_t routeCount() const { return m_routes.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& r : m_routes) if (r.enabled()) ++n;
        return n;
    }
    void dispatch(LogLevel level, const std::string& /*message*/) {
        for (auto& r : m_routes) {
            if (r.enabled() && static_cast<uint8_t>(level) >= static_cast<uint8_t>(r.minLevel()))
                r.incrementMessageCount();
        }
    }

private:
    std::vector<LrRoute> m_routes;
};

} // namespace NF
