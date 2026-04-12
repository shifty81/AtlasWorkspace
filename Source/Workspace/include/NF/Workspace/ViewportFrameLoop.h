#pragma once
// NF::ViewportFrameLoop — Per-frame render orchestration (Item 3 of 10).
//
// Drives the per-frame sequence for all active viewport slots:
//   1. Iterate ViewportHostRegistry::slots()
//   2. Skip non-active slots
//   3. Call ViewportSceneProviderRegistry::dispatchProvide() to get scene state
//   4. Bind IViewportSurface, execute render pass (placeholder — real backends
//      inject draw calls here), unbind surface
//   5. Read colorAttachment() and store in result (used by ViewportPanel)
//   6. Call ViewportHostRegistry::onFrameRendered() to increment frameCount
//
// EditorApp::renderAll() calls renderFrame() each frame and uses the returned
// ViewportFrameResult list to drive AtlasUI::ViewportPanel::setColorAttachment()
// so the scene texture appears inside each panel.

#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include <vector>

namespace NF {

// ── ViewportFrameResult ────────────────────────────────────────────────────────
// Result for one active viewport slot after a renderFrame() call.

struct ViewportFrameResult {
    ViewportHandle    handle           = kInvalidViewportHandle;
    uint32_t          colorAttachmentId = 0;  ///< surface texture (0 = nothing rendered)
    bool              rendered          = false;
    ViewportSceneState sceneState;
};

// ── ViewportFrameStats ─────────────────────────────────────────────────────────

struct ViewportFrameStats {
    uint32_t activeSlots   = 0;
    uint32_t renderedSlots = 0;  ///< slots with valid surface + rendered
    uint32_t skippedSlots  = 0;  ///< active slots with no valid surface
    uint64_t frameNumber   = 0;  ///< increments each renderFrame() call
};

// ── ViewportFrameLoop ──────────────────────────────────────────────────────────

class ViewportFrameLoop {
public:
    void setViewportRegistry(ViewportHostRegistry* r)       { m_viewportReg = r; }
    void setSceneRegistry(ViewportSceneProviderRegistry* r) { m_sceneReg = r; }
    void setSurfaceRegistry(ViewportSurfaceRegistry* r)     { m_surfaceReg = r; }

    [[nodiscard]] ViewportHostRegistry*            viewportRegistry() const { return m_viewportReg; }
    [[nodiscard]] ViewportSceneProviderRegistry*   sceneRegistry()    const { return m_sceneReg; }
    [[nodiscard]] ViewportSurfaceRegistry*         surfaceRegistry()  const { return m_surfaceReg; }

    /// Execute one frame.
    /// Iterates all active slots, calls scene providers, bind/unbind surfaces,
    /// increments frameCount, and returns one result per active slot.
    std::vector<ViewportFrameResult> renderFrame() {
        ++m_stats.frameNumber;
        m_stats.activeSlots   = 0;
        m_stats.renderedSlots = 0;
        m_stats.skippedSlots  = 0;

        std::vector<ViewportFrameResult> results;
        if (!m_viewportReg) return results;

        for (const auto& slot : m_viewportReg->slots()) {
            if (!slot.isActive()) continue;
            ++m_stats.activeSlots;

            ViewportFrameResult result;
            result.handle = slot.handle;

            // Step 1 — collect scene state from the registered provider
            if (m_sceneReg)
                result.sceneState = m_sceneReg->dispatchProvide(slot);

            // Step 2 — bind surface, execute (placeholder) render, unbind
            if (m_surfaceReg) {
                if (auto* surface = m_surfaceReg->findSurface(slot.handle)) {
                    if (surface->isValid()) {
                        surface->bind();
                        // Real render backends inject draw calls between bind/unbind.
                        // The frame loop itself is backend-agnostic.
                        surface->unbind();
                        result.colorAttachmentId = surface->colorAttachment();
                        result.rendered = true;
                    }
                }
            }

            // Step 3 — notify registry that a frame was processed
            m_viewportReg->onFrameRendered(slot.handle);

            if (result.rendered) ++m_stats.renderedSlots;
            else                 ++m_stats.skippedSlots;

            results.push_back(result);
        }

        return results;
    }

    [[nodiscard]] const ViewportFrameStats& stats() const { return m_stats; }
    void resetStats() {
        uint64_t fn = m_stats.frameNumber;
        m_stats = {};
        m_stats.frameNumber = fn;
    }

private:
    ViewportHostRegistry*          m_viewportReg = nullptr;
    ViewportSceneProviderRegistry* m_sceneReg    = nullptr;
    ViewportSurfaceRegistry*       m_surfaceReg  = nullptr;
    ViewportFrameStats             m_stats;
};

} // namespace NF
