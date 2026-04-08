#pragma once
// NF::Editor — Editor application
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/SelectionService.h"
#include "NF/Editor/ContentBrowser.h"
#include "NF/Editor/ProjectServices.h"
#include "NF/Editor/EditorTheme.h"
#include "NF/Editor/DockLayout.h"
#include "NF/Editor/EditorPanel.h"
#include "NF/Editor/ViewportPanel.h"
#include "NF/Editor/InspectorPanel.h"
#include "NF/Editor/HierarchyPanel.h"
#include "NF/Editor/ConsolePanel.h"
#include "NF/Editor/ContentBrowserPanel.h"
#include "NF/Editor/EditorToolbar.h"
#include "NF/Editor/IDEIntegration.h"
#include "NF/Editor/MenuBar.h"
#include "NF/Editor/Notifications.h"
#include "NF/Editor/EditorCamera.h"
#include "NF/Editor/EditorSettings.h"
#include "NF/Editor/GraphEditorPanel.h"
#include "NF/Editor/ToolWindowManager.h"
#include "NF/Editor/PipelineMonitorPanel.h"
#include "NF/Editor/FrameStatsTracker.h"
#include "NF/Editor/PCGTuning.h"
#include "NF/Editor/EntityPlacement.h"
#include "NF/Editor/VoxelPaint.h"
#include "NF/Editor/EditorUndoSystem.h"
#include "NF/Editor/WorldPreview.h"
#include "NF/Editor/EditorWorldSession.h"
#include "NF/Editor/AssetDatabase.h"
#include "NF/Editor/AssetImporters.h"
#include "NF/Editor/BlenderImporter.h"
#include "NF/Editor/WorkspacePanelHost.h"

