#pragma once
// NF::Editor — WorkspaceShell: top-level composition root.
//
// WorkspaceShell owns every registry and manager in the workspace:
//   - ToolRegistry        (hosted primary tools)
//   - PanelRegistry       (shared reusable panels)
//   - WorkspaceAppRegistry  (child-process apps)
//   - WorkspaceShellContract (event bus, notifications, layout)
//   - ProjectSystemsTool  (project-adapter panels)
//
// It provides a single initialize() / shutdown() lifecycle and exposes
// project adapter loading as a first-class concept.
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 1.

#include "NF/Editor/ToolRegistry.h"
#include "NF/Editor/PanelRegistry.h"
#include "NF/Editor/WorkspaceShellContract.h"
#include "NF/Editor/IGameProjectAdapter.h"
#include "NF/Editor/ProjectSystemsTool.h"
// ── Primary tool roster ───────────────────────────────────────────
#include "NF/Editor/SceneEditorTool.h"
#include "NF/Editor/AssetEditorTool.h"
#include "NF/Editor/MaterialEditorTool.h"
#include "NF/Editor/AnimationEditorTool.h"
#include "NF/Editor/DataEditorTool.h"
#include "NF/Editor/VisualLogicEditorTool.h"
#include "NF/Editor/BuildTool.h"
#include "NF/Editor/AtlasAITool.h"
#include <memory>
#include <string>

namespace NF {

// ── Shell Lifecycle Phase ─────────────────────────────────────────

enum class ShellPhase : uint8_t {
    Created,          // constructed, nothing initialized
    Initializing,     // registries being populated
    Ready,            // fully operational
    ShuttingDown,     // teardown in progress
    Destroyed,        // terminal state
};

inline const char* shellPhaseName(ShellPhase p) {
    switch (p) {
        case ShellPhase::Created:      return "Created";
        case ShellPhase::Initializing: return "Initializing";
        case ShellPhase::Ready:        return "Ready";
        case ShellPhase::ShuttingDown: return "ShuttingDown";
        case ShellPhase::Destroyed:    return "Destroyed";
    }
    return "Unknown";
}

// ── WorkspaceShell ────────────────────────────────────────────────

class WorkspaceShell {
public:
    // ── Lifecycle ─────────────────────────────────────────────────

    bool initialize() {
        if (m_phase != ShellPhase::Created) return false;
        m_phase = ShellPhase::Initializing;

        // Wire shell contract bindings
        m_shellContract.bindAppRegistry(&m_appRegistry);
        m_shellContract.bindLayoutManager(&m_layoutManager);
        m_shellContract.initialize();

        // Register canonical shared panels
        registerDefaultPanels();

        // Register the canonical primary tool roster
        registerCoreTools();

        // Initialize all registered tools
        m_toolRegistry.initializeAll();

        m_phase = ShellPhase::Ready;
        return true;
    }

    void shutdown() {
        if (m_phase != ShellPhase::Ready && m_phase != ShellPhase::Initializing) return;
        m_phase = ShellPhase::ShuttingDown;

        // Unload project adapter first
        unloadProject();

        // Shut down tools, then shell contract
        m_toolRegistry.shutdownAll();
        m_shellContract.shutdown();

        m_phase = ShellPhase::Destroyed;
    }

    void update(float dt) {
        if (m_phase != ShellPhase::Ready) return;
        m_toolRegistry.updateActive(dt);
    }

    // ── Project adapter ───────────────────────────────────────────

    bool loadProject(std::unique_ptr<IGameProjectAdapter> adapter) {
        if (!adapter || m_phase != ShellPhase::Ready) return false;
        if (!adapter->initialize()) return false;

        m_projectAdapter = std::move(adapter);
        m_projectSystemsTool.loadFromAdapter(*m_projectAdapter);
        m_toolRegistry.notifyProjectLoaded(m_projectAdapter->projectId());
        m_shellContract.postNotification(
            Notification{"Project loaded: " + m_projectAdapter->projectDisplayName()});
        return true;
    }

    void unloadProject() {
        if (!m_projectAdapter) return;
        m_toolRegistry.notifyProjectUnloaded();
        m_projectAdapter->shutdown();
        m_projectAdapter.reset();
    }

