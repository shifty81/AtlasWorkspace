// S146 editor tests: WidgetKitV1, TooltipSystemV1, TabBarV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── WidgetKitV1 ───────────────────────────────────────────────────────────────

TEST_CASE("WkWidgetType names", "[Editor][S146]") {
    REQUIRE(std::string(wkWidgetTypeName(WkWidgetType::Button))    == "Button");
    REQUIRE(std::string(wkWidgetTypeName(WkWidgetType::Label))     == "Label");
    REQUIRE(std::string(wkWidgetTypeName(WkWidgetType::Input))     == "Input");
    REQUIRE(std::string(wkWidgetTypeName(WkWidgetType::Slider))    == "Slider");
    REQUIRE(std::string(wkWidgetTypeName(WkWidgetType::Toggle))    == "Toggle");
    REQUIRE(std::string(wkWidgetTypeName(WkWidgetType::Dropdown))  == "Dropdown");
    REQUIRE(std::string(wkWidgetTypeName(WkWidgetType::Separator)) == "Separator");
}

TEST_CASE("WkState names", "[Editor][S146]") {
    REQUIRE(std::string(wkStateName(WkState::Normal))   == "Normal");
    REQUIRE(std::string(wkStateName(WkState::Hovered))  == "Hovered");
    REQUIRE(std::string(wkStateName(WkState::Pressed))  == "Pressed");
    REQUIRE(std::string(wkStateName(WkState::Disabled)) == "Disabled");
    REQUIRE(std::string(wkStateName(WkState::Focused))  == "Focused");
}

TEST_CASE("WkWidget defaults", "[Editor][S146]") {
    WkWidget w(1, WkWidgetType::Button);
    REQUIRE(w.id()      == 1u);
    REQUIRE(w.type()    == WkWidgetType::Button);
    REQUIRE(w.state()   == WkState::Normal);
    REQUIRE(w.label()   == "");
    REQUIRE(w.width()   == 100);
    REQUIRE(w.height()  == 24);
    REQUIRE(w.visible() == true);
    REQUIRE(w.enabled() == true);
}

TEST_CASE("WkWidget setters", "[Editor][S146]") {
    WkWidget w(2, WkWidgetType::Slider);
    w.setState(WkState::Hovered);
    w.setLabel("Volume");
    w.setWidth(200);
    w.setHeight(32);
    w.setVisible(false);
    w.setEnabled(false);
    REQUIRE(w.state()   == WkState::Hovered);
    REQUIRE(w.label()   == "Volume");
    REQUIRE(w.width()   == 200);
    REQUIRE(w.height()  == 32);
    REQUIRE(w.visible() == false);
    REQUIRE(w.enabled() == false);
}

TEST_CASE("WidgetKitV1 add and visibleCount", "[Editor][S146]") {
    WidgetKitV1 kit;
    kit.addWidget(WkWidget(1, WkWidgetType::Button));
    kit.addWidget(WkWidget(2, WkWidgetType::Label));
    REQUIRE(kit.widgetCount() == 2u);
    REQUIRE(kit.visibleCount() == 2u);
    kit.findWidget(1)->setVisible(false);
    REQUIRE(kit.visibleCount() == 1u);
}

TEST_CASE("WidgetKitV1 setState", "[Editor][S146]") {
    WidgetKitV1 kit;
    kit.addWidget(WkWidget(1, WkWidgetType::Button));
    REQUIRE(kit.setState(1, WkState::Pressed) == true);
    REQUIRE(kit.findWidget(1)->state() == WkState::Pressed);
    REQUIRE(kit.setState(99, WkState::Pressed) == false);
}

TEST_CASE("WidgetKitV1 filterByType", "[Editor][S146]") {
    WidgetKitV1 kit;
    kit.addWidget(WkWidget(1, WkWidgetType::Button));
    kit.addWidget(WkWidget(2, WkWidgetType::Button));
    kit.addWidget(WkWidget(3, WkWidgetType::Label));
    auto buttons = kit.filterByType(WkWidgetType::Button);
    REQUIRE(buttons.size() == 2u);
}

TEST_CASE("WidgetKitV1 duplicate and remove", "[Editor][S146]") {
    WidgetKitV1 kit;
    REQUIRE(kit.addWidget(WkWidget(5, WkWidgetType::Toggle)) == true);
    REQUIRE(kit.addWidget(WkWidget(5, WkWidgetType::Toggle)) == false);
    REQUIRE(kit.removeWidget(5) == true);
    REQUIRE(kit.widgetCount()   == 0u);
}

// ── TooltipSystemV1 ───────────────────────────────────────────────────────────

TEST_CASE("TtpPosition names", "[Editor][S146]") {
    REQUIRE(std::string(ttpPositionName(TtpPosition::Top))    == "Top");
    REQUIRE(std::string(ttpPositionName(TtpPosition::Bottom)) == "Bottom");
    REQUIRE(std::string(ttpPositionName(TtpPosition::Left))   == "Left");
    REQUIRE(std::string(ttpPositionName(TtpPosition::Right))  == "Right");
    REQUIRE(std::string(ttpPositionName(TtpPosition::Auto))   == "Auto");
}