namespace NF {

class EditorApp {
public:
    // Full init with an explicit executable path so the editor can locate the project root.
    bool init(int width, int height, const std::string& executablePath) {
        // Initialise project path service first so commands can reference paths.
        m_projectPaths.init(executablePath);

        // Wire ContentBrowser to the project's Content directory.
        if (!m_projectPaths.contentPath().empty()) {
            m_contentBrowser.setRootPath(m_projectPaths.contentPath());
            m_contentBrowser.refresh();
        }

        m_renderer.init(width, height);
        m_ui.init();

        // Register core editor commands
        m_commands.registerCommand("file.new", [this]() {
            NF_LOG_INFO("Editor", "New world");
            m_currentWorldPath.clear();
            m_commandStack.clear();
        }, "New World", "Ctrl+N");

        m_commands.registerCommand("file.open", [this]() {
            NF_LOG_INFO("Editor", "Open world");
        }, "Open World", "Ctrl+O");

        m_commands.registerCommand("file.save", [this]() {
            NF_LOG_INFO("Editor", "Save world");
            if (!m_currentWorldPath.empty()) {
                m_commandStack.markClean();
            }
        }, "Save", "Ctrl+S");

        m_commands.registerCommand("file.save_as", [this]() {
            NF_LOG_INFO("Editor", "Save world as...");
        }, "Save As", "Ctrl+Shift+S");

        m_commands.registerCommand("edit.undo", [this]() {
            if (m_commandStack.canUndo()) {
                std::string desc = m_commandStack.undoDescription();
                m_commandStack.undo();
                NF_LOG_INFO("Editor", "Undo: " + desc);
            }
        }, "Undo", "Ctrl+Z");
        m_commands.setEnabledCheck("edit.undo", [this]() { return m_commandStack.canUndo(); });

        m_commands.registerCommand("edit.redo", [this]() {
            if (m_commandStack.canRedo()) {
                std::string desc = m_commandStack.redoDescription();
                m_commandStack.redo();
                NF_LOG_INFO("Editor", "Redo: " + desc);
            }
        }, "Redo", "Ctrl+Y");
        m_commands.setEnabledCheck("edit.redo", [this]() { return m_commandStack.canRedo(); });

        m_commands.registerCommand("edit.select_all", [this]() {
            NF_LOG_INFO("Editor", "Select all");
        }, "Select All", "Ctrl+A");

        m_commands.registerCommand("edit.deselect", [this]() {
            m_selection.clearSelection();
        }, "Deselect All", "Ctrl+D");

        m_commands.registerCommand("view.reset_layout", [this]() {
            NF_LOG_INFO("Editor", "Reset panel layout");
        }, "Reset Layout", "");

        // View toggle commands
        m_commands.registerCommand("view.toggle_inspector", [this]() {
            togglePanelVisibility("Inspector");
        }, "Toggle Inspector", "");

        m_commands.registerCommand("view.toggle_hierarchy", [this]() {
            togglePanelVisibility("Hierarchy");
        }, "Toggle Hierarchy", "");

        m_commands.registerCommand("view.toggle_console", [this]() {
            togglePanelVisibility("Console");
        }, "Toggle Console", "");

        m_commands.registerCommand("view.toggle_content_browser", [this]() {
            togglePanelVisibility("ContentBrowser");
        }, "Toggle Content Browser", "");

        // Graph commands
        m_commands.registerCommand("graph.new_graph", [this]() {
            if (auto* gep = graphEditorPanel()) {
                gep->newGraph(GraphType::World, "Untitled");
                m_notifications.push(NotificationType::Success, "New graph created");
            }
            NF_LOG_INFO("Editor", "New graph");
        }, "New Graph", "");

        m_commands.registerCommand("graph.open_graph", [this]() {
            if (auto* gep = graphEditorPanel()) {
                gep->openGraph("Untitled");
            }
            NF_LOG_INFO("Editor", "Open graph");
        }, "Open Graph", "");

        m_commands.registerCommand("graph.add_node", [this]() {
            if (auto* gep = graphEditorPanel()) {
                int id = gep->addNode("NewNode");
                if (id >= 0)
                    m_notifications.push(NotificationType::Info, "Added node #" + std::to_string(id));
            }
        }, "Add Node", "");

        m_commands.registerCommand("graph.remove_node", [this]() {
            if (auto* gep = graphEditorPanel()) {
                if (gep->selectedNodeId() >= 0) {
                    gep->removeNode(gep->selectedNodeId());
                    m_notifications.push(NotificationType::Info, "Node removed");
                }
            }
        }, "Remove Node", "Delete");
        m_commands.setEnabledCheck("graph.remove_node",
            [this]() {
                auto* gep = graphEditorPanel();
                return gep && gep->selectedNodeId() >= 0;
            });

        m_commands.registerCommand("graph.compile", [this]() {
            if (auto* gep = graphEditorPanel()) {
                if (gep->compileAndLoad())
                    m_notifications.push(NotificationType::Success, "Graph compiled");
            }
        }, "Compile Graph", "F7");

        // IDE commands
        m_ideService.init();

        m_commands.registerCommand("ide.go_to_definition", [this]() {
            NF_LOG_INFO("IDE", "Go to definition");
        }, "Go To Definition", "F12");

        m_commands.registerCommand("ide.find_references", [this]() {
            NF_LOG_INFO("IDE", "Find references");
        }, "Find References", "Shift+F12");

        m_commands.registerCommand("ide.go_back", [this]() {
            m_ideService.goBack();
        }, "Go Back", "Alt+Left");

        m_commands.registerCommand("ide.index_project", [this]() {
            // Index Source/, Content/, and Config/ so the entire repo is searchable.
            auto& indexer = m_ideService.indexer();
            indexer.clear();
            for (const std::string rel : {"Source", "Content", "Config"}) {
                std::string dir = m_projectPaths.resolvePath(rel);
                indexer.indexDirectory(dir);
            }
            // Populate CodeNavigator from all indexed symbols.
            auto& nav = m_ideService.navigator();
            nav.clear();
            for (auto& f : indexer.allFiles()) {
                for (auto& sym : f.symbols) {
                    NavigationEntry entry;
                    entry.symbol = sym;
                    entry.kind = SymbolKind::Unknown;
                    entry.filePath = f.path;
                    entry.line = 0;
                    nav.addEntry(std::move(entry));
                }
            }
            NF_LOG_INFO("IDE", "Project indexed: " +
                std::to_string(indexer.fileCount()) + " files, " +
                std::to_string(nav.entryCount()) + " symbols");
            m_notifications.push(NotificationType::Success,
                "Project indexed: " + std::to_string(indexer.fileCount()) + " files");
        }, "Index Project", "");

        // Entity commands
        m_commands.registerCommand("entity.create", [this]() {
            NF_LOG_INFO("Editor", "Create entity");
            m_notifications.push(NotificationType::Success, "Entity created");
        }, "Create Entity", "Ctrl+Shift+N");

        m_commands.registerCommand("entity.delete", [this]() {
            if (!m_selection.hasSelection()) return;
            NF_LOG_INFO("Editor", "Delete selected entities");
            m_selection.clearSelection();
            m_notifications.push(NotificationType::Info, "Entity deleted");
        }, "Delete Entity", "Delete");
        m_commands.setEnabledCheck("entity.delete",
            [this]() { return m_selection.hasSelection(); });

        m_commands.registerCommand("entity.duplicate", [this]() {
            if (!m_selection.hasSelection()) return;
            NF_LOG_INFO("Editor", "Duplicate selected entity");
            m_notifications.push(NotificationType::Success, "Entity duplicated");
        }, "Duplicate Entity", "Ctrl+D");
        m_commands.setEnabledCheck("entity.duplicate",
            [this]() { return m_selection.hasSelection(); });

        // Gizmo mode commands
        m_commands.registerCommand("gizmo.translate", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        }, "Translate Gizmo", "W");

        m_commands.registerCommand("gizmo.rotate", [this]() {
            m_gizmo.setMode(GizmoMode::Rotate);
        }, "Rotate Gizmo", "E");

        m_commands.registerCommand("gizmo.scale", [this]() {
            m_gizmo.setMode(GizmoMode::Scale);
        }, "Scale Gizmo", "R");

        // Toggle graph editor panel
        m_commands.registerCommand("view.toggle_graph_editor", [this]() {
            togglePanelVisibility("GraphEditor");
        }, "Toggle Graph Editor", "");

