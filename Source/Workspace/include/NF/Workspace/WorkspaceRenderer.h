#pragma once
// NF::WorkspaceRenderer — Workspace shell chrome renderer.
//
// Renders the workspace-level UI (title bar, tool launcher sidebar, project
// dashboard, and status bar) into a UIRenderer (GDI-backed on Windows).
//
// This is the WORKSPACE outer shell UI — NOT the game editor surface.
// The workspace lists and launches hosted tools (NovaForgeEditor, NovaForgeGame,
// NovaForgeServer, …) as child processes via WorkspaceLaunchService.
//
// WorkspaceRenderer maintains a UIContext for interactive hit regions.
// Mouse state (UIMouseState) must be supplied every frame so that hover
// highlights and click detection work correctly.
//
// ── Deferred action pattern ───────────────────────────────────────────────
// Clicks detected inside render() (which runs in WM_PAINT on Windows) must
// not open modal dialogs directly — doing so creates a nested message loop
// inside WM_PAINT and can deadlock or corrupt paint state.
//
// Instead, render() stores a WorkspaceAction in m_pendingAction.  The main
// loop calls takePendingAction() after each WM_PAINT dispatch and opens the
// appropriate dialog safely outside the paint callback.
//
// Similarly, launch errors queue a WorkspacePendingError so MessageBoxW can
// be shown from the main loop rather than from inside WM_PAINT.
//
// Usage (main loop):
//
//   NF::WorkspaceRenderer wsRenderer;
//   NF::NullLaunchService launchSvc;
//   while (running) {
//       ui.beginFrame(w, h);          // called internally by render()
//       wsRenderer.render(ui, w, h, shell, mouse, &launchSvc);
//       // ui.endFrame() is called by render() before returning
//       // After message dispatch, handle deferred actions:
//       auto action = wsRenderer.takePendingAction();   // WorkspaceAction
//       auto err    = wsRenderer.takePendingError();    // WorkspacePendingError
//   }
//
// Color format: RRGGBBAA (matches UIRenderer / GDIBackend convention).

#include "NF/UI/UI.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspaceActivityBar.h"
#include "NF/Workspace/WorkspaceLaunchContract.h"
#include <array>
#include <cstring>
#include <string>
#include <vector>

namespace NF {

// ── WorkspaceAction ───────────────────────────────────────────────
// UI actions queued by render() to be handled by the main loop outside
// WM_PAINT so that modal dialogs can be opened safely.

enum class WorkspaceAction : uint8_t {
    None,
    NewProject,     // open folder-browser dialog, create and load a new project
    OpenProject,    // open file-open dialog for .atlas files
    Exit,           // request application exit
};

// ── WorkspacePendingError ─────────────────────────────────────────
// A title + message pair queued by render() when a launch fails or
// another recoverable error occurs that the user should see.

struct WorkspacePendingError {
    std::string title;
    std::string message;
    [[nodiscard]] bool empty() const { return title.empty(); }
};

class WorkspaceRenderer {
public:
    WorkspaceRenderer() { initTheme(); initMenus(); }

    // ── Layout constants (pixels) ─────────────────────────────────
    static constexpr float kTitleH   = 28.f;
    static constexpr float kToolbarH = 28.f;
    static constexpr float kStatusH  = 24.f;
    static constexpr float kSidebarW = 224.f;
    static constexpr float kContentY = kToolbarH;

    // ── Colors (RRGGBBAA) ─────────────────────────────────────────
    static constexpr uint32_t kBackground    = 0x1E1E1EFF;
    static constexpr uint32_t kSurface       = 0x252525FF;
    static constexpr uint32_t kBorder        = 0x3C3C3CFF;
    static constexpr uint32_t kAccentBlue    = 0x007ACCFF;
    static constexpr uint32_t kAccentDark    = 0x005A9EFF;
    static constexpr uint32_t kCardBg        = 0x2E2E2EFF;
    static constexpr uint32_t kButtonBg      = 0x3A3A3AFF;
    static constexpr uint32_t kTextPrimary   = 0xE0E0E0FF;
    static constexpr uint32_t kTextSecondary = 0x888888FF;
    static constexpr uint32_t kTextMuted     = 0x555555FF;
    static constexpr uint32_t kGreen         = 0x4EC94EFF;

    // ── render ────────────────────────────────────────────────────
    // Main entry point. Call once per frame from the platform render callback.
    // |mouse| must reflect the current frame's mouse position and button
    // edge-transitions (leftPressed / leftReleased) for clicks to register.
    // |launchSvc| is used to spawn child apps from the sidebar; pass nullptr
    // to disable launching (actions are still logged).
    void render(UIRenderer& ui, float width, float height,
                WorkspaceShell& shell, const UIMouseState& mouse,
                WorkspaceLaunchService* launchSvc = nullptr)
    {
        ui.beginFrame(width, height);
        renderBackground(ui, width, height);
        renderToolbar(ui, width, shell, mouse);
        renderSidebar(ui, height, shell, mouse, launchSvc);
        renderMainArea(ui, width, height, shell, mouse);
        renderStatusBar(ui, width, height, shell);
        renderDropdownOverlay(ui, mouse, shell);  // drawn on top of all other chrome
        ui.endFrame();
    }

    // ── Deferred action accessors ─────────────────────────────────
    // Call these from the main loop AFTER dispatching WM_PAINT.
    // Each call returns the pending value and resets it to its default.

    /// Returns any UI action queued during the last render() call and clears it.
    WorkspaceAction takePendingAction() {
        auto a = m_pendingAction;
        m_pendingAction = WorkspaceAction::None;
        return a;
    }

    /// Returns any error queued during the last render() call and clears it.
    WorkspacePendingError takePendingError() {
        auto e = m_pendingError;
        m_pendingError = {};
        return e;
    }

private:
    // ── Dropdown menu item ────────────────────────────────────────
    struct DropItem {
        std::string    label;                          // empty string = separator
        WorkspaceAction action  = WorkspaceAction::None;
        std::string    command;                        // command bus key (fallback)
        bool           enabled  = true;
    };
    struct DropMenu { std::vector<DropItem> items; };

