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
// WorkspaceRenderer is stateless: it reads only from WorkspaceShell and writes
// only to UIRenderer. No state is cached between frames.
//
// Usage (main loop):
//
//   NF::WorkspaceRenderer wsRenderer;
//   while (running) {
//       ui.beginFrame(w, h);          // called internally by render()
//       wsRenderer.render(ui, w, h, shell);
//       // ui.endFrame() is called by render() before returning
//   }
//
// Color format: RRGGBBAA (matches UIRenderer / GDIBackend convention).

#include "NF/UI/UI.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <cstring>
#include <string>

namespace NF {

class WorkspaceRenderer {
public:
    // ── Layout constants (pixels) ─────────────────────────────────
    static constexpr float kTitleH   = 28.f;
    static constexpr float kToolbarH = 28.f;
    static constexpr float kStatusH  = 24.f;
    static constexpr float kSidebarW = 224.f;
    static constexpr float kContentY = kTitleH + kToolbarH;

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
    void render(UIRenderer& ui, float width, float height,
                const WorkspaceShell& shell)
    {
        ui.beginFrame(width, height);
        renderBackground(ui, width, height);
        renderTitleBar(ui, width, shell);
        renderToolbar(ui, width);
        renderSidebar(ui, height, shell);
        renderMainArea(ui, width, height, shell);
        renderStatusBar(ui, width, height, shell);
        ui.endFrame();
    }

private:
    // ── Background ────────────────────────────────────────────────
    void renderBackground(UIRenderer& ui, float w, float h) {
        ui.drawRect({0.f, 0.f, w, h}, kBackground);
    }

    // ── Title bar ─────────────────────────────────────────────────
    void renderTitleBar(UIRenderer& ui, float width, const WorkspaceShell& shell) {
        ui.drawRect({0.f, 0.f, width, kTitleH}, kSurface);
        // Left accent stripe
        ui.drawRect({0.f, 0.f, 3.f, kTitleH}, kAccentBlue);

        // Title + optional project name
        std::string title = "Atlas Workspace";
        if (shell.hasProject() && shell.projectAdapter())
            title += "  -  " + shell.projectAdapter()->projectDisplayName();
        ui.drawText(12.f, 8.f, title, kTextPrimary);

        // Phase indicator aligned to the right
        std::string phase = std::string("[") + shellPhaseName(shell.phase()) + "]";
        float phaseX = width - static_cast<float>(phase.size()) * 8.f - 12.f;
        if (phaseX > width * 0.5f)
            ui.drawText(phaseX, 8.f, phase, kTextSecondary);
    }

    // ── Toolbar ───────────────────────────────────────────────────
    void renderToolbar(UIRenderer& ui, float width) {
        float y = kTitleH;
        ui.drawRect({0.f, y, width, kToolbarH}, kSurface);
        // Bottom separator
        ui.drawRect({0.f, y + kToolbarH - 1.f, width, 1.f}, kBorder);

        static const char* kMenuItems[] = {"File", "Project", "Tools", "View", "Help"};
        float mx = 8.f;
        for (const char* item : kMenuItems) {
            float iw = static_cast<float>(std::strlen(item)) * 8.f + 16.f;
            ui.drawRect({mx, y + 4.f, iw, 20.f}, kButtonBg);
            ui.drawText(mx + 6.f, y + 7.f, item, kTextPrimary);
            mx += iw + 4.f;
        }
    }

    // ── Left sidebar: tool launcher ───────────────────────────────
    void renderSidebar(UIRenderer& ui, float height, const WorkspaceShell& shell) {
        float y = kContentY;
        float h = height - y - kStatusH;

        ui.drawRect({0.f, y, kSidebarW, h}, kSurface);
        // Right border
        ui.drawRect({kSidebarW - 1.f, y, 1.f, h}, kBorder);

        // Section header
        ui.drawText(8.f, y + 8.f, "LAUNCH TOOL", kTextSecondary);
        ui.drawRect({8.f, y + 22.f, kSidebarW - 16.f, 1.f}, kBorder);

        float ay = y + 30.f;
        for (const auto& desc : shell.appRegistry().apps()) {
            if (ay + 46.f > y + h) break;

            ui.drawRect({4.f, ay, kSidebarW - 8.f, 38.f}, kCardBg);
            ui.drawRectOutline({4.f, ay, kSidebarW - 8.f, 38.f}, kBorder, 1.f);

            // App name
            ui.drawText(12.f, ay + 5.f, desc.name, kTextPrimary);

            // Executable path (truncate to fit)
            std::string path = desc.executablePath;
            if (path.size() > 24) path = path.substr(0, 21) + "...";
            ui.drawText(12.f, ay + 20.f, path, kTextMuted);

            ay += 46.f;
        }

        if (shell.appRegistry().empty()) {
            ui.drawText(8.f, ay + 8.f, "(no apps registered)", kTextMuted);
        }
    }

    // ── Main area: welcome or project dashboard ───────────────────
    void renderMainArea(UIRenderer& ui, float width, float height,
                        const WorkspaceShell& shell)
    {
        float x = kSidebarW;
        float y = kContentY;
        float w = width - x;
        float h = height - y - kStatusH;

        ui.drawRect({x, y, w, h}, kBackground);

        if (!shell.hasProject()) {
            renderWelcomeScreen(ui, x, y, w, h);
        } else {
            renderProjectDashboard(ui, x, y, w, h, shell);
        }
    }

    void renderWelcomeScreen(UIRenderer& ui, float x, float y, float w, float h) {
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

        // New Project card
        ui.drawRect({card1X, cardY, cardW, cardH}, kCardBg);
        ui.drawRectOutline({card1X, cardY, cardW, cardH}, kBorder, 1.f);
        ui.drawRect({card1X, cardY, cardW, 3.f}, kAccentBlue);
        ui.drawText(card1X + 10.f, cardY + 9.f,  "New Project",         kTextPrimary);
        ui.drawText(card1X + 10.f, cardY + 26.f, "File > New Project",  kTextMuted);

        // Open Project card
        ui.drawRect({card2X, cardY, cardW, cardH}, kCardBg);
        ui.drawRectOutline({card2X, cardY, cardW, cardH}, kBorder, 1.f);
        ui.drawRect({card2X, cardY, cardW, 3.f}, kButtonBg);
        ui.drawText(card2X + 10.f, cardY + 9.f,  "Open Project",        kTextPrimary);
        ui.drawText(card2X + 10.f, cardY + 26.f, "File > Open Project", kTextMuted);
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
};

} // namespace NF
