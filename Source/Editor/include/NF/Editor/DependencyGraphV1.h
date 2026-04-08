#pragma once
// NF::Editor — dependency graph
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

enum class DepEdgeType : uint8_t { Required, Optional, Conflict, Provides };
inline const char* depEdgeTypeName(DepEdgeType v) {
    switch (v) {
        case DepEdgeType::Required: return "Required";
        case DepEdgeType::Optional: return "Optional";
        case DepEdgeType::Conflict: return "Conflict";
        case DepEdgeType::Provides: return "Provides";
    }
    return "Unknown";
}

enum class DepNodeState : uint8_t { Resolved, Unresolved, Conflicted, Excluded };
inline const char* depNodeStateName(DepNodeState v) {
    switch (v) {
        case DepNodeState::Resolved:   return "Resolved";
        case DepNodeState::Unresolved: return "Unresolved";
        case DepNodeState::Conflicted: return "Conflicted";
        case DepNodeState::Excluded:   return "Excluded";
    }
    return "Unknown";
}

class DepNode {
public:
    explicit DepNode(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setState(DepNodeState v)         { m_state   = v; }
    void setVersion(const std::string& v) { m_version = v; }
    void setDepth(int v)                  { m_depth   = v; }

    [[nodiscard]] uint32_t           id()      const { return m_id;      }
    [[nodiscard]] const std::string& name()    const { return m_name;    }
    [[nodiscard]] DepNodeState       state()   const { return m_state;   }
    [[nodiscard]] const std::string& version() const { return m_version; }
    [[nodiscard]] int                depth()   const { return m_depth;   }

private:
    uint32_t     m_id;
    std::string  m_name;
    DepNodeState m_state   = DepNodeState::Unresolved;
    std::string  m_version = "";
    int          m_depth   = 0;
};

class DepEdge {
public:
    explicit DepEdge(uint32_t id, uint32_t fromId, uint32_t toId)
        : m_id(id), m_fromId(fromId), m_toId(toId) {}

    void setEdgeType(DepEdgeType v) { m_edgeType = v; }

    [[nodiscard]] uint32_t    id()       const { return m_id;       }
    [[nodiscard]] uint32_t    fromId()   const { return m_fromId;   }
    [[nodiscard]] uint32_t    toId()     const { return m_toId;     }
    [[nodiscard]] DepEdgeType edgeType() const { return m_edgeType; }

private:
    uint32_t    m_id;
    uint32_t    m_fromId;
    uint32_t    m_toId;
    DepEdgeType m_edgeType = DepEdgeType::Required;
};

class DependencyGraphV1 {
public:
    bool addNode(const DepNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const DepNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    bool addEdge(const DepEdge& e) {
        for (auto& x : m_edges) if (x.id() == e.id()) return false;
        m_edges.push_back(e); return true;
    }
    bool removeEdge(uint32_t id) {
        auto it = std::find_if(m_edges.begin(), m_edges.end(),
            [&](const DepEdge& e){ return e.id() == id; });
        if (it == m_edges.end()) return false;
        m_edges.erase(it); return true;
    }
    [[nodiscard]] DepNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t edgeCount() const { return m_edges.size(); }
    [[nodiscard]] std::vector<DepEdge> dependenciesOf(uint32_t nodeId) const {
        std::vector<DepEdge> result;
        for (auto& e : m_edges) if (e.fromId() == nodeId) result.push_back(e);
        return result;
    }
    bool setState(uint32_t nodeId, DepNodeState s) {
        auto* n = findNode(nodeId);
        if (!n) return false;
        n->setState(s); return true;
    }

private:
    std::vector<DepNode> m_nodes;
    std::vector<DepEdge> m_edges;
};

} // namespace NF
