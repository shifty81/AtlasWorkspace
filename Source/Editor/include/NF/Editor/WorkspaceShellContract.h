#pragma once
// NF::Editor — workspace shell operations contract
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

enum class WscPanel : uint8_t { Explorer, Inspector, Console, Output, Search };
inline const char* wscPanelName(WscPanel v) {
    switch (v) {
        case WscPanel::Explorer:  return "Explorer";
        case WscPanel::Inspector: return "Inspector";
        case WscPanel::Console:   return "Console";
        case WscPanel::Output:    return "Output";
        case WscPanel::Search:    return "Search";
    }
    return "Unknown";
}

enum class WscTheme : uint8_t { Dark, Light, HighContrast };
inline const char* wscThemeName(WscTheme v) {
    switch (v) {
        case WscTheme::Dark:         return "Dark";
        case WscTheme::Light:        return "Light";
        case WscTheme::HighContrast: return "HighContrast";
    }
    return "Unknown";
}

class WscPanelState {
public:
    explicit WscPanelState(WscPanel panel) : m_panel(panel) {}

    void setVisible(bool v) { m_visible = v; }
    void setWidth(int v)    { m_width   = v; }
    void setDocked(bool v)  { m_docked  = v; }

    [[nodiscard]] WscPanel panel()   const { return m_panel;   }
    [[nodiscard]] bool     visible() const { return m_visible; }
    [[nodiscard]] int      width()   const { return m_width;   }
    [[nodiscard]] bool     docked()  const { return m_docked;  }

private:
    WscPanel m_panel;
    bool     m_visible = true;
    int      m_width   = 300;
    bool     m_docked  = true;
};

class WorkspaceShellContract {
public:
    WorkspaceShellContract() {
        m_panels.emplace_back(WscPanel::Explorer);
        m_panels.emplace_back(WscPanel::Inspector);
        m_panels.emplace_back(WscPanel::Console);
        m_panels.emplace_back(WscPanel::Output);
        m_panels.emplace_back(WscPanel::Search);
    }

    [[nodiscard]] WscTheme theme() const { return m_theme; }
    void setTheme(WscTheme v) { m_theme = v; }

    void showPanel(WscPanel p) {
        auto* s = findPanelState(p);
        if (s) s->setVisible(true);
    }
    void hidePanel(WscPanel p) {
        auto* s = findPanelState(p);
        if (s) s->setVisible(false);
    }
    void togglePanel(WscPanel p) {
        auto* s = findPanelState(p);
        if (s) s->setVisible(!s->visible());
    }
    [[nodiscard]] WscPanelState* findPanelState(WscPanel p) {
        for (auto& s : m_panels) if (s.panel() == p) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0;
        for (auto& s : m_panels) if (s.visible()) ++n;
        return n;
    }

private:
    std::vector<WscPanelState> m_panels;
    WscTheme                   m_theme = WscTheme::Dark;
};

} // namespace NF
