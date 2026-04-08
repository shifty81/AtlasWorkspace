// S143 editor tests: PanelStateSerializer, DockTreeSerializer, LayoutManagerV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── PanelStateSerializer ──────────────────────────────────────────────────────

TEST_CASE("PssFormat names", "[Editor][S143]") {
    REQUIRE(std::string(pssFormatName(PssFormat::JSON))   == "JSON");
    REQUIRE(std::string(pssFormatName(PssFormat::Binary)) == "Binary");
    REQUIRE(std::string(pssFormatName(PssFormat::XML))    == "XML");
}

TEST_CASE("PssVersion names", "[Editor][S143]") {
    REQUIRE(std::string(pssVersionName(PssVersion::V1)) == "V1");
    REQUIRE(std::string(pssVersionName(PssVersion::V2)) == "V2");
    REQUIRE(std::string(pssVersionName(PssVersion::V3)) == "V3");
}

TEST_CASE("PssEntry constructor", "[Editor][S143]") {
    PssEntry e(1, "width", "320", PssVersion::V2);
    REQUIRE(e.panelId() == 1u);
    REQUIRE(e.key()     == "width");
    REQUIRE(e.value()   == "320");
    REQUIRE(e.version() == PssVersion::V2);
}

TEST_CASE("PanelStateSerializer serialize and deserialize", "[Editor][S143]") {
    PanelStateSerializer pss;
    pss.serialize(1, "width", "300");
    pss.serialize(1, "height", "200");
    REQUIRE(pss.entryCount()         == 2u);
    REQUIRE(pss.deserialize(1, "width")  == "300");
    REQUIRE(pss.deserialize(1, "height") == "200");
    REQUIRE(pss.deserialize(1, "missing") == "");
}

TEST_CASE("PanelStateSerializer update existing entry", "[Editor][S143]") {
    PanelStateSerializer pss;
    pss.serialize(1, "width", "300");
    pss.serialize(1, "width", "400");
    REQUIRE(pss.entryCount() == 1u);
    REQUIRE(pss.deserialize(1, "width") == "400");
}

TEST_CASE("PanelStateSerializer hasEntry", "[Editor][S143]") {
    PanelStateSerializer pss;
    pss.serialize(2, "visible", "true");
    REQUIRE(pss.hasEntry(2, "visible") == true);
    REQUIRE(pss.hasEntry(2, "missing") == false);
    REQUIRE(pss.hasEntry(9, "visible") == false);
}

TEST_CASE("PanelStateSerializer removePanel", "[Editor][S143]") {
    PanelStateSerializer pss;
    pss.serialize(1, "w", "100");
    pss.serialize(1, "h", "200");
    pss.serialize(2, "w", "150");
    REQUIRE(pss.entryCount() == 3u);
    pss.removePanel(1);
    REQUIRE(pss.entryCount() == 1u);
    REQUIRE(pss.hasEntry(2, "w") == true);
}

TEST_CASE("PanelStateSerializer multiple panels", "[Editor][S143]") {
    PanelStateSerializer pss;
    pss.serialize(1, "theme", "dark");
    pss.serialize(2, "theme", "light");
    pss.serialize(3, "theme", "dark");
    REQUIRE(pss.deserialize(1, "theme") == "dark");
    REQUIRE(pss.deserialize(2, "theme") == "light");
}

// ── DockTreeSerializer ────────────────────────────────────────────────────────

TEST_CASE("DtsNodeKind names", "[Editor][S143]") {
    REQUIRE(std::string(dtsNodeKindName(DtsNodeKind::Split)) == "Split");
    REQUIRE(std::string(dtsNodeKindName(DtsNodeKind::Tab))   == "Tab");
    REQUIRE(std::string(dtsNodeKindName(DtsNodeKind::Leaf))  == "Leaf");
    REQUIRE(std::string(dtsNodeKindName(DtsNodeKind::Root))  == "Root");
}

TEST_CASE("DtsSplitAxis names", "[Editor][S143]") {
    REQUIRE(std::string(dtsSplitAxisName(DtsSplitAxis::Horizontal)) == "Horizontal");
    REQUIRE(std::string(dtsSplitAxisName(DtsSplitAxis::Vertical))   == "Vertical");
}

