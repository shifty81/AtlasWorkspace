#pragma once
// NF::Editor — codex snippet mirroring
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

enum class CsmLanguage : uint8_t { Cpp, Python, CSharp, Lua, GLSL };
inline const char* csmLanguageName(CsmLanguage v) {
    switch (v) {
        case CsmLanguage::Cpp:    return "Cpp";
        case CsmLanguage::Python: return "Python";
        case CsmLanguage::CSharp: return "CSharp";
        case CsmLanguage::Lua:    return "Lua";
        case CsmLanguage::GLSL:   return "GLSL";
    }
    return "Unknown";
}

enum class CsmSyncState : uint8_t { Synced, Modified, Stale, Conflict };
inline const char* csmSyncStateName(CsmSyncState v) {
    switch (v) {
        case CsmSyncState::Synced:   return "Synced";
        case CsmSyncState::Modified: return "Modified";
        case CsmSyncState::Stale:    return "Stale";
        case CsmSyncState::Conflict: return "Conflict";
    }
    return "Unknown";
}

class CsmSnippet {
public:
    explicit CsmSnippet(uint32_t id, const std::string& title)
        : m_id(id), m_title(title) {}

    void setLanguage(CsmLanguage v)       { m_language  = v; }
    void setSyncState(CsmSyncState v)     { m_syncState = v; }
    void setCode(const std::string& v)    { m_code      = v; }
    void addTag(const std::string& tag)   { m_tags.push_back(tag); }

    [[nodiscard]] uint32_t                        id()        const { return m_id;        }
    [[nodiscard]] const std::string&              title()     const { return m_title;     }
    [[nodiscard]] CsmLanguage                     language()  const { return m_language;  }
    [[nodiscard]] CsmSyncState                    syncState() const { return m_syncState; }
    [[nodiscard]] const std::string&              code()      const { return m_code;      }
    [[nodiscard]] const std::vector<std::string>& tags()      const { return m_tags;      }
    [[nodiscard]] int lineCount() const {
        if (m_code.empty()) return 0;
        int n = 1;
        for (char c : m_code) if (c == '\n') ++n;
        return n;
    }

private:
    uint32_t                 m_id;
    std::string              m_title;
    CsmLanguage              m_language  = CsmLanguage::Cpp;
    CsmSyncState             m_syncState = CsmSyncState::Synced;
    std::string              m_code;
    std::vector<std::string> m_tags;
};

class CodexSnippetMirror {
public:
    bool addSnippet(const CsmSnippet& s) {
        for (auto& x : m_snippets) if (x.id() == s.id()) return false;
        m_snippets.push_back(s); return true;
    }
    bool removeSnippet(uint32_t id) {
        auto it = std::find_if(m_snippets.begin(), m_snippets.end(),
            [&](const CsmSnippet& s){ return s.id() == id; });
        if (it == m_snippets.end()) return false;
        m_snippets.erase(it); return true;
    }
    [[nodiscard]] CsmSnippet* findSnippet(uint32_t id) {
        for (auto& s : m_snippets) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t snippetCount() const { return m_snippets.size(); }
    [[nodiscard]] std::vector<CsmSnippet> filterByLanguage(CsmLanguage lang) const {
        std::vector<CsmSnippet> result;
        for (auto& s : m_snippets) if (s.language() == lang) result.push_back(s);
        return result;
    }
    bool markStale(uint32_t id) {
        auto* s = findSnippet(id);
        if (!s) return false;
        s->setSyncState(CsmSyncState::Stale);
        return true;
    }

private:
    std::vector<CsmSnippet> m_snippets;
};

} // namespace NF
