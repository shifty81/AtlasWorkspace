#pragma once
// NF::Editor — command palette
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

enum class CmdCategory : uint8_t { File, Edit, View, Run, Tools, Help };
inline const char* cmdCategoryName(CmdCategory v) {
    switch (v) {
        case CmdCategory::File:  return "File";
        case CmdCategory::Edit:  return "Edit";
        case CmdCategory::View:  return "View";
        case CmdCategory::Run:   return "Run";
        case CmdCategory::Tools: return "Tools";
        case CmdCategory::Help:  return "Help";
    }
    return "Unknown";
}

enum class CmdState : uint8_t { Available, Disabled, Pending, Hidden };
inline const char* cmdStateName(CmdState v) {
    switch (v) {
        case CmdState::Available: return "Available";
        case CmdState::Disabled:  return "Disabled";
        case CmdState::Pending:   return "Pending";
        case CmdState::Hidden:    return "Hidden";
    }
    return "Unknown";
}

class CmdEntry {
public:
    explicit CmdEntry(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setDescription(const std::string& v)  { m_description    = v; }
    void setCategory(CmdCategory v)             { m_category       = v; }
    void setState(CmdState v)                   { m_state          = v; }
    void setShortcut(const std::string& v)      { m_shortcut       = v; }
    void setExecutionCount(int v)               { m_executionCount = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] const std::string& description()    const { return m_description;    }
    [[nodiscard]] CmdCategory        category()       const { return m_category;       }
    [[nodiscard]] CmdState           state()          const { return m_state;          }
    [[nodiscard]] const std::string& shortcut()       const { return m_shortcut;       }
    [[nodiscard]] int                executionCount() const { return m_executionCount; }

private:
    uint32_t    m_id;
    std::string m_name;
    std::string m_description    = "";
    CmdCategory m_category       = CmdCategory::File;
    CmdState    m_state          = CmdState::Available;
    std::string m_shortcut       = "";
    int         m_executionCount = 0;
};

class CommandPaletteV1 {
public:
    bool addCommand(const CmdEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeCommand(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const CmdEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] CmdEntry* findCommand(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t commandCount() const { return m_entries.size(); }
    [[nodiscard]] size_t availableCount() const {
        size_t n = 0;
        for (auto& e : m_entries) if (e.state() == CmdState::Available) ++n;
        return n;
    }
    bool execute(uint32_t id) {
        auto* e = findCommand(id);
        if (!e) return false;
        e->setExecutionCount(e->executionCount() + 1);
        return true;
    }
    [[nodiscard]] std::vector<CmdEntry> filterByCategory(CmdCategory cat) const {
        std::vector<CmdEntry> result;
        for (auto& e : m_entries) if (e.category() == cat) result.push_back(e);
        return result;
    }

private:
    std::vector<CmdEntry> m_entries;
};

} // namespace NF
