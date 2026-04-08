// S147 editor tests: SettingsControlPanelV1, ProjectSurfaceV1, RepoSurfaceV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── SettingsControlPanelV1 ────────────────────────────────────────────────────

TEST_CASE("CpSettingType names", "[Editor][S147]") {
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Toggle))      == "Toggle");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Slider))      == "Slider");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Dropdown))    == "Dropdown");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::TextInput))   == "TextInput");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::ColorPicker)) == "ColorPicker");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::FilePicker))  == "FilePicker");
}

TEST_CASE("CpSection names", "[Editor][S147]") {
    REQUIRE(std::string(cpSectionName(CpSection::General))     == "General");
    REQUIRE(std::string(cpSectionName(CpSection::Appearance))  == "Appearance");
    REQUIRE(std::string(cpSectionName(CpSection::Keybindings)) == "Keybindings");
    REQUIRE(std::string(cpSectionName(CpSection::Extensions))  == "Extensions");
    REQUIRE(std::string(cpSectionName(CpSection::Advanced))    == "Advanced");
}

TEST_CASE("CpSettingEntry defaults", "[Editor][S147]") {
    CpSettingEntry e(1, "theme", "Theme", CpSettingType::Dropdown);
    REQUIRE(e.id()           == 1u);
    REQUIRE(e.key()          == "theme");
    REQUIRE(e.label()        == "Theme");
    REQUIRE(e.type()         == CpSettingType::Dropdown);
    REQUIRE(e.section()      == CpSection::General);
    REQUIRE(e.value()        == "");
    REQUIRE(e.defaultValue() == "");
    REQUIRE(e.visible()      == true);
    REQUIRE(e.enabled()      == true);
}

TEST_CASE("SettingsControlPanelV1 add and filterBySection", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSettingEntry e1(1, "theme", "Theme", CpSettingType::Dropdown);
    CpSettingEntry e2(2, "font-size", "Font Size", CpSettingType::Slider);
    e2.setSection(CpSection::Appearance);
    CpSettingEntry e3(3, "font-family", "Font Family", CpSettingType::Dropdown);
    e3.setSection(CpSection::Appearance);
    panel.addEntry(e1); panel.addEntry(e2); panel.addEntry(e3);
    REQUIRE(panel.entryCount() == 3u);
    auto appearance = panel.filterBySection(CpSection::Appearance);
    REQUIRE(appearance.size() == 2u);
}

TEST_CASE("SettingsControlPanelV1 resetToDefault", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSettingEntry e(1, "color", "Color", CpSettingType::ColorPicker);
    e.setDefaultValue("#FFFFFF");
    e.setValue("#FF0000");
    panel.addEntry(e);
    REQUIRE(panel.findEntry(1)->value() == "#FF0000");
    REQUIRE(panel.resetToDefault(1) == true);
    REQUIRE(panel.findEntry(1)->value() == "#FFFFFF");
    REQUIRE(panel.resetToDefault(99) == false);
}

TEST_CASE("SettingsControlPanelV1 visibleCount", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    panel.addEntry(CpSettingEntry(1, "a", "A", CpSettingType::Toggle));
    panel.addEntry(CpSettingEntry(2, "b", "B", CpSettingType::Toggle));
    REQUIRE(panel.visibleCount() == 2u);
    panel.findEntry(1)->setVisible(false);
    REQUIRE(panel.visibleCount() == 1u);
}

TEST_CASE("SettingsControlPanelV1 duplicate and remove", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    REQUIRE(panel.addEntry(CpSettingEntry(5, "x", "X", CpSettingType::TextInput)) == true);
    REQUIRE(panel.addEntry(CpSettingEntry(5, "x", "X", CpSettingType::TextInput)) == false);
    REQUIRE(panel.removeEntry(5) == true);
    REQUIRE(panel.entryCount()   == 0u);
}

// ── ProjectSurfaceV1 ──────────────────────────────────────────────────────────

TEST_CASE("PjsStatus names", "[Editor][S147]") {
    REQUIRE(std::string(pjsStatusName(PjsStatus::Unloaded)) == "Unloaded");
    REQUIRE(std::string(pjsStatusName(PjsStatus::Loading))  == "Loading");
    REQUIRE(std::string(pjsStatusName(PjsStatus::Loaded))   == "Loaded");
    REQUIRE(std::string(pjsStatusName(PjsStatus::Error))    == "Error");
}

TEST_CASE("PjsType names", "[Editor][S147]") {
    REQUIRE(std::string(pjsTypeName(PjsType::Game))    == "Game");
    REQUIRE(std::string(pjsTypeName(PjsType::Library)) == "Library");
    REQUIRE(std::string(pjsTypeName(PjsType::Tool))    == "Tool");
    REQUIRE(std::string(pjsTypeName(PjsType::Plugin))  == "Plugin");
}

TEST_CASE("ProjectSurfaceV1 defaults", "[Editor][S147]") {
    ProjectSurfaceV1 proj(1, "MyGame");
    REQUIRE(proj.id()       == 1u);
    REQUIRE(proj.name()     == "MyGame");
    REQUIRE(proj.status()   == PjsStatus::Unloaded);
    REQUIRE(proj.type()     == PjsType::Game);
    REQUIRE(proj.rootPath() == "");
    REQUIRE(proj.assetCount() == 0u);
}

