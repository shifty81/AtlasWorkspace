#pragma once
// NF::Editor — AI debug path visualization
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

enum class AiDbgPathType : uint8_t { NavMesh, Waypoint, Smooth, Raw };
inline const char* aiDbgPathTypeName(AiDbgPathType v) {
    switch (v) {
        case AiDbgPathType::NavMesh:  return "NavMesh";
        case AiDbgPathType::Waypoint: return "Waypoint";
        case AiDbgPathType::Smooth:   return "Smooth";
        case AiDbgPathType::Raw:      return "Raw";
    }
    return "Unknown";
}

enum class AiDbgPathStatus : uint8_t { Idle, Computing, Valid, Failed };
inline const char* aiDbgPathStatusName(AiDbgPathStatus v) {
    switch (v) {
        case AiDbgPathStatus::Idle:      return "Idle";
        case AiDbgPathStatus::Computing: return "Computing";
        case AiDbgPathStatus::Valid:     return "Valid";
        case AiDbgPathStatus::Failed:    return "Failed";
    }
    return "Unknown";
}

class AiDbgWaypoint {
public:
    explicit AiDbgWaypoint(uint32_t id, float x, float y, float z)
        : m_id(id), m_x(x), m_y(y), m_z(z) {}

    void setReached(bool v) { m_reached = v; }

    [[nodiscard]] uint32_t id()      const { return m_id;      }
    [[nodiscard]] float    x()       const { return m_x;       }
    [[nodiscard]] float    y()       const { return m_y;       }
    [[nodiscard]] float    z()       const { return m_z;       }
    [[nodiscard]] bool     reached() const { return m_reached; }

private:
    uint32_t m_id;
    float    m_x;
    float    m_y;
    float    m_z;
    bool     m_reached = false;
};

class AiDbgPath {
public:
    explicit AiDbgPath(uint32_t id) : m_id(id) {}

    void setType(AiDbgPathType v)    { m_type   = v; }
    void setStatus(AiDbgPathStatus v){ m_status = v; }

    bool addWaypoint(const AiDbgWaypoint& wp) {
        for (auto& x : m_waypoints) if (x.id() == wp.id()) return false;
        m_waypoints.push_back(wp); return true;
    }
    void clearWaypoints() { m_waypoints.clear(); }

    [[nodiscard]] uint32_t        id()            const { return m_id;              }
    [[nodiscard]] AiDbgPathType   type()          const { return m_type;            }
    [[nodiscard]] AiDbgPathStatus status()        const { return m_status;          }
    [[nodiscard]] size_t          waypointCount() const { return m_waypoints.size();}
    [[nodiscard]] size_t          reachedCount()  const {
        size_t n = 0;
        for (auto& w : m_waypoints) if (w.reached()) ++n;
        return n;
    }

private:
    uint32_t                  m_id;
    AiDbgPathType             m_type   = AiDbgPathType::NavMesh;
    AiDbgPathStatus           m_status = AiDbgPathStatus::Idle;
    std::vector<AiDbgWaypoint> m_waypoints;
};

class AIDebugPathV1 {
public:
    bool addPath(const AiDbgPath& p) {
        for (auto& x : m_paths) if (x.id() == p.id()) return false;
        m_paths.push_back(p); return true;
    }
    bool removePath(uint32_t id) {
        auto it = std::find_if(m_paths.begin(), m_paths.end(),
            [&](const AiDbgPath& p){ return p.id() == id; });
        if (it == m_paths.end()) return false;
        m_paths.erase(it); return true;
    }
    [[nodiscard]] AiDbgPath* findPath(uint32_t id) {
        for (auto& p : m_paths) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t pathCount() const { return m_paths.size(); }
    bool setStatus(uint32_t id, AiDbgPathStatus status) {
        auto* p = findPath(id);
        if (!p) return false;
        p->setStatus(status);
        return true;
    }

private:
    std::vector<AiDbgPath> m_paths;
};

} // namespace NF
