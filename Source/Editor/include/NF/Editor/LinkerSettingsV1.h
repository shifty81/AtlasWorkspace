#pragma once
// NF::Editor — linker settings
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

enum class LsLinkType : uint8_t { Static, Dynamic, Executable };
inline const char* lsLinkTypeName(LsLinkType v) {
    switch (v) {
        case LsLinkType::Static:     return "Static";
        case LsLinkType::Dynamic:    return "Dynamic";
        case LsLinkType::Executable: return "Executable";
    }
    return "Unknown";
}

enum class LsRpathMode : uint8_t { None, Origin, Absolute, Relative };
inline const char* lsRpathModeName(LsRpathMode v) {
    switch (v) {
        case LsRpathMode::None:     return "None";
        case LsRpathMode::Origin:   return "Origin";
        case LsRpathMode::Absolute: return "Absolute";
        case LsRpathMode::Relative: return "Relative";
    }
    return "Unknown";
}

class LsLibrary {
public:
    explicit LsLibrary(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setPath(const std::string& v) { m_path     = v; }
    void setLinkType(LsLinkType v)     { m_linkType = v; }
    void setRequired(bool v)           { m_required = v; }
    void setEnabled(bool v)            { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] const std::string& path()     const { return m_path;     }
    [[nodiscard]] LsLinkType         linkType() const { return m_linkType; }
    [[nodiscard]] bool               required() const { return m_required; }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t    m_id;
    std::string m_name;
    std::string m_path     = "";
    LsLinkType  m_linkType = LsLinkType::Dynamic;
    bool        m_required = true;
    bool        m_enabled  = true;
};

class LinkerSettingsV1 {
public:
    bool addLibrary(const LsLibrary& l) {
        for (auto& x : m_libraries) if (x.id() == l.id()) return false;
        m_libraries.push_back(l); return true;
    }
    bool removeLibrary(uint32_t id) {
        auto it = std::find_if(m_libraries.begin(), m_libraries.end(),
            [&](const LsLibrary& l){ return l.id() == id; });
        if (it == m_libraries.end()) return false;
        m_libraries.erase(it); return true;
    }
    [[nodiscard]] LsLibrary* findLibrary(uint32_t id) {
        for (auto& l : m_libraries) if (l.id() == id) return &l;
        return nullptr;
    }
    [[nodiscard]] size_t libraryCount() const { return m_libraries.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& l : m_libraries) if (l.enabled()) ++n;
        return n;
    }
    void setRpathMode(LsRpathMode m) { m_rpathMode = m; }
    [[nodiscard]] LsRpathMode rpathMode() const { return m_rpathMode; }
    [[nodiscard]] size_t requiredLibraries() const {
        size_t n = 0;
        for (auto& l : m_libraries) if (l.required() && l.enabled()) ++n;
        return n;
    }

private:
    std::vector<LsLibrary> m_libraries;
    LsRpathMode            m_rpathMode = LsRpathMode::None;
};

} // namespace NF
