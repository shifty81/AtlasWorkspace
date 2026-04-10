#pragma once
// NF::Workspace — WorkspaceShell: top-level composition root.
//
// WorkspaceShell is the "OS" layer of Atlas Workspace. It owns every registry
// and manager but does NOT know about specific editor tools. Tools are injected
// via registerToolFactory() or registerTool() before initialize() is called,
// or by calling registerCoreTools() from the default tool roster (CoreToolRoster.h).
//
// This separation means the workspace shell is a generic host — it can run
// with any set of tools, not just the hardcoded primary roster.
//
// Owns:
//   - ToolRegistry        (hosted primary tools)
//   - PanelRegistry       (shared reusable panels)
//   - WorkspaceAppRegistry  (child-process apps)
//   - WorkspaceShellContract (event bus, notifications, layout)
//   - ProjectSystemsTool  (project-adapter panels)
//
// See Docs/Canon/01_LOCKED_DIRECTION.md — "Atlas Workspace is a generic
// development host, not a game."
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 1+3.

#include "NF/Workspace/ToolRegistry.h"
#include "NF/Workspace/PanelRegistry.h"
#include "NF/Workspace/SharedPanels.h"
#include "NF/Workspace/WorkspaceShellContract.h"
#include "NF/Workspace/IGameProjectAdapter.h"
#include "NF/Workspace/ProjectSystemsTool.h"
#include "NF/Workspace/ConsoleCommandBus.h"
#include "NF/Workspace/SelectionService.h"
#include "NF/Workspace/EditorEventBus.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF {

// ── Tool factory type ─────────────────────────────────────────────
// A callable that creates and returns a unique_ptr<IHostedTool>.
// Factories are invoked during initialize() to populate the tool registry.

using ToolFactory = std::function<std::unique_ptr<IHostedTool>()>;

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
    // ── Tool registration (call before initialize) ────────────────

    /// Register a tool factory. The factory is called during initialize().
    void registerToolFactory(ToolFactory factory) {
        if (m_phase == ShellPhase::Created)
            m_toolFactories.push_back(std::move(factory));
    }

    /// Register a pre-created tool directly.
    bool registerTool(std::unique_ptr<IHostedTool> tool) {
        return m_toolRegistry.registerTool(std::move(tool));
    }

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

        // Invoke registered tool factories to populate the tool registry
        for (auto& factory : m_toolFactories) {
            auto tool = factory();
            if (tool && !m_toolRegistry.isRegistered(tool->toolId()))
                m_toolRegistry.registerTool(std::move(tool));
        }
        m_toolFactories.clear(); // factories are one-shot

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

        // Shut down tools, then panels, then shell contract
        m_toolRegistry.shutdownAll();
        m_panelRegistry.shutdownAll();
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

    [[nodiscard]] ConsoleCommandBus&         commandBus()          { return m_commandBus;       }
    [[nodiscard]] const ConsoleCommandBus&   commandBus()    const { return m_commandBus;       }

    [[nodiscard]] SelectionService&          selectionService()          { return m_selectionService; }
    [[nodiscard]] const SelectionService&    selectionService()    const { return m_selectionService; }

    [[nodiscard]] EditorEventBus&            eventBus()            { return m_eventBus;         }
    [[nodiscard]] const EditorEventBus&      eventBus()      const { return m_eventBus;         }

    [[nodiscard]] ShellPhase phase() const { return m_phase; }

private:
    // Register the canonical set of shared panels that every tool can use.
    // Panels with factories create ISharedPanel instances on demand.
    void registerDefaultPanels() {
        // Panels with ISharedPanel factory (workspace-owned, lifecycle-managed)
        m_panelRegistry.registerPanelWithFactory(
            {"content_browser",      "Content Browser",      SharedPanelCategory::Navigation},
            [] { return std::make_unique<ContentBrowserSharedPanel>(); });
        m_panelRegistry.registerPanelWithFactory(
            {"component_inspector",  "Component Inspector",  SharedPanelCategory::Inspector},
            [] { return std::make_unique<ComponentInspectorSharedPanel>(); });
        m_panelRegistry.registerPanelWithFactory(
            {"diagnostics",          "Diagnostics",          SharedPanelCategory::Output, false},
            [] { return std::make_unique<DiagnosticsSharedPanel>(); });
        m_panelRegistry.registerPanelWithFactory(
            {"memory_profiler",      "Memory Profiler",      SharedPanelCategory::Output, false},
            [] { return std::make_unique<MemoryProfilerSharedPanel>(); });
        m_panelRegistry.registerPanelWithFactory(
            {"pipeline_monitor",     "Pipeline Monitor",     SharedPanelCategory::Output, false},
            [] { return std::make_unique<PipelineMonitorSharedPanel>(); });
        m_panelRegistry.registerPanelWithFactory(
            {"notification_center",  "Notification Center",  SharedPanelCategory::Status},
            [] { return std::make_unique<NotificationCenterSharedPanel>(); });

        // Descriptor-only panels (no factory yet — legacy or lightweight)
        m_panelRegistry.registerPanel({"inspector",       "Inspector",       SharedPanelCategory::Inspector});
        m_panelRegistry.registerPanel({"outliner",         "Outliner",        SharedPanelCategory::Navigation});
        m_panelRegistry.registerPanel({"console",          "Console",         SharedPanelCategory::Output});
        m_panelRegistry.registerPanel({"log_output",       "Log Output",      SharedPanelCategory::Output});
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
    ConsoleCommandBus         m_commandBus;
    SelectionService          m_selectionService;
    EditorEventBus            m_eventBus;

    std::vector<ToolFactory>  m_toolFactories;   // pending factories (before init)
    std::unique_ptr<IGameProjectAdapter> m_projectAdapter;
    ShellPhase m_phase = ShellPhase::Created;
};

} // namespace NF
