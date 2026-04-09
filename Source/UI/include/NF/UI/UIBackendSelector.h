#pragma once
// NF::UIBackendSelector — Centralized backend selection for the UI system.
//
// Instead of hardcoding GDI or OpenGL in main.cpp, use this selector
// to choose the appropriate backend at startup based on availability
// and platform capabilities.
//
// Target priority:
//   1. D3D11 + DirectWrite (primary — when implemented)
//   2. OpenGL (compatibility)
//   3. GDI (fallback/bootstrap — Windows only)
//   4. Null (headless/testing)

#include "NF/UI/UIBackend.h"
#include <memory>
#include <string>

namespace NF {

enum class UIBackendType {
    Auto,       // Let selector pick best available
    D3D11,      // Primary target (not yet implemented)
    OpenGL,     // Compatibility backend
    GDI,        // Fallback/bootstrap (Windows only)
    Null        // Headless/testing
};

inline const char* uiBackendTypeName(UIBackendType t) {
    switch (t) {
    case UIBackendType::Auto:   return "Auto";
    case UIBackendType::D3D11:  return "D3D11";
    case UIBackendType::OpenGL: return "OpenGL";
    case UIBackendType::GDI:    return "GDI";
    case UIBackendType::Null:   return "Null";
    }
    return "Unknown";
}

// Selects and creates the best available backend.
// If 'preferred' is Auto, picks the highest-priority available backend.
// If 'preferred' is a specific type but unavailable, falls back to next best.
inline std::unique_ptr<UIBackend> selectUIBackend(
    UIBackendType preferred = UIBackendType::Auto)
{
    (void)preferred;
    // TODO: Implement D3D11 backend and selection logic.
    // For now, backend creation is still handled directly in main.cpp.
    // This header establishes the contract and enum for future use.
    return nullptr;
}

} // namespace NF
