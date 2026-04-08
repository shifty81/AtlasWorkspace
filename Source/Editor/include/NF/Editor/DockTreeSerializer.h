#pragma once
// NF::Editor — dock layout tree serialization
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

enum class DtsNodeKind : uint8_t { Split, Tab, Leaf, Root };
inline const char* dtsNodeKindName(DtsNodeKind v) {
    switch (v) {
        case DtsNodeKind::Split: return "Split";
        case DtsNodeKind::Tab:   return "Tab";
        case DtsNodeKind::Leaf:  return "Leaf";
        case DtsNodeKind::Root:  return "Root";
    }
    return "Unknown";
}

enum class DtsSplitAxis : uint8_t { Horizontal, Vertical };
inline const char* dtsSplitAxisName(DtsSplitAxis v) {
    switch (v) {
        case DtsSplitAxis::Horizontal: return "Horizontal";
        case DtsSplitAxis::Vertical:   return "Vertical";
    }
    return "Unknown";
}

class DtsNode {
public:
    explicit DtsNode(uint32_t id, DtsNodeKind kind)
        : m_id(id), m_kind(kind) {}

    void setAxis(DtsSplitAxis v)    { m_axis     = v; }
    void setRatio(float v)          { m_ratio    = v; }
    void setPanelId(uint32_t v)     { m_panelId  = v; }
    void setParentId(uint32_t v)    { m_parentId = v; }

    [[nodiscard]] uint32_t      id()       const { return m_id;       }
    [[nodiscard]] DtsNodeKind   kind()     const { return m_kind;     }
    [[nodiscard]] DtsSplitAxis  axis()     const { return m_axis;     }
    [[nodiscard]] float         ratio()    const { return m_ratio;    }
    [[nodiscard]] uint32_t      panelId()  const { return m_panelId;  }
    [[nodiscard]] uint32_t      parentId() const { return m_parentId; }

private:
    uint32_t    m_id;
    DtsNodeKind m_kind;
    DtsSplitAxis m_axis     = DtsSplitAxis::Horizontal;
    float        m_ratio    = 0.5f;
    uint32_t     m_panelId  = 0;
    uint32_t     m_parentId = 0;
};

class DockTreeSerializer {
public:
    bool addNode(const DtsNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const DtsNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    [[nodiscard]] DtsNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] uint32_t rootNode() const {
        for (auto& n : m_nodes) if (n.parentId() == 0) return n.id();
        return 0;
    }
    [[nodiscard]] std::vector<uint32_t> childrenOf(uint32_t parentId) const {
        std::vector<uint32_t> result;
        for (auto& n : m_nodes) if (n.parentId() == parentId) result.push_back(n.id());
        return result;
    }

private:
    std::vector<DtsNode> m_nodes;
};

} // namespace NF