        // Toggle settings
        m_commands.registerCommand("view.toggle_dark_mode", [this]() {
            m_editorSettings.setDarkMode(!m_editorSettings.settings().darkMode);
            m_editorSettings.applyTheme(m_theme);
            NF_LOG_INFO("Editor", std::string("Dark mode: ") +
                (m_editorSettings.settings().darkMode ? "on" : "off"));
        }, "Toggle Dark Mode", "");

        // Create default panels
        {
            auto viewport = std::make_unique<ViewportPanel>();
            m_dockLayout.addPanel(viewport->name(), viewport->slot());
            m_editorPanels.push_back(std::move(viewport));
        }
        {
            auto inspector = std::make_unique<InspectorPanel>(&m_selection, nullptr);
            m_dockLayout.addPanel(inspector->name(), inspector->slot());
            m_editorPanels.push_back(std::move(inspector));
        }
        {
            auto hierarchy = std::make_unique<HierarchyPanel>(&m_selection);
            m_dockLayout.addPanel(hierarchy->name(), hierarchy->slot());
            m_editorPanels.push_back(std::move(hierarchy));
        }
        {
            auto console = std::make_unique<ConsolePanel>();
            m_dockLayout.addPanel(console->name(), console->slot());
            m_editorPanels.push_back(std::move(console));
        }
        {
            auto cb = std::make_unique<ContentBrowserPanel>(&m_contentBrowser);
            m_dockLayout.addPanel(cb->name(), cb->slot());
            m_editorPanels.push_back(std::move(cb));
        }
        {
            auto graphEd = std::make_unique<GraphEditorPanel>(m_graphVM);
            m_dockLayout.addPanel(graphEd->name(), graphEd->slot());
            m_dockLayout.setPanelVisible(graphEd->name(), false);  // hidden by default
            m_editorPanels.push_back(std::move(graphEd));
        }
        {
            auto pcg = std::make_unique<PCGTuningPanel>();
            m_dockLayout.addPanel(pcg->name(), pcg->slot());
            m_dockLayout.setPanelVisible(pcg->name(), false);  // hidden by default
            m_editorPanels.push_back(std::move(pcg));
        }

        // M2/S1 undo system
        m_editorUndo = std::make_unique<EditorUndoSystem>(m_commandStack);

        // Bind WorkspacePanelHost dispatcher to the shared UIRenderer
        m_workspacePanelHost.setRenderer(&m_ui);

        // M3/S2 Play-in-Editor
        m_playInEditor = std::make_unique<PlayInEditorSystem>(
            &m_entityPlacement, pcgTuningPanel(), viewportPanel());

        // M3/S2 PIE commands
        m_commands.registerCommand("play.start", [this]() {
            if (m_playInEditor->isStopped())
                m_playInEditor->start(m_currentWorldPath);
            else if (m_playInEditor->isPaused())
                m_playInEditor->resume();
            m_notifications.push(NotificationType::Success, "Play");
        }, "Play", "F5");

        m_commands.registerCommand("play.pause", [this]() {
            if (m_playInEditor->isRunning()) {
                m_playInEditor->pause();
                m_notifications.push(NotificationType::Info, "Paused");
            }
        }, "Pause", "F6");
        m_commands.setEnabledCheck("play.pause", [this]() { return m_playInEditor->isRunning(); });

        m_commands.registerCommand("play.stop", [this]() {
            if (!m_playInEditor->isStopped()) {
                m_playInEditor->stop();
                m_notifications.push(NotificationType::Info, "Stopped — world restored");
            }
        }, "Stop", "Shift+F5");
        m_commands.setEnabledCheck("play.stop", [this]() { return !m_playInEditor->isStopped(); });

        // M2/S1 tool commands
        m_commands.registerCommand("view.toggle_pcg_tuning", [this]() {
            togglePanelVisibility("PCGTuning");
        }, "Toggle PCG Tuning", "");

        m_commands.registerCommand("tools.entity_placement", [this]() {
            NF_LOG_INFO("Editor", "Entity Placement tool activated");
        }, "Entity Placement Tool", "");

        m_commands.registerCommand("tools.voxel_paint", [this]() {
            NF_LOG_INFO("Editor", "Voxel Paint tool activated");
        }, "Voxel Paint Tool", "");

        // M4/S3 Asset Pipeline commands
        m_commands.registerCommand("assets.scan", [this]() {
            if (!m_projectPaths.contentPath().empty()) {
                size_t n = m_assetDatabase.scanDirectory(m_projectPaths.contentPath());
                NF_LOG_INFO("Assets", "Scanned " + std::to_string(n) + " assets");
                m_notifications.push(NotificationType::Success,
                    "Asset scan: " + std::to_string(n) + " files found");
            }
        }, "Scan Assets", "");

        m_commands.registerCommand("assets.reimport", [this]() {
            size_t reimported = 0;
            for (auto& guid : m_assetWatcher.dirtyAssets()) {
                auto* entry = m_assetDatabase.findByGuid(guid);
                if (!entry) continue;
                if (m_meshImporter.canImport(entry->path))
                    m_meshImporter.import(m_assetDatabase, entry->path);
                else if (m_textureImporter.canImport(entry->path))
                    m_textureImporter.import(m_assetDatabase, entry->path);
                ++reimported;
            }
            m_assetWatcher.clearAll();
            if (reimported > 0) {
                NF_LOG_INFO("Assets", "Re-imported " + std::to_string(reimported) + " assets");
                m_notifications.push(NotificationType::Success,
                    "Re-imported " + std::to_string(reimported) + " assets");
            }
        }, "Reimport Changed Assets", "");
        m_commands.setEnabledCheck("assets.reimport",
            [this]() { return m_assetWatcher.dirtyCount() > 0; });

