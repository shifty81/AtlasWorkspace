// S144 editor tests: ScrollVirtualizerV1, GraphHostContractV1, WorkspaceShellContract
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ScrollVirtualizerV1 ───────────────────────────────────────────────────────

TEST_CASE("SvScrollDir names", "[Editor][S144]") {
    REQUIRE(std::string(svScrollDirName(SvScrollDir::Vertical))   == "Vertical");
    REQUIRE(std::string(svScrollDirName(SvScrollDir::Horizontal)) == "Horizontal");
    REQUIRE(std::string(svScrollDirName(SvScrollDir::Both))       == "Both");
}

TEST_CASE("SvOverflowMode names", "[Editor][S144]") {
    REQUIRE(std::string(svOverflowModeName(SvOverflowMode::Clip))   == "Clip");
    REQUIRE(std::string(svOverflowModeName(SvOverflowMode::Scroll)) == "Scroll");
    REQUIRE(std::string(svOverflowModeName(SvOverflowMode::Auto))   == "Auto");
}

TEST_CASE("ScrollVirtualizerV1 defaults", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    REQUIRE(sv.dir()            == SvScrollDir::Vertical);
    REQUIRE(sv.overflow()       == SvOverflowMode::Auto);
    REQUIRE(sv.viewportHeight() == 400.0f);
    REQUIRE(sv.itemHeight()     == 24.0f);
    REQUIRE(sv.scrollOffset()   == 0.0f);
}

TEST_CASE("SvVirtualItem defaults", "[Editor][S144]") {
    SvVirtualItem item(0, 0.0f, 24.0f);
    REQUIRE(item.index()   == 0);
    REQUIRE(item.top()     == 0.0f);
    REQUIRE(item.height()  == 24.0f);
    REQUIRE(item.visible() == false);
}

TEST_CASE("SvVisibleRange count", "[Editor][S144]") {
    SvVisibleRange r(2, 7);
    REQUIRE(r.firstIndex() == 2);
    REQUIRE(r.lastIndex()  == 7);
    REQUIRE(r.count()      == 6);
}

TEST_CASE("ScrollVirtualizerV1 updateVisibility basic", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    sv.setViewportHeight(72.0f);
    sv.setScrollOffset(0.0f);
    sv.addItem(SvVirtualItem(0, 0.0f,  24.0f));
    sv.addItem(SvVirtualItem(1, 24.0f, 24.0f));
    sv.addItem(SvVirtualItem(2, 48.0f, 24.0f));
    sv.addItem(SvVirtualItem(3, 72.0f, 24.0f));
    sv.updateVisibility();
    REQUIRE(sv.visibleItems() == 3u);
}

TEST_CASE("ScrollVirtualizerV1 computeRange", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    sv.setViewportHeight(48.0f);
    sv.setScrollOffset(24.0f);
    sv.addItem(SvVirtualItem(0, 0.0f,  24.0f));
    sv.addItem(SvVirtualItem(1, 24.0f, 24.0f));
    sv.addItem(SvVirtualItem(2, 48.0f, 24.0f));
    sv.updateVisibility();
    auto range = sv.computeRange();
    REQUIRE(range.firstIndex() == 1);
    REQUIRE(range.lastIndex()  == 2);
}

TEST_CASE("ScrollVirtualizerV1 empty computeRange", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    sv.setScrollOffset(1000.0f);
    sv.addItem(SvVirtualItem(0, 0.0f, 24.0f));
    sv.updateVisibility();
    auto range = sv.computeRange();
    REQUIRE(range.firstIndex() == 0);
    REQUIRE(range.lastIndex()  == -1);
}

TEST_CASE("ScrollVirtualizerV1 duplicate addItem", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    REQUIRE(sv.addItem(SvVirtualItem(0, 0.0f, 24.0f)) == true);
    REQUIRE(sv.addItem(SvVirtualItem(0, 0.0f, 24.0f)) == false);
}

// ── GraphHostContractV1 ───────────────────────────────────────────────────────

TEST_CASE("GhcNodeKind names", "[Editor][S144]") {
    REQUIRE(std::string(ghcNodeKindName(GhcNodeKind::Exec))    == "Exec");
    REQUIRE(std::string(ghcNodeKindName(GhcNodeKind::Data))    == "Data");
    REQUIRE(std::string(ghcNodeKindName(GhcNodeKind::Comment)) == "Comment");
    REQUIRE(std::string(ghcNodeKindName(GhcNodeKind::Reroute)) == "Reroute");
}