    // ── UITheme matching workspace dark palette ────────────────────
    void initTheme() {
        m_wsTheme.hoverHighlight    = 0x3D3D3DFF;  // subtle card hover tint
        m_wsTheme.panelBackground   = kSurface;
        m_wsTheme.panelBorder       = kBorder;
        m_wsTheme.panelHeader       = kSurface;
        m_wsTheme.panelText         = kTextPrimary;
        m_wsTheme.buttonBackground  = kCardBg;
        m_wsTheme.buttonHover       = kButtonBg;
        m_wsTheme.buttonPressed     = kAccentBlue;
        m_wsTheme.buttonText        = kTextPrimary;
        m_wsTheme.itemSpacing       = 0.f;
    }

    void initMenus() {
        // File menu
        m_dropMenus[0].items = {
            {"New Project",  WorkspaceAction::NewProject,  "", true},
            {"Open Project", WorkspaceAction::OpenProject, "", true},
            {"", WorkspaceAction::None, "", false},  // separator
            {"Exit",         WorkspaceAction::Exit,        "", true},
        };
        // Project menu
        m_dropMenus[1].items = {
            {"Close Project",    WorkspaceAction::None, "workspace.project.close",    false},
            {"", WorkspaceAction::None, "", false},  // separator
            {"Project Settings", WorkspaceAction::None, "workspace.project.settings", false},
        };
        // Tools menu
        m_dropMenus[2].items = {
            {"Preferences",     WorkspaceAction::None, "workspace.preferences",      false},
            {"Command Palette", WorkspaceAction::None, "workspace.command_palette",   false},
            {"", WorkspaceAction::None, "", false},  // separator
            {"Diagnostics",     WorkspaceAction::None, "workspace.diagnostics",       false},
        };
        // View menu
        m_dropMenus[3].items = {
            {"Content Browser", WorkspaceAction::None, "workspace.view.content_browser", false},
            {"Inspector",       WorkspaceAction::None, "workspace.view.inspector",        false},
            {"Outliner",        WorkspaceAction::None, "workspace.view.outliner",         false},
            {"Console",         WorkspaceAction::None, "workspace.view.console",          false},
        };
        // Help menu
        m_dropMenus[4].items = {
            {"Documentation", WorkspaceAction::None, "workspace.help.docs",  false},
            {"About",         WorkspaceAction::None, "workspace.help.about", false},
        };
    }

    // ── Background ────────────────────────────────────────────────
    void renderBackground(UIRenderer& ui, float w, float h) {
        ui.drawRect({0.f, 0.f, w, h}, kBackground);
    }

    // ── Toolbar ───────────────────────────────────────────────────
    void renderToolbar(UIRenderer& ui, float width, WorkspaceShell& shell,
                       const UIMouseState& mouse)
    {
        float y = 0.f;
        ui.drawRect({0.f, y, width, kToolbarH}, kSurface);
        // Bottom separator
        ui.drawRect({0.f, y + kToolbarH - 1.f, width, 1.f}, kBorder);
        // Left accent stripe
        ui.drawRect({0.f, y, 3.f, kToolbarH}, kAccentBlue);

        static const char* kMenuItems[] = {"File", "Project", "Tools", "View", "Help"};

        // Begin widget pass for this frame's toolbar hit regions
        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);

        float mx = 10.f;
        for (size_t i = 0; i < 5; ++i) {
            const char* item = kMenuItems[i];
            float iw = static_cast<float>(std::strlen(item)) * 8.f + 16.f;
            Rect btnR{mx, y + 4.f, iw, 20.f};

            // Determine visual state: open > hovered > normal.
            // Non-hovered items use the toolbar surface color so they blend
            // in; hovered items lift to kButtonBg; the open item uses the
            // accent color to show which menu is active.
            bool menuOpen = (m_openMenuIdx == static_cast<int>(i));
            bool hovered  = btnR.contains(mouse.x, mouse.y);
            uint32_t btnBg = menuOpen ? kAccentBlue
                           : (hovered  ? kButtonBg : kSurface);
            ui.drawRect(btnR, btnBg);
            ui.drawText(mx + 6.f, y + 7.f, item,
                        menuOpen ? 0xFFFFFFFF : kTextPrimary);

            // Click: toggle the corresponding dropdown.
            if (m_ctx.hitRegion(btnR, false)) {
                NF_LOG_INFO("WorkspaceUI", std::string("Menu clicked: ") + item);
                m_openMenuIdx = (m_openMenuIdx == static_cast<int>(i))
                                ? -1 : static_cast<int>(i);
            }

            mx += iw + 4.f;
        }

        m_ctx.end();

        // Phase indicator aligned to the right of the toolbar row
        std::string phase = std::string("[") + shellPhaseName(shell.phase()) + "]";
        float phaseX = width - static_cast<float>(phase.size()) * 8.f - 12.f;
        if (phaseX > mx + 8.f)
            ui.drawText(phaseX, y + 7.f, phase, kTextSecondary);
    }

    // ── Left sidebar: tool launcher ───────────────────────────────
    void renderSidebar(UIRenderer& ui, float height, WorkspaceShell& shell,
                       const UIMouseState& mouse, WorkspaceLaunchService* launchSvc)
    {
        float y = kContentY;
        float h = height - y - kStatusH;

        ui.drawRect({0.f, y, kSidebarW, h}, kSurface);
        // Right border
        ui.drawRect({kSidebarW - 1.f, y, 1.f, h}, kBorder);

        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);
        float ay = y + 8.f;

