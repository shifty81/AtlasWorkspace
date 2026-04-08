#pragma once
// NF::Editor — material node editor
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

enum class MneNodeType : uint8_t { Input, Output, Math, Texture, Constant, Blend, Normal };
inline const char* mneNodeTypeName(MneNodeType v) {
    switch (v) {
        case MneNodeType::Input:    return "Input";
        case MneNodeType::Output:   return "Output";
        case MneNodeType::Math:     return "Math";
        case MneNodeType::Texture:  return "Texture";
        case MneNodeType::Constant: return "Constant";
        case MneNodeType::Blend:    return "Blend";
        case MneNodeType::Normal:   return "Normal";
    }
    return "Unknown";
}

enum class MnePortType : uint8_t { Float, Vec2, Vec3, Vec4, Color, Sampler, Bool };
inline const char* mnePortTypeName(MnePortType v) {
    switch (v) {
        case MnePortType::Float:   return "Float";
        case MnePortType::Vec2:    return "Vec2";
        case MnePortType::Vec3:    return "Vec3";
        case MnePortType::Vec4:    return "Vec4";
        case MnePortType::Color:   return "Color";
        case MnePortType::Sampler: return "Sampler";
        case MnePortType::Bool:    return "Bool";
    }
    return "Unknown";
}

class MnePort {
public:
    explicit MnePort(uint32_t id, uint32_t nodeId, const std::string& name)
        : m_id(id), m_nodeId(nodeId), m_name(name) {}

    void setPortType(MnePortType v) { m_portType  = v; }
    void setIsInput(bool v)         { m_isInput   = v; }
    void setConnected(bool v)       { m_connected = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] uint32_t           nodeId()    const { return m_nodeId;    }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] MnePortType        portType()  const { return m_portType;  }
    [[nodiscard]] bool               isInput()   const { return m_isInput;   }
    [[nodiscard]] bool               connected() const { return m_connected; }

private:
    uint32_t    m_id;
    uint32_t    m_nodeId;
    std::string m_name;
    MnePortType m_portType  = MnePortType::Float;
    bool        m_isInput   = true;
    bool        m_connected = false;
};

class MneNode {
public:
    explicit MneNode(uint32_t id, const std::string& name, MneNodeType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setX(float v)        { m_x        = v; }
    void setY(float v)        { m_y        = v; }
    void setSelected(bool v)  { m_selected = v; }

    [[nodiscard]] uint32_t                   id()       const { return m_id;       }
    [[nodiscard]] const std::string&         name()     const { return m_name;     }
    [[nodiscard]] MneNodeType                type()     const { return m_type;     }
    [[nodiscard]] float                      x()        const { return m_x;        }
    [[nodiscard]] float                      y()        const { return m_y;        }
    [[nodiscard]] bool                       selected() const { return m_selected; }
    [[nodiscard]] const std::vector<MnePort>& ports()   const { return m_ports;    }

    bool addPort(const MnePort& p) {
        for (auto& x : m_ports) if (x.id() == p.id()) return false;
        m_ports.push_back(p); return true;
    }

private:
    uint32_t             m_id;
    std::string          m_name;
    MneNodeType          m_type;
    float                m_x        = 0.0f;
    float                m_y        = 0.0f;
    bool                 m_selected = false;
    std::vector<MnePort> m_ports;
};

class MaterialNodeEditorV1 {
public:
    bool addNode(const MneNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const MneNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    [[nodiscard]] MneNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t n = 0;
        for (auto& x : m_nodes) if (x.selected()) ++n;
        return n;
    }
    [[nodiscard]] std::vector<MneNode> filterByType(MneNodeType type) const {
        std::vector<MneNode> result;
        for (auto& n : m_nodes) if (n.type() == type) result.push_back(n);
        return result;
    }

private:
    std::vector<MneNode> m_nodes;
};

} // namespace NF
