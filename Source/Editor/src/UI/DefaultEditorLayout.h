#pragma once
#include <string>

namespace atlas::editor {

/// Returns the default editor layout expressed in the Atlas GUI DSL.
/// This is the bootstrap layout used when no user-customized layout
/// exists.  The editor UI is self-hosted: the editor shell itself is
/// described in the same DSL format used for game UI.
///
/// Layout: Left sidebar (Asset Browser) is preserved.  The rest is
/// reworked around the Ship Editor as the main workspace — ship
/// viewport, properties, component palette, ship systems, and console.
inline std::string DefaultEditorDSL() {
    return R"(layout "DefaultEditor" {
    split horizontal 0.18 {
        panel "AssetBrowser"
        split vertical 0.75 {
            panel "ShipEditor"
            split horizontal 0.60 {
                panel "Console"
                panel "ShipSystems"
            }
        }
    }
})";
}

} // namespace atlas::editor
