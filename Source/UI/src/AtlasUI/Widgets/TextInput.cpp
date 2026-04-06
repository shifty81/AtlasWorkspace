#include "NF/UI/AtlasUI/Widgets/TextInput.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

TextInput::TextInput(std::string placeholder)
    : m_placeholder(std::move(placeholder)) {}

void TextInput::setText(const std::string& text) {
    m_text = text;
    m_cursor = m_text.size();
}

void TextInput::measure(ILayoutContext&) {
    m_bounds.h = 28.f;
}

void TextInput::paint(IPaintContext& context) {
    if (!m_visible) return;

    Color bg = Theme::ColorToken::Background;
    Color border = m_focused ? Theme::ColorToken::Accent : Theme::ColorToken::Border;
    context.fillRect(m_bounds, bg);
    context.drawRect(m_bounds, border);

    NF::Rect textArea = insetRect(m_bounds, Theme::Spacing::Small, Theme::Spacing::Tiny);
    if (m_text.empty()) {
        context.drawText(textArea, m_placeholder, 0, Theme::ColorToken::TextMuted);
    } else {
        context.drawText(textArea, m_text, 0, Theme::ColorToken::Text);
    }

    if (m_focused) {
        constexpr float kCharWidth = 8.f;
        float cursorX = textArea.x + static_cast<float>(m_cursor) * kCharWidth;
        NF::Rect cursorRect = {cursorX, textArea.y, 1.f, textArea.h};
        context.fillRect(cursorRect, Theme::ColorToken::Text);
    }
}

bool TextInput::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    bool inside = rectContains(m_bounds, context.mousePosition());
    if (inside && context.primaryDown()) {
        m_focused = true;
        context.requestFocus(this);
        return true;
    }
    if (!inside && context.primaryDown()) {
        m_focused = false;
        return false;
    }
    return inside;
}

} // namespace NF::UI::AtlasUI
