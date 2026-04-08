#pragma once
// NF::Editor — rigid body editor
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

enum class RbvBodyType : uint8_t { Dynamic, Kinematic, Static };
inline const char* rbvBodyTypeName(RbvBodyType v) {
    switch (v) {
        case RbvBodyType::Dynamic:   return "Dynamic";
        case RbvBodyType::Kinematic: return "Kinematic";
        case RbvBodyType::Static:    return "Static";
    }
    return "Unknown";
}

class RbvBody {
public:
    explicit RbvBody(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setBodyType(RbvBodyType v)   { m_bodyType   = v; }
    void setMass(float v)             { m_mass       = v; }
    void setLinearDamping(float v)    { m_linearDamp = v; }
    void setAngularDamping(float v)   { m_angDamp    = v; }
    void setGravityScale(float v)     { m_gravScale  = v; }
    void setEnabled(bool v)           { m_enabled    = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;         }
    [[nodiscard]] const std::string& name()          const { return m_name;       }
    [[nodiscard]] RbvBodyType        bodyType()      const { return m_bodyType;   }
    [[nodiscard]] float              mass()          const { return m_mass;       }
    [[nodiscard]] float              linearDamping() const { return m_linearDamp; }
    [[nodiscard]] float              angularDamping()const { return m_angDamp;    }
    [[nodiscard]] float              gravityScale()  const { return m_gravScale;  }
    [[nodiscard]] bool               enabled()       const { return m_enabled;    }

private:
    uint32_t    m_id;
    std::string m_name;
    RbvBodyType m_bodyType   = RbvBodyType::Dynamic;
    float       m_mass       = 1.0f;
    float       m_linearDamp = 0.0f;
    float       m_angDamp    = 0.05f;
    float       m_gravScale  = 1.0f;
    bool        m_enabled    = true;
};

class RigidBodyEditorV1 {
public:
    bool addBody(const RbvBody& b) {
        for (auto& x : m_bodies) if (x.id() == b.id()) return false;
        m_bodies.push_back(b); return true;
    }
    bool removeBody(uint32_t id) {
        auto it = std::find_if(m_bodies.begin(), m_bodies.end(),
            [&](const RbvBody& b){ return b.id() == id; });
        if (it == m_bodies.end()) return false;
        m_bodies.erase(it); return true;
    }
    [[nodiscard]] RbvBody* findBody(uint32_t id) {
        for (auto& b : m_bodies) if (b.id() == id) return &b;
        return nullptr;
    }
    [[nodiscard]] size_t bodyCount()    const { return m_bodies.size(); }
    [[nodiscard]] size_t dynamicCount() const {
        size_t n = 0;
        for (auto& b : m_bodies) if (b.bodyType() == RbvBodyType::Dynamic) ++n;
        return n;
    }

private:
    std::vector<RbvBody> m_bodies;
};

} // namespace NF
