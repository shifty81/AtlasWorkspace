#pragma once
// NF::Editor — tooltip management system
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

enum class TtpPosition : uint8_t { Top, Bottom, Left, Right, Auto };
inline const char* ttpPositionName(TtpPosition v) {
    switch (v) {
        case TtpPosition::Top:    return "Top";
        case TtpPosition::Bottom: return "Bottom";
        case TtpPosition::Left:   return "Left";
        case TtpPosition::Right:  return "Right";
        case TtpPosition::Auto:   return "Auto";
    }
    return "Unknown";
}

enum class TtpTrigger : uint8_t { Hover, Focus, Click, Manual };
inline const char* ttpTriggerName(TtpTrigger v) {
    switch (v) {
        case TtpTrigger::Hover:  return "Hover";
        case TtpTrigger::Focus:  return "Focus";
        case TtpTrigger::Click:  return "Click";
        case TtpTrigger::Manual: return "Manual";
    }
    return "Unknown";
}

class TtpEntry {
public:
    explicit TtpEntry(uint32_t id, uint32_t targetId, const std::string& text)
        : m_id(id), m_targetId(targetId), m_text(text) {}

    void setPosition(TtpPosition v) { m_position = v; }
    void setTrigger(TtpTrigger v)   { m_trigger  = v; }
    void setDelay(float v)          { m_delay    = v; }
    void setVisible(bool v)         { m_visible  = v; }
    void setEnabled(bool v)         { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] uint32_t           targetId() const { return m_targetId; }
    [[nodiscard]] const std::string& text()     const { return m_text;     }
    [[nodiscard]] TtpPosition        position() const { return m_position; }
    [[nodiscard]] TtpTrigger         trigger()  const { return m_trigger;  }
    [[nodiscard]] float              delay()    const { return m_delay;    }
    [[nodiscard]] bool               visible()  const { return m_visible;  }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t    m_id;
    uint32_t    m_targetId;
    std::string m_text;
    TtpPosition m_position = TtpPosition::Auto;
    TtpTrigger  m_trigger  = TtpTrigger::Hover;
    float       m_delay    = 0.5f;
    bool        m_visible  = false;
    bool        m_enabled  = true;
};

class TooltipSystemV1 {
public:
    bool register_(uint32_t id, uint32_t targetId, const std::string& text) {
        for (auto& x : m_entries) if (x.id() == id) return false;
        m_entries.emplace_back(id, targetId, text); return true;
    }
    bool unregister_(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const TtpEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] TtpEntry* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    bool show(uint32_t id) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->setVisible(true); return true;
    }
    bool hide(uint32_t id) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->setVisible(false); return true;
    }
    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0;
        for (auto& e : m_entries) if (e.visible()) ++n;
        return n;
    }

private:
    std::vector<TtpEntry> m_entries;
};

} // namespace NF
