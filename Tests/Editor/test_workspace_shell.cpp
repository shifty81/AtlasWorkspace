// Tests/Editor/test_workspace_shell.cpp
// Comprehensive tests for Phase 1 workspace core stabilization:
//   IHostedTool, ToolRegistry, PanelRegistry, WorkspaceShell
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/WorkspaceShell.h"
#include "NF/Editor/ToolRegistry.h"
#include "NF/Editor/PanelRegistry.h"
#include "NF/Editor/IHostedTool.h"

// ── Concrete test tool ────────────────────────────────────────────
// Minimal IHostedTool implementation for test purposes.

class StubHostedTool : public NF::IHostedTool {
public:
    StubHostedTool(const std::string& id, const std::string& name,
                   NF::HostedToolCategory cat = NF::HostedToolCategory::Utility,
                   bool primary = true)
    {
        m_desc.toolId      = id;
        m_desc.displayName = name;
        m_desc.category    = cat;
        m_desc.isPrimary   = primary;
    }

    const NF::HostedToolDescriptor& descriptor() const override { return m_desc; }
    const std::string& toolId() const override { return m_desc.toolId; }

    bool initialize() override {
        if (m_state != NF::HostedToolState::Unloaded) return false;
        m_state = NF::HostedToolState::Ready;
        ++m_initCount;
        return true;
    }
    void shutdown() override { m_state = NF::HostedToolState::Unloaded; ++m_shutdownCount; }
    void activate() override { m_state = NF::HostedToolState::Active; }
    void suspend()  override { m_state = NF::HostedToolState::Suspended; }
    void update(float dt) override { m_totalDt += dt; ++m_updateCount; }

    NF::HostedToolState state() const override { return m_state; }

    void onProjectLoaded(const std::string& pid) override { m_lastProjectId = pid; }
    void onProjectUnloaded() override { m_lastProjectId.clear(); }

    // Test accessors
    size_t initCount()     const { return m_initCount; }
    size_t shutdownCount() const { return m_shutdownCount; }
    size_t updateCount()   const { return m_updateCount; }
    float  totalDt()       const { return m_totalDt; }
    const std::string& lastProjectId() const { return m_lastProjectId; }

    void addSupportedPanel(const std::string& pid) { m_desc.supportedPanels.push_back(pid); }

private:
    NF::HostedToolDescriptor m_desc;
    NF::HostedToolState      m_state = NF::HostedToolState::Unloaded;
    size_t m_initCount     = 0;
    size_t m_shutdownCount = 0;
    size_t m_updateCount   = 0;
    float  m_totalDt       = 0.f;
    std::string m_lastProjectId;
};

// ── Concrete test adapter ─────────────────────────────────────────

class StubProjectAdapter : public NF::IGameProjectAdapter {
public:
    StubProjectAdapter(const std::string& pid, const std::string& name)
        : m_pid(pid), m_name(name) {}

    std::string projectId()          const override { return m_pid; }
    std::string projectDisplayName() const override { return m_name; }
    bool initialize() override { m_inited = true; return true; }
    void shutdown()   override { m_inited = false; }

    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        NF::GameplaySystemPanelDescriptor d;
        d.id          = "test.economy";
        d.displayName = "Economy";
        d.hostToolId  = NF::HostToolId::ProjectSystems;
        d.category    = "gameplay";
        d.projectId   = m_pid;
        d.enabled     = true;
        return { d };
    }

    bool isInited() const { return m_inited; }

private:
    std::string m_pid, m_name;
    bool m_inited = false;
};

// ═══════════════════════════════════════════════════════════════════
// IHostedTool + HostedToolDescriptor
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("IHostedTool: descriptor identity", "[workspace][ihostedtool]") {
    StubHostedTool tool("workspace.scene_editor", "Scene Editor",
                        NF::HostedToolCategory::SceneEditing);
    REQUIRE(tool.toolId() == "workspace.scene_editor");
    REQUIRE(tool.descriptor().displayName == "Scene Editor");
    REQUIRE(tool.descriptor().category == NF::HostedToolCategory::SceneEditing);
    REQUIRE(tool.descriptor().isValid());
}