TEST_CASE("ProjectSurfaceV1 addAsset and modifiedCount", "[Editor][S147]") {
    ProjectSurfaceV1 proj(1, "Game");
    PjsAsset a1(1, "player.mesh", "/assets/player.mesh");
    PjsAsset a2(2, "sky.tex",     "/assets/sky.tex");
    a1.setModified(true);
    proj.addAsset(a1);
    proj.addAsset(a2);
    REQUIRE(proj.assetCount()    == 2u);
    REQUIRE(proj.modifiedCount() == 1u);
}

TEST_CASE("ProjectSurfaceV1 findAsset and removeAsset", "[Editor][S147]") {
    ProjectSurfaceV1 proj(1, "Game");
    proj.addAsset(PjsAsset(10, "model.fbx", "/model.fbx"));
    REQUIRE(proj.findAsset(10) != nullptr);
    REQUIRE(proj.removeAsset(10) == true);
    REQUIRE(proj.findAsset(10)   == nullptr);
    REQUIRE(proj.removeAsset(10) == false);
}

TEST_CASE("ProjectSurfaceV1 setters", "[Editor][S147]") {
    ProjectSurfaceV1 proj(1, "LibProject");
    proj.setStatus(PjsStatus::Loaded);
    proj.setType(PjsType::Library);
    proj.setRootPath("/home/user/mylib");
    REQUIRE(proj.status()   == PjsStatus::Loaded);
    REQUIRE(proj.type()     == PjsType::Library);
    REQUIRE(proj.rootPath() == "/home/user/mylib");
}

// ── RepoSurfaceV1 ─────────────────────────────────────────────────────────────

TEST_CASE("RsvBranchStatus names", "[Editor][S147]") {
    REQUIRE(std::string(rsvBranchStatusName(RsvBranchStatus::Clean))    == "Clean");
    REQUIRE(std::string(rsvBranchStatusName(RsvBranchStatus::Modified)) == "Modified");
    REQUIRE(std::string(rsvBranchStatusName(RsvBranchStatus::Ahead))    == "Ahead");
    REQUIRE(std::string(rsvBranchStatusName(RsvBranchStatus::Behind))   == "Behind");
    REQUIRE(std::string(rsvBranchStatusName(RsvBranchStatus::Conflict)) == "Conflict");
}

TEST_CASE("RsvRemote names", "[Editor][S147]") {
    REQUIRE(std::string(rsvRemoteName(RsvRemote::None))      == "None");
    REQUIRE(std::string(rsvRemoteName(RsvRemote::GitHub))    == "GitHub");
    REQUIRE(std::string(rsvRemoteName(RsvRemote::GitLab))    == "GitLab");
    REQUIRE(std::string(rsvRemoteName(RsvRemote::Bitbucket)) == "Bitbucket");
    REQUIRE(std::string(rsvRemoteName(RsvRemote::Custom))    == "Custom");
}

TEST_CASE("RsvCommit constructor", "[Editor][S147]") {
    RsvCommit c(1, "abc123", "Initial commit", "Alice");
    REQUIRE(c.id()        == 1u);
    REQUIRE(c.sha()       == "abc123");
    REQUIRE(c.message()   == "Initial commit");
    REQUIRE(c.author()    == "Alice");
    REQUIRE(c.timestamp() == 0u);
    c.setTimestamp(1700000000u);
    REQUIRE(c.timestamp() == 1700000000u);
}

TEST_CASE("RepoSurfaceV1 defaults", "[Editor][S147]") {
    RepoSurfaceV1 repo(1, "AtlasWorkspace");
    REQUIRE(repo.id()            == 1u);
    REQUIRE(repo.name()          == "AtlasWorkspace");
    REQUIRE(repo.branchName()    == "main");
    REQUIRE(repo.status()        == RsvBranchStatus::Clean);
    REQUIRE(repo.remote()        == RsvRemote::None);
    REQUIRE(repo.commitCount()   == 0u);
    REQUIRE(repo.currentBranch() == "main");
}

TEST_CASE("RepoSurfaceV1 addCommit and clearCommits", "[Editor][S147]") {
    RepoSurfaceV1 repo(1, "repo");
    repo.addCommit(RsvCommit(1, "sha1", "feat: add login", "Bob"));
    repo.addCommit(RsvCommit(2, "sha2", "fix: null ptr",   "Alice"));
    REQUIRE(repo.commitCount() == 2u);
    REQUIRE(repo.addCommit(RsvCommit(1, "sha1", "dup", "Bob")) == false);
    repo.clearCommits();
    REQUIRE(repo.commitCount() == 0u);
}

TEST_CASE("RepoSurfaceV1 setters", "[Editor][S147]") {
    RepoSurfaceV1 repo(1, "repo");
    repo.setBranchName("feature/login");
    repo.setStatus(RsvBranchStatus::Ahead);
    repo.setRemote(RsvRemote::GitHub);
    REQUIRE(repo.branchName()    == "feature/login");
    REQUIRE(repo.currentBranch() == "feature/login");
    REQUIRE(repo.status()        == RsvBranchStatus::Ahead);
    REQUIRE(repo.remote()        == RsvRemote::GitHub);
}