TEST_CASE("GhcPinDir names", "[Editor][S144]") {
    REQUIRE(std::string(ghcPinDirName(GhcPinDir::Input))         == "Input");
    REQUIRE(std::string(ghcPinDirName(GhcPinDir::Output))        == "Output");
    REQUIRE(std::string(ghcPinDirName(GhcPinDir::Bidirectional)) == "Bidirectional");
}

TEST_CASE("GhcNode defaults", "[Editor][S144]") {
    GhcNode n(1, GhcNodeKind::Exec, "Print");
    REQUIRE(n.id()       == 1u);
    REQUIRE(n.kind()     == GhcNodeKind::Exec);
    REQUIRE(n.title()    == "Print");
    REQUIRE(n.x()        == 0.0f);
    REQUIRE(n.y()        == 0.0f);
    REQUIRE(n.selected() == false);
    REQUIRE(n.pins().empty());
}

TEST_CASE("GraphHostContractV1 addNode and selectedCount", "[Editor][S144]") {
    GraphHostContractV1 graph;
    GhcNode n1(1, GhcNodeKind::Exec, "A");
    GhcNode n2(2, GhcNodeKind::Data, "B");
    n1.setSelected(true);
    graph.addNode(n1);
    graph.addNode(n2);
    REQUIRE(graph.nodeCount()    == 2u);
    REQUIRE(graph.selectedCount() == 1u);
}

TEST_CASE("GraphHostContractV1 addPin and connect", "[Editor][S144]") {
    GraphHostContractV1 graph;
    graph.addNode(GhcNode(1, GhcNodeKind::Data, "Source"));
    graph.addNode(GhcNode(2, GhcNodeKind::Data, "Dest"));
    GhcPin p1(10, 1, GhcPinDir::Output, "out");
    GhcPin p2(20, 2, GhcPinDir::Input,  "in");
    REQUIRE(graph.addPin(1, p1) == true);
    REQUIRE(graph.addPin(2, p2) == true);
    REQUIRE(graph.addPin(99, p1) == false);
    REQUIRE(graph.connect(10, 20) == true);
    REQUIRE(graph.connect(10, 99) == false);
}

TEST_CASE("GraphHostContractV1 removeNode", "[Editor][S144]") {
    GraphHostContractV1 graph;
    graph.addNode(GhcNode(1, GhcNodeKind::Exec, "A"));
    REQUIRE(graph.removeNode(1)  == true);
    REQUIRE(graph.nodeCount()    == 0u);
    REQUIRE(graph.removeNode(1)  == false);
}

// ── WorkspaceShellContract ────────────────────────────────────────────────────

TEST_CASE("WscPanel names", "[Editor][S144]") {
    REQUIRE(std::string(wscPanelName(WscPanel::Explorer))  == "Explorer");
    REQUIRE(std::string(wscPanelName(WscPanel::Inspector)) == "Inspector");
    REQUIRE(std::string(wscPanelName(WscPanel::Console))   == "Console");
    REQUIRE(std::string(wscPanelName(WscPanel::Output))    == "Output");
    REQUIRE(std::string(wscPanelName(WscPanel::Search))    == "Search");
}

TEST_CASE("WscTheme names", "[Editor][S144]") {
    REQUIRE(std::string(wscThemeName(WscTheme::Dark))         == "Dark");
    REQUIRE(std::string(wscThemeName(WscTheme::Light))        == "Light");
    REQUIRE(std::string(wscThemeName(WscTheme::HighContrast)) == "HighContrast");
}

TEST_CASE("WorkspaceShellContract initializes all panels", "[Editor][S144]") {
    WorkspaceShellContract ws;
    REQUIRE(ws.visibleCount() == 5u);
    REQUIRE(ws.theme()        == WscTheme::Dark);
}

TEST_CASE("WorkspaceShellContract show and hide", "[Editor][S144]") {
    WorkspaceShellContract ws;
    ws.hidePanel(WscPanel::Console);
    REQUIRE(ws.visibleCount() == 4u);
    ws.showPanel(WscPanel::Console);
    REQUIRE(ws.visibleCount() == 5u);
}

TEST_CASE("WorkspaceShellContract togglePanel", "[Editor][S144]") {
    WorkspaceShellContract ws;
    ws.togglePanel(WscPanel::Search);
    REQUIRE(ws.findPanelState(WscPanel::Search)->visible() == false);
    ws.togglePanel(WscPanel::Search);
    REQUIRE(ws.findPanelState(WscPanel::Search)->visible() == true);
}

TEST_CASE("WorkspaceShellContract setTheme", "[Editor][S144]") {
    WorkspaceShellContract ws;
    ws.setTheme(WscTheme::Light);
    REQUIRE(ws.theme() == WscTheme::Light);
}
