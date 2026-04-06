#include "NF/UI/AtlasUI/Panels/ConsolePanel.h"

namespace NF::UI::AtlasUI {

void ConsolePanel::paint(IPaintContext& context) {
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

    // Messages
    for (const auto& msg : m_messages) {
        if (y > m_bounds.y + m_bounds.h - 4.f) break;

        Color textColor = Theme::ColorToken::Text;
        if (msg.level == MessageLevel::Warning) textColor = 0xFFE8A435;
        else if (msg.level == MessageLevel::Error) textColor = 0xFFF44747;

        context.drawText({left, y, contentW, 14.f}, msg.text, 0, textColor);
        y += 16.f;
    }
}

} // namespace NF::UI::AtlasUI
