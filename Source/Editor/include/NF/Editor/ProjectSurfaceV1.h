#pragma once
// NF::Editor — project surface representation
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

enum class PjsStatus : uint8_t { Unloaded, Loading, Loaded, Error };
inline const char* pjsStatusName(PjsStatus v) {
    switch (v) {
        case PjsStatus::Unloaded: return "Unloaded";
        case PjsStatus::Loading:  return "Loading";
        case PjsStatus::Loaded:   return "Loaded";
        case PjsStatus::Error:    return "Error";
    }
    return "Unknown";
}

enum class PjsType : uint8_t { Game, Library, Tool, Plugin };
inline const char* pjsTypeName(PjsType v) {
    switch (v) {
        case PjsType::Game:    return "Game";
        case PjsType::Library: return "Library";
        case PjsType::Tool:    return "Tool";
        case PjsType::Plugin:  return "Plugin";
    }
    return "Unknown";
}

class PjsAsset {
public:
    explicit PjsAsset(uint32_t id, const std::string& name, const std::string& path)
        : m_id(id), m_name(name), m_path(path) {}

    void setType(const std::string& v) { m_type     = v; }
    void setModified(bool v)           { m_modified = v; }

    [[nodiscard]] uint32_t           id()       const { return m_id;       }
    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] const std::string& path()     const { return m_path;     }
    [[nodiscard]] const std::string& type()     const { return m_type;     }
    [[nodiscard]] bool               modified() const { return m_modified; }

private:
    uint32_t    m_id;
    std::string m_name;
    std::string m_path;
    std::string m_type;
    bool        m_modified = false;
};

class ProjectSurfaceV1 {
public:
    explicit ProjectSurfaceV1(uint32_t id, const std::string& name)
        : m_id(id), m_name(name) {}

    void setStatus(PjsStatus v)          { m_status   = v; }
    void setType(PjsType v)              { m_type     = v; }
    void setRootPath(const std::string& v){ m_rootPath = v; }

    bool addAsset(const PjsAsset& a) {
        for (auto& x : m_assets) if (x.id() == a.id()) return false;
        m_assets.push_back(a); return true;
    }
    bool removeAsset(uint32_t id) {
        auto it = std::find_if(m_assets.begin(), m_assets.end(),
            [&](const PjsAsset& a){ return a.id() == id; });
        if (it == m_assets.end()) return false;
        m_assets.erase(it); return true;
    }
    [[nodiscard]] PjsAsset* findAsset(uint32_t id) {
        for (auto& a : m_assets) if (a.id() == id) return &a;
        return nullptr;
    }

    [[nodiscard]] uint32_t           id()            const { return m_id;             }
    [[nodiscard]] const std::string& name()          const { return m_name;           }
    [[nodiscard]] PjsStatus          status()        const { return m_status;         }
    [[nodiscard]] PjsType            type()          const { return m_type;           }
    [[nodiscard]] const std::string& rootPath()      const { return m_rootPath;       }
    [[nodiscard]] size_t             assetCount()    const { return m_assets.size();  }
    [[nodiscard]] size_t             modifiedCount() const {
        size_t n = 0;
        for (auto& a : m_assets) if (a.modified()) ++n;
        return n;
    }

private:
    uint32_t             m_id;
    std::string          m_name;
    PjsStatus            m_status   = PjsStatus::Unloaded;
    PjsType              m_type     = PjsType::Game;
    std::string          m_rootPath;
    std::vector<PjsAsset> m_assets;
};

} // namespace NF