TEST_CASE("IHostedTool: lifecycle transitions", "[workspace][ihostedtool]") {
    StubHostedTool tool("test.tool", "Test Tool");
    REQUIRE(tool.state() == NF::HostedToolState::Unloaded);

    REQUIRE(tool.initialize());
    REQUIRE(tool.state() == NF::HostedToolState::Ready);
    REQUIRE(tool.initCount() == 1);

    // double-init should fail
    REQUIRE_FALSE(tool.initialize());

    tool.activate();
    REQUIRE(tool.state() == NF::HostedToolState::Active);

    tool.suspend();
    REQUIRE(tool.state() == NF::HostedToolState::Suspended);

    tool.activate();
    REQUIRE(tool.state() == NF::HostedToolState::Active);

    tool.update(0.016f);
    REQUIRE(tool.updateCount() == 1);

    tool.shutdown();
    REQUIRE(tool.state() == NF::HostedToolState::Unloaded);
    REQUIRE(tool.shutdownCount() == 1);
}

TEST_CASE("IHostedTool: project hooks", "[workspace][ihostedtool]") {
    StubHostedTool tool("test.tool", "Test Tool");
    tool.onProjectLoaded("novaforge");
    REQUIRE(tool.lastProjectId() == "novaforge");
    tool.onProjectUnloaded();
    REQUIRE(tool.lastProjectId().empty());
}

TEST_CASE("HostedToolDescriptor: validation", "[workspace][ihostedtool]") {
    NF::HostedToolDescriptor good;
    good.toolId      = "test.tool";
    good.displayName = "Test";
    REQUIRE(good.isValid());

    NF::HostedToolDescriptor badId;
    badId.displayName = "Test";
    REQUIRE_FALSE(badId.isValid());

    NF::HostedToolDescriptor badName;
    badName.toolId = "test.tool";
    REQUIRE_FALSE(badName.isValid());
}

TEST_CASE("HostedToolCategory: name round-trip", "[workspace][ihostedtool]") {
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::ProjectBrowser)) == "ProjectBrowser");
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::SceneEditing)) == "SceneEditing");
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::AssetAuthoring)) == "AssetAuthoring");
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::AIAssistant)) == "AIAssistant");
}

TEST_CASE("HostedToolState: name round-trip", "[workspace][ihostedtool]") {
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Unloaded)) == "Unloaded");
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Ready)) == "Ready");
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Active)) == "Active");
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Suspended)) == "Suspended");
}

// ═══════════════════════════════════════════════════════════════════
// ToolRegistry
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("ToolRegistry: register and find", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    REQUIRE(reg.empty());
    auto t = std::make_unique<StubHostedTool>("workspace.scene_editor", "Scene Editor");
    REQUIRE(reg.registerTool(std::move(t)));
    REQUIRE(reg.count() == 1);
    REQUIRE(reg.isRegistered("workspace.scene_editor"));
    REQUIRE_FALSE(reg.isRegistered("workspace.unknown"));
    auto* found = reg.find("workspace.scene_editor");
    REQUIRE(found != nullptr);
    REQUIRE(found->toolId() == "workspace.scene_editor");
}

TEST_CASE("ToolRegistry: reject invalid and duplicate", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    // null
    REQUIRE_FALSE(reg.registerTool(nullptr));
    // invalid descriptor (empty id)
    auto bad = std::make_unique<StubHostedTool>("", "Bad");
    REQUIRE_FALSE(reg.registerTool(std::move(bad)));
    // valid
    auto a = std::make_unique<StubHostedTool>("t.a", "A");
    REQUIRE(reg.registerTool(std::move(a)));
    // duplicate id
    auto a2 = std::make_unique<StubHostedTool>("t.a", "A2");
    REQUIRE_FALSE(reg.registerTool(std::move(a2)));
}

TEST_CASE("ToolRegistry: unregister", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    REQUIRE(reg.count() == 1);
    REQUIRE(reg.unregisterTool("t.a"));
    REQUIRE(reg.count() == 0);
    REQUIRE_FALSE(reg.unregisterTool("t.a"));
}

