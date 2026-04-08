#pragma once
// NF::Editor — hotkey registry
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

enum class HkModifier : uint8_t { None, Ctrl, Alt, Shift, CtrlShift, CtrlAlt, CtrlAltShift };
inline const char* hkModifierName(HkModifier v) {
    switch (v) {
        case HkModifier::None:         return "None";
        case HkModifier::Ctrl:         return "Ctrl";
        case HkModifier::Alt:          return "Alt";
        case HkModifier::Shift:        return "Shift";
        case HkModifier::CtrlShift:    return "CtrlShift";
        case HkModifier::CtrlAlt:      return "CtrlAlt";
        case HkModifier::CtrlAltShift: return "CtrlAltShift";
    }
    return "Unknown";
}

enum class HkScope : uint8_t { Global, Editor, Viewport, Dialog };
inline const char* hkScopeName(HkScope v) {
    switch (v) {
        case HkScope::Global:   return "Global";
        case HkScope::Editor:   return "Editor";
        case HkScope::Viewport: return "Viewport";
        case HkScope::Dialog:   return "Dialog";
    }
    return "Unknown";
}

class HkBinding {
public:
    explicit HkBinding(uint32_t id, const std::string& key) : m_id(id), m_key(key) {}

    void setModifier(HkModifier v)   { m_modifier   = v; }
    void setScope(HkScope v)         { m_scope      = v; }
    void setCommandId(uint32_t v)    { m_commandId  = v; }
    void setEnabled(bool v)          { m_enabled    = v; }
    void setConflicted(bool v)       { m_conflicted = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& key()        const { return m_key;        }
    [[nodiscard]] HkModifier         modifier()   const { return m_modifier;   }
    [[nodiscard]] HkScope            scope()      const { return m_scope;      }
    [[nodiscard]] uint32_t           commandId()  const { return m_commandId;  }
    [[nodiscard]] bool               enabled()    const { return m_enabled;    }
    [[nodiscard]] bool               conflicted() const { return m_conflicted; }

private:
    uint32_t    m_id;
    std::string m_key;
    HkModifier  m_modifier   = HkModifier::None;
    HkScope     m_scope      = HkScope::Global;
    uint32_t    m_commandId  = 0;
    bool        m_enabled    = true;
    bool        m_conflicted = false;
};

class HotkeyRegistryV1 {
public:
    bool addBinding(const HkBinding& b) {
        for (auto& x : m_bindings) if (x.id() == b.id()) return false;
        m_bindings.push_back(b); return true;
    }
    bool removeBinding(uint32_t id) {
        auto it = std::find_if(m_bindings.begin(), m_bindings.end(),
            [&](const HkBinding& b){ return b.id() == id; });
        if (it == m_bindings.end()) return false;
        m_bindings.erase(it); return true;
    }
    [[nodiscard]] HkBinding* findBinding(uint32_t id) {
        for (auto& b : m_bindings) if (b.id() == id) return &b;
        return nullptr;
    }
    [[nodiscard]] size_t bindingCount() const { return m_bindings.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& b : m_bindings) if (b.enabled()) ++n;
        return n;
    }
    void detectConflicts() {
        for (auto& b : m_bindings) b.setConflicted(false);
        for (size_t i = 0; i < m_bindings.size(); ++i) {
            for (size_t j = i + 1; j < m_bindings.size(); ++j) {
                if (m_bindings[i].key() == m_bindings[j].key() &&
                    m_bindings[i].modifier() == m_bindings[j].modifier() &&
                    m_bindings[i].scope() == m_bindings[j].scope()) {
                    m_bindings[i].setConflicted(true);
                    m_bindings[j].setConflicted(true);
                }
            }
        }
    }

private:
    std::vector<HkBinding> m_bindings;
};

} // namespace NF
