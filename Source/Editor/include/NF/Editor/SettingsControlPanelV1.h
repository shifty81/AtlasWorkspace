#pragma once
// NF::Editor — settings control panel UI
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

enum class CpSettingType : uint8_t { Toggle, Slider, Dropdown, TextInput, ColorPicker, FilePicker };
inline const char* cpSettingTypeName(CpSettingType v) {
    switch (v) {
        case CpSettingType::Toggle:      return "Toggle";
        case CpSettingType::Slider:      return "Slider";
        case CpSettingType::Dropdown:    return "Dropdown";
        case CpSettingType::TextInput:   return "TextInput";
        case CpSettingType::ColorPicker: return "ColorPicker";
        case CpSettingType::FilePicker:  return "FilePicker";
    }
    return "Unknown";
}

enum class CpSection : uint8_t { General, Appearance, Keybindings, Extensions, Advanced };
inline const char* cpSectionName(CpSection v) {
    switch (v) {
        case CpSection::General:     return "General";
        case CpSection::Appearance:  return "Appearance";
        case CpSection::Keybindings: return "Keybindings";
        case CpSection::Extensions:  return "Extensions";
        case CpSection::Advanced:    return "Advanced";
    }
    return "Unknown";
}

class CpSettingEntry {
public:
    explicit CpSettingEntry(uint32_t id, const std::string& key, const std::string& label,
                            CpSettingType type)
        : m_id(id), m_key(key), m_label(label), m_type(type) {}

    void setSection(CpSection v)           { m_section      = v; }
    void setValue(const std::string& v)    { m_value        = v; }
    void setDefaultValue(const std::string& v){ m_defaultValue = v; }
    void setVisible(bool v)                { m_visible      = v; }
    void setEnabled(bool v)                { m_enabled      = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& key()          const { return m_key;          }
    [[nodiscard]] const std::string& label()        const { return m_label;        }
    [[nodiscard]] CpSettingType      type()         const { return m_type;         }
    [[nodiscard]] CpSection          section()      const { return m_section;      }
    [[nodiscard]] const std::string& value()        const { return m_value;        }
    [[nodiscard]] const std::string& defaultValue() const { return m_defaultValue; }
    [[nodiscard]] bool               visible()      const { return m_visible;      }
    [[nodiscard]] bool               enabled()      const { return m_enabled;      }

private:
    uint32_t      m_id;
    std::string   m_key;
    std::string   m_label;
    CpSettingType m_type;
    CpSection     m_section      = CpSection::General;
    std::string   m_value;
    std::string   m_defaultValue;
    bool          m_visible      = true;
    bool          m_enabled      = true;
};

class SettingsControlPanelV1 {
public:
    bool addEntry(const CpSettingEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const CpSettingEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] CpSettingEntry* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0;
        for (auto& e : m_entries) if (e.visible()) ++n;
        return n;
    }
    bool resetToDefault(uint32_t id) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->setValue(e->defaultValue());
        return true;
    }
    [[nodiscard]] std::vector<CpSettingEntry> filterBySection(CpSection section) const {
        std::vector<CpSettingEntry> result;
        for (auto& e : m_entries) if (e.section() == section) result.push_back(e);
        return result;
    }

private:
    std::vector<CpSettingEntry> m_entries;
};

} // namespace NF
