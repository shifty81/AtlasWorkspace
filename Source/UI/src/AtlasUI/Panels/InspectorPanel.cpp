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
        // Entity ID
        char idBuf[32];
        std::snprintf(idBuf, sizeof(idBuf), "Entity #%d", m_selectedEntityId);
        context.drawText({left, y, contentW, 16.f}, idBuf, 0, Theme::ColorToken::Text);
        y += 20.f;

        // Separator
        context.fillRect({left, y, contentW, 1.f}, Theme::ColorToken::Border);
        y += Theme::Spacing::Small;

        // Transform header
        context.drawText({left, y, contentW, 16.f}, "Transform", 0, Theme::ColorToken::Accent);
        y += 18.f;

        // Transform values
        char buf[48];
        std::snprintf(buf, sizeof(buf), "X: %.2f", m_transformX);
        context.drawText({left + 8.f, y, contentW - 8.f, 14.f}, buf, 0, Theme::ColorToken::TextMuted);
        y += 16.f;
        std::snprintf(buf, sizeof(buf), "Y: %.2f", m_transformY);
        context.drawText({left + 8.f, y, contentW - 8.f, 14.f}, buf, 0, Theme::ColorToken::TextMuted);
        y += 16.f;
        std::snprintf(buf, sizeof(buf), "Z: %.2f", m_transformZ);
        context.drawText({left + 8.f, y, contentW - 8.f, 14.f}, buf, 0, Theme::ColorToken::TextMuted);
        y += 20.f;

        // Custom properties
        for (const auto& prop : m_properties) {
            if (y > m_bounds.y + m_bounds.h - 4.f) break;
            context.drawText({left, y, contentW * 0.4f, 14.f}, prop.label, 0, Theme::ColorToken::TextMuted);
            context.drawText({left + contentW * 0.4f, y, contentW * 0.6f, 14.f}, prop.value, 0, Theme::ColorToken::Text);
            y += 16.f;
        }
    } else {
        context.drawText({left, y, contentW, 16.f}, "No entity selected", 0, Theme::ColorToken::TextMuted);
    }
}

} // namespace NF::UI::AtlasUI
