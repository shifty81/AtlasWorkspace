#pragma once
// NF::Editor — mesh inspector
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

enum class MiPrimType : uint8_t { Triangles, Lines, Points, TriStrip, TriFan };
inline const char* miPrimTypeName(MiPrimType v) {
    switch (v) {
        case MiPrimType::Triangles: return "Triangles";
        case MiPrimType::Lines:     return "Lines";
        case MiPrimType::Points:    return "Points";
        case MiPrimType::TriStrip:  return "TriStrip";
        case MiPrimType::TriFan:    return "TriFan";
    }
    return "Unknown";
}

enum class MiAttribType : uint8_t { Position, Normal, Tangent, Bitangent, TexCoord, Color, BoneIndex, BoneWeight };
inline const char* miAttribTypeName(MiAttribType v) {
    switch (v) {
        case MiAttribType::Position:   return "Position";
        case MiAttribType::Normal:     return "Normal";
        case MiAttribType::Tangent:    return "Tangent";
        case MiAttribType::Bitangent:  return "Bitangent";
        case MiAttribType::TexCoord:   return "TexCoord";
        case MiAttribType::Color:      return "Color";
        case MiAttribType::BoneIndex:  return "BoneIndex";
        case MiAttribType::BoneWeight: return "BoneWeight";
    }
    return "Unknown";
}

class MiVertexAttrib {
public:
    explicit MiVertexAttrib(uint32_t id, MiAttribType type) : m_id(id), m_type(type) {}

    void setComponents(uint32_t v) { m_components  = v; }
    void setNormalized(bool v)     { m_normalized  = v; }

    [[nodiscard]] uint32_t     id()         const { return m_id;         }
    [[nodiscard]] MiAttribType type()       const { return m_type;       }
    [[nodiscard]] uint32_t     components() const { return m_components; }
    [[nodiscard]] bool         normalized() const { return m_normalized; }

private:
    uint32_t    m_id;
    MiAttribType m_type;
    uint32_t    m_components = 3;
    bool        m_normalized = false;
};

class MiSubmesh {
public:
    explicit MiSubmesh(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setVertexCount(uint32_t v)  { m_vertexCount   = v; }
    void setIndexCount(uint32_t v)   { m_indexCount    = v; }
    void setPrimType(MiPrimType v)   { m_primType      = v; }
    void setMaterialIndex(uint32_t v){ m_materialIndex = v; }
    void setVisible(bool v)          { m_visible       = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;            }
    [[nodiscard]] const std::string& name()          const { return m_name;          }
    [[nodiscard]] uint32_t           vertexCount()   const { return m_vertexCount;   }
    [[nodiscard]] uint32_t           indexCount()    const { return m_indexCount;    }
    [[nodiscard]] MiPrimType         primType()      const { return m_primType;      }
    [[nodiscard]] uint32_t           materialIndex() const { return m_materialIndex; }
    [[nodiscard]] bool               visible()       const { return m_visible;       }

private:
    uint32_t    m_id;
    std::string m_name;
    uint32_t    m_vertexCount   = 0;
    uint32_t    m_indexCount    = 0;
    MiPrimType  m_primType      = MiPrimType::Triangles;
    uint32_t    m_materialIndex = 0;
    bool        m_visible       = true;
};

class MeshInspectorV1 {
public:
    bool addSubmesh(const MiSubmesh& s) {
        for (auto& x : m_submeshes) if (x.id() == s.id()) return false;
        m_submeshes.push_back(s); return true;
    }
    bool removeSubmesh(uint32_t id) {
        auto it = std::find_if(m_submeshes.begin(), m_submeshes.end(),
            [&](const MiSubmesh& s){ return s.id() == id; });
        if (it == m_submeshes.end()) return false;
        m_submeshes.erase(it); return true;
    }
    [[nodiscard]] MiSubmesh* findSubmesh(uint32_t id) {
        for (auto& s : m_submeshes) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t submeshCount() const { return m_submeshes.size(); }
    [[nodiscard]] size_t visibleSubmeshCount() const {
        size_t n = 0;
        for (auto& s : m_submeshes) if (s.visible()) ++n;
        return n;
    }
    bool addAttrib(const MiVertexAttrib& a) {
        for (auto& x : m_attribs) if (x.id() == a.id()) return false;
        m_attribs.push_back(a); return true;
    }
    [[nodiscard]] size_t attribCount() const { return m_attribs.size(); }
    [[nodiscard]] uint32_t totalVertexCount() const {
        uint32_t total = 0;
        for (auto& s : m_submeshes) total += s.vertexCount();
        return total;
    }
    [[nodiscard]] uint32_t totalIndexCount() const {
        uint32_t total = 0;
        for (auto& s : m_submeshes) total += s.indexCount();
        return total;
    }

private:
    std::vector<MiSubmesh>     m_submeshes;
    std::vector<MiVertexAttrib> m_attribs;
};

} // namespace NF
