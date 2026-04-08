#pragma once
// NF::Editor — particle system editor
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

enum class PsEmitterShape : uint8_t { Point, Sphere, Box, Cone, Disk, Mesh };
inline const char* psEmitterShapeName(PsEmitterShape v) {
    switch (v) {
        case PsEmitterShape::Point:  return "Point";
        case PsEmitterShape::Sphere: return "Sphere";
        case PsEmitterShape::Box:    return "Box";
        case PsEmitterShape::Cone:   return "Cone";
        case PsEmitterShape::Disk:   return "Disk";
        case PsEmitterShape::Mesh:   return "Mesh";
    }
    return "Unknown";
}

enum class PsSimSpace : uint8_t { World, Local };
inline const char* psSimSpaceName(PsSimSpace v) {
    switch (v) {
        case PsSimSpace::World: return "World";
        case PsSimSpace::Local: return "Local";
    }
    return "Unknown";
}

class PsEmitter {
public:
    explicit PsEmitter(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setShape(PsEmitterShape v) { m_shape    = v; }
    void setSimSpace(PsSimSpace v)  { m_simSpace = v; }
    void setRate(float v)           { m_rate     = v; }
    void setLifetime(float v)       { m_lifetime = v; }
    void setEnabled(bool v)         { m_enabled  = v; }
    void setLooping(bool v)         { m_looping  = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] PsEmitterShape     shape()    const { return m_shape;    }
    [[nodiscard]] PsSimSpace         simSpace() const { return m_simSpace; }
    [[nodiscard]] float              rate()     const { return m_rate;     }
    [[nodiscard]] float              lifetime() const { return m_lifetime; }
    [[nodiscard]] bool               enabled()  const { return m_enabled;  }
    [[nodiscard]] bool               looping()  const { return m_looping;  }

private:
    uint32_t       m_id;
    std::string    m_name;
    PsEmitterShape m_shape    = PsEmitterShape::Point;
    PsSimSpace     m_simSpace = PsSimSpace::World;
    float          m_rate     = 10.0f;
    float          m_lifetime = 2.0f;
    bool           m_enabled  = true;
    bool           m_looping  = true;
};

class ParticleSystemEditorV1 {
public:
    bool addEmitter(const PsEmitter& e) {
        for (auto& x : m_emitters) if (x.id() == e.id()) return false;
        m_emitters.push_back(e); return true;
    }
    bool removeEmitter(uint32_t id) {
        auto it = std::find_if(m_emitters.begin(), m_emitters.end(),
            [&](const PsEmitter& e){ return e.id() == id; });
        if (it == m_emitters.end()) return false;
        m_emitters.erase(it); return true;
    }
    [[nodiscard]] PsEmitter* findEmitter(uint32_t id) {
        for (auto& e : m_emitters) if (e.id() == id) return &e;
        return nullptr;
    }
    [[nodiscard]] size_t emitterCount() const { return m_emitters.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& e : m_emitters) if (e.enabled()) ++n;
        return n;
    }
    [[nodiscard]] size_t loopingCount() const {
        size_t n = 0;
        for (auto& e : m_emitters) if (e.looping()) ++n;
        return n;
    }
    [[nodiscard]] std::vector<PsEmitter> filterByShape(PsEmitterShape shape) const {
        std::vector<PsEmitter> result;
        for (auto& e : m_emitters) if (e.shape() == shape) result.push_back(e);
        return result;
    }

private:
    std::vector<PsEmitter> m_emitters;
};

} // namespace NF
