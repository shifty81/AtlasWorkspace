#pragma once
// NF::Editor — reusable UI widget kit
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

enum class WkWidgetType : uint8_t { Button, Label, Input, Slider, Toggle, Dropdown, Separator };
inline const char* wkWidgetTypeName(WkWidgetType v) {
    switch (v) {
        case WkWidgetType::Button:    return "Button";
        case WkWidgetType::Label:     return "Label";
        case WkWidgetType::Input:     return "Input";
        case WkWidgetType::Slider:    return "Slider";
        case WkWidgetType::Toggle:    return "Toggle";
        case WkWidgetType::Dropdown:  return "Dropdown";
        case WkWidgetType::Separator: return "Separator";
    }
    return "Unknown";
}

enum class WkState : uint8_t { Normal, Hovered, Pressed, Disabled, Focused };
inline const char* wkStateName(WkState v) {
    switch (v) {
        case WkState::Normal:   return "Normal";
        case WkState::Hovered:  return "Hovered";
        case WkState::Pressed:  return "Pressed";
        case WkState::Disabled: return "Disabled";
        case WkState::Focused:  return "Focused";
    }
    return "Unknown";
}

class WkWidget {
public:
    explicit WkWidget(uint32_t id, WkWidgetType type)
        : m_id(id), m_type(type) {}

    void setState(WkState v)          { m_state   = v; }
    void setLabel(const std::string& v){ m_label  = v; }
    void setWidth(int v)              { m_width   = v; }
    void setHeight(int v)             { m_height  = v; }
    void setVisible(bool v)           { m_visible = v; }
    void setEnabled(bool v)           { m_enabled = v; }

    [[nodiscard]] uint32_t           id()      const { return m_id;      }
    [[nodiscard]] WkWidgetType       type()    const { return m_type;    }
    [[nodiscard]] WkState            state()   const { return m_state;   }
    [[nodiscard]] const std::string& label()   const { return m_label;   }
    [[nodiscard]] int                width()   const { return m_width;   }
    [[nodiscard]] int                height()  const { return m_height;  }
    [[nodiscard]] bool               visible() const { return m_visible; }
    [[nodiscard]] bool               enabled() const { return m_enabled; }

private:
    uint32_t     m_id;
    WkWidgetType m_type;
    WkState      m_state   = WkState::Normal;
    std::string  m_label;
    int          m_width   = 100;
    int          m_height  = 24;
    bool         m_visible = true;
    bool         m_enabled = true;
};

class WidgetKitV1 {
public:
    bool addWidget(const WkWidget& w) {
        for (auto& x : m_widgets) if (x.id() == w.id()) return false;
        m_widgets.push_back(w); return true;
    }
    bool removeWidget(uint32_t id) {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [&](const WkWidget& w){ return w.id() == id; });
        if (it == m_widgets.end()) return false;
        m_widgets.erase(it); return true;
    }
    [[nodiscard]] WkWidget* findWidget(uint32_t id) {
        for (auto& w : m_widgets) if (w.id() == id) return &w;
        return nullptr;
    }
    [[nodiscard]] size_t widgetCount() const { return m_widgets.size(); }
    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0;
        for (auto& w : m_widgets) if (w.visible()) ++n;
        return n;
    }
    bool setState(uint32_t id, WkState state) {
        auto* w = findWidget(id);
        if (!w) return false;
        w->setState(state);
        return true;
    }
    [[nodiscard]] std::vector<WkWidget> filterByType(WkWidgetType type) const {
        std::vector<WkWidget> result;
        for (auto& w : m_widgets) if (w.type() == type) result.push_back(w);
        return result;
    }

private:
    std::vector<WkWidget> m_widgets;
};

} // namespace NF
