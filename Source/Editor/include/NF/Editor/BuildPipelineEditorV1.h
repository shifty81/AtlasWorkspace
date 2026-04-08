#pragma once
// NF::Editor — build pipeline editor
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

enum class BpeBuildTarget : uint8_t { Debug, Release, RelWithDebInfo, MinSizeRel };
inline const char* bpeBuildTargetName(BpeBuildTarget v) {
    switch (v) {
        case BpeBuildTarget::Debug:          return "Debug";
        case BpeBuildTarget::Release:        return "Release";
        case BpeBuildTarget::RelWithDebInfo: return "RelWithDebInfo";
        case BpeBuildTarget::MinSizeRel:     return "MinSizeRel";
    }
    return "Unknown";
}

enum class BuildStage : uint8_t { Configure, Compile, Link, Package, Deploy };
inline const char* buildStageName(BuildStage v) {
    switch (v) {
        case BuildStage::Configure: return "Configure";
        case BuildStage::Compile:   return "Compile";
        case BuildStage::Link:      return "Link";
        case BuildStage::Package:   return "Package";
        case BuildStage::Deploy:    return "Deploy";
    }
    return "Unknown";
}

class BuildStep {
public:
    explicit BuildStep(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setStage(BuildStage v)             { m_stage      = v; }
    void setEnabled(bool v)                 { m_enabled    = v; }
    void setOrder(int v)                    { m_order      = v; }
    void setCommand(const std::string& v)   { m_command    = v; }
    void setLastResult(const std::string& v){ m_lastResult = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] BuildStage         stage()      const { return m_stage;      }
    [[nodiscard]] bool               enabled()    const { return m_enabled;    }
    [[nodiscard]] int                order()      const { return m_order;      }
    [[nodiscard]] const std::string& command()    const { return m_command;    }
    [[nodiscard]] const std::string& lastResult() const { return m_lastResult; }

private:
    uint32_t    m_id;
    std::string m_name;
    BuildStage  m_stage      = BuildStage::Compile;
    bool        m_enabled    = true;
    int         m_order      = 0;
    std::string m_command    = "";
    std::string m_lastResult = "";
};

class BuildPipelineEditorV1 {
public:
    bool addStep(const BuildStep& s) {
        for (auto& x : m_steps) if (x.id() == s.id()) return false;
        m_steps.push_back(s); return true;
    }
    bool removeStep(uint32_t id) {
        auto it = std::find_if(m_steps.begin(), m_steps.end(),
            [&](const BuildStep& s){ return s.id() == id; });
        if (it == m_steps.end()) return false;
        m_steps.erase(it); return true;
    }
    [[nodiscard]] BuildStep* findStep(uint32_t id) {
        for (auto& s : m_steps) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t stepCount() const { return m_steps.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& s : m_steps) if (s.enabled()) ++n;
        return n;
    }
    void setTarget(BpeBuildTarget t) { m_target = t; }
    [[nodiscard]] BpeBuildTarget target() const { return m_target; }
    [[nodiscard]] std::vector<BuildStep> stepsForStage(BuildStage stage) const {
        std::vector<BuildStep> result;
        for (auto& s : m_steps) if (s.stage() == stage) result.push_back(s);
        std::sort(result.begin(), result.end(),
            [](const BuildStep& a, const BuildStep& b){ return a.order() < b.order(); });
        return result;
    }

private:
    std::vector<BuildStep> m_steps;
    BpeBuildTarget            m_target = BpeBuildTarget::Debug;
};

} // namespace NF
