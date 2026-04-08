#pragma once
// NF::Editor — graph editor host contract
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

enum class GhcNodeKind : uint8_t { Exec, Data, Comment, Reroute };
inline const char* ghcNodeKindName(GhcNodeKind v) {
    switch (v) {
        case GhcNodeKind::Exec:    return "Exec";
        case GhcNodeKind::Data:    return "Data";
        case GhcNodeKind::Comment: return "Comment";
        case GhcNodeKind::Reroute: return "Reroute";
    }
    return "Unknown";
}

enum class GhcPinDir : uint8_t { Input, Output, Bidirectional };
inline const char* ghcPinDirName(GhcPinDir v) {
    switch (v) {
        case GhcPinDir::Input:         return "Input";
        case GhcPinDir::Output:        return "Output";
        case GhcPinDir::Bidirectional: return "Bidirectional";
    }
    return "Unknown";
}

class GhcPin {
public:
    explicit GhcPin(uint32_t id, uint32_t nodeId, GhcPinDir dir, const std::string& label)
        : m_id(id), m_nodeId(nodeId), m_dir(dir), m_label(label) {}

    void setConnected(bool v) { m_connected = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] uint32_t           nodeId()    const { return m_nodeId;    }
    [[nodiscard]] GhcPinDir          dir()       const { return m_dir;       }
    [[nodiscard]] const std::string& label()     const { return m_label;     }
    [[nodiscard]] bool               connected() const { return m_connected; }

private:
    uint32_t    m_id;
    uint32_t    m_nodeId;
    GhcPinDir   m_dir;
    std::string m_label;
    bool        m_connected = false;
};

class GhcNode {
public:
    explicit GhcNode(uint32_t id, GhcNodeKind kind, const std::string& title)
        : m_id(id), m_kind(kind), m_title(title) {}

    void setX(float v)              { m_x        = v; }
    void setY(float v)              { m_y        = v; }
    void setSelected(bool v)        { m_selected = v; }
    void addPin(const GhcPin& pin)  { m_pins.push_back(pin); }

    [[nodiscard]] uint32_t                      id()       const { return m_id;       }
    [[nodiscard]] GhcNodeKind                   kind()     const { return m_kind;     }
    [[nodiscard]] const std::string&            title()    const { return m_title;    }
    [[nodiscard]] float                         x()        const { return m_x;        }
    [[nodiscard]] float                         y()        const { return m_y;        }
    [[nodiscard]] bool                          selected() const { return m_selected; }
    [[nodiscard]] const std::vector<GhcPin>&    pins()     const { return m_pins;     }
    [[nodiscard]] std::vector<GhcPin>&          pins()           { return m_pins;     }

private:
    uint32_t             m_id;
    GhcNodeKind          m_kind;
    std::string          m_title;
    float                m_x        = 0.0f;
    float                m_y        = 0.0f;
    bool                 m_selected = false;
    std::vector<GhcPin>  m_pins;
};

class GraphHostContractV1 {
public:
    bool addNode(const GhcNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const GhcNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    [[nodiscard]] GhcNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t n = 0;
        for (auto& x : m_nodes) if (x.selected()) ++n;
        return n;
    }
    bool addPin(uint32_t nodeId, const GhcPin& pin) {
        auto* n = findNode(nodeId);
        if (!n) return false;
        n->addPin(pin);
        return true;
    }
    bool connect(uint32_t pinIdA, uint32_t pinIdB) {
        GhcPin* a = findPin(pinIdA);
        GhcPin* b = findPin(pinIdB);
        if (!a || !b) return false;
        a->setConnected(true);
        b->setConnected(true);
        return true;
    }

private:
    GhcPin* findPin(uint32_t pinId) {
        for (auto& n : m_nodes)
            for (auto& p : n.pins())
                if (p.id() == pinId) return &p;
        return nullptr;
    }

    std::vector<GhcNode> m_nodes;
};

} // namespace NF
