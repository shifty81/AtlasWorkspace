#pragma once
// NF::Editor — memory tracker
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

enum class MtrAllocType : uint8_t { General, Mesh, Texture, Audio, Script, Physics, Cache };
inline const char* mtrAllocTypeName(MtrAllocType v) {
    switch (v) {
        case MtrAllocType::General: return "General";
        case MtrAllocType::Mesh:    return "Mesh";
        case MtrAllocType::Texture: return "Texture";
        case MtrAllocType::Audio:   return "Audio";
        case MtrAllocType::Script:  return "Script";
        case MtrAllocType::Physics: return "Physics";
        case MtrAllocType::Cache:   return "Cache";
    }
    return "Unknown";
}

enum class MtrSnapMode : uint8_t { Live, Snapshot, Delta };
inline const char* mtrSnapModeName(MtrSnapMode v) {
    switch (v) {
        case MtrSnapMode::Live:     return "Live";
        case MtrSnapMode::Snapshot: return "Snapshot";
        case MtrSnapMode::Delta:    return "Delta";
    }
    return "Unknown";
}

class MtrAllocation {
public:
    explicit MtrAllocation(uint32_t id, const std::string& tag) : m_id(id), m_tag(tag) {}

    void setType(MtrAllocType v)  { m_type      = v; }
    void setSizeBytes(uint64_t v) { m_sizeBytes = v; }
    void setAddress(uint64_t v)   { m_address   = v; }
    void setActive(bool v)        { m_active    = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& tag()       const { return m_tag;       }
    [[nodiscard]] MtrAllocType       type()      const { return m_type;      }
    [[nodiscard]] uint64_t           sizeBytes() const { return m_sizeBytes; }
    [[nodiscard]] uint64_t           address()   const { return m_address;   }
    [[nodiscard]] bool               active()    const { return m_active;    }

private:
    uint32_t    m_id;
    std::string m_tag;
    MtrAllocType m_type      = MtrAllocType::General;
    uint64_t     m_sizeBytes = 0;
    uint64_t     m_address   = 0;
    bool         m_active    = true;
};

class MemoryTrackerV1 {
public:
    bool addAllocation(const MtrAllocation& a) {
        for (auto& x : m_allocations) if (x.id() == a.id()) return false;
        m_allocations.push_back(a); return true;
    }
    bool removeAllocation(uint32_t id) {
        auto it = std::find_if(m_allocations.begin(), m_allocations.end(),
            [&](const MtrAllocation& a){ return a.id() == id; });
        if (it == m_allocations.end()) return false;
        m_allocations.erase(it); return true;
    }
    [[nodiscard]] MtrAllocation* findAllocation(uint32_t id) {
        for (auto& a : m_allocations) if (a.id() == id) return &a;
        return nullptr;
    }
    [[nodiscard]] size_t allocationCount() const { return m_allocations.size(); }
    void setSnapMode(MtrSnapMode v) { m_snapMode = v; }
    [[nodiscard]] MtrSnapMode snapMode() const { return m_snapMode; }
    [[nodiscard]] size_t activeCount() const {
        size_t n = 0;
        for (auto& a : m_allocations) if (a.active()) ++n;
        return n;
    }
    [[nodiscard]] uint64_t totalBytes() const {
        uint64_t total = 0;
        for (auto& a : m_allocations) if (a.active()) total += a.sizeBytes();
        return total;
    }
    [[nodiscard]] std::vector<MtrAllocation> filterByType(MtrAllocType t) const {
        std::vector<MtrAllocation> result;
        for (auto& a : m_allocations) if (a.type() == t) result.push_back(a);
        return result;
    }

private:
    std::vector<MtrAllocation> m_allocations;
    MtrSnapMode                m_snapMode = MtrSnapMode::Live;
};

} // namespace NF
