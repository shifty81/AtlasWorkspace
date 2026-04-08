#pragma once
// NF::Editor — pipeline state editor
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

enum class PseBlendFactor : uint8_t { Zero, One, SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha };
inline const char* pseBlendFactorName(PseBlendFactor v) {
    switch (v) {
        case PseBlendFactor::Zero:             return "Zero";
        case PseBlendFactor::One:              return "One";
        case PseBlendFactor::SrcAlpha:         return "SrcAlpha";
        case PseBlendFactor::OneMinusSrcAlpha: return "OneMinusSrcAlpha";
        case PseBlendFactor::DstAlpha:         return "DstAlpha";
        case PseBlendFactor::OneMinusDstAlpha: return "OneMinusDstAlpha";
    }
    return "Unknown";
}

enum class PseCullMode : uint8_t { None, Front, Back };
inline const char* pseCullModeName(PseCullMode v) {
    switch (v) {
        case PseCullMode::None:  return "None";
        case PseCullMode::Front: return "Front";
        case PseCullMode::Back:  return "Back";
    }
    return "Unknown";
}

class PseBlendState {
public:
    explicit PseBlendState(uint32_t id) : m_id(id) {}

    void setEnabled(bool v)            { m_enabled   = v; }
    void setSrcFactor(PseBlendFactor v){ m_srcFactor = v; }
    void setDstFactor(PseBlendFactor v){ m_dstFactor = v; }

    [[nodiscard]] uint32_t       id()        const { return m_id;        }
    [[nodiscard]] bool           enabled()   const { return m_enabled;   }
    [[nodiscard]] PseBlendFactor srcFactor() const { return m_srcFactor; }
    [[nodiscard]] PseBlendFactor dstFactor() const { return m_dstFactor; }

private:
    uint32_t       m_id;
    bool           m_enabled   = false;
    PseBlendFactor m_srcFactor = PseBlendFactor::SrcAlpha;
    PseBlendFactor m_dstFactor = PseBlendFactor::OneMinusSrcAlpha;
};

class PsePipelineState {
public:
    explicit PsePipelineState(uint32_t id, const std::string& name)
        : m_id(id), m_name(name) {}

    void setCullMode(PseCullMode v)           { m_cullMode   = v; }
    void setDepthTest(bool v)                 { m_depthTest  = v; }
    void setDepthWrite(bool v)                { m_depthWrite = v; }
    void setWireframe(bool v)                 { m_wireframe  = v; }
    void setBlendState(const PseBlendState& v){ m_blendState = v; }

    [[nodiscard]] uint32_t             id()         const { return m_id;         }
    [[nodiscard]] const std::string&   name()       const { return m_name;       }
    [[nodiscard]] PseCullMode          cullMode()   const { return m_cullMode;   }
    [[nodiscard]] bool                 depthTest()  const { return m_depthTest;  }
    [[nodiscard]] bool                 depthWrite() const { return m_depthWrite; }
    [[nodiscard]] bool                 wireframe()  const { return m_wireframe;  }
    [[nodiscard]] const PseBlendState& blendState() const { return m_blendState; }

private:
    uint32_t      m_id;
    std::string   m_name;
    PseCullMode   m_cullMode   = PseCullMode::Back;
    bool          m_depthTest  = true;
    bool          m_depthWrite = true;
    bool          m_wireframe  = false;
    PseBlendState m_blendState = PseBlendState(0);
};

class PipelineStateEditorV1 {
public:
    bool addState(const PsePipelineState& s) {
        for (auto& x : m_states) if (x.id() == s.id()) return false;
        m_states.push_back(s); return true;
    }
    bool removeState(uint32_t id) {
        auto it = std::find_if(m_states.begin(), m_states.end(),
            [&](const PsePipelineState& s){ return s.id() == id; });
        if (it == m_states.end()) return false;
        m_states.erase(it); return true;
    }
    [[nodiscard]] PsePipelineState* findState(uint32_t id) {
        for (auto& s : m_states) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t stateCount() const { return m_states.size(); }
    [[nodiscard]] uint32_t activeState() const { return m_activeState; }
    bool setActiveState(uint32_t id) {
        if (!findState(id)) return false;
        m_activeState = id; return true;
    }

private:
    std::vector<PsePipelineState> m_states;
    uint32_t                      m_activeState = 0;
};

} // namespace NF
