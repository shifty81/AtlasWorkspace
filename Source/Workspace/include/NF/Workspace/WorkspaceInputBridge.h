#pragma once
// NF::Editor::WorkspaceInputBridge
// Syncs NF::InputSystem state into an AtlasUI BasicInputContext each frame.
// This bridges the Win32/platform input pipeline to the AtlasUI widget system.

#include "NF/Input/Input.h"
#include "NF/UI/AtlasUI/Contexts.h"

namespace NF {

class WorkspaceInputBridge {
public:
    /// Copy current InputSystem state into a BasicInputContext.
    static void sync(const InputSystem& input, UI::AtlasUI::BasicInputContext& ctx) {
        const InputState& state = input.state();
        ctx.setMousePosition({state.mouse.x, state.mouse.y});
        ctx.setPrimaryDown(state.keys[static_cast<size_t>(KeyCode::Mouse1)]);
        ctx.setSecondaryDown(state.keys[static_cast<size_t>(KeyCode::Mouse2)]);
        // Mirror a representative set of keyboard keys used by widgets.
        for (size_t k = 0; k < static_cast<size_t>(KeyCode::COUNT); ++k) {
            ctx.setKeyDown(static_cast<int>(k), state.keys[k]);
        }
    }
};

} // namespace NF
