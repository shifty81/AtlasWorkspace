// S157 editor tests: EventTimelineV1, StateGraphV1, TriggerEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── EventTimelineV1 ───────────────────────────────────────────────────────

TEST_CASE("EventTimelineV1 basic", "[Editor][S157]") {
    EventTimelineV1 tl;
    REQUIRE(tl.eventCount() == 0);
    REQUIRE(tl.duration() == Catch::Approx(10.0f));
    REQUIRE_FALSE(tl.playing());
}

TEST_CASE("EventTimelineV1 add/remove events", "[Editor][S157]") {
    EventTimelineV1 tl;
    EtlEvent e1(1, "start");
    e1.setType(EtlEventType::Trigger);
    e1.setTime(1.0f);
    REQUIRE(tl.addEvent(e1));
    REQUIRE_FALSE(tl.addEvent(e1));
    REQUIRE(tl.eventCount() == 1);
    REQUIRE(tl.findEvent(1) != nullptr);
    REQUIRE(tl.removeEvent(1));
    REQUIRE(tl.eventCount() == 0);
}

TEST_CASE("EventTimelineV1 eventsInRange", "[Editor][S157]") {
    EventTimelineV1 tl;
    tl.addEvent(EtlEvent(1, "a")); // time=0 default
    EtlEvent e2(2, "b"); e2.setTime(5.0f); tl.addEvent(e2);
    EtlEvent e3(3, "c"); e3.setTime(9.0f); tl.addEvent(e3);
    auto inRange = tl.eventsInRange(4.0f, 9.5f);
    REQUIRE(inRange.size() == 2);
}

TEST_CASE("EtlEventType names", "[Editor][S157]") {
    REQUIRE(std::string(etlEventTypeName(EtlEventType::Trigger))    == "Trigger");
    REQUIRE(std::string(etlEventTypeName(EtlEventType::State))      == "State");
    REQUIRE(std::string(etlEventTypeName(EtlEventType::Action))     == "Action");
    REQUIRE(std::string(etlEventTypeName(EtlEventType::Transition)) == "Transition");
    REQUIRE(std::string(etlEventTypeName(EtlEventType::Note))       == "Note");
}

// ── StateGraphV1 ──────────────────────────────────────────────────────────

TEST_CASE("StateGraphV1 basic", "[Editor][S157]") {
    StateGraphV1 sg;
    REQUIRE(sg.nodeCount() == 0);
    REQUIRE(sg.edgeCount() == 0);
}

TEST_CASE("StateGraphV1 nodes and edges", "[Editor][S157]") {
    StateGraphV1 sg;
    SgNode entry(1, "Entry"); entry.setKind(SgNodeKind::Entry); entry.setInitial(true);
    SgNode idle(2, "Idle");
    SgNode exit(3, "Exit"); exit.setKind(SgNodeKind::Exit); exit.setFinal(true);
    REQUIRE(sg.addNode(entry));
    REQUIRE(sg.addNode(idle));
    REQUIRE(sg.addNode(exit));
    REQUIRE_FALSE(sg.addNode(idle));
    REQUIRE(sg.nodeCount() == 3);
    SgEdge e1(10, 1, 2); e1.setCondition("always");
    REQUIRE(sg.addEdge(e1));
    REQUIRE(sg.edgeCount() == 1);
    REQUIRE(sg.findNode(2) != nullptr);
    REQUIRE(sg.removeNode(2));
    REQUIRE(sg.nodeCount() == 2);
}

TEST_CASE("SgNodeKind names", "[Editor][S157]") {
    REQUIRE(std::string(sgNodeKindName(SgNodeKind::State))  == "State");
    REQUIRE(std::string(sgNodeKindName(SgNodeKind::Choice)) == "Choice");
    REQUIRE(std::string(sgNodeKindName(SgNodeKind::Fork))   == "Fork");
    REQUIRE(std::string(sgNodeKindName(SgNodeKind::Join))   == "Join");
    REQUIRE(std::string(sgNodeKindName(SgNodeKind::Entry))  == "Entry");
    REQUIRE(std::string(sgNodeKindName(SgNodeKind::Exit))   == "Exit");
}

// ── TriggerEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("TriggerEditorV1 basic", "[Editor][S157]") {
    TriggerEditorV1 te;
    REQUIRE(te.conditionCount() == 0);
    REQUIRE(te.enabled());
}

TEST_CASE("TriggerEditorV1 conditions", "[Editor][S157]") {
    TriggerEditorV1 te;
    TevCondition c1(1); c1.setVariable("health"); c1.setOp(TevCondOp::LessThan); c1.setValue("50");
    REQUIRE(te.addCondition(c1));
    REQUIRE_FALSE(te.addCondition(c1));
    REQUIRE(te.conditionCount() == 1);
    REQUIRE(te.findCondition(1) != nullptr);
    REQUIRE(te.removeCondition(1));
    REQUIRE(te.conditionCount() == 0);
}

TEST_CASE("TevCondOp names", "[Editor][S157]") {
    REQUIRE(std::string(tevCondOpName(TevCondOp::Equals))      == "Equals");
    REQUIRE(std::string(tevCondOpName(TevCondOp::NotEquals))   == "NotEquals");
    REQUIRE(std::string(tevCondOpName(TevCondOp::GreaterThan)) == "GreaterThan");
    REQUIRE(std::string(tevCondOpName(TevCondOp::LessThan))    == "LessThan");
    REQUIRE(std::string(tevCondOpName(TevCondOp::Contains))    == "Contains");
}
