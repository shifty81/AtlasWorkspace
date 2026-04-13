#pragma once
// NF::Workspace — WorkspaceShell: top-level composition root.
//
// ┌─────────────────────────────────────────────────────────────────────────┐
// │  OWNERSHIP BOUNDARY — WorkspaceShell is a host/runtime owner ONLY.     │
// │                                                                         │
// │  WorkspaceShell owns:                                                   │
// │    • ToolRegistry, PanelRegistry, WorkspaceAppRegistry                  │
// │    • WorkspaceShellContract (event bus, notifications, layout)          │
// │    • ProjectSystemsTool, ConsoleCommandBus, InputRouter                 │
// │    • SelectionService, EditorEventBus, WorkspaceViewportManager         │
// │    • AssetCatalog, AssetCatalogPopulator, SettingsStore, LayoutPersist. │
// │    • Active IGameProjectAdapter (non-owning ref exposed via adapter())  │
// │    • WorkspaceProjectState (session-level project truth) ← Phase G wiring│
// │                                                                         │
// │  WorkspaceShell does NOT own:                                           │
// │    • Per-document authored edit data (→ NovaForgeDocument)              │
// │    • Aggregate dirty/save orchestration (→ WorkspaceProjectState)       │
// │    • Panel-local authored data                                           │
// │    • .atlas parsing (→ AtlasProjectFileLoader)                          │
// │                                                                         │
// │  Do NOT add authored project-data fields to this class.                 │
// └─────────────────────────────────────────────────────────────────────────┘
//
// WorkspaceShell is the "OS" layer of Atlas Workspace. It owns every registry
// and manager but does NOT know about specific editor tools. Tools are injected
// via registerToolFactory() or registerTool() before initialize() is called,
// or by calling registerCoreTools() from the default tool roster (CoreToolRoster.h).
//
// This separation means the workspace shell is a generic host — it can run
// with any set of tools, not just the hardcoded primary roster.
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
#include "NF/Workspace/InputRouter.h"
#include "NF/Workspace/SelectionService.h"
#include "NF/Workspace/EditorEventBus.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/AssetCatalogPopulator.h"
#include "NF/Workspace/SettingsStore.h"
#include "NF/Workspace/LayoutPersistence.h"
#include "NF/Workspace/WorkspaceProjectState.h"
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

        // Seed the command bus with default workspace commands
        registerDefaultCommands();

        // Register default workspace settings
        registerDefaultSettings();

        // Invoke registered tool factories to populate the tool registry
        for (auto& factory : m_toolFactories) {
            auto tool = factory();
            if (tool && !m_toolRegistry.isRegistered(tool->toolId()))
                m_toolRegistry.registerTool(std::move(tool));
        }
        m_toolFactories.clear(); // factories are one-shot

        // Initialize all registered tools
        m_toolRegistry.initializeAll();

        // Register each tool's declared commands into the command bus.
        // Tools list commands in their descriptor; we wire a default handler
        // that logs execution so the command palette can discover them.
        registerAllToolCommands();

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

        // Populate asset catalog from project content roots
        populateAssetCatalog();

        m_toolRegistry.notifyProjectLoaded(m_projectAdapter->projectId());
        m_shellContract.postNotification(
            Notification{"Project loaded: " + m_projectAdapter->projectDisplayName()});

        // Notify WorkspaceProjectState — the session-level authority for project truth.
        m_projectState.onProjectLoaded(m_projectAdapter.get(), {});

        // Publish project-loaded event so all bus subscribers are notified.
        m_eventBus.post({"project.loaded", m_projectAdapter->projectId(),
                          EditorEventPriority::High});
        m_eventBus.flush();
        return true;
    }

    void unloadProject() {
        if (!m_projectAdapter) return;
        const std::string pid = m_projectAdapter->projectId();
        m_toolRegistry.notifyProjectUnloaded();
        m_assetCatalog.clear();
        m_settingsStore.clearLayer(SettingsLayer::Project);
        m_projectAdapter->shutdown();
        m_projectAdapter.reset();
        // Clear project state so panels and viewport know no project is loaded.
        m_projectState.onProjectUnloaded();
        m_eventBus.post({"project.unloaded", pid, EditorEventPriority::High});
        m_eventBus.flush();
    }

    [[nodiscard]] bool hasProject() const { return m_projectAdapter != nullptr; }
    [[nodiscard]] const IGameProjectAdapter* projectAdapter() const {
        return m_projectAdapter.get();
    }

    // ── Tool activation helpers ───────────────────────────────────
    // These wrappers delegate to ToolRegistry and publish events on
    // the EditorEventBus so subscribers (panels, renderers) can react.

    bool activateTool(const std::string& toolId) {
        bool ok = m_toolRegistry.activateTool(toolId);
        if (ok) {
            m_eventBus.post({"tool.activated", toolId, EditorEventPriority::Normal});
            m_eventBus.flush();
        }
        return ok;
    }

    void deactivateTool() {
        const std::string prev = m_toolRegistry.activeToolId();
        m_toolRegistry.deactivateTool();
        if (!prev.empty()) {
            m_eventBus.post({"tool.deactivated", prev, EditorEventPriority::Normal});
            m_eventBus.flush();
        }
    }

    // ── Project state (session-level source of truth) ─────────────────────
    [[nodiscard]] WorkspaceProjectState&       projectState()       { return m_projectState; }
    [[nodiscard]] const WorkspaceProjectState& projectState() const { return m_projectState; }

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

    [[nodiscard]] InputRouter&               inputRouter()         { return m_inputRouter;      }
    [[nodiscard]] const InputRouter&         inputRouter()   const { return m_inputRouter;      }

    [[nodiscard]] SelectionService&          selectionService()          { return m_selectionService; }
    [[nodiscard]] const SelectionService&    selectionService()    const { return m_selectionService; }

    [[nodiscard]] EditorEventBus&            eventBus()            { return m_eventBus;         }
    [[nodiscard]] const EditorEventBus&      eventBus()      const { return m_eventBus;         }

    [[nodiscard]] WorkspaceViewportManager&       viewportManager()       { return m_viewportManager; }
    [[nodiscard]] const WorkspaceViewportManager& viewportManager() const { return m_viewportManager; }

    [[nodiscard]] AssetCatalog&               assetCatalog()          { return m_assetCatalog;     }
    [[nodiscard]] const AssetCatalog&         assetCatalog()    const { return m_assetCatalog;     }

    [[nodiscard]] SettingsStore&              settingsStore()          { return m_settingsStore;    }
    [[nodiscard]] const SettingsStore&        settingsStore()    const { return m_settingsStore;    }

    [[nodiscard]] LayoutPersistenceManager&   layoutPersistence()       { return m_layoutPersistence; }
    [[nodiscard]] const LayoutPersistenceManager& layoutPersistence() const { return m_layoutPersistence; }

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

    // Seed the command bus with the default workspace-level command set.
    // Each command is registered with a live handler so menu items and the
    // command palette can dispatch real behavior instead of no-ops.
    void registerDefaultCommands() {
        auto addCmd = [this](const std::string& name,
                              ConsoleCmdScope scope,
                              const std::string& description,
                              ConsoleCommandHandler handler,
                              bool enabled = true) {
            ConsoleCommand cmd(name, scope, ConsoleCmdArgType::None);
            cmd.setDescription(description);
            cmd.setEnabled(enabled);
            (void)m_commandBus.registerCommand(cmd, std::move(handler));
        };

        // Project commands
        addCmd("workspace.project.close", ConsoleCmdScope::Global,
               "Close the active project",
               [this]() -> ConsoleCmdExecResult {
                   if (!hasProject()) return ConsoleCmdExecResult::PermissionDenied;
                   unloadProject();
                   return ConsoleCmdExecResult::Ok;
               });

        addCmd("workspace.project.settings", ConsoleCmdScope::Global,
               "Open project settings",
               [this]() -> ConsoleCmdExecResult {
                   m_shellContract.postNotification(
                       Notification{"Project settings — not yet implemented."});
                   return ConsoleCmdExecResult::Ok;
               });

        // Tools menu
        addCmd("workspace.preferences", ConsoleCmdScope::Global,
               "Open workspace preferences",
               [this]() -> ConsoleCmdExecResult {
                   m_shellContract.postNotification(
                       Notification{"Preferences — not yet implemented."});
                   return ConsoleCmdExecResult::Ok;
               });

        addCmd("workspace.command_palette", ConsoleCmdScope::Global,
               "Open the command palette",
               [this]() -> ConsoleCmdExecResult {
                   m_panelRegistry.setVisible("command_palette", true);
                   return ConsoleCmdExecResult::Ok;
               });

        addCmd("workspace.diagnostics", ConsoleCmdScope::Global,
               "Open diagnostics panel",
               [this]() -> ConsoleCmdExecResult {
                   m_panelRegistry.setVisible("diagnostics", true);
                   return ConsoleCmdExecResult::Ok;
               });

        // View menu — panel visibility toggles
        addCmd("workspace.view.content_browser", ConsoleCmdScope::Global,
               "Toggle Content Browser",
               [this]() -> ConsoleCmdExecResult {
                   m_panelRegistry.toggleVisible("content_browser");
                   return ConsoleCmdExecResult::Ok;
               });

        addCmd("workspace.view.inspector", ConsoleCmdScope::Global,
               "Toggle Inspector",
               [this]() -> ConsoleCmdExecResult {
                   m_panelRegistry.toggleVisible("inspector");
                   return ConsoleCmdExecResult::Ok;
               });

        addCmd("workspace.view.outliner", ConsoleCmdScope::Global,
               "Toggle Outliner",
               [this]() -> ConsoleCmdExecResult {
                   m_panelRegistry.toggleVisible("outliner");
                   return ConsoleCmdExecResult::Ok;
               });

        addCmd("workspace.view.console", ConsoleCmdScope::Global,
               "Toggle Console",
               [this]() -> ConsoleCmdExecResult {
                   m_panelRegistry.toggleVisible("console");
                   return ConsoleCmdExecResult::Ok;
               });

        // Help menu
        addCmd("workspace.help.docs", ConsoleCmdScope::Global,
               "Open documentation",
               [this]() -> ConsoleCmdExecResult {
                   m_shellContract.postNotification(
                       Notification{"Documentation: see Docs/ in the repository."});
                   return ConsoleCmdExecResult::Ok;
               });

        addCmd("workspace.help.about", ConsoleCmdScope::Global,
               "Show About",
               [this]() -> ConsoleCmdExecResult {
                   m_shellContract.postNotification(
                       Notification{"Atlas Workspace — development host platform."});
                   return ConsoleCmdExecResult::Ok;
               });
    }

    // Populate default workspace settings.
    void registerDefaultSettings() {
        m_settingsStore.setDefault("workspace.theme", "dark");
        m_settingsStore.setDefault("workspace.auto_save", "true");
        m_settingsStore.setDefault("workspace.auto_save_interval_sec", "300");
        m_settingsStore.setDefault("workspace.show_welcome", "true");
        m_settingsStore.setDefault("workspace.ui_scale", "1.0");
        m_settingsStore.setDefault("workspace.max_recent_projects", "10");
    }

    // Register commands declared by each tool's descriptor into the command bus.
    // Each tool lists its command IDs in descriptor().commands; we wire a
    // logging handler so the command palette can find and dispatch them.
    void registerAllToolCommands() {
        for (const auto* desc : m_toolRegistry.allDescriptors()) {
            const std::string toolId = desc->toolId;
            for (const auto& cmdName : desc->commands) {
                if (m_commandBus.findCommand(cmdName) != nullptr) continue;
                ConsoleCommand cmd(cmdName, ConsoleCmdScope::Editor, ConsoleCmdArgType::None);
                cmd.setDescription(desc->displayName + ": " + cmdName);
                cmd.setEnabled(true);
                // Lifetime: m_commandBus and the captured members (m_toolRegistry,
                // m_shellContract) are all members of WorkspaceShell.  The command
                // bus is destroyed with the shell, so these handlers cannot outlive
                // the shell.  No weak reference is needed.
                (void)m_commandBus.registerCommand(cmd,
                    [this, toolId, cmdName]() -> ConsoleCmdExecResult {
                        m_toolRegistry.activateTool(toolId);
                        m_shellContract.postNotification(
                            Notification{"Command: " + cmdName});
                        return ConsoleCmdExecResult::Ok;
                    });
            }
        }
    }

    // Populate asset catalog from project content roots.
    // Called automatically during loadProject() after the adapter initializes.
    void populateAssetCatalog() {
        if (!m_projectAdapter) return;
        m_assetCatalog.clear();
        AssetCatalogPopulator populator;
        for (const auto& root : m_projectAdapter->contentRoots()) {
            auto result = populator.populateFromDirectory(m_assetCatalog, root, true);
            NF_LOG_INFO("WorkspaceShell",
                "Asset scan: " + root +
                " — " + std::to_string(result.assetsAdded) + " assets added, " +
                std::to_string(result.filesScanned) + " files scanned");
        }
    }

    // ── Owned subsystems ──────────────────────────────────────────

    ToolRegistry              m_toolRegistry;
    PanelRegistry             m_panelRegistry;
    WorkspaceAppRegistry      m_appRegistry;
    WorkspaceShellContract    m_shellContract;
    ProjectSystemsTool        m_projectSystemsTool;
    LayoutManagerV1           m_layoutManager;
    ConsoleCommandBus         m_commandBus;
    InputRouter               m_inputRouter;
    SelectionService          m_selectionService;
    EditorEventBus            m_eventBus;
    WorkspaceViewportManager  m_viewportManager;
    AssetCatalog              m_assetCatalog;
    SettingsStore             m_settingsStore;
    LayoutPersistenceManager  m_layoutPersistence;

    std::vector<ToolFactory>  m_toolFactories;   // pending factories (before init)
    std::unique_ptr<IGameProjectAdapter> m_projectAdapter;
    WorkspaceProjectState                m_projectState;  ///< session-level project truth
    ShellPhase m_phase = ShellPhase::Created;
};

} // namespace NF
