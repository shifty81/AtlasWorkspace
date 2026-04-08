#pragma once
// NF::Editor — shader editor
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

enum class SeShaderStage : uint8_t { Vertex, Fragment, Geometry, TessControl, TessEval, Compute };
inline const char* seShaderStageName(SeShaderStage v) {
    switch (v) {
        case SeShaderStage::Vertex:      return "Vertex";
        case SeShaderStage::Fragment:    return "Fragment";
        case SeShaderStage::Geometry:    return "Geometry";
        case SeShaderStage::TessControl: return "TessControl";
        case SeShaderStage::TessEval:    return "TessEval";
        case SeShaderStage::Compute:     return "Compute";
    }
    return "Unknown";
}

enum class SeShaderLang : uint8_t { GLSL, HLSL, Metal, WGSL, SpirV };
inline const char* seShaderLangName(SeShaderLang v) {
    switch (v) {
        case SeShaderLang::GLSL:  return "GLSL";
        case SeShaderLang::HLSL:  return "HLSL";
        case SeShaderLang::Metal: return "Metal";
        case SeShaderLang::WGSL:  return "WGSL";
        case SeShaderLang::SpirV: return "SpirV";
    }
    return "Unknown";
}

class SeUniform {
public:
    explicit SeUniform(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setType(const std::string& v)  { m_type     = v; }
    void setLocation(int v)             { m_location = v; }
    void setValue(const std::string& v) { m_value    = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] const std::string& type()     const { return m_type;     }
    [[nodiscard]] int                location() const { return m_location; }
    [[nodiscard]] const std::string& value()    const { return m_value;    }

private:
    uint32_t    m_id;
    std::string m_name;
    std::string m_type     = "float";
    int         m_location = -1;
    std::string m_value    = "";
};

class SeShader {
public:
    explicit SeShader(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setStage(SeShaderStage v)       { m_stage    = v; }
    void setLanguage(SeShaderLang v)     { m_language = v; }
    void setSource(const std::string& v) { m_source   = v; }
    void setCompiled(bool v)             { m_compiled = v; }
    void setError(const std::string& v)  { m_error    = v; }

    [[nodiscard]] uint32_t                     id()       const { return m_id;       }
    [[nodiscard]] const std::string&           name()     const { return m_name;     }
    [[nodiscard]] SeShaderStage                stage()    const { return m_stage;    }
    [[nodiscard]] SeShaderLang                 language() const { return m_language; }
    [[nodiscard]] const std::string&           source()   const { return m_source;   }
    [[nodiscard]] bool                         compiled() const { return m_compiled; }
    [[nodiscard]] const std::string&           error()    const { return m_error;    }
    [[nodiscard]] const std::vector<SeUniform>& uniforms() const { return m_uniforms; }

    bool addUniform(const SeUniform& u) {
        for (auto& x : m_uniforms) if (x.id() == u.id()) return false;
        m_uniforms.push_back(u); return true;
    }

private:
    uint32_t              m_id;
    std::string           m_name;
    SeShaderStage         m_stage    = SeShaderStage::Vertex;
    SeShaderLang          m_language = SeShaderLang::GLSL;
    std::string           m_source   = "";
    bool                  m_compiled = false;
    std::string           m_error    = "";
    std::vector<SeUniform> m_uniforms;
};

class ShaderEditorV1 {
public:
    bool addShader(const SeShader& s) {
        for (auto& x : m_shaders) if (x.id() == s.id()) return false;
        m_shaders.push_back(s); return true;
    }
    bool removeShader(uint32_t id) {
        auto it = std::find_if(m_shaders.begin(), m_shaders.end(),
            [&](const SeShader& s){ return s.id() == id; });
        if (it == m_shaders.end()) return false;
        m_shaders.erase(it); return true;
    }
    [[nodiscard]] SeShader* findShader(uint32_t id) {
        for (auto& s : m_shaders) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t shaderCount() const { return m_shaders.size(); }
    [[nodiscard]] size_t compiledCount() const {
        size_t n = 0;
        for (auto& s : m_shaders) if (s.compiled()) ++n;
        return n;
    }
    bool addUniform(uint32_t shaderId, const SeUniform& u) {
        auto* s = findShader(shaderId);
        if (!s) return false;
        return s->addUniform(u);
    }
    bool setSource(uint32_t id, const std::string& src) {
        auto* s = findShader(id);
        if (!s) return false;
        s->setSource(src); return true;
    }

private:
    std::vector<SeShader> m_shaders;
};

} // namespace NF
