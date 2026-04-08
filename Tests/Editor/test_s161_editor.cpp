// S161 editor tests: AINavMeshEditorV1, AISpawnEditorV1, AIGoalEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── AINavMeshEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("AINavMeshEditorV1 basic", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    REQUIRE(nm.areaCount() == 0);
    REQUIRE(nm.cellSize() == Catch::Approx(0.3f));
    REQUIRE(nm.agentHeight() == Catch::Approx(2.0f));
}

TEST_CASE("AINavMeshEditorV1 areas", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    AnvArea a1(1, "ground"); a1.setAreaType(AnvAreaType::Walkable); a1.setCost(1.0f);
    AnvArea a2(2, "water"); a2.setAreaType(AnvAreaType::Swim); a2.setCost(3.0f);
    REQUIRE(nm.addArea(a1));
    REQUIRE(nm.addArea(a2));
    REQUIRE_FALSE(nm.addArea(a1));
    REQUIRE(nm.areaCount() == 2);
    REQUIRE(nm.findArea(2)->cost() == Catch::Approx(3.0f));
    REQUIRE(nm.removeArea(2));
    REQUIRE(nm.areaCount() == 1);
}

TEST_CASE("AnvAreaType names", "[Editor][S161]") {
    REQUIRE(std::string(anvAreaTypeName(AnvAreaType::Walkable))    == "Walkable");
    REQUIRE(std::string(anvAreaTypeName(AnvAreaType::NotWalkable)) == "NotWalkable");
    REQUIRE(std::string(anvAreaTypeName(AnvAreaType::Jump))        == "Jump");
    REQUIRE(std::string(anvAreaTypeName(AnvAreaType::Swim))        == "Swim");
    REQUIRE(std::string(anvAreaTypeName(AnvAreaType::Crouch))      == "Crouch");
}

// ── AISpawnEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("AISpawnEditorV1 basic", "[Editor][S161]") {
    AISpawnEditorV1 se;
    REQUIRE(se.pointCount() == 0);
    REQUIRE(se.enabledCount() == 0);
}

TEST_CASE("AISpawnEditorV1 spawn points", "[Editor][S161]") {
    AISpawnEditorV1 se;
    AspSpawnPoint p1(1, "wave1"); p1.setMode(AspSpawnMode::Wave); p1.setMaxCount(5);
    AspSpawnPoint p2(2, "cont1"); p2.setMode(AspSpawnMode::Continuous); p2.setEnabled(false);
    REQUIRE(se.addPoint(p1));
    REQUIRE(se.addPoint(p2));
    REQUIRE_FALSE(se.addPoint(p1));
    REQUIRE(se.pointCount() == 2);
    REQUIRE(se.enabledCount() == 1);
    REQUIRE(se.findPoint(1)->maxCount() == 5);
    REQUIRE(se.removePoint(2));
    REQUIRE(se.pointCount() == 1);
}

TEST_CASE("AspSpawnMode names", "[Editor][S161]") {
    REQUIRE(std::string(aspSpawnModeName(AspSpawnMode::OnDemand))   == "OnDemand");
    REQUIRE(std::string(aspSpawnModeName(AspSpawnMode::Wave))       == "Wave");
    REQUIRE(std::string(aspSpawnModeName(AspSpawnMode::Continuous)) == "Continuous");
    REQUIRE(std::string(aspSpawnModeName(AspSpawnMode::Scripted))   == "Scripted");
}

// ── AIGoalEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("AIGoalEditorV1 basic", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    REQUIRE(ge.goalCount() == 0);
    REQUIRE(ge.criticalCount() == 0);
}

TEST_CASE("AIGoalEditorV1 goals", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    AglGoal g1(1, "patrol"); g1.setPriority(AglGoalPriority::Normal); g1.setWeight(1.0f);
    AglGoal g2(2, "attack"); g2.setPriority(AglGoalPriority::Critical); g2.setWeight(5.0f);
    REQUIRE(ge.addGoal(g1));
    REQUIRE(ge.addGoal(g2));
    REQUIRE_FALSE(ge.addGoal(g1));
    REQUIRE(ge.goalCount() == 2);
    REQUIRE(ge.criticalCount() == 1);
    REQUIRE(ge.findGoal(2)->weight() == Catch::Approx(5.0f));
    REQUIRE(ge.removeGoal(1));
    REQUIRE(ge.goalCount() == 1);
}

TEST_CASE("AglGoalPriority names", "[Editor][S161]") {
    REQUIRE(std::string(aglGoalPriorityName(AglGoalPriority::Low))      == "Low");
    REQUIRE(std::string(aglGoalPriorityName(AglGoalPriority::Normal))   == "Normal");
    REQUIRE(std::string(aglGoalPriorityName(AglGoalPriority::High))     == "High");
    REQUIRE(std::string(aglGoalPriorityName(AglGoalPriority::Critical)) == "Critical");
}
