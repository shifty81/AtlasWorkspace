#include "NF/UI/AtlasUI/Panels/InspectorPanel.h"

namespace NF::UI::AtlasUI {

void InspectorPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Title bar
    NF::Rect hdr = m_bounds;
    hdr.h = 22.f;
    context.fillRect(hdr, Theme::ColorToken::SurfaceAlt);
    context.drawText({hdr.x + 8.f, hdr.y + 4.f, hdr.w - 16.f, 14.f},
                     m_title, 0, Theme::ColorToken::Text);

    float y = m_bounds.y + 26.f;
    const float left = m_bounds.x + 8.f;
    const float contentW = m_bounds.w - 16.f;

    if (m_selectedEntityId >= 0) {
        // Entity ID row
        char idBuf[32];
        std::snprintf(idBuf, sizeof(idBuf), "Entity #%d", m_selectedEntityId);
        context.drawText({left, y, contentW, 16.f}, idBuf, 0, Theme::ColorToken::Text);
        y += 20.f;

        // Separator
        context.fillRect({left, y, contentW, 1.f}, Theme::ColorToken::Border);
        y += Theme::Spacing::Small;

        // Transform header (quick summary above the PropertyGrid)
        context.drawText({left, y, contentW, 16.f}, "Transform", 0, Theme::ColorToken::Accent);
        y += 18.f;

        // The PropertyGrid occupies the remainder of the panel (arranged in arrange()).
        m_grid->paint(context);
    } else {
        context.drawText({left, y, contentW, 16.f}, "No entity selected", 0, Theme::ColorToken::TextMuted);
    }
}

bool InspectorPanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (m_selectedEntityId < 0) return false;
    // Forward all input to the PropertyGrid child.  It handles hover, click,
    // group toggle, and will host editable text entry as the grid matures.
    return m_grid->handleInput(context);
}

} // namespace NF::UI::AtlasUI
