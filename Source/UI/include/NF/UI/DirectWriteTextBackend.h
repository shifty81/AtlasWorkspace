#pragma once
// NF::DirectWriteTextBackend — Primary text rendering backend (planned).
//
// This is the intended primary text rendering path for Atlas Workspace.
// It will use DirectWrite for font loading, glyph layout, and text measurement.
// Windows only.
//
// STATUS: Planned — interface stub only. Not yet implemented.

#include "NF/Core/Core.h"
#include <string>

namespace NF {

// Interface for text rendering backends.
// The UI system uses this to draw and measure text independently of the
// rendering backend (D3D11, GDI, OpenGL).
class ITextBackend {
public:
    virtual ~ITextBackend() = default;

    virtual bool init() = 0;
    virtual void shutdown() = 0;

    virtual void drawText(const std::string& text, float x, float y,
                          float size, uint32_t color) = 0;
    virtual float measureTextWidth(const std::string& text, float size) = 0;
    virtual float lineHeight(float size) = 0;
};

#ifdef _WIN32

class DirectWriteTextBackend final : public ITextBackend {
public:
    bool init() override {
        NF_LOG_WARN("UI", "DirectWriteTextBackend is not yet implemented");
        return false;
    }

    void shutdown() override {}

    void drawText(const std::string& text, float x, float y,
                  float size, uint32_t color) override {
        (void)text; (void)x; (void)y; (void)size; (void)color;
    }

    float measureTextWidth(const std::string& text, float size) override {
        (void)text; (void)size;
        return 0.0f;
    }

    float lineHeight(float size) override {
        (void)size;
        return 0.0f;
    }
};

#endif // _WIN32

} // namespace NF
