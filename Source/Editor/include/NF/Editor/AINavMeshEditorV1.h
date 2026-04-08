#pragma once
// NF::Editor — AI navmesh editor
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

enum class AnvAreaType : uint8_t { Walkable, NotWalkable, Jump, Swim, Crouch };
inline const char* anvAreaTypeName(AnvAreaType v) {
    switch (v) {
        case AnvAreaType::Walkable:    return "Walkable";
        case AnvAreaType::NotWalkable: return "NotWalkable";
        case AnvAreaType::Jump:        return "Jump";
        case AnvAreaType::Swim:        return "Swim";
        case AnvAreaType::Crouch:      return "Crouch";
    }
    return "Unknown";
}

class AnvArea {
public:
    explicit AnvArea(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setAreaType(AnvAreaType v)   { m_areaType = v; }
    void setCost(float v)             { m_cost     = v; }
    void setEnabled(bool v)           { m_enabled  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] AnvAreaType        areaType() const { return m_areaType; }
    [[nodiscard]] float              cost()     const { return m_cost;     }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }

private:
    uint32_t    m_id;
    std::string m_name;
    AnvAreaType m_areaType = AnvAreaType::Walkable;
    float       m_cost     = 1.0f;
    bool        m_enabled  = true;
};

class AINavMeshEditorV1 {
public:
    bool addArea(const AnvArea& a) {
        for (auto& x : m_areas) if (x.id() == a.id()) return false;
        m_areas.push_back(a); return true;
    }
    bool removeArea(uint32_t id) {
        auto it = std::find_if(m_areas.begin(), m_areas.end(),
            [&](const AnvArea& a){ return a.id() == id; });
        if (it == m_areas.end()) return false;
        m_areas.erase(it); return true;
    }
    [[nodiscard]] AnvArea* findArea(uint32_t id) {
        for (auto& a : m_areas) if (a.id() == id) return &a;
        return nullptr;
    }
    [[nodiscard]] size_t areaCount()   const { return m_areas.size(); }
    void setCellSize(float v)                { m_cellSize = v; }
    [[nodiscard]] float cellSize()     const { return m_cellSize; }
    void setAgentHeight(float v)             { m_agentHeight = v; }
    [[nodiscard]] float agentHeight()  const { return m_agentHeight; }

private:
    std::vector<AnvArea> m_areas;
    float                m_cellSize    = 0.3f;
    float                m_agentHeight = 2.0f;
};

} // namespace NF
