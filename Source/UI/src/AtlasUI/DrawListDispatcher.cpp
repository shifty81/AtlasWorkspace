#include "NF/UI/AtlasUI/DrawListDispatcher.h"
#include "NF/UI/UIBackend.h"
#include <variant>

namespace NF::UI::AtlasUI {

void DrawListDispatcher::dispatch(const DrawList& drawList) const {
    if (!m_renderer) return;

    NF::UIBackend* backend = m_renderer->backend();

    for (const DrawCommand& cmd : drawList.commands()) {
        std::visit([this, backend](const auto& c) {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, FillRectCmd>) {
                m_renderer->drawRect(c.rect, atlasToRendererColor(c.color));
            }
            else if constexpr (std::is_same_v<T, DrawRectCmd>) {
                m_renderer->drawRectOutline(c.rect, atlasToRendererColor(c.color), 1.f);
            }
            else if constexpr (std::is_same_v<T, DrawTextCmd>) {
                uint32_t col = atlasToRendererColor(c.color);
                if (backend) {
                    // Prefer native platform text (GDI TextOutA on Win32).
                    backend->drawTextNative(c.rect.x, c.rect.y, c.text, col);
                } else {
                    // Fallback: pixel-block characters via UIRenderer.
                    m_renderer->drawText(c.rect.x, c.rect.y, c.text, col);
                }
            }
            // PushClipCmd / PopClipCmd: not implemented in GDI batch path.
            // The GDI path doesn't support clip rectangles on the batched quad
            // pipeline; ignore silently so panel paint never crashes.
        }, cmd);
    }
}

} // namespace NF::UI::AtlasUI
