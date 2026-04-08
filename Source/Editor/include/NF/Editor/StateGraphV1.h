#pragma once
// NF::Editor — state graph editor
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

enum class SgNodeKind : uint8_t { State, Choice, Fork, Join, Entry, Exit };
inline const char* sgNodeKindName(SgNodeKind v) {
    switch (v) {
        case SgNodeKind::State:  return "State";
        case SgNodeKind::Choice: return "Choice";
        case SgNodeKind::Fork:   return "Fork";
        case SgNodeKind::Join:   return "Join";
        case SgNodeKind::Entry:  return "Entry";
        case SgNodeKind::Exit:   return "Exit";
    }
    return "Unknown";
}

class SgNode {
public:
    explicit SgNode(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setKind(SgNodeKind v)          { m_kind    = v; }
    void setInitial(bool v)             { m_initial = v; }
    void setFinal(bool v)               { m_final   = v; }
    void setX(float v)                  { m_x       = v; }
    void setY(float v)                  { m_y       = v; }

    [[nodiscard]] uint32_t           id()      const { return m_id;      }
    [[nodiscard]] const std::string& name()    const { return m_name;    }
    [[nodiscard]] SgNodeKind         kind()    const { return m_kind;    }
    [[nodiscard]] bool               initial() const { return m_initial; }
    [[nodiscard]] bool               final_()  const { return m_final;   }
    [[nodiscard]] float              x()       const { return m_x;       }
    [[nodiscard]] float              y()       const { return m_y;       }

private:
    uint32_t    m_id;
    std::string m_name;
    SgNodeKind  m_kind    = SgNodeKind::State;
    bool        m_initial = false;
    bool        m_final   = false;
    float       m_x       = 0.0f;
    float       m_y       = 0.0f;
};

class SgEdge {
public:
    explicit SgEdge(uint32_t id, uint32_t from, uint32_t to) : m_id(id), m_from(from), m_to(to) {}

    void setCondition(const std::string& v) { m_condition = v; }
    void setAction(const std::string& v)    { m_action    = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] uint32_t           from()      const { return m_from;      }
    [[nodiscard]] uint32_t           to()        const { return m_to;        }
    [[nodiscard]] const std::string& condition() const { return m_condition; }
    [[nodiscard]] const std::string& action()    const { return m_action;    }

private:
    uint32_t    m_id;
    uint32_t    m_from;
    uint32_t    m_to;
    std::string m_condition = "";
    std::string m_action    = "";
};

class StateGraphV1 {
public:
    bool addNode(const SgNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool addEdge(const SgEdge& e) {
        for (auto& x : m_edges) if (x.id() == e.id()) return false;
        m_edges.push_back(e); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const SgNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    [[nodiscard]] SgNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t edgeCount() const { return m_edges.size(); }

private:
    std::vector<SgNode> m_nodes;
    std::vector<SgEdge> m_edges;
};

} // namespace NF