    [[nodiscard]] bool hasProject() const { return m_projectAdapter != nullptr; }
    [[nodiscard]] const IGameProjectAdapter* projectAdapter() const {
        return m_projectAdapter.get();
    }

    // ── Accessors ─────────────────────────────────────────────────

    [[nodiscard]] ToolRegistry&              toolRegistry()       { return m_toolRegistry;    }
    [[nodiscard]] const ToolRegistry&        toolRegistry() const { return m_toolRegistry;    }

    [[nodiscard]] PanelRegistry&             panelRegistry()       { return m_panelRegistry;   }
    [[nodiscard]] const PanelRegistry&       panelRegistry() const { return m_panelRegistry;   }

    [[nodiscard]] WorkspaceAppRegistry&      appRegistry()       { return m_appRegistry;      }
    [[nodiscard]] const WorkspaceAppRegistry& appRegistry() const { return m_appRegistry;     }

    [[nodiscard]] WorkspaceShellContract&    shellContract()       { return m_shellContract;   }
    [[nodiscard]] const WorkspaceShellContract& shellContract() const { return m_shellContract; }

    [[nodiscard]] ProjectSystemsTool&        projectSystemsTool()       { return m_projectSystemsTool; }
    [[nodiscard]] const ProjectSystemsTool&  projectSystemsTool() const { return m_projectSystemsTool; }

    [[nodiscard]] LayoutManagerV1&           layoutManager()       { return m_layoutManager;   }
    [[nodiscard]] const LayoutManagerV1&     layoutManager() const { return m_layoutManager;   }

    [[nodiscard]] ShellPhase phase() const { return m_phase; }

private:
    // Register the canonical primary tool roster for this workspace session.
    // Tools are registered in Atlas-bar display order.
    // Any tool already registered (e.g. injected by tests) is silently skipped.
    void registerCoreTools() {
        auto tryRegister = [this](std::unique_ptr<IHostedTool> t) {
            if (!m_toolRegistry.isRegistered(t->toolId()))
                m_toolRegistry.registerTool(std::move(t));
        };
        tryRegister(std::make_unique<SceneEditorTool>());
        tryRegister(std::make_unique<AssetEditorTool>());
        tryRegister(std::make_unique<MaterialEditorTool>());
        tryRegister(std::make_unique<AnimationEditorTool>());
        tryRegister(std::make_unique<DataEditorTool>());
        tryRegister(std::make_unique<VisualLogicEditorTool>());
        tryRegister(std::make_unique<BuildTool>());
        tryRegister(std::make_unique<AtlasAITool>());
    }

    // Register the canonical set of shared panels that every tool can use.
    void registerDefaultPanels() {
        m_panelRegistry.registerPanel({"inspector",       "Inspector",       SharedPanelCategory::Inspector});
        m_panelRegistry.registerPanel({"outliner",         "Outliner",        SharedPanelCategory::Navigation});
        m_panelRegistry.registerPanel({"content_browser",  "Content Browser", SharedPanelCategory::Navigation});
        m_panelRegistry.registerPanel({"console",          "Console",         SharedPanelCategory::Output});
        m_panelRegistry.registerPanel({"log_output",       "Log Output",      SharedPanelCategory::Output});
        m_panelRegistry.registerPanel({"diagnostics",      "Diagnostics",     SharedPanelCategory::Output, false});
        m_panelRegistry.registerPanel({"atlasai_chat",     "AtlasAI Chat",    SharedPanelCategory::AI, false});
        m_panelRegistry.registerPanel({"notifications",    "Notifications",   SharedPanelCategory::Status});
        m_panelRegistry.registerPanel({"command_palette",  "Command Palette", SharedPanelCategory::Editing, false});
        m_panelRegistry.registerPanel({"asset_preview",    "Asset Preview",   SharedPanelCategory::Preview, false});
    }

    // ── Owned subsystems ──────────────────────────────────────────

    ToolRegistry              m_toolRegistry;
    PanelRegistry             m_panelRegistry;
    WorkspaceAppRegistry      m_appRegistry;
    WorkspaceShellContract    m_shellContract;
    ProjectSystemsTool        m_projectSystemsTool;
    LayoutManagerV1           m_layoutManager;

    std::unique_ptr<IGameProjectAdapter> m_projectAdapter;
    ShellPhase m_phase = ShellPhase::Created;
};

} // namespace NF
