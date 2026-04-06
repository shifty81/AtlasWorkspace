#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include <cstdio>

namespace NF::UI::AtlasUI {

void ViewportPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Viewport background (darker than normal panels)
    context.fillRect(m_bounds, Theme::ColorToken::Background);

    // Grid overlay
    if (m_gridEnabled) {
        const float step = 40.f;
        const Color gridColor = 0xFF333333;
        for (float gx = m_bounds.x; gx < m_bounds.x + m_bounds.w; gx += step)
            context.fillRect({gx, m_bounds.y, 1.f, m_bounds.h}, gridColor);
        for (float gy = m_bounds.y; gy < m_bounds.y + m_bounds.h; gy += step)
            context.fillRect({m_bounds.x, gy, m_bounds.w, 1.f}, gridColor);
    }

    // Tool mode indicator (top-left)
    const char* toolNames[] = {"Select", "Move", "Rotate", "Scale", "Paint", "Erase"};
    context.drawText({m_bounds.x + 8.f, m_bounds.y + 8.f, 100.f, 14.f},
                     toolNames[static_cast<int>(m_toolMode)], 0, Theme::ColorToken::TextMuted);

    // Render mode indicator (top-right)
    const char* renderNames[] = {"Shaded", "Wireframe", "Unlit"};
    context.drawText({m_bounds.x + m_bounds.w - 80.f, m_bounds.y + 8.f, 72.f, 14.f},
                     renderNames[static_cast<int>(m_renderMode)], 0, Theme::ColorToken::TextMuted);

    // Center label
    const char* label = "[ 3D Viewport ]";
    context.drawText({m_bounds.x + (m_bounds.w - 120.f) * 0.5f,
                      m_bounds.y + (m_bounds.h - 14.f) * 0.5f, 120.f, 14.f},
                     label, 0, 0xFF444444);

    // Camera info overlay (bottom-left)
    char camText[64];
    std::snprintf(camText, sizeof(camText), "Cam: %.1f, %.1f, %.1f", m_camX, m_camY, m_camZ);
    context.drawText({m_bounds.x + 8.f, m_bounds.y + m_bounds.h - 20.f, 200.f, 14.f},
                     camText, 0, Theme::ColorToken::TextMuted);
}

} // namespace NF::UI::AtlasUI
