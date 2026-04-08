#pragma once
// NF::Editor — AI panel host container
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

enum class AIPanelMode : uint8_t { Chat, CodeGen, Review, Debug, Explain };
inline const char* aiPanelModeName(AIPanelMode v) {
    switch (v) {
        case AIPanelMode::Chat:    return "Chat";
        case AIPanelMode::CodeGen: return "CodeGen";
        case AIPanelMode::Review:  return "Review";
        case AIPanelMode::Debug:   return "Debug";
        case AIPanelMode::Explain: return "Explain";
    }
    return "Unknown";
}

enum class AIPanelState : uint8_t { Idle, Thinking, Responding, Error };
inline const char* aiPanelStateName(AIPanelState v) {
    switch (v) {
        case AIPanelState::Idle:       return "Idle";
        case AIPanelState::Thinking:   return "Thinking";
        case AIPanelState::Responding: return "Responding";
        case AIPanelState::Error:      return "Error";
    }
    return "Unknown";
}

class AIPanelPanel {
public:
    explicit AIPanelPanel(uint32_t id, const std::string& title,
                          AIPanelMode mode = AIPanelMode::Chat)
        : m_id(id), m_title(title), m_mode(mode) {}

    void setMode(AIPanelMode v)    { m_mode    = v; }
    void setState(AIPanelState v)  { m_state   = v; }
    void setVisible(bool v)        { m_visible = v; }
    void setPinned(bool v)         { m_pinned  = v; }

    [[nodiscard]] uint32_t           id()      const { return m_id;      }
    [[nodiscard]] const std::string& title()   const { return m_title;   }
    [[nodiscard]] AIPanelMode        mode()    const { return m_mode;    }
    [[nodiscard]] AIPanelState       state()   const { return m_state;   }
    [[nodiscard]] bool               visible() const { return m_visible; }
    [[nodiscard]] bool               pinned()  const { return m_pinned;  }

private:
    uint32_t     m_id;
    std::string  m_title;
    AIPanelMode  m_mode;
    AIPanelState m_state   = AIPanelState::Idle;
    bool         m_visible = true;
    bool         m_pinned  = false;
};

class AtlasAIPanelHost {
public:
    bool addPanel(const AIPanelPanel& p) {
        for (auto& x : m_panels) if (x.id() == p.id()) return false;
        m_panels.push_back(p); return true;
    }
    bool removePanel(uint32_t id) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const AIPanelPanel& p){ return p.id() == id; });
        if (it == m_panels.end()) return false;
        m_panels.erase(it); return true;
    }
    [[nodiscard]] AIPanelPanel* findPanel(uint32_t id) {
        for (auto& p : m_panels) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t panelCount() const { return m_panels.size(); }
    [[nodiscard]] size_t visiblePanels() const {
        size_t n = 0;
        for (auto& p : m_panels) if (p.visible()) ++n;
        return n;
    }

private:
    std::vector<AIPanelPanel> m_panels;
};

} // namespace NF