        // ── Section 1: Registered Tools (in-process IHostedTool) ──────
        if (!shell.toolRegistry().empty()) {
            ui.drawText(8.f, ay, "TOOLS", kTextSecondary);
            ui.drawRect({8.f, ay + 14.f, kSidebarW - 16.f, 1.f}, kBorder);
            ay += 20.f;

            const std::string& activeId = shell.toolRegistry().activeToolId();
            for (const auto* desc : shell.toolRegistry().allDescriptors()) {
                if (ay + 34.f > y + h - 4.f) break;

                bool active  = (desc->toolId == activeId);
                Rect cardR{4.f, ay, kSidebarW - 8.f, 30.f};
                bool hovered = cardR.contains(mouse.x, mouse.y);

                // Background — active tint / hover tint / default
                uint32_t bg = active  ? kAccentDark
                            : (hovered ? m_wsTheme.hoverHighlight : 0x252525FF);
                ui.drawRect(cardR, bg);
                ui.drawRectOutline(cardR, active ? kAccentBlue : kBorder, 1.f);
                // Left accent stripe
                ui.drawRect({4.f, ay, 3.f, 30.f}, active ? kAccentBlue : 0x404040FF);

                // Tool label (truncate to sidebar width)
                std::string lbl = desc->displayName;
                if (lbl.size() > 16) lbl = lbl.substr(0, 13) + "...";
                ui.drawText(12.f, ay + 8.f, lbl, active ? kTextPrimary : kTextSecondary);

                // "✓" for active
                if (active)
                    ui.drawText(kSidebarW - 14.f, ay + 8.f, "*", kAccentBlue);

                // Click: activate when inactive, deactivate when active
                if (m_ctx.hitRegion(cardR, false)) {
                    if (active) {
                        NF_LOG_INFO("WorkspaceUI",
                            std::string("ActivityBar: deactivating ") + desc->toolId);
                        shell.toolRegistry().deactivateTool();
                    } else {
                        NF_LOG_INFO("WorkspaceUI",
                            std::string("ActivityBar: activating ") + desc->toolId);
                        shell.toolRegistry().activateTool(desc->toolId);
                    }
                }

                ay += 34.f;
            }

            // Separator between Tools and Apps sections
            ui.drawRect({8.f, ay + 4.f, kSidebarW - 16.f, 1.f}, kBorder);
            ay += 14.f;
        }

        // ── Section 2: Launch Apps (external executables) ─────────────
        ui.drawText(8.f, ay, "LAUNCH TOOL", kTextSecondary);
        ui.drawRect({8.f, ay + 14.f, kSidebarW - 16.f, 1.f}, kBorder);
        ay += 20.f;

        for (const auto& desc : shell.appRegistry().apps()) {
            if (ay + 46.f > y + h) break;

            Rect cardR{4.f, ay, kSidebarW - 8.f, 38.f};

            // Determine running state so we can show an indicator and
            // let a click on a running app stop it.
            bool running = launchSvc && launchSvc->isRunning(desc.id);

            // Draw card background with hover tint BEFORE text so the
            // highlight does not overwrite the card content.
            bool hovered = cardR.contains(mouse.x, mouse.y);
            ui.drawRect(cardR, hovered ? m_wsTheme.hoverHighlight : kCardBg);
            ui.drawRectOutline(cardR, running ? kGreen : kBorder, 1.f);

            // Running status dot (6x6 filled rect in top-right corner)
            if (running) {
                ui.drawRect({cardR.x + cardR.w - 10.f, ay + 4.f, 6.f, 6.f}, kGreen);
            }

            // App name
            ui.drawText(12.f, ay + 5.f, desc.name, kTextPrimary);

            // Executable path (truncate to fit); green when running
            std::string exePath = desc.executablePath;
            if (exePath.size() > 24) exePath = exePath.substr(0, 21) + "...";
            ui.drawText(12.f, ay + 20.f, exePath, running ? kGreen : kTextMuted);

            // Click: launch when stopped, stop when running.
            if (m_ctx.hitRegion(cardR, false)) {
                if (running && launchSvc) {
                    NF_LOG_INFO("WorkspaceUI",
                        std::string("Stopping: ") + desc.name);
                    launchSvc->shutdownApp(desc.id);
                    Notification n;
                    n.title   = std::string("Stopped: ") + desc.name;
                    n.message = desc.executablePath;
                    shell.shellContract().postNotification(n);
                } else {
                    NF_LOG_INFO("WorkspaceUI",
                        std::string("Launching: ") + desc.name
                        + "  (" + desc.executablePath + ")");

                    if (launchSvc) {
                        // Build launch context from the active project adapter so
                        // child executables receive the correct project path.
                        WorkspaceLaunchContext ctx;
                        if (shell.hasProject() && shell.projectAdapter()) {
                            const auto* adapter = shell.projectAdapter();
                            auto roots = adapter->contentRoots();
                            ctx.projectPath = roots.empty() ? "." : roots[0];
                            auto lastSepPos = ctx.projectPath.find_last_of("/\\");
                            ctx.workspaceRoot = (lastSepPos != std::string::npos)
                                                ? ctx.projectPath.substr(0, lastSepPos) : ".";
                        } else {
                            ctx.workspaceRoot = ".";
                            ctx.projectPath   = desc.isProjectScoped
                                                ? kStubProjectPath : ".";
                        }
                        ctx.sessionId = "workspace-session-" + desc.name
                                        + "-" + std::to_string(++m_launchCounter);
                        ctx.mode = WorkspaceLaunchMode::Hosted;
                        auto result = launchSvc->launchApp(desc, ctx);
                        if (result.succeeded()) {
                            NF_LOG_INFO("WorkspaceUI",
                                std::string("Launch OK: ") + desc.name
                                + "  pid=" + std::to_string(result.pid));
                            Notification n;
                            n.title   = std::string("Launched: ") + desc.name;
                            n.message = desc.executablePath;
                            shell.shellContract().postNotification(n);
                        } else {
                            NF_LOG_WARN("WorkspaceUI",
                                std::string("Launch failed: ") + desc.name
                                + "  status=" + workspaceLaunchStatusName(result.status));
                            if (m_pendingError.empty()) {
                                m_pendingError.title   = "Launch Failed: " + desc.name;
                                m_pendingError.message =
                                    "Could not launch " + desc.name + ".\n\n"
                                    + result.errorDetail + "\n\n"
                                    "Place the executable next to AtlasWorkspace.exe "
                                    "and try again.";
                            }
                        }
                    } else {
                        Notification n;
                        n.title   = std::string("Launch requested: ") + desc.name;
                        n.message = "No launch service wired";
                        shell.shellContract().postNotification(n);
                    }
                }
            }

            ay += 46.f;
        }

        m_ctx.end();