TEST_CASE("ToolRegistry: byCategory", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.scene", "Scene", NF::HostedToolCategory::SceneEditing));
    reg.registerTool(std::make_unique<StubHostedTool>("t.asset", "Asset", NF::HostedToolCategory::AssetAuthoring));
    reg.registerTool(std::make_unique<StubHostedTool>("t.scene2", "Scene2", NF::HostedToolCategory::SceneEditing));
    auto scenes = reg.byCategory(NF::HostedToolCategory::SceneEditing);
    REQUIRE(scenes.size() == 2);
    auto assets = reg.byCategory(NF::HostedToolCategory::AssetAuthoring);
    REQUIRE(assets.size() == 1);
}

TEST_CASE("ToolRegistry: primaryTools filter", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A", NF::HostedToolCategory::Utility, true));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B", NF::HostedToolCategory::Utility, false));
    auto primary = reg.primaryTools();
    REQUIRE(primary.size() == 1);
    REQUIRE(primary[0]->toolId() == "t.a");
}

TEST_CASE("ToolRegistry: activateTool and suspend previous", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B"));
    reg.initializeAll();

    REQUIRE(reg.activateTool("t.a"));
    REQUIRE(reg.activeToolId() == "t.a");
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Active);

    REQUIRE(reg.activateTool("t.b"));
    REQUIRE(reg.activeToolId() == "t.b");
    REQUIRE(reg.find("t.b")->state() == NF::HostedToolState::Active);
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Suspended);
}

TEST_CASE("ToolRegistry: activateTool fails if unloaded", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    // Not initialized => Unloaded
    REQUIRE_FALSE(reg.activateTool("t.a"));
}

TEST_CASE("ToolRegistry: initializeAll and shutdownAll", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B"));
    REQUIRE(reg.initializeAll());
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Ready);
    REQUIRE(reg.find("t.b")->state() == NF::HostedToolState::Ready);

    reg.activateTool("t.a");
    reg.shutdownAll();
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Unloaded);
    REQUIRE(reg.find("t.b")->state() == NF::HostedToolState::Unloaded);
    REQUIRE(reg.activeToolId().empty());
}

TEST_CASE("ToolRegistry: updateActive", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.initializeAll();
    reg.activateTool("t.a");
    reg.updateActive(0.1f);
    auto* tool = static_cast<StubHostedTool*>(reg.find("t.a"));
    REQUIRE(tool->updateCount() == 1);
}

TEST_CASE("ToolRegistry: allDescriptors", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B"));
    auto descs = reg.allDescriptors();
    REQUIRE(descs.size() == 2);
}

TEST_CASE("ToolRegistry: notifyProject events", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.notifyProjectLoaded("novaforge");
    auto* tool = static_cast<StubHostedTool*>(reg.find("t.a"));
    REQUIRE(tool->lastProjectId() == "novaforge");
    reg.notifyProjectUnloaded();
    REQUIRE(tool->lastProjectId().empty());
}

// ═══════════════════════════════════════════════════════════════════
// PanelRegistry
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("PanelRegistry: register and find", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    REQUIRE(reg.empty());
    NF::SharedPanelDescriptor d;
    d.panelId     = "inspector";
    d.displayName = "Inspector";
    d.category    = NF::SharedPanelCategory::Inspector;
    REQUIRE(reg.registerPanel(d));
    REQUIRE(reg.count() == 1);
    auto* found = reg.find("inspector");
    REQUIRE(found != nullptr);
    REQUIRE(found->displayName == "Inspector");
}

TEST_CASE("PanelRegistry: reject invalid and duplicate", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    NF::SharedPanelDescriptor bad;
    REQUIRE_FALSE(reg.registerPanel(bad)); // invalid
    bad.panelId = "x";
    REQUIRE_FALSE(reg.registerPanel(bad)); // missing displayName
    bad.displayName = "X";
    REQUIRE(reg.registerPanel(bad));
    REQUIRE_FALSE(reg.registerPanel(bad)); // duplicate
}

