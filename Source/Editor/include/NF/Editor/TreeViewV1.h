#pragma once
// NF::Editor — tree view widget
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

enum class TvNodeState : uint8_t { Collapsed, Expanded, Leaf };
inline const char* tvNodeStateName(TvNodeState v) {
    switch (v) {
        case TvNodeState::Collapsed: return "Collapsed";
        case TvNodeState::Expanded:  return "Expanded";
        case TvNodeState::Leaf:      return "Leaf";
    }
    return "Unknown";
}

enum class TvSelectMode : uint8_t { Single, Multi, None };
inline const char* tvSelectModeName(TvSelectMode v) {
    switch (v) {
        case TvSelectMode::Single: return "Single";
        case TvSelectMode::Multi:  return "Multi";
        case TvSelectMode::None:   return "None";
    }
    return "Unknown";
}

class TvNode {
public:
    explicit TvNode(uint32_t id, const std::string& label)
        : m_id(id), m_label(label) {}

    void setState(TvNodeState v)    { m_state    = v; }
    void setParentId(uint32_t v)    { m_parentId = v; }
    void setSelected(bool v)        { m_selected = v; }
    void setEnabled(bool v)         { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& label()    const { return m_label;    }
    [[nodiscard]] TvNodeState        state()    const { return m_state;    }
    [[nodiscard]] uint32_t           parentId() const { return m_parentId; }
    [[nodiscard]] bool               selected() const { return m_selected; }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t    m_id;
    std::string m_label;
    TvNodeState m_state    = TvNodeState::Leaf;
    uint32_t    m_parentId = 0;
    bool        m_selected = false;
    bool        m_enabled  = true;
};

class TreeViewV1 {
public:
    [[nodiscard]] TvSelectMode selectMode() const { return m_selectMode; }
    void setSelectMode(TvSelectMode v) { m_selectMode = v; }

    bool addNode(const TvNode& n) {
        for (auto& x : m_nodes) if (x.id() == n.id()) return false;
        m_nodes.push_back(n); return true;
    }
    bool removeNode(uint32_t id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const TvNode& n){ return n.id() == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it); return true;
    }
    [[nodiscard]] TvNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t n = 0;
        for (auto& x : m_nodes) if (x.selected()) ++n;
        return n;
    }
    bool expand(uint32_t id) {
        auto* n = findNode(id);
        if (!n || n->state() == TvNodeState::Leaf) return false;
        n->setState(TvNodeState::Expanded);
        return true;
    }
    bool collapse(uint32_t id) {
        auto* n = findNode(id);
        if (!n || n->state() == TvNodeState::Leaf) return false;
        n->setState(TvNodeState::Collapsed);
        return true;
    }
    [[nodiscard]] std::vector<uint32_t> childrenOf(uint32_t parentId) const {
        std::vector<uint32_t> result;
        for (auto& n : m_nodes) if (n.parentId() == parentId) result.push_back(n.id());
        return result;
    }

private:
    std::vector<TvNode> m_nodes;
    TvSelectMode        m_selectMode = TvSelectMode::Single;
};

} // namespace NF