TEST_CASE("DtsNode defaults", "[Editor][S143]") {
    DtsNode n(1, DtsNodeKind::Root);
    REQUIRE(n.id()       == 1u);
    REQUIRE(n.kind()     == DtsNodeKind::Root);
    REQUIRE(n.axis()     == DtsSplitAxis::Horizontal);
    REQUIRE(n.ratio()    == 0.5f);
    REQUIRE(n.panelId()  == 0u);
    REQUIRE(n.parentId() == 0u);
}

TEST_CASE("DockTreeSerializer add and rootNode", "[Editor][S143]") {
    DockTreeSerializer dts;
    DtsNode root(1, DtsNodeKind::Root);
    DtsNode child(2, DtsNodeKind::Leaf);
    child.setParentId(1);
    dts.addNode(root);
    dts.addNode(child);
    REQUIRE(dts.nodeCount() == 2u);
    REQUIRE(dts.rootNode()  == 1u);
}

TEST_CASE("DockTreeSerializer childrenOf", "[Editor][S143]") {
    DockTreeSerializer dts;
    DtsNode r(1, DtsNodeKind::Root);
    DtsNode a(2, DtsNodeKind::Leaf); a.setParentId(1);
    DtsNode b(3, DtsNodeKind::Leaf); b.setParentId(1);
    dts.addNode(r); dts.addNode(a); dts.addNode(b);
    auto children = dts.childrenOf(1);
    REQUIRE(children.size() == 2u);
}

TEST_CASE("DockTreeSerializer remove and findNode", "[Editor][S143]") {
    DockTreeSerializer dts;
    dts.addNode(DtsNode(1, DtsNodeKind::Split));
    REQUIRE(dts.findNode(1) != nullptr);
    REQUIRE(dts.removeNode(1) == true);
    REQUIRE(dts.findNode(1)   == nullptr);
    REQUIRE(dts.removeNode(1) == false);
}

// ── LayoutManagerV1 ───────────────────────────────────────────────────────────

TEST_CASE("LmLayout names", "[Editor][S143]") {
    REQUIRE(std::string(lmLayoutName(LmLayout::Default))   == "Default");
    REQUIRE(std::string(lmLayoutName(LmLayout::Coding))    == "Coding");
    REQUIRE(std::string(lmLayoutName(LmLayout::Art))       == "Art");
    REQUIRE(std::string(lmLayoutName(LmLayout::Animation)) == "Animation");
    REQUIRE(std::string(lmLayoutName(LmLayout::Custom))    == "Custom");
}

TEST_CASE("LmTransition names", "[Editor][S143]") {
    REQUIRE(std::string(lmTransitionName(LmTransition::Instant)) == "Instant");
    REQUIRE(std::string(lmTransitionName(LmTransition::Fade))    == "Fade");
    REQUIRE(std::string(lmTransitionName(LmTransition::Slide))   == "Slide");
}

TEST_CASE("LmLayoutProfile defaults", "[Editor][S143]") {
    LmLayoutProfile p(1, "Default Layout");
    REQUIRE(p.id()         == 1u);
    REQUIRE(p.name()       == "Default Layout");
    REQUIRE(p.layout()     == LmLayout::Default);
    REQUIRE(p.transition() == LmTransition::Instant);
    REQUIRE(p.data()       == "");
    REQUIRE(p.isActive()   == false);
}

TEST_CASE("LayoutManagerV1 add and activeProfile", "[Editor][S143]") {
    LayoutManagerV1 lm;
    lm.addProfile(LmLayoutProfile(1, "Default"));
    lm.addProfile(LmLayoutProfile(2, "Coding"));
    REQUIRE(lm.profileCount()  == 2u);
    REQUIRE(lm.activeProfile() == 0u);
    REQUIRE(lm.activateProfile(2) == true);
    REQUIRE(lm.activeProfile()    == 2u);
    REQUIRE(lm.findProfile(1)->isActive() == false);
}

TEST_CASE("LayoutManagerV1 activateProfile not found", "[Editor][S143]") {
    LayoutManagerV1 lm;
    lm.addProfile(LmLayoutProfile(1, "p1"));
    REQUIRE(lm.activateProfile(99) == false);
}

TEST_CASE("LayoutManagerV1 remove profile", "[Editor][S143]") {
    LayoutManagerV1 lm;
    lm.addProfile(LmLayoutProfile(1, "p"));
    REQUIRE(lm.removeProfile(1) == true);
    REQUIRE(lm.profileCount()   == 0u);
    REQUIRE(lm.removeProfile(1) == false);
}
