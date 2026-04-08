#pragma once
// NF::Editor — collider editor
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

enum class CldShape : uint8_t { Box, Sphere, Capsule, Mesh, Convex, Plane };
inline const char* cldShapeName(CldShape v) {
    switch (v) {
        case CldShape::Box:     return "Box";
        case CldShape::Sphere:  return "Sphere";
        case CldShape::Capsule: return "Capsule";
        case CldShape::Mesh:    return "Mesh";
        case CldShape::Convex:  return "Convex";
        case CldShape::Plane:   return "Plane";
    }
    return "Unknown";
}

class CldCollider {
public:
    explicit CldCollider(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setShape(CldShape v)        { m_shape    = v; }
    void setIsTrigger(bool v)        { m_trigger  = v; }
    void setFriction(float v)        { m_friction = v; }
    void setRestitution(float v)     { m_restitution = v; }
    void setEnabled(bool v)          { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] CldShape           shape()       const { return m_shape;       }
    [[nodiscard]] bool               isTrigger()   const { return m_trigger;     }
    [[nodiscard]] float              friction()    const { return m_friction;    }
    [[nodiscard]] float              restitution() const { return m_restitution; }
    [[nodiscard]] bool               enabled()     const { return m_enabled;     }

private:
    uint32_t    m_id;
    std::string m_name;
    CldShape    m_shape       = CldShape::Box;
    bool        m_trigger     = false;
    float       m_friction    = 0.6f;
    float       m_restitution = 0.0f;
    bool        m_enabled     = true;
};

class ColliderEditorV1 {
public:
    bool addCollider(const CldCollider& c) {
        for (auto& x : m_colliders) if (x.id() == c.id()) return false;
        m_colliders.push_back(c); return true;
    }
    bool removeCollider(uint32_t id) {
        auto it = std::find_if(m_colliders.begin(), m_colliders.end(),
            [&](const CldCollider& c){ return c.id() == id; });
        if (it == m_colliders.end()) return false;
        m_colliders.erase(it); return true;
    }
    [[nodiscard]] CldCollider* findCollider(uint32_t id) {
        for (auto& c : m_colliders) if (c.id() == id) return &c;
        return nullptr;
    }
    [[nodiscard]] size_t colliderCount() const { return m_colliders.size(); }
    [[nodiscard]] size_t triggerCount()  const {
        size_t n = 0;
        for (auto& c : m_colliders) if (c.isTrigger()) ++n;
        return n;
    }

private:
    std::vector<CldCollider> m_colliders;
};

} // namespace NF
