#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"

namespace NF {

void UIRenderer::endFrame() {
    // Flush batched geometry to the active backend (if any).
    if (m_backend && !m_vertices.empty()) {
        m_backend->flush(m_vertices.data(), m_vertices.size(),
                         m_indices.data(), m_indices.size());
    }
    m_lastFrameQuadCount = m_quadCount;
    m_lastFrameTextCount = m_textDrawCount;
}

Vec2 UIRenderer::measureText(std::string_view text, float fontSize) const {
    if (m_backend) return m_backend->measureText(text, fontSize);
    // Fallback: fixed-width 8×14 glyphs
    constexpr float kCharWidth  = 8.f;
    constexpr float kCharHeight = 14.f;
    float maxW = 0.f, cx = 0.f, lines = 1.f;
    for (char ch : text) {
        if (ch == '\n') { maxW = std::max(maxW, cx); cx = 0.f; lines += 1.f; continue; }
        cx += kCharWidth;
    }
    return {std::max(maxW, cx), lines * (kCharHeight + 2.f)};
}

} // namespace NF
