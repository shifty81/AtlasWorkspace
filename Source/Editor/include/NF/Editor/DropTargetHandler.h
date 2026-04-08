#pragma once
// NF::Editor — drag-and-drop target management
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

enum class DropAction : uint8_t { Copy, Move, Link, Ask };
inline const char* dropActionName(DropAction v) {
    switch (v) {
        case DropAction::Copy: return "Copy";
        case DropAction::Move: return "Move";
        case DropAction::Link: return "Link";
        case DropAction::Ask:  return "Ask";
    }
    return "Unknown";
}

enum class DropZone : uint8_t { AssetFolder, SceneView, ContentBrowser, Inspector };
inline const char* dropZoneName(DropZone v) {
    switch (v) {
        case DropZone::AssetFolder:     return "AssetFolder";
        case DropZone::SceneView:       return "SceneView";
        case DropZone::ContentBrowser:  return "ContentBrowser";
        case DropZone::Inspector:       return "Inspector";
    }
    return "Unknown";
}

class DropSession {
public:
    explicit DropSession(uint32_t id, DropZone zone, DropAction action)
        : m_id(id), m_zone(zone), m_action(action) {}

    void setActive(bool v)              { m_active = v; }
    void addPath(const std::string& p)  { m_paths.push_back(p); }

    [[nodiscard]] uint32_t                          id()        const { return m_id;     }
    [[nodiscard]] DropZone                          zone()      const { return m_zone;   }
    [[nodiscard]] DropAction                        action()    const { return m_action; }
    [[nodiscard]] bool                              active()    const { return m_active; }
    [[nodiscard]] const std::vector<std::string>&   paths()     const { return m_paths;  }
    [[nodiscard]] size_t                            pathCount() const { return m_paths.size(); }

private:
    uint32_t                 m_id;
    DropZone                 m_zone;
    DropAction               m_action;
    bool                     m_active = false;
    std::vector<std::string> m_paths;
};

class DropTargetHandler {
public:
    bool beginDrop(uint32_t id, DropZone zone) {
        for (auto& s : m_sessions) if (s.id() == id) return false;
        DropSession session(id, zone, DropAction::Copy);
        session.setActive(true);
        m_sessions.push_back(session);
        return true;
    }
    bool endDrop(uint32_t id) {
        auto* s = findSession(id);
        if (!s) return false;
        s->setActive(false);
        return true;
    }
    [[nodiscard]] DropSession* findSession(uint32_t id) {
        for (auto& s : m_sessions) if (s.id() == id) return &s;
        return nullptr;
    }
    [[nodiscard]] size_t activeSessions() const {
        size_t n = 0;
        for (auto& s : m_sessions) if (s.active()) ++n;
        return n;
    }
    bool addPath(uint32_t id, const std::string& path) {
        auto* s = findSession(id);
        if (!s) return false;
        s->addPath(path);
        return true;
    }

private:
    std::vector<DropSession> m_sessions;
};

} // namespace NF