TEST_CASE("TtpTrigger names", "[Editor][S146]") {
    REQUIRE(std::string(ttpTriggerName(TtpTrigger::Hover))  == "Hover");
    REQUIRE(std::string(ttpTriggerName(TtpTrigger::Focus))  == "Focus");
    REQUIRE(std::string(ttpTriggerName(TtpTrigger::Click))  == "Click");
    REQUIRE(std::string(ttpTriggerName(TtpTrigger::Manual)) == "Manual");
}

TEST_CASE("TtpEntry defaults", "[Editor][S146]") {
    TtpEntry e(1, 10, "Save the file");
    REQUIRE(e.id()       == 1u);
    REQUIRE(e.targetId() == 10u);
    REQUIRE(e.text()     == "Save the file");
    REQUIRE(e.position() == TtpPosition::Auto);
    REQUIRE(e.trigger()  == TtpTrigger::Hover);
    REQUIRE(e.delay()    == 0.5f);
    REQUIRE(e.visible()  == false);
    REQUIRE(e.enabled()  == true);
}

TEST_CASE("TooltipSystemV1 register and visibleCount", "[Editor][S146]") {
    TooltipSystemV1 sys;
    sys.register_(1, 100, "Tip A");
    sys.register_(2, 200, "Tip B");
    REQUIRE(sys.entryCount()  == 2u);
    REQUIRE(sys.visibleCount() == 0u);
    sys.show(1);
    REQUIRE(sys.visibleCount() == 1u);
    sys.hide(1);
    REQUIRE(sys.visibleCount() == 0u);
}

TEST_CASE("TooltipSystemV1 duplicate and unregister", "[Editor][S146]") {
    TooltipSystemV1 sys;
    REQUIRE(sys.register_(1, 5, "tip") == true);
    REQUIRE(sys.register_(1, 5, "tip") == false);
    REQUIRE(sys.unregister_(1) == true);
    REQUIRE(sys.entryCount()   == 0u);
    REQUIRE(sys.unregister_(1) == false);
}

TEST_CASE("TooltipSystemV1 show and hide missing", "[Editor][S146]") {
    TooltipSystemV1 sys;
    REQUIRE(sys.show(99) == false);
    REQUIRE(sys.hide(99) == false);
}

// ── TabBarV1 ─────────────────────────────────────────────────────────────────

TEST_CASE("TbOrientation names", "[Editor][S146]") {
    REQUIRE(std::string(tbOrientationName(TbOrientation::Top))    == "Top");
    REQUIRE(std::string(tbOrientationName(TbOrientation::Bottom)) == "Bottom");
    REQUIRE(std::string(tbOrientationName(TbOrientation::Left))   == "Left");
    REQUIRE(std::string(tbOrientationName(TbOrientation::Right))  == "Right");
}

TEST_CASE("TbClosePolicy names", "[Editor][S146]") {
    REQUIRE(std::string(tbClosePolicyName(TbClosePolicy::NoClose))     == "NoClose");
    REQUIRE(std::string(tbClosePolicyName(TbClosePolicy::CloseButton)) == "CloseButton");
    REQUIRE(std::string(tbClosePolicyName(TbClosePolicy::DoubleClick)) == "DoubleClick");
    REQUIRE(std::string(tbClosePolicyName(TbClosePolicy::MiddleClick)) == "MiddleClick");
}

TEST_CASE("TbTab defaults", "[Editor][S146]") {
    TbTab t(1, "Scene");
    REQUIRE(t.id()          == 1u);
    REQUIRE(t.label()       == "Scene");
    REQUIRE(t.tooltip()     == "");
    REQUIRE(t.orientation() == TbOrientation::Top);
    REQUIRE(t.closeable()   == true);
    REQUIRE(t.active()      == false);
    REQUIRE(t.enabled()     == true);
}

TEST_CASE("TabBarV1 defaults and setters", "[Editor][S146]") {
    TabBarV1 bar;
    REQUIRE(bar.closePolicy()  == TbClosePolicy::CloseButton);
    REQUIRE(bar.orientation()  == TbOrientation::Top);
    bar.setClosePolicy(TbClosePolicy::NoClose);
    bar.setOrientation(TbOrientation::Bottom);
    REQUIRE(bar.closePolicy()  == TbClosePolicy::NoClose);
    REQUIRE(bar.orientation()  == TbOrientation::Bottom);
}

TEST_CASE("TabBarV1 setActive exclusive", "[Editor][S146]") {
    TabBarV1 bar;
    bar.addTab(TbTab(1, "A"));
    bar.addTab(TbTab(2, "B"));
    bar.addTab(TbTab(3, "C"));
    REQUIRE(bar.setActive(2) == true);
    REQUIRE(bar.activeTab()  == 2u);
    REQUIRE(bar.findTab(1)->active() == false);
    REQUIRE(bar.findTab(3)->active() == false);
    REQUIRE(bar.setActive(99) == false);
}

TEST_CASE("TabBarV1 closeTab and tabCount", "[Editor][S146]") {
    TabBarV1 bar;
    bar.addTab(TbTab(1, "A"));
    bar.addTab(TbTab(2, "B"));
    REQUIRE(bar.tabCount()  == 2u);
    REQUIRE(bar.closeTab(1) == true);
    REQUIRE(bar.tabCount()  == 1u);
    REQUIRE(bar.closeTab(1) == false);
}
