#pragma once
// NF::Editor — trigger editor
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

enum class TevCondOp : uint8_t { Equals, NotEquals, GreaterThan, LessThan, Contains };
inline const char* tevCondOpName(TevCondOp v) {
    switch (v) {
        case TevCondOp::Equals:      return "Equals";
        case TevCondOp::NotEquals:   return "NotEquals";
        case TevCondOp::GreaterThan: return "GreaterThan";
        case TevCondOp::LessThan:    return "LessThan";
        case TevCondOp::Contains:    return "Contains";
    }
    return "Unknown";
}

class TevCondition {
public:
    explicit TevCondition(uint32_t id) : m_id(id) {}

    void setVariable(const std::string& v) { m_variable = v; }
    void setOp(TevCondOp v)               { m_op       = v; }
    void setValue(const std::string& v)   { m_value    = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& variable() const { return m_variable; }
    [[nodiscard]] TevCondOp          op()       const { return m_op;       }
    [[nodiscard]] const std::string& value()    const { return m_value;    }

private:
    uint32_t    m_id;
    std::string m_variable = "";
    TevCondOp   m_op       = TevCondOp::Equals;
    std::string m_value    = "";
};

class TriggerEditorV1 {
public:
    bool addCondition(const TevCondition& c) {
        for (auto& x : m_conditions) if (x.id() == c.id()) return false;
        m_conditions.push_back(c); return true;
    }
    bool removeCondition(uint32_t id) {
        auto it = std::find_if(m_conditions.begin(), m_conditions.end(),
            [&](const TevCondition& c){ return c.id() == id; });
        if (it == m_conditions.end()) return false;
        m_conditions.erase(it); return true;
    }
    [[nodiscard]] TevCondition* findCondition(uint32_t id) {
        for (auto& c : m_conditions) if (c.id() == id) return &c;
        return nullptr;
    }
    [[nodiscard]] size_t conditionCount()  const { return m_conditions.size(); }
    void setEnabled(bool v)                      { m_enabled = v; }
    [[nodiscard]] bool enabled()           const { return m_enabled; }
    void setName(const std::string& v)           { m_name = v; }
    [[nodiscard]] const std::string& name() const { return m_name; }

private:
    std::vector<TevCondition> m_conditions;
    bool        m_enabled = true;
    std::string m_name    = "";
};

} // namespace NF
