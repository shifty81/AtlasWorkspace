#pragma once
// NF::Editor — audio graph editor
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

enum class AgNodeType : uint8_t { Source, Effect, Mixer, Output, Bus, Send, Receive };
inline const char* agNodeTypeName(AgNodeType v) {
    switch (v) {
        case AgNodeType::Source:  return "Source";
        case AgNodeType::Effect:  return "Effect";
        case AgNodeType::Mixer:   return "Mixer";
        case AgNodeType::Output:  return "Output";
        case AgNodeType::Bus:     return "Bus";
        case AgNodeType::Send:    return "Send";
        case AgNodeType::Receive: return "Receive";
    }
    return "Unknown";
}

enum class AgPortDir : uint8_t { Input, Output };
inline const char* agPortDirName(AgPortDir v) {
    switch (v) {
        case AgPortDir::Input:  return "Input";
        case AgPortDir::Output: return "Output";
    }
    return "Unknown";
}

class AgPort {
public:
    explicit AgPort(uint32_t id, uint32_t nodeId, AgPortDir dir)
        : m_id(id), m_nodeId(nodeId), m_dir(dir) {}

    void setLabel(const std::string& v) { m_label     = v; }
    void setConnected(bool v)           { m_connected = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] uint32_t           nodeId()    const { return m_nodeId;    }
    [[nodiscard]] AgPortDir          dir()       const { return m_dir;       }
    [[nodiscard]] const std::string& label()     const { return m_label;     }
    [[nodiscard]] bool               connected() const { return m_connected; }

private:
    uint32_t    m_id;
    uint32_t    m_nodeId;
    AgPortDir   m_dir;
    std::string m_label     = "";
    bool        m_connected = false;
};

class AgNode {
public:
    explicit AgNode(uint32_t id, AgNodeType type, const std::string& name)
        : m_id(id), m_type(type), m_name(name) {}

    void setX(float v)    { m_x     = v; }
    void setY(float v)    { m_y     = v; }
    void setMuted(bool v) { m_muted = v; }

    [[nodiscard]] uint32_t                   id()    const { return m_id;    }
    [[nodiscard]] AgNodeType                 type()  const { return m_type;  }
    [[nodiscard]] const std::string&         name()  const { return m_name;  }
    [[nodiscard]] float                      x()     const { return m_x;     }
    [[nodiscard]] float                      y()     const { return m_y;     }
    [[nodiscard]] bool                       muted() const { return m_muted; }
    [[nodiscard]] const std::vector<AgPort>& ports() const { return m_ports; }
    [[nodiscard]] std::vector<AgPort>&       ports()       { return m_ports; }

    bool addPort(const AgPort& p) {
        for (auto& x : m_ports) if (x.id() == p.id()) return false;
        m_ports.push_back(p); return true;
    }

private:
    uint32_t            m_id;
    AgNodeType          m_type;
    std::string         m_name;
    float               m_x     = 0.0f;
    float               m_y     = 0.0f;
    bool                m_muted = false;
    std::vector<AgPort> m_ports;
};

class AudioGraphEditorV1 {
public:
    bool addNode(const AgNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const AgNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    [[nodiscard]] AgNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t mutedCount() const {
        size_t n = 0;
        for (auto& x : m_nodes) if (x.muted()) ++n;
        return n;
    }
    bool addPort(uint32_t nodeId, const AgPort& p) {
        auto* n = findNode(nodeId);
        if (!n) return false;
        return n->addPort(p);
    }
    bool connect(uint32_t portAId, uint32_t portBId) {
        AgPort* a = nullptr;
        AgPort* b = nullptr;
        for (auto& node : m_nodes) {
            for (auto& port : node.ports()) {
                if (port.id() == portAId) a = &port;
                if (port.id() == portBId) b = &port;
            }
        }
        if (!a || !b) return false;
        a->setConnected(true);
        b->setConnected(true);
        return true;
    }

private:
    std::vector<AgNode> m_nodes;
};

} // namespace NF
