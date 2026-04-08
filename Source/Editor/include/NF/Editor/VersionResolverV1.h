#pragma once
// NF::Editor — version resolver
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

enum class VrConstraint : uint8_t { Exact, Compatible, Minimum, Maximum, Range };
inline const char* vrConstraintName(VrConstraint v) {
    switch (v) {
        case VrConstraint::Exact:      return "Exact";
        case VrConstraint::Compatible: return "Compatible";
        case VrConstraint::Minimum:    return "Minimum";
        case VrConstraint::Maximum:    return "Maximum";
        case VrConstraint::Range:      return "Range";
    }
    return "Unknown";
}

enum class VrConflictPolicy : uint8_t { Fail, ChooseNewest, ChooseOldest, Ask };
inline const char* vrConflictPolicyName(VrConflictPolicy v) {
    switch (v) {
        case VrConflictPolicy::Fail:         return "Fail";
        case VrConflictPolicy::ChooseNewest: return "ChooseNewest";
        case VrConflictPolicy::ChooseOldest: return "ChooseOldest";
        case VrConflictPolicy::Ask:          return "Ask";
    }
    return "Unknown";
}

class VrRequirement {
public:
    explicit VrRequirement(uint32_t id, const std::string& packageName)
        : m_id(id), m_packageName(packageName) {}

    void setConstraintType(VrConstraint v)        { m_constraintType  = v; }
    void setVersionSpec(const std::string& v)      { m_versionSpec     = v; }
    void setResolved(bool v)                       { m_resolved        = v; }
    void setResolvedVersion(const std::string& v)  { m_resolvedVersion = v; }

    [[nodiscard]] uint32_t           id()              const { return m_id;              }
    [[nodiscard]] const std::string& packageName()     const { return m_packageName;     }
    [[nodiscard]] VrConstraint       constraintType()  const { return m_constraintType;  }
    [[nodiscard]] const std::string& versionSpec()     const { return m_versionSpec;     }
    [[nodiscard]] bool               resolved()        const { return m_resolved;        }
    [[nodiscard]] const std::string& resolvedVersion() const { return m_resolvedVersion; }

private:
    uint32_t     m_id;
    std::string  m_packageName;
    VrConstraint m_constraintType  = VrConstraint::Minimum;
    std::string  m_versionSpec     = "1.0.0";
    bool         m_resolved        = false;
    std::string  m_resolvedVersion = "";
};

class VersionResolverV1 {
public:
    bool addRequirement(const VrRequirement& r) {
        for (auto& x : m_requirements) if (x.id() == r.id()) return false;
        m_requirements.push_back(r); return true;
    }
    bool removeRequirement(uint32_t id) {
        auto it = std::find_if(m_requirements.begin(), m_requirements.end(),
            [&](const VrRequirement& r){ return r.id() == id; });
        if (it == m_requirements.end()) return false;
        m_requirements.erase(it); return true;
    }
    [[nodiscard]] VrRequirement* findRequirement(uint32_t id) {
        for (auto& r : m_requirements) if (r.id() == id) return &r;
        return nullptr;
    }
    [[nodiscard]] size_t requirementCount() const { return m_requirements.size(); }
    [[nodiscard]] size_t resolvedCount() const {
        size_t n = 0;
        for (auto& r : m_requirements) if (r.resolved()) ++n;
        return n;
    }
    void setPolicy(VrConflictPolicy p) { m_policy = p; }
    [[nodiscard]] VrConflictPolicy policy() const { return m_policy; }
    bool resolve(uint32_t id, const std::string& version) {
        auto* r = findRequirement(id);
        if (!r) return false;
        r->setResolved(true);
        r->setResolvedVersion(version);
        return true;
    }

private:
    std::vector<VrRequirement> m_requirements;
    VrConflictPolicy           m_policy = VrConflictPolicy::Fail;
};

} // namespace NF