TEST_CASE("PanelRegistry: unregister", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    NF::SharedPanelDescriptor d;
    d.panelId = "x"; d.displayName = "X";
    reg.registerPanel(d);
    REQUIRE(reg.unregisterPanel("x"));
    REQUIRE(reg.count() == 0);
    REQUIRE_FALSE(reg.unregisterPanel("x"));
}

TEST_CASE("PanelRegistry: byCategory", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    reg.registerPanel({"inspector", "Inspector", NF::SharedPanelCategory::Inspector});
    reg.registerPanel({"outliner", "Outliner", NF::SharedPanelCategory::Navigation});
    reg.registerPanel({"content", "Content", NF::SharedPanelCategory::Navigation});
    auto navs = reg.byCategory(NF::SharedPanelCategory::Navigation);
    REQUIRE(navs.size() == 2);
    auto insp = reg.byCategory(NF::SharedPanelCategory::Inspector);
    REQUIRE(insp.size() == 1);
}

TEST_CASE("PanelRegistry: visibility", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    reg.registerPanel({"x", "X", NF::SharedPanelCategory::Inspector, true});
    reg.registerPanel({"y", "Y", NF::SharedPanelCategory::Inspector, false});
    REQUIRE(reg.isPanelVisible("x"));
    REQUIRE_FALSE(reg.isPanelVisible("y"));
    REQUIRE(reg.setPanelVisible("y", true));
    REQUIRE(reg.isPanelVisible("y"));
    auto visible = reg.visiblePanels();
    REQUIRE(visible.size() == 2);
}

TEST_CASE("PanelRegistry: panelsForTool", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    reg.registerPanel({"inspector", "Inspector", NF::SharedPanelCategory::Inspector});
    reg.registerPanel({"outliner", "Outliner", NF::SharedPanelCategory::Navigation});
    reg.registerPanel({"console", "Console", NF::SharedPanelCategory::Output});
    auto match = reg.panelsForTool({"inspector", "console"});
    REQUIRE(match.size() == 2);
    auto noMatch = reg.panelsForTool({"unknown"});
    REQUIRE(noMatch.empty());
}

TEST_CASE("SharedPanelCategory: name round-trip", "[workspace][panelregistry]") {
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Inspector)) == "Inspector");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Navigation)) == "Navigation");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Output)) == "Output");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::AI)) == "AI");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Preview)) == "Preview");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Editing)) == "Editing");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Status)) == "Status");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Custom)) == "Custom");
}

// ═══════════════════════════════════════════════════════════════════
// WorkspaceShell
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell: default state", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE(shell.phase() == NF::ShellPhase::Created);
    REQUIRE_FALSE(shell.hasProject());
}

TEST_CASE("WorkspaceShell: initialize registers default panels", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE(shell.initialize());
    REQUIRE(shell.phase() == NF::ShellPhase::Ready);
    // canonical panels
    REQUIRE(shell.panelRegistry().isRegistered("inspector"));
    REQUIRE(shell.panelRegistry().isRegistered("outliner"));
    REQUIRE(shell.panelRegistry().isRegistered("content_browser"));
    REQUIRE(shell.panelRegistry().isRegistered("console"));
    REQUIRE(shell.panelRegistry().isRegistered("notifications"));
    REQUIRE(shell.panelRegistry().isRegistered("atlasai_chat"));
    REQUIRE(shell.panelRegistry().isRegistered("command_palette"));
    REQUIRE(shell.panelRegistry().isRegistered("asset_preview"));
    REQUIRE(shell.panelRegistry().count() == 10);
}

TEST_CASE("WorkspaceShell: double-init fails", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE(shell.initialize());
    REQUIRE_FALSE(shell.initialize());
}

TEST_CASE("WorkspaceShell: shutdown transitions to Destroyed", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.shutdown();
    REQUIRE(shell.phase() == NF::ShellPhase::Destroyed);
}

