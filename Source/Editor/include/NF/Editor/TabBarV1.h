#pragma once
// NF::Editor — tab bar widget
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

enum class TbOrientation : uint8_t { Top, Bottom, Left, Right };
inline const char* tbOrientationName(TbOrientation v) {
    switch (v) {
        case TbOrientation::Top:    return "Top";
        case TbOrientation::Bottom: return "Bottom";
        case TbOrientation::Left:   return "Left";
        case TbOrientation::Right:  return "Right";
    }
    return "Unknown";
}

enum class TbClosePolicy : uint8_t { NoClose, CloseButton, DoubleClick, MiddleClick };
inline const char* tbClosePolicyName(TbClosePolicy v) {
    switch (v) {
        case TbClosePolicy::NoClose:     return "NoClose";
        case TbClosePolicy::CloseButton: return "CloseButton";
        case TbClosePolicy::DoubleClick: return "DoubleClick";
        case TbClosePolicy::MiddleClick: return "MiddleClick";
    }
    return "Unknown";
}

class TbTab {
public:
    explicit TbTab(uint32_t id, const std::string& label)
        : m_id(id), m_label(label) {}

    void setTooltip(const std::string& v)  { m_tooltip     = v; }
    void setOrientation(TbOrientation v)   { m_orientation = v; }
    void setCloseable(bool v)              { m_closeable   = v; }
    void setActive(bool v)                 { m_active      = v; }
    void setEnabled(bool v)                { m_enabled     = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& label()       const { return m_label;       }
    [[nodiscard]] const std::string& tooltip()     const { return m_tooltip;     }
    [[nodiscard]] TbOrientation      orientation() const { return m_orientation; }
    [[nodiscard]] bool               closeable()   const { return m_closeable;   }
    [[nodiscard]] bool               active()      const { return m_active;      }
    [[nodiscard]] bool               enabled()     const { return m_enabled;     }

private:
    uint32_t      m_id;
    std::string   m_label;
    std::string   m_tooltip;
    TbOrientation m_orientation = TbOrientation::Top;
    bool          m_closeable   = true;
    bool          m_active      = false;
    bool          m_enabled     = true;
};

class TabBarV1 {
public:
    TabBarV1() : m_closePolicy(TbClosePolicy::CloseButton), m_orientation(TbOrientation::Top) {}

    [[nodiscard]] TbClosePolicy  closePolicy()  const { return m_closePolicy;  }
    [[nodiscard]] TbOrientation  orientation()  const { return m_orientation;  }
    void setClosePolicy(TbClosePolicy v)  { m_closePolicy  = v; }
    void setOrientation(TbOrientation v)  { m_orientation  = v; }

    bool addTab(const TbTab& t) {
        for (auto& x : m_tabs) if (x.id() == t.id()) return false;
        m_tabs.push_back(t); return true;
    }
    bool removeTab(uint32_t id) {
        auto it = std::find_if(m_tabs.begin(), m_tabs.end(),
            [&](const TbTab& t){ return t.id() == id; });
        if (it == m_tabs.end()) return false;
        m_tabs.erase(it); return true;
    }
    bool closeTab(uint32_t id) { return removeTab(id); }
    [[nodiscard]] TbTab* findTab(uint32_t id) {
        for (auto& t : m_tabs) if (t.id() == id) return &t;
        return nullptr;
    }
    [[nodiscard]] size_t tabCount() const { return m_tabs.size(); }
    [[nodiscard]] uint32_t activeTab() const {
        for (auto& t : m_tabs) if (t.active()) return t.id();
        return 0;
    }
    bool setActive(uint32_t id) {
        auto* target = findTab(id);
        if (!target) return false;
        for (auto& t : m_tabs) t.setActive(false);
        target->setActive(true);
        return true;
    }

private:
    std::vector<TbTab> m_tabs;
    TbClosePolicy      m_closePolicy;
    TbOrientation      m_orientation;
};

} // namespace NF
