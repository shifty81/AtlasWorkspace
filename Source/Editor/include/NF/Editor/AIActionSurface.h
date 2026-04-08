#pragma once
// NF::Editor — AI-driven editor action surface
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

enum class AIActionType : uint8_t { Insert, Replace, Delete, Refactor, Generate };
inline const char* aiActionTypeName(AIActionType v) {
    switch (v) {
        case AIActionType::Insert:   return "Insert";
        case AIActionType::Replace:  return "Replace";
        case AIActionType::Delete:   return "Delete";
        case AIActionType::Refactor: return "Refactor";
        case AIActionType::Generate: return "Generate";
    }
    return "Unknown";
}

enum class AIActionState : uint8_t { Pending, Applied, Rejected, Undone };
inline const char* aiActionStateName(AIActionState v) {
    switch (v) {
        case AIActionState::Pending:  return "Pending";
        case AIActionState::Applied:  return "Applied";
        case AIActionState::Rejected: return "Rejected";
        case AIActionState::Undone:   return "Undone";
    }
    return "Unknown";
}

class AIAction {
public:
    explicit AIAction(uint32_t id, AIActionType type, const std::string& description)
        : m_id(id), m_type(type), m_description(description) {}

    void setState(AIActionState v)          { m_state      = v; }
    void setTargetPath(const std::string& v){ m_targetPath = v; }
    void setCanUndo(bool v)                 { m_canUndo    = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] AIActionType       type()        const { return m_type;        }
    [[nodiscard]] AIActionState      state()       const { return m_state;       }
    [[nodiscard]] const std::string& description() const { return m_description; }
    [[nodiscard]] const std::string& targetPath()  const { return m_targetPath;  }
    [[nodiscard]] bool               canUndo()     const { return m_canUndo;     }

private:
    uint32_t      m_id;
    AIActionType  m_type;
    AIActionState m_state       = AIActionState::Pending;
    std::string   m_description;
    std::string   m_targetPath;
    bool          m_canUndo     = true;
};

class AIActionSurface {
public:
    bool submitAction(const AIAction& a) {
        for (auto& x : m_actions) if (x.id() == a.id()) return false;
        m_actions.push_back(a); return true;
    }
    bool rejectAction(uint32_t id) {
        auto* a = findAction(id);
        if (!a) return false;
        a->setState(AIActionState::Rejected);
        return true;
    }
    bool undoAction(uint32_t id) {
        auto* a = findAction(id);
        if (!a || !a->canUndo()) return false;
        a->setState(AIActionState::Undone);
        return true;
    }
    [[nodiscard]] AIAction* findAction(uint32_t id) {
        for (auto& a : m_actions) if (a.id() == id) return &a;
        return nullptr;
    }
    [[nodiscard]] size_t actionCount() const { return m_actions.size(); }
    [[nodiscard]] size_t pendingCount() const {
        size_t n = 0;
        for (auto& a : m_actions) if (a.state() == AIActionState::Pending) ++n;
        return n;
    }

private:
    std::vector<AIAction> m_actions;
};

} // namespace NF
