#pragma once
// NF::Editor — constraint editor
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

enum class CevConstraintType : uint8_t { Fixed, Hinge, Slider, Ball, Cone, Spring };
inline const char* cevConstraintTypeName(CevConstraintType v) {
    switch (v) {
        case CevConstraintType::Fixed:  return "Fixed";
        case CevConstraintType::Hinge:  return "Hinge";
        case CevConstraintType::Slider: return "Slider";
        case CevConstraintType::Ball:   return "Ball";
        case CevConstraintType::Cone:   return "Cone";
        case CevConstraintType::Spring: return "Spring";
    }
    return "Unknown";
}

class CevConstraint {
public:
    explicit CevConstraint(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setType(CevConstraintType v)   { m_type     = v; }
    void setBodyA(uint32_t v)           { m_bodyA    = v; }
    void setBodyB(uint32_t v)           { m_bodyB    = v; }
    void setEnabled(bool v)             { m_enabled  = v; }
    void setBreakForce(float v)         { m_breakForce = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] CevConstraintType  type()       const { return m_type;       }
    [[nodiscard]] uint32_t           bodyA()      const { return m_bodyA;      }
    [[nodiscard]] uint32_t           bodyB()      const { return m_bodyB;      }
    [[nodiscard]] bool               enabled()    const { return m_enabled;    }
    [[nodiscard]] float              breakForce() const { return m_breakForce; }

private:
    uint32_t          m_id;
    std::string       m_name;
    CevConstraintType m_type       = CevConstraintType::Fixed;
    uint32_t          m_bodyA      = 0;
    uint32_t          m_bodyB      = 0;
    bool              m_enabled    = true;
    float             m_breakForce = 0.0f;
};

class ConstraintEditorV1 {
public:
    bool addConstraint(const CevConstraint& c) {
        for (auto& x : m_constraints) if (x.id() == c.id()) return false;
        m_constraints.push_back(c); return true;
    }
    bool removeConstraint(uint32_t id) {
        auto it = std::find_if(m_constraints.begin(), m_constraints.end(),
            [&](const CevConstraint& c){ return c.id() == id; });
        if (it == m_constraints.end()) return false;
        m_constraints.erase(it); return true;
    }
    [[nodiscard]] CevConstraint* findConstraint(uint32_t id) {
        for (auto& c : m_constraints) if (c.id() == id) return &c;
        return nullptr;
    }
    [[nodiscard]] size_t constraintCount()  const { return m_constraints.size(); }
    [[nodiscard]] size_t enabledCount()     const {
        size_t n = 0;
        for (auto& c : m_constraints) if (c.enabled()) ++n;
        return n;
    }

private:
    std::vector<CevConstraint> m_constraints;
};

} // namespace NF
