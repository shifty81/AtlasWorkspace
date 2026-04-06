#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <cstdint>
#include <string>

namespace NF::UI::AtlasUI {

/// Render modes for the viewport.
enum class ViewportRenderMode : uint8_t { Shaded, Wireframe, Unlit };

/// Tool modes for the viewport.
enum class ViewportToolMode : uint8_t { Select, Move, Rotate, Scale, Paint, Erase };

/// AtlasUI ViewportPanel — 3D viewport shell with camera info overlay.
/// Replaces the legacy NF::Editor::ViewportPanel for the AtlasUI framework.
/// This is a shell implementation; actual 3D rendering is handled by the
/// render backend. This panel provides the UI chrome and overlays.
class ViewportPanel final : public PanelBase {
public:
    ViewportPanel()
        : PanelBase("atlas.viewport", "Viewport") {}

    void paint(IPaintContext& context) override;

    void setCameraPosition(float x, float y, float z) {
        m_camX = x; m_camY = y; m_camZ = z;
    }
    [[nodiscard]] float cameraX() const { return m_camX; }
    [[nodiscard]] float cameraY() const { return m_camY; }
    [[nodiscard]] float cameraZ() const { return m_camZ; }

    void setGridEnabled(bool enabled) { m_gridEnabled = enabled; }
    [[nodiscard]] bool gridEnabled() const { return m_gridEnabled; }

    void setRenderMode(ViewportRenderMode mode) { m_renderMode = mode; }
    [[nodiscard]] ViewportRenderMode renderMode() const { return m_renderMode; }

    void setToolMode(ViewportToolMode mode) { m_toolMode = mode; }
    [[nodiscard]] ViewportToolMode toolMode() const { return m_toolMode; }

private:
    float m_camX = 0.f, m_camY = 0.f, m_camZ = 0.f;
    bool m_gridEnabled = true;
    ViewportRenderMode m_renderMode = ViewportRenderMode::Shaded;
    ViewportToolMode m_toolMode = ViewportToolMode::Select;
};

} // namespace NF::UI::AtlasUI
