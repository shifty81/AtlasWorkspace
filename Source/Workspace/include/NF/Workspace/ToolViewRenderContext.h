#pragma once
// NF::ToolViewRenderContext — per-frame render context passed to IHostedTool::renderToolView().
//
// WorkspaceRenderer::renderActiveToolView() builds one of these each frame and
// passes it to the active tool.  The tool uses it to draw its own panel layout
// (hierarchy, viewport, inspector, etc.) backed by the tool's live runtime state
// rather than static placeholder text.
//
// The shared color palette and drawPanel helper are provided here so that every
// tool produces a visually consistent chrome without duplicating the definitions.
//
// WorkspaceShell is passed by pointer (never null in practice, but nullable so
// that unit tests can exercise renderToolView without a full shell).
//
// uiCtx — nullable UIContext* provided by WorkspaceRenderer so that tools can
// register hit-regions for interactive click handling.  Always non-null in the
// live runtime path; null only in headless tests that call renderToolView directly.
// Tools must guard every uiCtx call with a null check, or use the hitRegion()
// / isHovered() convenience helpers that do so automatically.

#include "NF/UI/UI.h"
#include "NF/UI/UIWidgets.h"
#include <cstring>

namespace NF {

class WorkspaceShell; // forward – avoids circular dep (WorkspaceShell → ToolRegistry → IHostedTool)

// ── ToolViewRenderContext ─────────────────────────────────────────

struct ToolViewRenderContext {
    UIRenderer&          ui;
    const UIMouseState&  mouse;
    float                x, y, w, h;   // bounding box of the tool content area
    WorkspaceShell*      shell  = nullptr;
    UIContext*           uiCtx  = nullptr; // nullable — null in headless tests

    // ── Shared color palette (RRGGBBAA, matches GDIBackend convention) ─
    static constexpr uint32_t kSurface      = 0x252525FF;
    static constexpr uint32_t kBorder       = 0x3C3C3CFF;
    static constexpr uint32_t kCardBg       = 0x2E2E2EFF;
    static constexpr uint32_t kButtonBg     = 0x3A3A3AFF;
    static constexpr uint32_t kAccentBlue   = 0x007ACCFF;
    static constexpr uint32_t kGreen        = 0x4EC94EFF;
    static constexpr uint32_t kRed          = 0xE05050FF;
    static constexpr uint32_t kTextPrimary  = 0xE0E0E0FF;
    static constexpr uint32_t kTextSecond   = 0x888888FF;
    static constexpr uint32_t kTextMuted    = 0x555555FF;

    // ── Click / hover helpers ─────────────────────────────────────
    // These delegate to uiCtx when non-null.  Safe to call from const
    // renderToolView() implementations.

    // Returns true when the user completes a left-click inside |r|.
    // Draws a subtle hover highlight when hovered (if uiCtx is set).
    // Always returns false when uiCtx is null (headless / test path).
    [[nodiscard]] bool hitRegion(const Rect& r, bool drawHover = true) const {
        if (!uiCtx) return false;
        return uiCtx->hitRegion(r, drawHover);
    }

    // Returns true when the mouse cursor is currently inside |r|.
    // Uses the mouse snapshot embedded in the context — no uiCtx needed.
    [[nodiscard]] bool isHovered(const Rect& r) const {
        return r.contains(mouse.x, mouse.y);
    }

    // Draws a button-style rect at (px,py,pw,ph) with |label|.
    // Returns true when clicked. Honours hover tint automatically.
    [[nodiscard]] bool drawButton(float px, float py, float pw, float ph,
                                   const char* label,
                                   uint32_t normalBg = kButtonBg,
                                   uint32_t hoverBg  = 0x4A4A4AFF) const {
        Rect r{px, py, pw, ph};
        bool hov = isHovered(r);
        ui.drawRect(r, hov ? hoverBg : normalBg);
        ui.drawRectOutline(r, kBorder, 1.f);
        float tw = static_cast<float>(std::strlen(label)) * 7.f;
        ui.drawText(px + (pw - tw) * 0.5f, py + (ph - 14.f) * 0.5f, label, kTextPrimary);
        return hitRegion(r, false);
    }

    // ── Panel chrome helper ───────────────────────────────────────
    // Draws a titled panel zone with an optional centered hint string.
    // px/py/pw/ph are absolute screen coordinates inside the tool area.
    void drawPanel(float px, float py, float pw, float ph,
                   const char* title, const char* hint = nullptr) const {
        ui.drawRect({px, py, pw, ph}, kCardBg);
        ui.drawRectOutline({px, py, pw, ph}, kBorder, 1.f);
        // header strip
        ui.drawRect({px, py, pw, 22.f}, kSurface);
        ui.drawRect({px, py + 22.f, pw, 1.f}, kBorder);
        ui.drawText(px + 8.f, py + 4.f, title, kTextSecond);
        if (hint) {
            float hx = px + (pw - static_cast<float>(std::strlen(hint)) * 8.f) * 0.5f;
            float hy = py + 22.f + (ph - 22.f - 14.f) * 0.5f;
            ui.drawText(hx, hy, hint, kTextMuted);
        }
    }

    // ── Status pill helper ────────────────────────────────────────
    // Draws a small pill (px, py) with label text.
    void drawStatusPill(float px, float py, const char* label, uint32_t pillColor) const {
        float labelW = static_cast<float>(std::strlen(label)) * 7.f + 12.f;
        ui.drawRect({px, py, labelW, 18.f}, pillColor);
        ui.drawText(px + 6.f, py + 2.f, label, kTextPrimary);
    }

    // ── Stat row helper ───────────────────────────────────────────
    // Draws "key: value" on one row at (px, py).
    void drawStatRow(float px, float py, const char* key, const char* value) const {
        ui.drawText(px,       py, key,   kTextSecond);
        ui.drawText(px + 110.f, py, value, kTextPrimary);
    }
};

} // namespace NF