        // S4 Blender Bridge commands
        m_commands.registerCommand("blender.set_export_dir", [this]() {
            // In a real editor, this would open a folder picker.
            // For now, default to Content/BlenderExports/
            std::string dir = m_projectPaths.contentPath() + "/BlenderExports";
            std::filesystem::create_directories(dir);
            m_blenderImporter.setExportDirectory(dir);
            NF_LOG_INFO("BlenderBridge", "Export dir: " + dir);
            m_notifications.push(NotificationType::Success,
                "Blender export dir set: " + dir);
        }, "Set Blender Export Dir", "");

        m_commands.registerCommand("blender.scan_exports", [this]() {
            size_t n = m_blenderImporter.scanExports();
            NF_LOG_INFO("BlenderBridge", "Scanned: " + std::to_string(n) + " new exports");
            if (n > 0) {
                m_notifications.push(NotificationType::Info,
                    std::to_string(n) + " new Blender exports found");
            }
        }, "Scan Blender Exports", "");

        m_commands.registerCommand("blender.import_pending", [this]() {
            size_t n = m_blenderImporter.importPending(m_assetDatabase, m_meshImporter);
            if (n > 0) {
                NF_LOG_INFO("BlenderBridge", "Imported " + std::to_string(n) + " assets");
                m_notifications.push(NotificationType::Success,
                    "Imported " + std::to_string(n) + " Blender assets");
            }
        }, "Import Pending Blender Assets", "");
        m_commands.setEnabledCheck("blender.import_pending",
            [this]() { return m_blenderImporter.pendingCount() > 0; });

        m_commands.registerCommand("blender.toggle_auto_import", [this]() {
            bool enabled = !m_blenderImporter.isAutoImportEnabled();
            m_blenderImporter.setAutoImportEnabled(enabled);
            NF_LOG_INFO("BlenderBridge", std::string("Auto-import: ") +
                (enabled ? "ON" : "OFF"));
        }, "Toggle Blender Auto-Import", "");

