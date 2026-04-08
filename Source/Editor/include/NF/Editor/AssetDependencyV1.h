#pragma once
// NF::Editor — asset dependency editor
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

enum class AdvDepType : uint8_t { Hard, Soft, Optional, Generated };
inline const char* advDepTypeName(AdvDepType v) {
    switch (v) {
        case AdvDepType::Hard:      return "Hard";
        case AdvDepType::Soft:      return "Soft";
        case AdvDepType::Optional:  return "Optional";
        case AdvDepType::Generated: return "Generated";
    }
    return "Unknown";
}

class AdvDep {
public:
    explicit AdvDep(uint32_t id, uint32_t from, uint32_t to) : m_id(id), m_from(from), m_to(to) {}

    void setType(AdvDepType v)    { m_type    = v; }
    void setEnabled(bool v)       { m_enabled = v; }

    [[nodiscard]] uint32_t  id()      const { return m_id;      }
    [[nodiscard]] uint32_t  from()    const { return m_from;    }
    [[nodiscard]] uint32_t  to()      const { return m_to;      }
    [[nodiscard]] AdvDepType type()   const { return m_type;    }
    [[nodiscard]] bool      enabled() const { return m_enabled; }

private:
    uint32_t   m_id;
    uint32_t   m_from;
    uint32_t   m_to;
    AdvDepType m_type    = AdvDepType::Hard;
    bool       m_enabled = true;
};

class AssetDependencyV1 {
public:
    bool addDep(const AdvDep& d) {
        for (auto& x : m_deps) if (x.id() == d.id()) return false;
        m_deps.push_back(d); return true;
    }
    bool removeDep(uint32_t id) {
        auto it = std::find_if(m_deps.begin(), m_deps.end(),
            [&](const AdvDep& d){ return d.id() == id; });
        if (it == m_deps.end()) return false;
        m_deps.erase(it); return true;
    }
    [[nodiscard]] AdvDep* findDep(uint32_t id) {
        for (auto& d : m_deps) if (d.id() == id) return &d;
        return nullptr;
    }
    [[nodiscard]] size_t depCount()      const { return m_deps.size(); }
    [[nodiscard]] size_t hardDepCount()  const {
        size_t n = 0;
        for (auto& d : m_deps) if (d.type() == AdvDepType::Hard) ++n;
        return n;
    }
    [[nodiscard]] std::vector<AdvDep> depsFrom(uint32_t from) const {
        std::vector<AdvDep> result;
        for (auto& d : m_deps) if (d.from() == from) result.push_back(d);
        return result;
    }

private:
    std::vector<AdvDep> m_deps;
};

} // namespace NF
