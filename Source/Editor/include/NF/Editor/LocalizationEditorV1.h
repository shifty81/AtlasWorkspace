#pragma once
// NF::Editor — localization editor
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

enum class LeStatus : uint8_t { Missing, Draft, Review, Approved, Published };
inline const char* leStatusName(LeStatus v) {
    switch (v) {
        case LeStatus::Missing:   return "Missing";
        case LeStatus::Draft:     return "Draft";
        case LeStatus::Review:    return "Review";
        case LeStatus::Approved:  return "Approved";
        case LeStatus::Published: return "Published";
    }
    return "Unknown";
}

enum class LeLocale : uint8_t { EnUS, EnGB, FrFR, DeDe, EsES, JaJP, ZhCN, KoKR };
inline const char* leLocaleName(LeLocale v) {
    switch (v) {
        case LeLocale::EnUS: return "EnUS";
        case LeLocale::EnGB: return "EnGB";
        case LeLocale::FrFR: return "FrFR";
        case LeLocale::DeDe: return "DeDe";
        case LeLocale::EsES: return "EsES";
        case LeLocale::JaJP: return "JaJP";
        case LeLocale::ZhCN: return "ZhCN";
        case LeLocale::KoKR: return "KoKR";
    }
    return "Unknown";
}

class LeEntry {
public:
    explicit LeEntry(uint32_t id, const std::string& key, LeLocale locale)
        : m_id(id), m_key(key), m_locale(locale) {}

    void setSourceText(const std::string& v)     { m_sourceText     = v; }
    void setTranslatedText(const std::string& v) { m_translatedText = v; }
    void setStatus(LeStatus v)                   { m_status         = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& key()            const { return m_key;            }
    [[nodiscard]] LeLocale           locale()         const { return m_locale;         }
    [[nodiscard]] const std::string& sourceText()     const { return m_sourceText;     }
    [[nodiscard]] const std::string& translatedText() const { return m_translatedText; }
    [[nodiscard]] LeStatus           status()         const { return m_status;         }

private:
    uint32_t    m_id;
    std::string m_key;
    LeLocale    m_locale;
    std::string m_sourceText     = "";
    std::string m_translatedText = "";
    LeStatus    m_status         = LeStatus::Missing;
};

class LocalizationEditorV1 {
public:
    bool addEntry(const LeEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const LeEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] LeEntry* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    [[nodiscard]] size_t countByStatus(LeStatus s) const {
        size_t n = 0;
        for (auto& e : m_entries) if (e.status() == s) ++n;
        return n;
    }
    [[nodiscard]] std::vector<LeEntry> filterByLocale(LeLocale loc) const {
        std::vector<LeEntry> result;
        for (auto& e : m_entries) if (e.locale() == loc) result.push_back(e);
        return result;
    }
    [[nodiscard]] size_t approvedCount() const {
        return countByStatus(LeStatus::Approved);
    }

private:
    std::vector<LeEntry> m_entries;
};

} // namespace NF
