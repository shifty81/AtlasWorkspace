#pragma once
// NF::Editor — compiler settings
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

enum class CsStandard : uint8_t { Cpp11, Cpp14, Cpp17, Cpp20, Cpp23 };
inline const char* csStandardName(CsStandard v) {
    switch (v) {
        case CsStandard::Cpp11: return "Cpp11";
        case CsStandard::Cpp14: return "Cpp14";
        case CsStandard::Cpp17: return "Cpp17";
        case CsStandard::Cpp20: return "Cpp20";
        case CsStandard::Cpp23: return "Cpp23";
    }
    return "Unknown";
}

enum class CsOptLevel : uint8_t { O0, O1, O2, O3, Os, Oz };
inline const char* csOptLevelName(CsOptLevel v) {
    switch (v) {
        case CsOptLevel::O0: return "O0";
        case CsOptLevel::O1: return "O1";
        case CsOptLevel::O2: return "O2";
        case CsOptLevel::O3: return "O3";
        case CsOptLevel::Os: return "Os";
        case CsOptLevel::Oz: return "Oz";
    }
    return "Unknown";
}

class CsFlag {
public:
    explicit CsFlag(uint32_t id, const std::string& flag) : m_id(id), m_flag(flag) {}

    void setEnabled(bool v)                  { m_enabled     = v; }
    void setDescription(const std::string& v){ m_description = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& flag()        const { return m_flag;        }
    [[nodiscard]] bool               enabled()     const { return m_enabled;     }
    [[nodiscard]] const std::string& description() const { return m_description; }

private:
    uint32_t    m_id;
    std::string m_flag;
    bool        m_enabled     = true;
    std::string m_description = "";
};

class CompilerSettingsV1 {
public:
    bool addFlag(const CsFlag& f) {
        for (auto& x : m_flags) if (x.id() == f.id()) return false;
        m_flags.push_back(f); return true;
    }
    bool removeFlag(uint32_t id) {
        auto it = std::find_if(m_flags.begin(), m_flags.end(),
            [&](const CsFlag& f){ return f.id() == id; });
        if (it == m_flags.end()) return false;
        m_flags.erase(it); return true;
    }
    [[nodiscard]] CsFlag* findFlag(uint32_t id) {
        for (auto& f : m_flags) if (f.id() == id) return &f;
        return nullptr;
    }
    [[nodiscard]] size_t flagCount() const { return m_flags.size(); }
    [[nodiscard]] size_t enabledFlags() const {
        size_t n = 0;
        for (auto& f : m_flags) if (f.enabled()) ++n;
        return n;
    }
    void setStandard(CsStandard s) { m_standard = s; }
    [[nodiscard]] CsStandard standard() const { return m_standard; }
    void setOptLevel(CsOptLevel o) { m_optLevel = o; }
    [[nodiscard]] CsOptLevel optLevel() const { return m_optLevel; }

private:
    std::vector<CsFlag> m_flags;
    CsStandard          m_standard = CsStandard::Cpp17;
    CsOptLevel          m_optLevel = CsOptLevel::O0;
};

} // namespace NF