        // Create default toolbar items
        m_toolbar.addItem("Select", "select", "Select tool", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        });
        m_toolbar.addItem("Move", "move", "Move tool", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        });
        m_toolbar.addItem("Rotate", "rotate", "Rotate tool", [this]() {
            m_gizmo.setMode(GizmoMode::Rotate);
        });
        m_toolbar.addItem("Scale", "scale", "Scale tool", [this]() {
            m_gizmo.setMode(GizmoMode::Scale);
        });
        m_toolbar.addSeparator();
        m_toolbar.addItem("Play", "play", "Play (F5)", [this]() {
            m_commands.executeCommand("play.start");
        });
        m_toolbar.addItem("Pause", "pause", "Pause (F6)", [this]() {
            m_commands.executeCommand("play.pause");
        });
        m_toolbar.addItem("Stop", "stop", "Stop (Shift+F5)", [this]() {
            m_commands.executeCommand("play.stop");
        });

        // Build menu bar
        initMenuBar();

        // Load default hotkeys from registered commands
        m_hotkeyDispatcher.loadDefaults(m_commands);

        // ── Register log sink to feed ConsolePanel ──────────────────
        m_logSinkId = Logger::instance().addSink(
            [this](LogLevel level, std::string_view category, std::string_view message) {
                // Feed the console panel
                for (auto& p : m_editorPanels) {
                    if (auto* cp = dynamic_cast<ConsolePanel*>(p.get())) {
                        cp->addLogMessage(level, category, message);
                        break;
                    }
                }
                // Write to log file (thread-safe)
                {
                    std::lock_guard<std::mutex> lock(m_logFileMutex);
                    if (m_logFile.is_open()) {
                        m_logFile << "[" << Logger::levelTag(level) << "] ["
                                  << category << "] " << message << "\n";
                        m_logFile.flush();
                    }
                }
            });

        // ── Open log file in Logs/ directory ────────────────────────
        {
            std::string logDir = m_projectPaths.resolvePath("Logs");
            std::filesystem::create_directories(logDir);
            std::string logPath = logDir + "/editor.log";
            m_logFile.open(logPath, std::ios::out | std::ios::app);
            if (m_logFile.is_open()) {
                auto now = std::chrono::system_clock::now();
                auto time = std::chrono::system_clock::to_time_t(now);
                m_logFile << "\n=== Editor Session Started: " << std::ctime(&time);
            }
        }

        // ── Load persisted layout from Saved/ ───────────────────────
        loadEditorState();

        NF_LOG_INFO("Editor", "Atlas Editor initialized");
        return true;
    }

    // Convenience overload: uses current working directory as the project root.
    bool init(int width, int height) { return init(width, height, "."); }

    void shutdown() {
        // ── Save editor state before shutdown ───────────────────────
        saveEditorState();

        // ── Remove log sink ─────────────────────────────────────────
        if (m_logSinkId != 0) {
            Logger::instance().removeSink(m_logSinkId);
            m_logSinkId = 0;
        }
        if (m_logFile.is_open()) {
            m_logFile << "=== Editor Session Ended ===\n";
            m_logFile.close();
        }

        m_ideService.shutdown();
        m_editorPanels.clear();
        m_ui.shutdown();
        m_renderer.shutdown();
        NF_LOG_INFO("Editor", "Atlas Editor shutdown");
    }

    void update() {}
    void render() {}

    // Render the full editor UI through the UIRenderer pipeline.
    // Replaces the old paintEditorGDI() function.
    void renderAll(float width, float height) {
        m_dockLayout.computeLayout(width, height, 56.f, 24.f);
        m_ui.beginFrame(width, height);

        // Menu bar
        m_ui.drawRect({0.f, 0.f, width, 28.f}, m_theme.toolbarBackground);
        float mx = 8.f;
        for (auto& cat : m_menuBar.categories()) {
            m_ui.drawText(mx, 7.f, cat.name, m_theme.panelText);
            mx += static_cast<float>(cat.name.size()) * 8.f + 16.f;
        }

        // Toolbar
        m_ui.drawRect({0.f, 28.f, width, 28.f}, m_theme.toolbarBackground);
        float tx = 8.f;
        for (auto& item : m_toolbar.items()) {
            if (item.isSeparator) {
                m_ui.drawRect({tx, 32.f, 1.f, 20.f}, m_theme.toolbarSeparator);
                tx += 12.f;
            } else {
                m_ui.drawText(tx, 35.f, item.name, item.enabled ? m_theme.buttonText : m_theme.buttonDisabledText);
                tx += static_cast<float>(item.name.size()) * 8.f + 12.f;
            }
        }

        // Panels (legacy EditorPanel loop)
        for (auto& panel : m_editorPanels) {
            if (!panel->isVisible()) continue;
            auto* dp = m_dockLayout.findPanel(panel->name());
            if (!dp || !dp->visible) continue;
            panel->render(m_ui, dp->bounds, m_theme);
        }

        // AtlasUI panels — render on top of legacy panels using proper widgets
        m_workspacePanelHost.renderPanels(m_dockLayout);

        // Splitter dividers
        for (auto& dp : m_dockLayout.panels()) {
            if (!dp.visible) continue;
            m_ui.drawRectOutline(dp.bounds, m_theme.panelBorder, 1.f);
        }

        // Status bar
        m_ui.drawRect({0.f, height - 24.f, width, 24.f}, m_theme.statusBarBackground);
        m_ui.drawText(8.f, height - 19.f, m_statusBar.buildText(), m_theme.statusBarText);

        // Notifications overlay
        if (m_notifications.hasActive()) {
            auto* n = m_notifications.current();
            if (n) {
                uint32_t bg = 0x333333FF;
                if (n->type == NotificationType::Error)   bg = 0x8B0000FF;
                if (n->type == NotificationType::Success) bg = 0x006400FF;
                if (n->type == NotificationType::Warning) bg = 0x8B8000FF;
                float nw = static_cast<float>(n->message.size()) * 8.f + 24.f;
                Rect nr{width - nw - 12.f, 60.f, nw, 28.f};
                m_ui.drawRect(nr, bg);
                m_ui.drawRectOutline(nr, m_theme.panelBorder, 1.f);
                m_ui.drawText(nr.x + 8.f, nr.y + 7.f, n->message, 0xFFFFFFFF);
            }
        }

        m_ui.endFrame();
    }

    // Per-frame update with input: routes right-click WASD fly-cam to the viewport,
    // dispatches hotkeys, ticks notifications, updates status bar and frame stats.
    void update(float dt, InputSystem& input) {
        input.update();
        if (auto* vp = viewportPanel())
            vp->updateCamera(dt, input);

        m_notifications.tick(dt);
        m_frameStats.beginFrame(dt);

        // M3/S2: tick Play-in-Editor
        if (m_playInEditor)
            m_playInEditor->tick(dt);

        // Update status bar mode to reflect PIE state
        std::string mode = "Editor";
        if (m_playInEditor && m_playInEditor->isRunning()) mode = "Playing";
        else if (m_playInEditor && m_playInEditor->isPaused()) mode = "Paused";

        m_statusBar.update(
            mode,
            m_currentWorldPath,
            m_commandStack.isDirty(),
            static_cast<int>(m_selection.selectionCount()),
            m_frameStats.stats().fps);

        // Route input to AtlasUI panels
        m_workspacePanelHost.handleInput(m_dockLayout, input);
    }

    // Process a hotkey string and dispatch matching commands.
    int processHotkey(const std::string& hotkey) {
        return m_hotkeyDispatcher.dispatch(hotkey, m_commands);
    }

    // Returns a pointer to the first ViewportPanel, or nullptr.
    [[nodiscard]] ViewportPanel* viewportPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* vp = dynamic_cast<ViewportPanel*>(p.get())) return vp;
        }
        return nullptr;
    }
    [[nodiscard]] const ViewportPanel* viewportPanel() const {
        for (auto& p : m_editorPanels) {
            if (auto* vp = dynamic_cast<const ViewportPanel*>(p.get())) return vp;
        }
        return nullptr;
    }

    [[nodiscard]] GraphEditorPanel* graphEditorPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* gep = dynamic_cast<GraphEditorPanel*>(p.get())) return gep;
        }
        return nullptr;
    }

    // Accessors for editor services
    EditorCommandRegistry& commands() { return m_commands; }
    CommandStack& commandStack() { return m_commandStack; }
    SelectionService& selection() { return m_selection; }
    ContentBrowser& contentBrowser() { return m_contentBrowser; }
    RecentFilesList& recentFiles() { return m_recentFiles; }
    LaunchService& launchService() { return m_launchService; }
    ProjectPathService& projectPaths() { return m_projectPaths; }
    const ProjectPathService& projectPaths() const { return m_projectPaths; }
    DockLayout& dockLayout() { return m_dockLayout; }
    EditorToolbar& toolbar() { return m_toolbar; }
    MenuBar& menuBar() { return m_menuBar; }
    EditorStatusBar& statusBar() { return m_statusBar; }
    NotificationQueue& notifications() { return m_notifications; }
    EditorCameraOrbit& editorCamera() { return m_editorCamera; }
    GizmoState& gizmo() { return m_gizmo; }
    EditorSettingsService& settingsService() { return m_editorSettings; }
    HotkeyDispatcher& hotkeyDispatcher() { return m_hotkeyDispatcher; }
    FrameStatsTracker& frameStatsTracker() { return m_frameStats; }
    EditorTheme& theme() { return m_theme; }
    UIContext& uiContext() { return m_uiContext; }
    ToolWindowManager& toolManager() { return m_toolManager; }
    UIRenderer& uiRenderer() { return m_ui; }

    [[nodiscard]] const std::string& currentWorldPath() const { return m_currentWorldPath; }
    void setCurrentWorldPath(const std::string& path) {
        m_currentWorldPath = path;
        m_recentFiles.addFile(path);
    }

    [[nodiscard]] bool isDirty() const { return m_commandStack.isDirty(); }

    void addPanel(std::unique_ptr<EditorPanel> panel) {
        m_dockLayout.addPanel(panel->name(), panel->slot());
        m_editorPanels.push_back(std::move(panel));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<EditorPanel>>& editorPanels() const {
        return m_editorPanels;
    }

    void setGraphVM(GraphVM* vm) {
        m_graphVM = vm;
        if (auto* gep = graphEditorPanel()) gep->setGraphVM(vm);
    }
    [[nodiscard]] GraphVM* graphVM() const { return m_graphVM; }

    IDEService& ideService() { return m_ideService; }

    // AtlasUI workspace panel host accessor
    [[nodiscard]] WorkspacePanelHost& workspacePanelHost() { return m_workspacePanelHost; }
    [[nodiscard]] const WorkspacePanelHost& workspacePanelHost() const { return m_workspacePanelHost; }

    // M2/S1 accessors
    EntityPlacementTool& entityPlacementTool() { return m_entityPlacement; }
    VoxelPaintTool& voxelPaintTool() { return m_voxelPaint; }
    EditorUndoSystem& editorUndoSystem() { return *m_editorUndo; }
    WorldPreviewService& worldPreview() { return m_worldPreview; }

    // M3/S2 accessors
    PlayInEditorSystem& playInEditor() { return *m_playInEditor; }
    const PlayInEditorSystem& playInEditor() const { return *m_playInEditor; }

    // M4/S3 accessors
    AssetDatabase& assetDatabase() { return m_assetDatabase; }
    const AssetDatabase& assetDatabase() const { return m_assetDatabase; }
    MeshImporter& meshImporter() { return m_meshImporter; }
    TextureImporter& textureImporter() { return m_textureImporter; }
    AssetWatcher& assetWatcher() { return m_assetWatcher; }

    // S4 accessors
    BlenderAutoImporter& blenderAutoImporter() { return m_blenderImporter; }
    const BlenderAutoImporter& blenderAutoImporter() const { return m_blenderImporter; }

    [[nodiscard]] PCGTuningPanel* pcgTuningPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* pcg = dynamic_cast<PCGTuningPanel*>(p.get())) return pcg;
        }
        return nullptr;
    }

