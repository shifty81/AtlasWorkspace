#pragma once
// NF::D3D11Backend — Primary GPU rendering backend for the UI system (planned).
//
// This is the intended primary backend for Atlas Workspace.
// It will use Direct3D 11 for quad batching, texture atlas, and UI rendering.
// Windows only.
//
// STATUS: Planned — interface stub only. Not yet implemented.

#include "NF/UI/UIBackend.h"

#ifdef _WIN32

namespace NF {

class D3D11Backend final : public UIBackend {
public:
    bool init(int width, int height) override {
        (void)width; (void)height;
        NF_LOG_WARN("UI", "D3D11Backend is not yet implemented — use GDI fallback");
        return false;
    }

    void shutdown() override {}

    void beginFrame(int width, int height) override {
        (void)width; (void)height;
    }

    void endFrame() override {}

    void flush(const UIVertex* vertices, size_t vertexCount,
               const uint32_t* indices, size_t indexCount) override {
        (void)vertices; (void)vertexCount;
        (void)indices;  (void)indexCount;
    }

    void drawText(const std::string& text, float x, float y,
                  float size, uint32_t color) override {
        (void)text; (void)x; (void)y; (void)size; (void)color;
    }

    float measureTextWidth(const std::string& text, float size) override {
        (void)text; (void)size;
        return 0.0f;
    }

    void setClipRect(float x, float y, float w, float h) override {
        (void)x; (void)y; (void)w; (void)h;
    }

    void clearClipRect() override {}
};

} // namespace NF

#endif // _WIN32
