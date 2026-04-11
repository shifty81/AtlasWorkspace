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

        // Section header
        ui.drawText(8.f, y + 8.f, "LAUNCH TOOL", kTextSecondary);
        ui.drawRect({8.f, y + 22.f, kSidebarW - 16.f, 1.f}, kBorder);

        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);

        float ay = y + 30.f;
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
                            auto sep = ctx.projectPath.find_last_of("/\\");
                            ctx.workspaceRoot = (sep != std::string::npos)
                                                ? ctx.projectPath.substr(0, sep) : ".";
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

        if (shell.appRegistry().empty()) {
            ui.drawText(8.f, ay + 8.f, "(no apps registered)", kTextMuted);
        }
    }

    // ── Main area: welcome or project dashboard ───────────────────
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
        } else {
            renderProjectDashboard(ui, x, y, w, h, shell);
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
                                const WorkspaceShell& shell)
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

        // Registered tools list
        ui.drawText(rx, ry, "Registered Tools", kTextSecondary);
        ui.drawRect({rx, ry + 16.f, w - 32.f, 1.f}, kBorder);
        ry += 22.f;

        for (const auto* desc : shell.toolRegistry().allDescriptors()) {
            float rowH = 26.f;
            ui.drawRect({rx, ry, w - 32.f, rowH}, 0x272727FF);
            ui.drawRectOutline({rx, ry, w - 32.f, rowH}, kBorder, 1.f);
            ui.drawText(rx + 8.f, ry + 6.f, desc->displayName + "  (" + desc->toolId + ")",
                        kTextPrimary);
            ry += rowH + 2.f;
        }

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
