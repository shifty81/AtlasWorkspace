#pragma once
// NF::Editor — component inspector
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

enum class CiCompType : uint8_t { Transform, Mesh, Light, Camera, Script, Collider, RigidBody, Audio };
inline const char* ciCompTypeName(CiCompType v) {
    switch (v) {
        case CiCompType::Transform: return "Transform";
        case CiCompType::Mesh:      return "Mesh";
        case CiCompType::Light:     return "Light";
        case CiCompType::Camera:    return "Camera";
        case CiCompType::Script:    return "Script";
        case CiCompType::Collider:  return "Collider";
        case CiCompType::RigidBody: return "RigidBody";
        case CiCompType::Audio:     return "Audio";
    }
    return "Unknown";
}

enum class CiEditState : uint8_t { Viewing, Editing, Dirty, ReadOnly };
inline const char* ciEditStateName(CiEditState v) {
    switch (v) {
        case CiEditState::Viewing:  return "Viewing";
        case CiEditState::Editing:  return "Editing";
        case CiEditState::Dirty:    return "Dirty";
        case CiEditState::ReadOnly: return "ReadOnly";
    }
    return "Unknown";
}

class CiComponent {
public:
    explicit CiComponent(uint32_t id, uint32_t entityId, CiCompType type)
        : m_id(id), m_entityId(entityId), m_type(type) {}

    void setEditState(CiEditState v)           { m_editState   = v; }
    void setExpanded(bool v)                   { m_expanded    = v; }
    void setEnabled(bool v)                    { m_enabled     = v; }
    void setDisplayName(const std::string& v)  { m_displayName = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] uint32_t           entityId()    const { return m_entityId;    }
    [[nodiscard]] CiCompType         type()        const { return m_type;        }
    [[nodiscard]] CiEditState        editState()   const { return m_editState;   }
    [[nodiscard]] bool               expanded()    const { return m_expanded;    }
    [[nodiscard]] bool               enabled()     const { return m_enabled;     }
    [[nodiscard]] const std::string& displayName() const { return m_displayName; }

private:
    uint32_t    m_id;
    uint32_t    m_entityId;
    CiCompType  m_type;
    CiEditState m_editState   = CiEditState::Viewing;
    bool        m_expanded    = true;
    bool        m_enabled     = true;
    std::string m_displayName = "";
};

class ComponentInspectorV1 {
public:
    bool addComponent(const CiComponent& c) {
        for (auto& x : m_components) if (x.id() == c.id()) return false;
        m_components.push_back(c); return true;
    }
    bool removeComponent(uint32_t id) {
        auto it = std::find_if(m_components.begin(), m_components.end(),
            [&](const CiComponent& c){ return c.id() == id; });
        if (it == m_components.end()) return false;
        m_components.erase(it); return true;
    }
    [[nodiscard]] CiComponent* findComponent(uint32_t id) {
        for (auto& c : m_components) if (c.id() == id) return &c;
        return nullptr;
    }
    [[nodiscard]] size_t componentCount() const { return m_components.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& c : m_components) if (c.enabled()) ++n;
        return n;
    }
    bool setEditState(uint32_t id, CiEditState s) {
        auto* c = findComponent(id);
        if (!c) return false;
        c->setEditState(s); return true;
    }
    [[nodiscard]] std::vector<CiComponent> componentsForEntity(uint32_t entityId) const {
        std::vector<CiComponent> result;
        for (auto& c : m_components) if (c.entityId() == entityId) result.push_back(c);
        return result;
    }

private:
    std::vector<CiComponent> m_components;
};

} // namespace NF
