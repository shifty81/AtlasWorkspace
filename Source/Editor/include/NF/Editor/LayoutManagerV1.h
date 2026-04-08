#pragma once
// NF::Editor — editor window layout management
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

enum class LmLayout : uint8_t { Default, Coding, Art, Animation, Custom };
inline const char* lmLayoutName(LmLayout v) {
    switch (v) {
        case LmLayout::Default:   return "Default";
        case LmLayout::Coding:    return "Coding";
        case LmLayout::Art:       return "Art";
        case LmLayout::Animation: return "Animation";
        case LmLayout::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class LmTransition : uint8_t { Instant, Fade, Slide };
inline const char* lmTransitionName(LmTransition v) {
    switch (v) {
        case LmTransition::Instant: return "Instant";
        case LmTransition::Fade:    return "Fade";
        case LmTransition::Slide:   return "Slide";
    }
    return "Unknown";
}

class LmLayoutProfile {
public:
    explicit LmLayoutProfile(uint32_t id, const std::string& name)
        : m_id(id), m_name(name) {}

    void setLayout(LmLayout v)          { m_layout     = v; }
    void setTransition(LmTransition v)  { m_transition = v; }
    void setData(const std::string& v)  { m_data       = v; }
    void setIsActive(bool v)            { m_isActive   = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] LmLayout           layout()     const { return m_layout;     }
    [[nodiscard]] LmTransition       transition() const { return m_transition; }
    [[nodiscard]] const std::string& data()       const { return m_data;       }
    [[nodiscard]] bool               isActive()   const { return m_isActive;   }

private:
    uint32_t     m_id;
    std::string  m_name;
    LmLayout     m_layout     = LmLayout::Default;
    LmTransition m_transition = LmTransition::Instant;
    std::string  m_data;
    bool         m_isActive   = false;
};

class LayoutManagerV1 {
public:
    bool addProfile(const LmLayoutProfile& p) {
        for (auto& x : m_profiles) if (x.id() == p.id()) return false;
        m_profiles.push_back(p); return true;
    }
    bool removeProfile(uint32_t id) {
        auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
            [&](const LmLayoutProfile& p){ return p.id() == id; });
        if (it == m_profiles.end()) return false;
        m_profiles.erase(it); return true;
    }
    [[nodiscard]] LmLayoutProfile* findProfile(uint32_t id) {
        for (auto& p : m_profiles) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t profileCount() const { return m_profiles.size(); }
    [[nodiscard]] uint32_t activeProfile() const {
        for (auto& p : m_profiles) if (p.isActive()) return p.id();
        return 0;
    }
    bool activateProfile(uint32_t id) {
        auto* target = findProfile(id);
        if (!target) return false;
        for (auto& p : m_profiles) p.setIsActive(false);
        target->setIsActive(true);
        return true;
    }

private:
    std::vector<LmLayoutProfile> m_profiles;
};

} // namespace NF
