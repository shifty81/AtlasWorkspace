#pragma once
// NF::Editor — scene tree editor
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

enum class SteNodeType : uint8_t { Entity, Prefab, Light, Camera, Volume, Trigger };
inline const char* steNodeTypeName(SteNodeType v) {
    switch (v) {
        case SteNodeType::Entity:  return "Entity";
        case SteNodeType::Prefab:  return "Prefab";
        case SteNodeType::Light:   return "Light";
        case SteNodeType::Camera:  return "Camera";
        case SteNodeType::Volume:  return "Volume";
        case SteNodeType::Trigger: return "Trigger";
    }
    return "Unknown";
}

enum class SteVisibility : uint8_t { Visible, Hidden, PartiallyHidden };
inline const char* steVisibilityName(SteVisibility v) {
    switch (v) {
        case SteVisibility::Visible:         return "Visible";
        case SteVisibility::Hidden:          return "Hidden";
        case SteVisibility::PartiallyHidden: return "PartiallyHidden";
    }
    return "Unknown";
}

class SteNode {
public:
    explicit SteNode(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setType(SteNodeType v)        { m_type       = v; }
    void setVisibility(SteVisibility v){ m_visibility = v; }
    void setParentId(uint32_t v)       { m_parentId   = v; }
    void setSelected(bool v)           { m_selected   = v; }
    void setLocked(bool v)             { m_locked     = v; }
    void setActive(bool v)             { m_active     = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] SteNodeType        type()       const { return m_type;       }
    [[nodiscard]] SteVisibility      visibility() const { return m_visibility; }
    [[nodiscard]] uint32_t           parentId()   const { return m_parentId;   }
    [[nodiscard]] bool               selected()   const { return m_selected;   }
    [[nodiscard]] bool               locked()     const { return m_locked;     }
    [[nodiscard]] bool               active()     const { return m_active;     }

private:
    uint32_t      m_id;
    std::string   m_name;
    SteNodeType   m_type       = SteNodeType::Entity;
    SteVisibility m_visibility = SteVisibility::Visible;
    uint32_t      m_parentId   = 0;
    bool          m_selected   = false;
    bool          m_locked     = false;
    bool          m_active     = true;
};

class SceneTreeEditorV1 {
public:
    bool addNode(const SteNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const SteNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    [[nodiscard]] SteNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t n = 0;
        for (auto& x : m_nodes) if (x.selected()) ++n;
        return n;
    }
    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0;
        for (auto& x : m_nodes) if (x.visibility() == SteVisibility::Visible) ++n;
        return n;
    }
    [[nodiscard]] std::vector<SteNode> childrenOf(uint32_t parentId) const {
        std::vector<SteNode> result;
        for (auto& n : m_nodes) if (n.parentId() == parentId) result.push_back(n);
        return result;
    }
    bool setVisibility(uint32_t id, SteVisibility v) {
        auto* n = findNode(id);
        if (!n) return false;
        n->setVisibility(v); return true;
    }

private:
    std::vector<SteNode> m_nodes;
};

} // namespace NF
