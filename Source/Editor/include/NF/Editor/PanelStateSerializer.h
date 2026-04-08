#pragma once
// NF::Editor — panel state serialization
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

enum class PssFormat : uint8_t { JSON, Binary, XML };
inline const char* pssFormatName(PssFormat v) {
    switch (v) {
        case PssFormat::JSON:   return "JSON";
        case PssFormat::Binary: return "Binary";
        case PssFormat::XML:    return "XML";
    }
    return "Unknown";
}

enum class PssVersion : uint8_t { V1, V2, V3 };
inline const char* pssVersionName(PssVersion v) {
    switch (v) {
        case PssVersion::V1: return "V1";
        case PssVersion::V2: return "V2";
        case PssVersion::V3: return "V3";
    }
    return "Unknown";
}

class PssEntry {
public:
    explicit PssEntry(uint32_t panelId, const std::string& key, const std::string& value,
                      PssVersion version = PssVersion::V1)
        : m_panelId(panelId), m_key(key), m_value(value), m_version(version) {}

    [[nodiscard]] uint32_t           panelId() const { return m_panelId; }
    [[nodiscard]] const std::string& key()     const { return m_key;     }
    [[nodiscard]] const std::string& value()   const { return m_value;   }
    [[nodiscard]] PssVersion         version() const { return m_version; }

    void setValue(const std::string& v) { m_value = v; }

private:
    uint32_t    m_panelId;
    std::string m_key;
    std::string m_value;
    PssVersion  m_version;
};

class PanelStateSerializer {
public:
    void serialize(uint32_t panelId, const std::string& key, const std::string& value) {
        for (auto& e : m_entries) {
            if (e.panelId() == panelId && e.key() == key) {
                e.setValue(value); return;
            }
        }
        m_entries.emplace_back(panelId, key, value);
    }
    [[nodiscard]] std::string deserialize(uint32_t panelId, const std::string& key) const {
        for (auto& e : m_entries) {
            if (e.panelId() == panelId && e.key() == key) return e.value();
        }
        return "";
    }
    void removePanel(uint32_t id) {
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                [&](const PssEntry& e){ return e.panelId() == id; }),
            m_entries.end());
    }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    [[nodiscard]] bool hasEntry(uint32_t panelId, const std::string& key) const {
        for (auto& e : m_entries) {
            if (e.panelId() == panelId && e.key() == key) return true;
        }
        return false;
    }

private:
    std::vector<PssEntry> m_entries;
};

} // namespace NF
