#pragma once
// NF::Editor — AI goal editor
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

enum class AglGoalPriority : uint8_t { Low, Normal, High, Critical };
inline const char* aglGoalPriorityName(AglGoalPriority v) {
    switch (v) {
        case AglGoalPriority::Low:      return "Low";
        case AglGoalPriority::Normal:   return "Normal";
        case AglGoalPriority::High:     return "High";
        case AglGoalPriority::Critical: return "Critical";
    }
    return "Unknown";
}

class AglGoal {
public:
    explicit AglGoal(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setPriority(AglGoalPriority v)      { m_priority   = v; }
    void setWeight(float v)                  { m_weight     = v; }
    void setEnabled(bool v)                  { m_enabled    = v; }
    void setDescription(const std::string& v){ m_desc       = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;       }
    [[nodiscard]] const std::string& name()        const { return m_name;     }
    [[nodiscard]] AglGoalPriority    priority()    const { return m_priority; }
    [[nodiscard]] float              weight()      const { return m_weight;   }
    [[nodiscard]] bool               enabled()     const { return m_enabled;  }
    [[nodiscard]] const std::string& description() const { return m_desc;     }

private:
    uint32_t        m_id;
    std::string     m_name;
    AglGoalPriority m_priority = AglGoalPriority::Normal;
    float           m_weight   = 1.0f;
    bool            m_enabled  = true;
    std::string     m_desc     = "";
};

class AIGoalEditorV1 {
public:
    bool addGoal(const AglGoal& g) {
        for (auto& x : m_goals) if (x.id() == g.id()) return false;
        m_goals.push_back(g); return true;
    }
    bool removeGoal(uint32_t id) {
        auto it = std::find_if(m_goals.begin(), m_goals.end(),
            [&](const AglGoal& g){ return g.id() == id; });
        if (it == m_goals.end()) return false;
        m_goals.erase(it); return true;
    }
    [[nodiscard]] AglGoal* findGoal(uint32_t id) {
        for (auto& g : m_goals) if (g.id() == id) return &g;
        return nullptr;
    }
    [[nodiscard]] size_t goalCount()    const { return m_goals.size(); }
    [[nodiscard]] size_t criticalCount() const {
        size_t n = 0;
        for (auto& g : m_goals) if (g.priority() == AglGoalPriority::Critical) ++n;
        return n;
    }

private:
    std::vector<AglGoal> m_goals;
};

} // namespace NF
