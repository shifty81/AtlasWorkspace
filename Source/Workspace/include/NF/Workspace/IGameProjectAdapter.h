#pragma once
// NF::IGameProjectAdapter — Contract for hosted projects.
//
// This interface defines how a hosted project (e.g., NovaForge) integrates
// with the Atlas Workspace. Projects implement this interface to register
// their extensions, panels, schemas, and commands.
//
// See Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md for boundary rules.

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF {

// ── IEditorPanel ────────────────────────────────────────────────
// Base interface for all project-specific editor panels.
// Panels are created by factory functions stored in
// GameplaySystemPanelDescriptor::createPanel and cached by ProjectSystemsTool.

class IEditorPanel {
public:
    virtual ~IEditorPanel() = default;

    // Panel identity
    [[nodiscard]] virtual const std::string& panelId()    const = 0;
    [[nodiscard]] virtual const std::string& panelTitle() const = 0;

    // Lifecycle hooks — called when the host project is loaded/unloaded.
    virtual void onProjectLoaded(const std::string& /*projectRoot*/) {}
    virtual void onProjectUnloaded() {}

    // Called once per frame when the panel is visible and active.
    virtual void update(float /*dt*/) {}

    // Status — override to expose panel-specific health or state.
    [[nodiscard]] virtual bool isReady() const { return true; }
};

// ── Panel descriptor ────────────────────────────────────────────
// Describes a project-specific panel that should be hosted inside
// a workspace tool. The workspace uses these descriptors to insert
// project panels into the correct host tool by category.

struct GameplaySystemPanelDescriptor {
    std::string id;           // unique panel identifier
    std::string displayName;  // user-facing panel title
    std::string hostToolId;   // which workspace tool hosts this panel
    std::string category;     // grouping within the host tool
    std::string projectId;    // owning project identifier
    bool defaultVisible = false;
    bool enabled = true;
    std::function<std::unique_ptr<IEditorPanel>()> createPanel;
};

// ── Project adapter interface ───────────────────────────────────
// Workspace calls this to discover and load project-specific extensions.

class IGameProjectAdapter {
public:
    virtual ~IGameProjectAdapter() = default;

    // Project identity
    virtual std::string projectId() const = 0;
    virtual std::string projectDisplayName() const = 0;

    // Extension registration
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // Panel provider — returns descriptors for all project-specific panels
    virtual std::vector<GameplaySystemPanelDescriptor> panelDescriptors() const = 0;

    // Schema/validation hooks (optional)
    virtual std::vector<std::string> contentRoots() const { return {}; }
    virtual std::vector<std::string> customCommands() const { return {}; }
};

// ── Panel factory interface ─────────────────────────────────────
// Creates project-specific panels on demand.

class IProjectPanelFactory {
public:
    virtual ~IProjectPanelFactory() = default;
    virtual std::unique_ptr<IEditorPanel> createPanel(const std::string& panelId) = 0;
    virtual std::vector<std::string> availablePanelIds() const = 0;
};

// ── Schema provider interface ───────────────────────────────────
// Supplies project-specific schemas for data validation and editing.

class IGameplaySystemSchemaProvider {
public:
    virtual ~IGameplaySystemSchemaProvider() = default;
    virtual std::vector<std::string> schemaIds() const = 0;
    virtual std::string schemaDefinition(const std::string& id) const = 0;
};

} // namespace NF