TEST_CASE("WorkspaceShell: register tool before init, initialized by init", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    auto t = std::make_unique<StubHostedTool>("workspace.scene_editor", "Scene Editor",
                                               NF::HostedToolCategory::SceneEditing);
    auto* raw = t.get();
    shell.toolRegistry().registerTool(std::move(t));
    shell.initialize();
    REQUIRE(raw->state() == NF::HostedToolState::Ready);
    REQUIRE(shell.toolRegistry().count() == 1);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: activate tool through shell", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("workspace.scene_editor", "Scene Editor"));
    shell.initialize();
    REQUIRE(shell.toolRegistry().activateTool("workspace.scene_editor"));
    REQUIRE(shell.toolRegistry().activeToolId() == "workspace.scene_editor");
    auto* active = shell.toolRegistry().activeTool();
    REQUIRE(active != nullptr);
    REQUIRE(active->state() == NF::HostedToolState::Active);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: update forwards to active tool", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.a", "A"));
    shell.initialize();
    shell.toolRegistry().activateTool("t.a");
    shell.update(0.05f);
    auto* tool = static_cast<StubHostedTool*>(shell.toolRegistry().find("t.a"));
    REQUIRE(tool->updateCount() == 1);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: loadProject", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    auto adapter = std::make_unique<StubProjectAdapter>("novaforge", "NovaForge");
    REQUIRE(shell.loadProject(std::move(adapter)));
    REQUIRE(shell.hasProject());
    REQUIRE(shell.projectAdapter()->projectId() == "novaforge");
    REQUIRE(shell.projectSystemsTool().panelCount() == 1);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: unloadProject", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.loadProject(std::make_unique<StubProjectAdapter>("nf", "NF"));
    REQUIRE(shell.hasProject());
    shell.unloadProject();
    REQUIRE_FALSE(shell.hasProject());
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: loadProject null fails", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE_FALSE(shell.loadProject(nullptr));
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: loadProject before init fails", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE_FALSE(shell.loadProject(
        std::make_unique<StubProjectAdapter>("nf", "NF")));
}

TEST_CASE("WorkspaceShell: loadProject notifies tools", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.a", "A"));
    shell.initialize();
    shell.loadProject(std::make_unique<StubProjectAdapter>("nf", "NF"));
    auto* tool = static_cast<StubHostedTool*>(shell.toolRegistry().find("t.a"));
    REQUIRE(tool->lastProjectId() == "nf");
    shell.unloadProject();
    REQUIRE(tool->lastProjectId().empty());
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: shell contract is wired", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE(shell.shellContract().isInitialized());
    shell.shutdown();
}

TEST_CASE("ShellPhase: name round-trip", "[workspace][shell]") {
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Created)) == "Created");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Initializing)) == "Initializing");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Ready)) == "Ready");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::ShuttingDown)) == "ShuttingDown");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Destroyed)) == "Destroyed");
}

TEST_CASE("WorkspaceShell: accessors return correct registries", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    // Panel registry has default panels
    REQUIRE(shell.panelRegistry().count() == 10);
    // Tool registry starts empty (no tools registered)
    REQUIRE(shell.toolRegistry().empty());
    // App registry is accessible
    REQUIRE(shell.appRegistry().empty());
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: multiple tools activate/suspend", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.a", "A", NF::HostedToolCategory::SceneEditing));
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.b", "B", NF::HostedToolCategory::AssetAuthoring));
    shell.initialize();

    shell.toolRegistry().activateTool("t.a");
    REQUIRE(shell.toolRegistry().find("t.a")->state() == NF::HostedToolState::Active);

    shell.toolRegistry().activateTool("t.b");
    REQUIRE(shell.toolRegistry().find("t.b")->state() == NF::HostedToolState::Active);
    REQUIRE(shell.toolRegistry().find("t.a")->state() == NF::HostedToolState::Suspended);

    shell.toolRegistry().activateTool("t.a");
    REQUIRE(shell.toolRegistry().find("t.a")->state() == NF::HostedToolState::Active);
    REQUIRE(shell.toolRegistry().find("t.b")->state() == NF::HostedToolState::Suspended);

    shell.shutdown();
}

TEST_CASE("WorkspaceShell: update with no active tool does not crash", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.update(0.016f); // no crash
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: update before init does nothing", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.update(0.016f); // no crash
}
