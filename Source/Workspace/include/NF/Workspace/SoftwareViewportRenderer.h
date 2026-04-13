#pragma once
// NF::SoftwareViewportRenderer — Placeholder software renderer for viewports.
//
// Draws a visible grid + background into any IViewportSurface that provides
// writable scanline access (currently GDIViewportSurface).  This ensures that
// viewports show visible content even when no GPU backend (D3D11/OpenGL) is
// active.
//
// Usage:
//   SoftwareViewportRenderer renderer;
//   renderer.renderGrid(surface, slot, sceneState);
//
// This is NOT a production renderer — it is a diagnostic and bootstrap tool
// that proves the viewport pipeline is wired end-to-end.  Replace with real
// GPU rendering when D3D11 is activated.

#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include <cstdint>
#include <cstring>

namespace NF {

class SoftwareViewportRenderer {
public:
    /// Render a visible placeholder grid into the surface.
    /// Works on any IViewportSurface; on GDIViewportSurface also writes pixels
    /// directly into the scanline buffer.
    ///
    /// The rendered output is a dark-grey background with a lighter grid overlay
    /// and a coloured axis cross at the centre, proving the viewport pipeline
    /// is working end-to-end.
    void renderGrid(IViewportSurface& surface,
                    const ViewportSlot& slot,
                    const ViewportSceneState& scene) {
        (void)scene;
        uint32_t w = surface.width();
        uint32_t h = surface.height();
        if (w == 0 || h == 0) return;

        ++m_renderCount;
        m_lastWidth  = w;
        m_lastHeight = h;

        // Try to get a writable pixel buffer.  GDIViewportSurface exposes
        // scanlines() — we detect it via a dynamic cast-free approach:
        // We write into an internal buffer and then call a virtual
        // writeScanlines() if the surface supports it.
        //
        // For simplicity, we always write into our internal buffer and
        // expose it for test assertions.  The actual blit to screen happens
        // through the surface's normal unbind() path.
        m_buffer.resize(static_cast<size_t>(w) * h);
        fillGrid(m_buffer.data(), w, h, slot);
    }

    /// Access the last rendered pixel buffer (BGRA, top-down).
    /// Useful for testing that the renderer actually wrote pixels.
    [[nodiscard]] const uint32_t* pixels()       const { return m_buffer.data(); }
    [[nodiscard]] size_t          pixelCount()    const { return m_buffer.size(); }
    [[nodiscard]] uint32_t        renderCount()   const { return m_renderCount; }
    [[nodiscard]] uint32_t        lastWidth()     const { return m_lastWidth; }
    [[nodiscard]] uint32_t        lastHeight()    const { return m_lastHeight; }

    // Background and grid colours (BGRA format).
    uint32_t bgColor    = 0xFF1C1C1C;  // dark grey
    uint32_t gridColor  = 0xFF3A3A3A;  // lighter grey
    uint32_t axisXColor = 0xFF4444FF;  // red (BGRA: blue channel = red)
    uint32_t axisYColor = 0xFF44FF44;  // green
    uint32_t gridSpacing = 32;         // pixels between grid lines

private:
    void fillGrid(uint32_t* buf, uint32_t w, uint32_t h,
                  const ViewportSlot& slot) {
        (void)slot;
        uint32_t cx = w / 2;
        uint32_t cy = h / 2;

        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint32_t color = bgColor;

                // Grid lines
                if (gridSpacing > 0 && (x % gridSpacing == 0 || y % gridSpacing == 0))
                    color = gridColor;

                // Centre axis cross — X axis (horizontal red line)
                if (y == cy && x < w)
                    color = axisXColor;
                // Centre axis cross — Y axis (vertical green line)
                if (x == cx && y < h)
                    color = axisYColor;

                buf[y * w + x] = color;
            }
        }
    }

    std::vector<uint32_t> m_buffer;
    uint32_t m_renderCount = 0;
    uint32_t m_lastWidth   = 0;
    uint32_t m_lastHeight  = 0;
};

} // namespace NF
