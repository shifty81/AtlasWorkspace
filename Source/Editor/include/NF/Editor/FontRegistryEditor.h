#pragma once
// NF::Editor — font registration and management
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

enum class FontFormat : uint8_t { TTF, OTF, WOFF, WOFF2 };
inline const char* fontFormatName(FontFormat v) {
    switch (v) {
        case FontFormat::TTF:   return "TTF";
        case FontFormat::OTF:   return "OTF";
        case FontFormat::WOFF:  return "WOFF";
        case FontFormat::WOFF2: return "WOFF2";
    }
    return "Unknown";
}

enum class FontCategory : uint8_t { Serif, SansSerif, Monospace, Display, Handwriting };
inline const char* fontCategoryName(FontCategory v) {
    switch (v) {
        case FontCategory::Serif:       return "Serif";
        case FontCategory::SansSerif:   return "SansSerif";
        case FontCategory::Monospace:   return "Monospace";
        case FontCategory::Display:     return "Display";
        case FontCategory::Handwriting: return "Handwriting";
    }
    return "Unknown";
}

class FontRecord {
public:
    explicit FontRecord(uint32_t id, const std::string& familyName, FontFormat format,
                        FontCategory category, const std::string& filePath)
        : m_id(id), m_familyName(familyName), m_format(format),
          m_category(category), m_filePath(filePath) {}

    void setWeight(int v)    { m_weight = v; }
    void setItalic(bool v)   { m_italic  = v; }
    void setLoaded(bool v)   { m_loaded  = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& familyName() const { return m_familyName; }
    [[nodiscard]] FontFormat         format()     const { return m_format;     }
    [[nodiscard]] FontCategory       category()   const { return m_category;   }
    [[nodiscard]] const std::string& filePath()   const { return m_filePath;   }
    [[nodiscard]] int                weight()     const { return m_weight;     }
    [[nodiscard]] bool               italic()     const { return m_italic;     }
    [[nodiscard]] bool               loaded()     const { return m_loaded;     }

private:
    uint32_t     m_id;
    std::string  m_familyName;
    FontFormat   m_format;
    FontCategory m_category;
    std::string  m_filePath;
    int          m_weight = 400;
    bool         m_italic = false;
    bool         m_loaded = false;
};

class FontRegistryEditor {
public:
    bool registerFont(const FontRecord& r) {
        for (auto& x : m_records) if (x.id() == r.id()) return false;
        m_records.push_back(r); return true;
    }
    bool unregisterFont(uint32_t id) {
        auto it = std::find_if(m_records.begin(), m_records.end(),
            [&](const FontRecord& r){ return r.id() == id; });
        if (it == m_records.end()) return false;
        m_records.erase(it); return true;
    }
    [[nodiscard]] FontRecord* findRecord(uint32_t id) {
        for (auto& r : m_records) if (r.id() == id) return &r;
        return nullptr;
    }
    [[nodiscard]] size_t recordCount() const { return m_records.size(); }
    [[nodiscard]] std::vector<FontRecord> filterByCategory(FontCategory c) const {
        std::vector<FontRecord> result;
        for (auto& r : m_records) if (r.category() == c) result.push_back(r);
        return result;
    }

private:
    std::vector<FontRecord> m_records;
};

} // namespace NF
