#pragma once
// NF::Editor — entity query
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

enum class EqOperator : uint8_t { Has, HasNot, Equal, NotEqual, GreaterThan, LessThan };
inline const char* eqOperatorName(EqOperator v) {
    switch (v) {
        case EqOperator::Has:         return "Has";
        case EqOperator::HasNot:      return "HasNot";
        case EqOperator::Equal:       return "Equal";
        case EqOperator::NotEqual:    return "NotEqual";
        case EqOperator::GreaterThan: return "GreaterThan";
        case EqOperator::LessThan:    return "LessThan";
    }
    return "Unknown";
}

enum class EqLogic : uint8_t { And, Or, Not };
inline const char* eqLogicName(EqLogic v) {
    switch (v) {
        case EqLogic::And: return "And";
        case EqLogic::Or:  return "Or";
        case EqLogic::Not: return "Not";
    }
    return "Unknown";
}

class EqFilter {
public:
    explicit EqFilter(uint32_t id, const std::string& field) : m_id(id), m_field(field) {}

    void setOp(EqOperator v)           { m_op     = v; }
    void setValue(const std::string& v){ m_value  = v; }
    void setLogic(EqLogic v)           { m_logic  = v; }
    void setActive(bool v)             { m_active = v; }

    [[nodiscard]] uint32_t           id()     const { return m_id;     }
    [[nodiscard]] const std::string& field()  const { return m_field;  }
    [[nodiscard]] EqOperator         op()     const { return m_op;     }
    [[nodiscard]] const std::string& value()  const { return m_value;  }
    [[nodiscard]] EqLogic            logic()  const { return m_logic;  }
    [[nodiscard]] bool               active() const { return m_active; }

private:
    uint32_t    m_id;
    std::string m_field;
    EqOperator  m_op     = EqOperator::Has;
    std::string m_value  = "";
    EqLogic     m_logic  = EqLogic::And;
    bool        m_active = true;
};

class EntityQueryV1 {
public:
    bool addFilter(const EqFilter& f) {
        for (auto& x : m_filters) if (x.id() == f.id()) return false;
        m_filters.push_back(f); return true;
    }
    bool removeFilter(uint32_t id) {
        auto it = std::find_if(m_filters.begin(), m_filters.end(),
            [&](const EqFilter& f){ return f.id() == id; });
        if (it == m_filters.end()) return false;
        m_filters.erase(it); return true;
    }
    [[nodiscard]] EqFilter* findFilter(uint32_t id) {
        for (auto& f : m_filters) if (f.id() == id) return &f;
        return nullptr;
    }
    [[nodiscard]] size_t filterCount() const { return m_filters.size(); }
    [[nodiscard]] size_t activeFilterCount() const {
        size_t n = 0;
        for (auto& f : m_filters) if (f.active()) ++n;
        return n;
    }
    void setResults(std::vector<uint32_t> ids) { m_resultIds = std::move(ids); }
    [[nodiscard]] size_t resultCount() const { return m_resultIds.size(); }
    void clearResults() { m_resultIds.clear(); }

private:
    std::vector<EqFilter>  m_filters;
    std::vector<uint32_t>  m_resultIds;
};

} // namespace NF
