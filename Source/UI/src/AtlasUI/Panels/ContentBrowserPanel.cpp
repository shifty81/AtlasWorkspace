#include "NF/UI/AtlasUI/Panels/ContentBrowserPanel.h"

namespace NF::UI::AtlasUI {

void ContentBrowserPanel::paint(IPaintContext& context) {
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

    // Current path
    context.drawText({left, y, contentW, 14.f}, m_currentPath, 0, Theme::ColorToken::TextMuted);
    y += 20.f;

    // Separator
    context.fillRect({left, y, contentW, 1.f}, Theme::ColorToken::Border);
    y += Theme::Spacing::Small;

    // File entries
    for (const auto& entry : m_entries) {
        if (y > m_bounds.y + m_bounds.h - 4.f) break;

        std::string icon = entry.isDirectory ? "[D] " : "[F] ";
        Color textColor = entry.isDirectory ? Theme::ColorToken::Accent : Theme::ColorToken::Text;
        context.drawText({left + 8.f, y, contentW - 8.f, 14.f},
                         icon + entry.name, 0, textColor);
        y += 18.f;
    }
}

} // namespace NF::UI::AtlasUI