private:
    void togglePanelVisibility(const std::string& name) {
        if (auto* p = m_dockLayout.findPanel(name)) {
            p->visible = !p->visible;
            NF_LOG_INFO("Editor", "Toggle panel '" + name + "' visible=" +
                        (p->visible ? "true" : "false"));
        }
    }

    void initMenuBar() {
        // File menu
        auto& file = m_menuBar.addCategory("File");
        file.addItem("New World",   "file.new",     "Ctrl+N");
        file.addItem("Open World",  "file.open",    "Ctrl+O");
        file.addItem("Save",        "file.save",    "Ctrl+S");
        file.addItem("Save As",     "file.save_as", "Ctrl+Shift+S");
        file.addSeparator();
        file.addItem("Exit",        "file.exit",    "Alt+F4");

        // Edit menu
        auto& edit = m_menuBar.addCategory("Edit");
        edit.addItem("Undo",        "edit.undo",    "Ctrl+Z");
        edit.addItem("Redo",        "edit.redo",    "Ctrl+Y");
        edit.addSeparator();
        edit.addItem("Select All",  "edit.select_all",  "Ctrl+A");
        edit.addItem("Deselect",    "edit.deselect",     "Ctrl+D");
        edit.addSeparator();
        edit.addItem("Create Entity",   "entity.create",    "Ctrl+Shift+N");
        edit.addItem("Delete Entity",   "entity.delete",    "Delete");
        edit.addItem("Duplicate Entity","entity.duplicate", "Ctrl+D");
        edit.addSeparator();
        edit.addItem("PCG Tuning",      "view.toggle_pcg_tuning", "");
        edit.addSeparator();
        edit.addItem("Play",            "play.start",            "F5");
        edit.addItem("Pause",           "play.pause",            "F6");
        edit.addItem("Stop",            "play.stop",             "Shift+F5");

        // View menu
        auto& view = m_menuBar.addCategory("View");
        view.addItem("Inspector",      "view.toggle_inspector",       "");
        view.addItem("Hierarchy",      "view.toggle_hierarchy",       "");
        view.addItem("Console",        "view.toggle_console",         "");
        view.addItem("Content Browser","view.toggle_content_browser", "");
        view.addItem("Graph Editor",   "view.toggle_graph_editor",    "");
        view.addSeparator();
        view.addItem("Reset Layout",   "view.reset_layout",           "");
        view.addItem("Dark Mode",      "view.toggle_dark_mode",       "");

        // Graph menu
        auto& graph = m_menuBar.addCategory("Graph");
        graph.addItem("New Graph",    "graph.new_graph",    "");
        graph.addItem("Open Graph",   "graph.open_graph",   "");
        graph.addSeparator();
        graph.addItem("Add Node",     "graph.add_node",     "");
        graph.addItem("Remove Node",  "graph.remove_node",  "");
        graph.addSeparator();
        graph.addItem("Compile",      "graph.compile",      "F7");

        // Code menu
        auto& code = m_menuBar.addCategory("Code");
        code.addItem("Go To Definition", "ide.go_to_definition", "F12");
        code.addItem("Find References",  "ide.find_references",  "Shift+F12");
        code.addItem("Go Back",          "ide.go_back",          "Alt+Left");
        code.addItem("Index Project",    "ide.index_project",    "");

        // Tools menu
        auto& tools = m_menuBar.addCategory("Tools");
        tools.addItem("Blender Bridge",     "tools.launch_blender_bridge");
        tools.addItem("Contract Scanner",   "tools.launch_contract_scanner");
        tools.addItem("Replay Minimizer",   "tools.launch_replay_minimizer");
        tools.addItem("Atlas AI",           "tools.launch_atlas_ai");
        tools.addSeparator();
        tools.addItem("Pipeline Monitor",   "tools.pipeline_monitor");
        tools.addSeparator();
        tools.addItem("Entity Placement",   "tools.entity_placement");
        tools.addItem("Voxel Paint",        "tools.voxel_paint");
        tools.addSeparator();
        tools.addItem("Scan Assets",        "assets.scan");
        tools.addItem("Reimport Changed",   "assets.reimport");
        tools.addSeparator();
        tools.addItem("Set Blender Export Dir",  "blender.set_export_dir");
        tools.addItem("Scan Blender Exports",    "blender.scan_exports");
        tools.addItem("Import Pending",          "blender.import_pending");
        tools.addItem("Toggle Auto-Import",      "blender.toggle_auto_import");
    }

    Renderer m_renderer;
    UIRenderer m_ui;
    EditorCommandRegistry m_commands;
    CommandStack m_commandStack;
    SelectionService m_selection;
    ContentBrowser m_contentBrowser;
    RecentFilesList m_recentFiles;
    LaunchService m_launchService;
    ProjectPathService m_projectPaths;
    DockLayout m_dockLayout;
    EditorToolbar m_toolbar;
    std::vector<std::unique_ptr<EditorPanel>> m_editorPanels;
    std::string m_currentWorldPath;
    GraphVM* m_graphVM = nullptr;
    IDEService m_ideService;

    // New systems
    MenuBar m_menuBar;
    EditorStatusBar m_statusBar;
    NotificationQueue m_notifications;
    EditorCameraOrbit m_editorCamera;
    GizmoState m_gizmo;
    EditorSettingsService m_editorSettings;
    HotkeyDispatcher m_hotkeyDispatcher;
    FrameStatsTracker m_frameStats;
    EditorTheme m_theme;
    UIContext m_uiContext;
    ToolWindowManager m_toolManager;
    size_t m_logSinkId = 0;
    std::ofstream m_logFile;
    std::mutex m_logFileMutex;

    // M2/S1 world-editing systems
    EntityPlacementTool m_entityPlacement;
    VoxelPaintTool m_voxelPaint;
    std::unique_ptr<EditorUndoSystem> m_editorUndo;
    WorldPreviewService m_worldPreview;

    // M3/S2 Play-in-Editor
    std::unique_ptr<PlayInEditorSystem> m_playInEditor;

    // M4/S3 Asset Pipeline
    AssetDatabase m_assetDatabase;
    MeshImporter m_meshImporter;
    TextureImporter m_textureImporter;
    AssetWatcher m_assetWatcher;

    // S4 Blender Bridge
    BlenderAutoImporter m_blenderImporter;

    // AtlasUI workspace panel host (owns the 8 core AtlasUI panels)
    WorkspacePanelHost m_workspacePanelHost;

    // ── State persistence ───────────────────────────────────────

    /// Escape a string for safe JSON embedding (handles quotes and backslashes).
    static std::string escapeJson(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else out += c;
        }
        return out;
    }

    /// Resolve the Saved/ directory path relative to project root.
    std::string savedDir() const {
        std::string dir = m_projectPaths.resolvePath("Saved");
        std::filesystem::create_directories(dir);
        return dir;
    }

    /// Save editor state (panel visibility, dock sizes, settings) to Saved/editor_state.json.
    void saveEditorState() {
        std::string path = savedDir() + "/editor_state.json";
        std::ofstream out(path);
        if (!out.is_open()) return;

        out << "{\n";
        out << "  \"version\": 1,\n";
        out << "  \"darkMode\": " << (m_editorSettings.settings().darkMode ? "true" : "false") << ",\n";
        out << "  \"dockSizes\": {\n";
        out << "    \"left\": " << m_dockLayout.leftWidth() << ",\n";
        out << "    \"right\": " << m_dockLayout.rightWidth() << ",\n";
        out << "    \"top\": " << m_dockLayout.topHeight() << ",\n";
        out << "    \"bottom\": " << m_dockLayout.bottomHeight() << "\n";
        out << "  },\n";
        out << "  \"panels\": [\n";
        auto& panels = m_dockLayout.panels();
        for (size_t i = 0; i < panels.size(); ++i) {
            out << "    {\"name\": \"" << escapeJson(panels[i].name)
                << "\", \"visible\": " << (panels[i].visible ? "true" : "false") << "}";
            if (i + 1 < panels.size()) out << ",";
            out << "\n";
        }
        out << "  ],\n";
        out << "  \"lastWorldPath\": \"" << escapeJson(m_currentWorldPath) << "\",\n";

        // Save recent files
        out << "  \"recentFiles\": [\n";
        auto& recent = m_recentFiles.files();
        for (size_t i = 0; i < recent.size(); ++i) {
            out << "    \"" << escapeJson(recent[i]) << "\"";
            if (i + 1 < recent.size()) out << ",";
            out << "\n";
        }
        out << "  ]\n";
        out << "}\n";

        NF_LOG_INFO("Editor", "Editor state saved to " + path);
    }

    /// Load editor state from Saved/editor_state.json if it exists.
    void loadEditorState() {
        std::string path = savedDir() + "/editor_state.json";
        std::ifstream in(path);
        if (!in.is_open()) {
            NF_LOG_INFO("Editor", "No saved editor state found (first run)");
            return;
        }

        std::string json((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());

        // Minimal JSON extraction — we use simple key scanning since
        // the format is our own well-known structure.
        auto extractFloat = [&](const std::string& key) -> float {
            auto pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos) return -1.f;
            pos = json.find(':', pos);
            if (pos == std::string::npos) return -1.f;
            try { return std::stof(json.substr(pos + 1)); }
            catch (...) { return -1.f; }
        };

        auto extractBool = [&](const std::string& key) -> int {
            auto pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos) return -1;
            pos = json.find(':', pos);
            if (pos == std::string::npos) return -1;
            auto val = json.substr(pos + 1, 10);
            if (val.find("true") != std::string::npos) return 1;
            if (val.find("false") != std::string::npos) return 0;
            return -1;
        };

        // Restore dark mode
        int darkMode = extractBool("darkMode");
        if (darkMode >= 0) {
            m_editorSettings.setDarkMode(darkMode == 1);
            m_editorSettings.applyTheme(m_theme);
        }

        // Restore dock sizes
        float left = extractFloat("left");
        float right = extractFloat("right");
        float top = extractFloat("top");
        float bottom = extractFloat("bottom");
        if (left > 0.f)   m_dockLayout.setLeftWidth(left);
        if (right > 0.f)  m_dockLayout.setRightWidth(right);
        if (top > 0.f)    m_dockLayout.setTopHeight(top);
        if (bottom > 0.f) m_dockLayout.setBottomHeight(bottom);

        // Restore panel visibility
        for (auto& dp : m_dockLayout.panels()) {
            std::string key = "\"name\": \"" + dp.name + "\"";
            auto pos = json.find(key);
            if (pos != std::string::npos) {
                auto vis = json.find("\"visible\"", pos);
                if (vis != std::string::npos && vis < pos + 200) {
                    auto colon = json.find(':', vis);
                    if (colon != std::string::npos) {
                        auto val = json.substr(colon + 1, 10);
                        bool visible = val.find("true") != std::string::npos;
                        m_dockLayout.setPanelVisible(dp.name, visible);
                    }
                }
            }
        }

        NF_LOG_INFO("Editor", "Editor state restored from " + path);
    }
};

// ── Property Editor ──────────────────────────────────────────────
// Reads and writes property values via reflection offsets.


} // namespace NF
