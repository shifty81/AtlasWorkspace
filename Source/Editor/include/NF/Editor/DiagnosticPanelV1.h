#pragma once
// NF::Editor — diagnostic panel
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

enum class DpSeverity : uint8_t { Info, Warning, Error, Critical };
inline const char* dpSeverityName(DpSeverity v) {
    switch (v) {
        case DpSeverity::Info:     return "Info";
        case DpSeverity::Warning:  return "Warning";
        case DpSeverity::Error:    return "Error";
        case DpSeverity::Critical: return "Critical";
    }
    return "Unknown";
}

enum class DpCategory : uint8_t { Render, Physics, Audio, Script, Asset, Network, System };
inline const char* dpCategoryName(DpCategory v) {
    switch (v) {
        case DpCategory::Render:  return "Render";
        case DpCategory::Physics: return "Physics";
        case DpCategory::Audio:   return "Audio";
        case DpCategory::Script:  return "Script";
        case DpCategory::Asset:   return "Asset";
        case DpCategory::Network: return "Network";
        case DpCategory::System:  return "System";
    }
    return "Unknown";
}

class DpEntry {
public:
    explicit DpEntry(uint32_t id, const std::string& message) : m_id(id), m_message(message) {}

    void setSeverity(DpSeverity v)   { m_severity      = v; }
    void setCategory(DpCategory v)   { m_category      = v; }
    void setTimestamp(uint64_t v)    { m_timestamp     = v; }
    void setAcknowledged(bool v)     { m_acknowledged  = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& message()      const { return m_message;      }
    [[nodiscard]] DpSeverity         severity()     const { return m_severity;     }
    [[nodiscard]] DpCategory         category()     const { return m_category;     }
    [[nodiscard]] uint64_t           timestamp()    const { return m_timestamp;    }
    [[nodiscard]] bool               acknowledged() const { return m_acknowledged; }

private:
    uint32_t    m_id;
    std::string m_message;
    DpSeverity  m_severity     = DpSeverity::Info;
    DpCategory  m_category     = DpCategory::System;
    uint64_t    m_timestamp    = 0;
    bool        m_acknowledged = false;
};

class DiagnosticPanelV1 {
public:
    bool addEntry(const DpEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const DpEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] DpEntry* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    void acknowledgeAll() {
        for (auto& e : m_entries) e.setAcknowledged(true);
    }
    [[nodiscard]] size_t unacknowledgedCount() const {
        size_t n = 0;
        for (auto& e : m_entries) if (!e.acknowledged()) ++n;
        return n;
    }
    [[nodiscard]] std::vector<DpEntry> filterBySeverity(DpSeverity s) const {
        std::vector<DpEntry> result;
        for (auto& e : m_entries) if (e.severity() == s) result.push_back(e);
        return result;
    }
    [[nodiscard]] std::vector<DpEntry> filterByCategory(DpCategory c) const {
        std::vector<DpEntry> result;
        for (auto& e : m_entries) if (e.category() == c) result.push_back(e);
        return result;
    }
    void clearAll() { m_entries.clear(); }

private:
    std::vector<DpEntry> m_entries;
};

} // namespace NF