        if (shell.appRegistry().empty() && shell.toolRegistry().empty()) {
            ui.drawText(8.f, ay + 8.f, "(no tools registered)", kTextMuted);
        }
    }

    // ── Main area: welcome, project dashboard, or active tool view ──
    void renderMainArea(UIRenderer& ui, float width, float height,
                        WorkspaceShell& shell, const UIMouseState& mouse)
    {
        float x = kSidebarW;
        float y = kContentY;
        float w = width - x;
        float h = height - y - kStatusH;

        ui.drawRect({x, y, w, h}, kBackground);

        if (!shell.hasProject()) {
            renderWelcomeScreen(ui, x, y, w, h, mouse);
        } else if (shell.toolRegistry().activeTool() != nullptr) {
            renderActiveToolView(ui, x, y, w, h, shell, mouse);
        } else {
            renderProjectDashboard(ui, x, y, w, h, shell, mouse);
        }
    }

    void renderWelcomeScreen(UIRenderer& ui, float x, float y, float w, float h,
                             const UIMouseState& mouse)
    {
        float cx = x + w * 0.5f;
        float cy = y + h * 0.5f;

        // Accent line above heading
        float lineW = 120.f;
        ui.drawRect({cx - lineW * 0.5f, cy - 54.f, lineW, 3.f}, kAccentBlue);

        // Heading and subheading
        ui.drawText(cx - 64.f, cy - 44.f, "Atlas Workspace", kTextPrimary);
        ui.drawText(cx - 88.f, cy - 26.f, "Open or create a project", kTextSecondary);
        ui.drawText(cx - 60.f, cy - 10.f, "to get started.", kTextMuted);

        // Action cards
        float cardW = 160.f;
        float cardH = 46.f;
        float cardY = cy + 12.f;
        float card1X = cx - cardW - 8.f;
        float card2X = cx + 8.f;

        Rect newProjR{card1X, cardY, cardW, cardH};
        Rect openProjR{card2X, cardY, cardW, cardH};

        // Interactive layer for welcome cards — begin before drawing so hover
        // state can be used to tint the background before the text is drawn.
        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);

        // New Project card
        bool newHovered  = newProjR.contains(mouse.x, mouse.y);
        ui.drawRect(newProjR,  newHovered  ? m_wsTheme.hoverHighlight : kCardBg);
        ui.drawRectOutline(newProjR, kBorder, 1.f);
        ui.drawRect({card1X, cardY, cardW, 3.f}, kAccentBlue);
        ui.drawText(card1X + 10.f, cardY + 9.f,  "New Project",         kTextPrimary);
        ui.drawText(card1X + 10.f, cardY + 26.f, "File > New Project",  kTextMuted);

        // Open Project card
        bool openHovered = openProjR.contains(mouse.x, mouse.y);
        ui.drawRect(openProjR, openHovered ? m_wsTheme.hoverHighlight : kCardBg);
        ui.drawRectOutline(openProjR, kBorder, 1.f);
        ui.drawRect({card2X, cardY, cardW, 3.f}, kButtonBg);
        ui.drawText(card2X + 10.f, cardY + 9.f,  "Open Project",        kTextPrimary);
        ui.drawText(card2X + 10.f, cardY + 26.f, "File > Open Project", kTextMuted);

        if (m_ctx.hitRegion(newProjR, false)) {
            NF_LOG_INFO("WorkspaceUI", "New Project clicked");
            // Queue the action; the main loop opens the folder dialog outside WM_PAINT.
            m_pendingAction = WorkspaceAction::NewProject;
        }
        if (m_ctx.hitRegion(openProjR, false)) {
            NF_LOG_INFO("WorkspaceUI", "Open Project clicked");
            // Queue the action; the main loop opens the file-open dialog outside WM_PAINT.
            m_pendingAction = WorkspaceAction::OpenProject;
        }

        m_ctx.end();
    }

    void renderProjectDashboard(UIRenderer& ui, float x, float y, float w, float h,
                                WorkspaceShell& shell, const UIMouseState& mouse)
    {
        (void)h;
        float rx = x + 16.f;
        float ry = y + 16.f;
        const auto* adapter = shell.projectAdapter();

        // Project name + id
        ui.drawText(rx, ry, "Project: " + adapter->projectDisplayName(), kTextPrimary);
        ry += 18.f;
        ui.drawText(rx, ry, "ID: " + adapter->projectId(), kTextSecondary);
        ry += 28.f;

        // Stat cards
        auto drawStatCard = [&](float cx2, float cy2, const std::string& label,
                                const std::string& value) {
            float cw = 140.f, ch = 52.f;
            ui.drawRect({cx2, cy2, cw, ch}, kCardBg);
            ui.drawRectOutline({cx2, cy2, cw, ch}, kBorder, 1.f);
            ui.drawText(cx2 + 8.f, cy2 + 7.f,  label, kTextSecondary);
            ui.drawText(cx2 + 8.f, cy2 + 26.f, value, kTextPrimary);
        };

        drawStatCard(rx,          ry, "Tools",  std::to_string(shell.toolRegistry().count()));
        drawStatCard(rx + 148.f,  ry, "Panels", std::to_string(shell.panelRegistry().count()));
        drawStatCard(rx + 296.f,  ry, "Apps",   std::to_string(shell.appRegistry().count()));
        ry += 68.f;

        // Registered tools list — clickable rows to activate each tool
        ui.drawText(rx, ry, "Registered Tools", kTextSecondary);
        ui.drawText(rx + w - 120.f, ry, "Click a tool to open it", kTextMuted);
        ui.drawRect({rx, ry + 16.f, w - 32.f, 1.f}, kBorder);
        ry += 22.f;

        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);

        const std::string& activeId = shell.toolRegistry().activeToolId();
        for (const auto* desc : shell.toolRegistry().allDescriptors()) {
            float rowH = 28.f;
            float rowW = w - 32.f;
            Rect  rowR = {rx, ry, rowW, rowH};
            bool  active  = (desc->toolId == activeId);
            bool  hovered = rowR.contains(mouse.x, mouse.y);

            uint32_t bg = active  ? kAccentDark
                        : (hovered ? m_wsTheme.hoverHighlight : 0x272727FF);
            ui.drawRect(rowR, bg);
            ui.drawRectOutline(rowR, kBorder, 1.f);
            // Left accent stripe indicates active tool
            ui.drawRect({rx, ry, 3.f, rowH}, active ? kAccentBlue : 0x444444FF);
            ui.drawText(rx + 10.f, ry + 7.f,
                        desc->displayName + "  (" + desc->toolId + ")", kTextPrimary);
            if (!active)
                ui.drawText(rx + rowW - 58.f, ry + 7.f, "Open >", kTextSecondary);

            if (m_ctx.hitRegion(rowR, false)) {
                NF_LOG_INFO("WorkspaceUI",
                    std::string("Activating tool: ") + desc->toolId);
                shell.toolRegistry().activateTool(desc->toolId);
            }

            ry += rowH + 2.f;
        }

        m_ctx.end();

        if (shell.toolRegistry().empty()) {
            ui.drawText(rx, ry + 4.f, "(no tools registered)", kTextMuted);
        }
    }

    // ── Dropdown overlay ──────────────────────────────────────────
    // Rendered after all other chrome so the open dropdown floats on top.
    // Also handles close-on-click-outside and item selection.
    void renderDropdownOverlay(UIRenderer& ui, const UIMouseState& mouse,
                               WorkspaceShell& shell)
    {
        if (m_openMenuIdx < 0 || m_openMenuIdx >= static_cast<int>(m_dropMenus.size()))
            return;

        const DropMenu& menu = m_dropMenus[m_openMenuIdx];

        // Compute X origin by summing widths of preceding menu labels.
        static const char* kMenuItems[] = {"File", "Project", "Tools", "View", "Help"};
        float dx = 10.f;
        for (int j = 0; j < m_openMenuIdx; ++j) {
            float jw = static_cast<float>(std::strlen(kMenuItems[j])) * 8.f + 16.f;
            dx += jw + 4.f;
        }
        float dy = kToolbarH;  // dropdown opens just below the toolbar

        // Compute dropdown dimensions
        float dw = 180.f;
        for (auto& it : menu.items) {
            if (!it.label.empty()) {
                float iw = static_cast<float>(it.label.size()) * 8.f + 24.f;
                if (iw > dw) dw = iw;
            }
        }
        float dh = 6.f;
        for (auto& it : menu.items)
            dh += it.label.empty() ? 9.f : 22.f;

        // Draw background and border
        ui.drawRect({dx, dy, dw, dh}, kSurface);
        ui.drawRectOutline({dx, dy, dw, dh}, kBorder, 1.f);

        // Draw items and handle clicks
        float iy = dy + 4.f;
        bool itemConsumedClick = false;
        for (auto& it : menu.items) {
            if (it.label.empty()) {
                // Separator line
                ui.drawRect({dx + 8.f, iy + 3.f, dw - 16.f, 1.f}, kBorder);
                iy += 9.f;
            } else {
                Rect itemR{dx + 2.f, iy, dw - 4.f, 20.f};
                bool iHover = itemR.contains(mouse.x, mouse.y);
                if (iHover && it.enabled)
                    ui.drawRect(itemR, kButtonBg);

                uint32_t textCol = it.enabled ? kTextPrimary : kTextMuted;
                ui.drawText(dx + 10.f, iy + 3.f, it.label, textCol);

                if (iHover && it.enabled && mouse.leftPressed) {
                    itemConsumedClick = true;
                    if (it.action != WorkspaceAction::None) {
                        m_pendingAction = it.action;
                    } else if (!it.command.empty()) {
                        (void)shell.commandBus().execute(it.command);
                    }
                    m_openMenuIdx = -1;
                }
                iy += 22.f;
            }
        }

        // Close the dropdown when the user clicks outside it (and outside the toolbar).
        if (!itemConsumedClick && mouse.leftPressed) {
            bool inDropdown = (mouse.x >= dx && mouse.x < dx + dw &&
                               mouse.y >= dy && mouse.y < dy + dh);
            bool inToolbar  = (mouse.y >= 0.f && mouse.y < kToolbarH);
            if (!inDropdown && !inToolbar)
                m_openMenuIdx = -1;
        }
    }

    // ── Status bar ────────────────────────────────────────────────
    void renderStatusBar(UIRenderer& ui, float width, float height,
                         const WorkspaceShell& shell)
    {
        float y = height - kStatusH;
        ui.drawRect({0.f, y, width, kStatusH}, kAccentBlue);

        // Left: workspace phase
        std::string left = std::string("  ") + shellPhaseName(shell.phase());
        ui.drawText(8.f, y + 5.f, left, 0xFFFFFFFF);

        // Right: counts
        std::string right =
            "Tools: "  + std::to_string(shell.toolRegistry().count())  +
            "  Panels: " + std::to_string(shell.panelRegistry().count()) +
            "  Apps: "   + std::to_string(shell.appRegistry().count());
        float rw = static_cast<float>(right.size()) * 8.f;
        ui.drawText(width - rw - 8.f, y + 5.f, right, 0xFFFFFFFF);
    }

    // ── Active tool editor view ───────────────────────────────────
    // Shown in the main area when a tool has been activated from the dashboard.
    // Displays a per-tool panel layout, a header bar with a "back" button,
    // and a bottom strip with Console + Metrics panels.
    void renderActiveToolView(UIRenderer& ui, float x, float y, float w, float h,
                              WorkspaceShell& shell, const UIMouseState& mouse)
    {
        const IHostedTool* tool = shell.toolRegistry().activeTool();
        if (!tool) return;
        const auto& desc = tool->descriptor();

        static constexpr float kHeaderH = 38.f;
        static constexpr float kBottomH = 130.f;
        const float kMainH = h - kHeaderH - kBottomH;

        // ── Tool header bar ────────────────────────────────────────
        ui.drawRect({x, y, w, kHeaderH}, kSurface);
        ui.drawRect({x, y + kHeaderH - 1.f, w, 1.f}, kBorder);
        ui.drawRect({x, y, 3.f, kHeaderH}, kAccentBlue);

        ui.drawText(x + 14.f, y + 11.f, desc.displayName, kTextPrimary);

        // Project breadcrumb
        if (shell.hasProject() && shell.projectAdapter()) {
            float nameW = static_cast<float>(desc.displayName.size()) * 8.f + 14.f;
            std::string crumb = "  /  " + shell.projectAdapter()->projectDisplayName();
            ui.drawText(x + nameW, y + 11.f, crumb, kTextSecondary);
        }

        // "< Dashboard" back button
        static constexpr float kBackW = 96.f;
        Rect backR{x + w - kBackW - 8.f, y + (kHeaderH - 22.f) * 0.5f, kBackW, 22.f};
        bool backHovered = backR.contains(mouse.x, mouse.y);
        ui.drawRect(backR, backHovered ? kButtonBg : kCardBg);
        ui.drawRectOutline(backR, kBorder, 1.f);
        ui.drawText(backR.x + 8.f, backR.y + 4.f, "< Dashboard", kTextSecondary);

        // ── Per-tool panel layout ──────────────────────────────────
        renderToolPanelsForCategory(ui, x, y + kHeaderH, w, kMainH, shell, desc);

        // ── Bottom strip: Console + Metrics ───────────────────────
        renderBottomStrip(ui, x, y + kHeaderH + kMainH, w, kBottomH, shell);

        // ── Click handling ─────────────────────────────────────────
        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);
        if (m_ctx.hitRegion(backR, false)) {
            NF_LOG_INFO("WorkspaceUI", "Tool deactivated: back to dashboard");
            shell.toolRegistry().deactivateTool();
        }
        m_ctx.end();
    }

    // ── Per-category panel layout ─────────────────────────────────
    // Dispatches to a tool-appropriate multi-panel arrangement.
    // All panels are chrome shells; actual content is handled by tool logic.
    void renderToolPanelsForCategory(UIRenderer& ui, float x, float y, float w, float h,
                                     const WorkspaceShell& shell,
                                     const HostedToolDescriptor& desc)
    {
        // Helper: draw a labeled panel zone with an optional centered hint.
        auto drawPanel = [&](float px, float py, float pw, float ph,
                             const char* title, const char* hint = nullptr) {
            ui.drawRect({px, py, pw, ph}, kCardBg);
            ui.drawRectOutline({px, py, pw, ph}, kBorder, 1.f);
            ui.drawRect({px, py, pw, 22.f}, kSurface);
            ui.drawRect({px, py + 22.f, pw, 1.f}, kBorder);
            ui.drawText(px + 8.f, py + 4.f, title, kTextSecondary);
            if (hint) {
                // Use the same 8px-per-glyph estimate as the rest of the renderer for centering.
                float hx = px + (pw - static_cast<float>(std::strlen(hint)) * 8.f) * 0.5f;
                float hy = py + 22.f + (ph - 22.f - 14.f) * 0.5f;
                ui.drawText(hx, hy, hint, kTextMuted);
            }
        };

        // Content-root hint shown at the bottom of the main viewport/panel.
        std::string contentHint;
        if (shell.hasProject() && shell.projectAdapter()) {
            auto roots = shell.projectAdapter()->contentRoots();
            if (!roots.empty()) {
                contentHint = "Content: " + roots[0];
                if (contentHint.size() > 50)
                    contentHint = contentHint.substr(0, 47) + "...";
            }
        }

        switch (desc.category) {

        case HostedToolCategory::SceneEditing: {
            // Hierarchy | 3D Viewport | Inspector
            float hierW  = w * 0.20f;
            float inspW  = w * 0.22f;
            float viewW  = w - hierW - inspW;
            drawPanel(x,              y, hierW, h, "HIERARCHY", "(entity tree)");
            // Viewport
            ui.drawRect({x + hierW, y, viewW, h}, 0x1A1A1AFF);
            ui.drawRectOutline({x + hierW, y, viewW, h}, kBorder, 1.f);
            ui.drawRect({x + hierW, y, viewW, 22.f}, kSurface);
            ui.drawRect({x + hierW, y + 22.f, viewW, 1.f}, kBorder);
            ui.drawText(x + hierW + 8.f, y + 4.f, "VIEWPORT", kTextSecondary);
            // Grid lines
            for (float gx = x + hierW; gx < x + hierW + viewW; gx += 38.f)
                ui.drawRect({gx, y + 22.f, 1.f, h - 22.f}, 0x2D2D2DFF);
            for (float gy = y + 22.f; gy < y + h; gy += 38.f)
                ui.drawRect({x + hierW, gy, viewW, 1.f}, 0x2D2D2DFF);
            ui.drawText(x + hierW + (viewW - 80.f) * 0.5f,
                        y + 22.f + (h - 22.f - 14.f) * 0.5f,
                        "[ 3D Scene ]", kTextMuted);
            if (!contentHint.empty())
                ui.drawText(x + hierW + 8.f, y + h - 18.f, contentHint, kTextMuted);
            drawPanel(x + hierW + viewW, y, inspW, h, "INSPECTOR", "(properties)");
            break;
        }

        case HostedToolCategory::AssetAuthoring: {
            // Content Browser | Inspector
            float cbW   = w * 0.65f;
            float inspW = w - cbW;
            ui.drawRect({x, y, cbW, h}, kCardBg);
            ui.drawRectOutline({x, y, cbW, h}, kBorder, 1.f);
            ui.drawRect({x, y, cbW, 22.f}, kSurface);
            ui.drawRect({x, y + 22.f, cbW, 1.f}, kBorder);
            ui.drawText(x + 8.f, y + 4.f, "CONTENT BROWSER", kTextSecondary);
            // Asset tile placeholders
            static const char* kTypes[] = {"Mesh","Texture","Material","Script","Prefab","Audio"};
            float ax = x + 8.f, ay = y + 30.f;
            for (int i = 0; i < 6 && ax + 62.f < x + cbW - 4.f; ++i) {
                ui.drawRect({ax, ay, 60.f, 58.f}, 0x333333FF);
                ui.drawRectOutline({ax, ay, 60.f, 58.f}, kBorder, 1.f);
                ui.drawText(ax + 6.f, ay + 22.f, kTypes[i], kTextMuted);
                ax += 68.f;
            }
            if (!contentHint.empty())
                ui.drawText(x + 8.f, y + h - 18.f, contentHint, kTextMuted);
            drawPanel(x + cbW, y, inspW, h, "INSPECTOR", "(asset preview)");
            break;
        }

        case HostedToolCategory::AnimationAuthoring: {
            // Animation Preview | Sequencer / Timeline
            float viewH = h * 0.55f;
            float timeH = h - viewH;
            drawPanel(x, y,          w, viewH, "ANIMATION PREVIEW", "[ 3D Preview ]");
            drawPanel(x, y + viewH,  w, timeH, "SEQUENCER / TIMELINE", "(keyframes)");
            if (!contentHint.empty())
                ui.drawText(x + 8.f, y + h - 18.f, contentHint, kTextMuted);
            break;
        }

        case HostedToolCategory::DataEditing: {
            // Data Table | Inspector
            float tableW = w * 0.70f;
            float inspW  = w - tableW;
            ui.drawRect({x, y, tableW, h}, kCardBg);
            ui.drawRectOutline({x, y, tableW, h}, kBorder, 1.f);
            ui.drawRect({x, y, tableW, 22.f}, kSurface);
            ui.drawRect({x, y + 22.f, tableW, 1.f}, kBorder);
            ui.drawText(x + 8.f, y + 4.f, "DATA TABLE", kTextSecondary);
            // Table header row
            ui.drawRect({x, y + 22.f, tableW, 22.f}, 0x303030FF);
            ui.drawText(x + 8.f,   y + 27.f, "ID",    kTextSecondary);
            ui.drawText(x + 80.f,  y + 27.f, "Name",  kTextSecondary);
            ui.drawText(x + 200.f, y + 27.f, "Value", kTextSecondary);
            ui.drawRect({x, y + 44.f, tableW, 1.f}, kBorder);
            if (!contentHint.empty())
                ui.drawText(x + 8.f, y + h - 18.f, contentHint, kTextMuted);
            drawPanel(x + tableW, y, inspW, h, "INSPECTOR", "(row details)");
            break;
        }

        case HostedToolCategory::LogicAuthoring: {
            // Full-width node graph canvas
            ui.drawRect({x, y, w, h}, 0x1C1C1CFF);
            ui.drawRectOutline({x, y, w, h}, kBorder, 1.f);
            ui.drawRect({x, y, w, 22.f}, kSurface);
            ui.drawRect({x, y + 22.f, w, 1.f}, kBorder);
            ui.drawText(x + 8.f, y + 4.f, "VISUAL LOGIC GRAPH", kTextSecondary);
            // Node placeholders
            auto drawNode = [&](float nx, float ny, const char* lbl) {
                ui.drawRect({nx, ny, 100.f, 44.f}, kCardBg);
                ui.drawRectOutline({nx, ny, 100.f, 44.f}, kBorder, 1.f);
                ui.drawRect({nx, ny, 100.f, 20.f}, kAccentDark);
                ui.drawText(nx + 6.f, ny + 4.f, lbl, kTextPrimary);
                ui.drawRect({nx - 5.f,  ny + 28.f, 8.f, 8.f}, kAccentBlue);
                ui.drawRect({nx + 97.f, ny + 28.f, 8.f, 8.f}, kAccentBlue);
            };
            float nx = x + w * 0.12f, ny = y + h * 0.30f;
            drawNode(nx,         ny,       "On Start");
            drawNode(nx + 160.f, ny - 28.f, "Branch");
            drawNode(nx + 320.f, ny - 56.f, "Set Variable");
            drawNode(nx + 320.f, ny + 22.f, "Spawn Entity");
            // Connecting wires
            ui.drawRect({nx + 105.f, ny + 32.f,       55.f, 1.f}, kTextMuted);
            ui.drawRect({nx + 265.f, ny + 6.f,         55.f, 1.f}, kTextMuted);
            ui.drawRect({nx + 265.f, ny + 50.f,        55.f, 1.f}, kTextMuted);
            if (!contentHint.empty())
                ui.drawText(x + 8.f, y + h - 18.f, contentHint, kTextMuted);
            break;
        }

        case HostedToolCategory::BuildPackaging: {
            // Build output log | Build status
            float logW    = w * 0.72f;
            float statusW = w - logW;
            ui.drawRect({x, y, logW, h}, 0x161616FF);
            ui.drawRectOutline({x, y, logW, h}, kBorder, 1.f);
            ui.drawRect({x, y, logW, 22.f}, kSurface);
            ui.drawRect({x, y + 22.f, logW, 1.f}, kBorder);
            ui.drawText(x + 8.f, y + 4.f, "BUILD OUTPUT", kTextSecondary);
            static const char* kBuildLines[] = {
                "[INFO]  Build system ready",
                "[INFO]  Project: awaiting configuration",
                "[INFO]  Target: Windows x64",
                "[INFO]  ---",
            };
            float ly = y + 28.f;
            for (auto* line : kBuildLines) {
                ui.drawText(x + 8.f, ly, line, kTextSecondary);
                ly += 18.f;
            }
            if (!contentHint.empty())
                ui.drawText(x + 8.f, y + h - 18.f, contentHint, kTextMuted);
            drawPanel(x + logW, y, statusW, h, "BUILD STATUS", "Ready");
            break;
        }

        case HostedToolCategory::AIAssistant: {
            // AI Chat | Context / Tools
            float chatW = w * 0.68f;
            float ctxW  = w - chatW;
            ui.drawRect({x, y, chatW, h}, 0x1A1A2AFF);
            ui.drawRectOutline({x, y, chatW, h}, kBorder, 1.f);
            ui.drawRect({x, y, chatW, 22.f}, kSurface);
            ui.drawRect({x, y + 22.f, chatW, 1.f}, kBorder);
            ui.drawText(x + 8.f, y + 4.f, "ATLAS AI CHAT", kAccentBlue);
            // Chat message stubs
            float cy = y + 30.f;
            ui.drawRect({x + 8.f, cy, chatW - 16.f, 28.f}, 0x252535FF);
            ui.drawText(x + 14.f, cy + 7.f,
                        "AtlasAI: Ready. Load a project to begin.", kTextSecondary);
            cy += 36.f;
            if (shell.hasProject() && shell.projectAdapter()) {
                std::string pmsg = "AtlasAI: Project '"
                    + shell.projectAdapter()->projectDisplayName()
                    + "' context loaded.";
                ui.drawRect({x + 8.f, cy, chatW - 16.f, 28.f}, 0x252535FF);
                ui.drawText(x + 14.f, cy + 7.f, pmsg, kGreen);
                cy += 36.f;
            }
            // Input box placeholder
            ui.drawRect({x + 4.f, y + h - 32.f, chatW - 8.f, 26.f}, 0x2A2A3AFF);
            ui.drawRectOutline({x + 4.f, y + h - 32.f, chatW - 8.f, 26.f}, kAccentBlue, 1.f);
            ui.drawText(x + 10.f, y + h - 24.f, "Type a message...", kTextMuted);
            // Context panel
            drawPanel(x + chatW, y, ctxW, h, "CONTEXT / TOOLS", nullptr);
            float rly = y + 28.f;
            ui.drawText(x + chatW + 8.f, rly, "Content Roots:", kTextSecondary);
            rly += 18.f;
            if (shell.hasProject() && shell.projectAdapter()) {
                for (const auto& root : shell.projectAdapter()->contentRoots()) {
                    std::string rstr = root;
                    if (rstr.size() > 26) rstr = "..." + rstr.substr(rstr.size() - 23);
                    ui.drawText(x + chatW + 12.f, rly, rstr, kTextMuted);
                    rly += 16.f;
                }
            } else {
                ui.drawText(x + chatW + 12.f, rly, "(no project)", kTextMuted);
            }
            break;
        }

        default: {
            // Generic single panel for unhandled categories
            drawPanel(x, y, w, h, desc.displayName.c_str(),
                      "Select a tool to configure");
            if (!contentHint.empty())
                ui.drawText(x + 8.f, y + h - 18.f, contentHint, kTextMuted);
            break;
        }
        }
    }

    // ── Bottom strip: Console + Metrics ──────────────────────────
    // Rendered below the per-tool panel layout in the active tool view.
    // Console shows project-context log lines; Metrics shows live shell stats
    // and the most recent workspace notification.
    void renderBottomStrip(UIRenderer& ui, float x, float y, float w, float h,
                           const WorkspaceShell& shell)
    {
        float consoleW  = w * 0.65f;
        float metricsW  = w - consoleW;

        // ── Console ────────────────────────────────────────────────
        ui.drawRect({x, y, consoleW, h}, 0x161616FF);
        ui.drawRectOutline({x, y, consoleW, h}, kBorder, 1.f);
        ui.drawRect({x, y, consoleW, 22.f}, kSurface);
        ui.drawRect({x, y + 22.f, consoleW, 1.f}, kBorder);
        ui.drawText(x + 8.f, y + 4.f, "CONSOLE", kTextSecondary);

        float cy = y + 28.f;
        if (shell.hasProject() && shell.projectAdapter()) {
            std::string projMsg = "[INFO]  Project: "
                + shell.projectAdapter()->projectDisplayName()
                + "  (id=" + shell.projectAdapter()->projectId() + ")";
            ui.drawText(x + 8.f, cy, projMsg, kGreen);
            cy += 18.f;
            for (const auto& root : shell.projectAdapter()->contentRoots()) {
                std::string rootMsg = "[INFO]  Content root: " + root;
                if (rootMsg.size() > 72) rootMsg = rootMsg.substr(0, 69) + "...";
                ui.drawText(x + 8.f, cy, rootMsg, kTextSecondary);
                cy += 16.f;
                if (cy + 16.f > y + h - 4.f) break;
            }
        } else {
            ui.drawText(x + 8.f, cy, "[INFO]  No project loaded", kTextMuted);
            cy += 18.f;
        }
        if (const auto* tool = shell.toolRegistry().activeTool()) {
            if (cy + 14.f < y + h - 4.f) {
                std::string toolMsg = "[INFO]  Active tool: "
                    + tool->descriptor().displayName
                    + "  (" + tool->toolId() + ")";
                ui.drawText(x + 8.f, cy, toolMsg, kTextSecondary);
            }
        }

        // ── Metrics ────────────────────────────────────────────────
        float mx = x + consoleW;
        ui.drawRect({mx, y, metricsW, h}, kCardBg);
        ui.drawRectOutline({mx, y, metricsW, h}, kBorder, 1.f);
        ui.drawRect({mx, y, metricsW, 22.f}, kSurface);
        ui.drawRect({mx, y + 22.f, metricsW, 1.f}, kBorder);
        ui.drawText(mx + 8.f, y + 4.f, "METRICS", kTextSecondary);

        float mcy = y + 28.f;
        ui.drawText(mx + 8.f, mcy,
            std::string("Workspace: ") + shellPhaseName(shell.phase()),
            kTextSecondary);
        mcy += 18.f;
        ui.drawText(mx + 8.f, mcy,
            "Tools: "  + std::to_string(shell.toolRegistry().count()),  kTextSecondary);
        mcy += 16.f;
        ui.drawText(mx + 8.f, mcy,
            "Panels: " + std::to_string(shell.panelRegistry().count()), kTextSecondary);
        mcy += 16.f;
        ui.drawText(mx + 8.f, mcy,
            "Apps: "   + std::to_string(shell.appRegistry().count()),   kTextSecondary);
        mcy += 16.f;
        if (const auto* tool = shell.toolRegistry().activeTool()) {
            ui.drawText(mx + 8.f, mcy,
                std::string("State: ") + hostedToolStateName(tool->state()), kGreen);
            mcy += 16.f;
        }
        // Most recent workspace notification
        const auto& notifs = shell.shellContract().recentNotifications();
        if (!notifs.empty() && mcy + 14.f < y + h - 4.f) {
            const auto& last = notifs.back();
            std::string nstr = "[!] " + last.title;
            if (nstr.size() > 28) nstr = nstr.substr(0, 25) + "...";
            ui.drawText(mx + 8.f, mcy, nstr, kAccentBlue);
        }
    }

    // ── Per-instance state ────────────────────────────────────────
    UIContext m_ctx;      // maintains scroll state across frames
    UITheme   m_wsTheme;  // workspace-palette theme for UIContext

    // Monotonically increasing counter used to build unique session IDs.
    // Each sidebar launch increments this so repeated launches of the same
    // app produce distinct session identifiers.
    uint32_t m_launchCounter = 0;

    // Deferred action queued during render() for the main loop to handle.
    // Only one action can be pending at a time (last write wins within a frame).
    WorkspaceAction      m_pendingAction = WorkspaceAction::None;
    WorkspacePendingError m_pendingError;

    // Dropdown menu data — one entry per menu label (File/Project/Tools/View/Help).
    std::array<DropMenu, 5> m_dropMenus;
    int m_openMenuIdx = -1;  // index of the currently open dropdown, -1 = none

    // Fallback project path for project-scoped apps when no project is loaded.
    static constexpr const char* kStubProjectPath = "stub.atlas";
};

} // namespace NF
