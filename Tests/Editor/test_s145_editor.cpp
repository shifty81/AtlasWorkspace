// S145 editor tests: LoggingRouteV1, AIDebugPathV1, CodexSnippetMirror
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── LoggingRouteV1 ────────────────────────────────────────────────────────────

TEST_CASE("LrDestination names", "[Editor][S145]") {
    REQUIRE(std::string(lrDestinationName(LrDestination::Console)) == "Console");
    REQUIRE(std::string(lrDestinationName(LrDestination::File))    == "File");
    REQUIRE(std::string(lrDestinationName(LrDestination::Network)) == "Network");
    REQUIRE(std::string(lrDestinationName(LrDestination::Memory))  == "Memory");
}

TEST_CASE("LrFilterMode names", "[Editor][S145]") {
    REQUIRE(std::string(lrFilterModeName(LrFilterMode::Include))     == "Include");
    REQUIRE(std::string(lrFilterModeName(LrFilterMode::Exclude))     == "Exclude");
    REQUIRE(std::string(lrFilterModeName(LrFilterMode::Passthrough)) == "Passthrough");
}

TEST_CASE("LrRoute defaults", "[Editor][S145]") {
    LrRoute r(1, "console-route", LrDestination::Console);
    REQUIRE(r.id()           == 1u);
    REQUIRE(r.name()         == "console-route");
    REQUIRE(r.destination()  == LrDestination::Console);
    REQUIRE(r.filterMode()   == LrFilterMode::Passthrough);
    REQUIRE(r.minLevel()     == LogLevel::Trace);
    REQUIRE(r.enabled()      == true);
    REQUIRE(r.messageCount() == 0);
}

TEST_CASE("LoggingRouteV1 add and enabledCount", "[Editor][S145]") {
    LoggingRouteV1 router;
    router.addRoute(LrRoute(1, "r1", LrDestination::Console));
    router.addRoute(LrRoute(2, "r2", LrDestination::File));
    REQUIRE(router.routeCount()   == 2u);
    REQUIRE(router.enabledCount() == 2u);
    router.findRoute(1)->setEnabled(false);
    REQUIRE(router.enabledCount() == 1u);
}

TEST_CASE("LoggingRouteV1 dispatch increments messageCount", "[Editor][S145]") {
    LoggingRouteV1 router;
    LrRoute r(1, "all", LrDestination::Memory);
    r.setMinLevel(LogLevel::Info);
    router.addRoute(r);
    router.dispatch(LogLevel::Debug, "debug msg");
    REQUIRE(router.findRoute(1)->messageCount() == 0);
    router.dispatch(LogLevel::Info,  "info msg");
    router.dispatch(LogLevel::Error, "error msg");
    REQUIRE(router.findRoute(1)->messageCount() == 2);
}

TEST_CASE("LoggingRouteV1 disabled route not counted", "[Editor][S145]") {
    LoggingRouteV1 router;
    LrRoute r(1, "r", LrDestination::Console);
    r.setEnabled(false);
    router.addRoute(r);
    router.dispatch(LogLevel::Fatal, "msg");
    REQUIRE(router.findRoute(1)->messageCount() == 0);
}

TEST_CASE("LoggingRouteV1 duplicate and remove", "[Editor][S145]") {
    LoggingRouteV1 router;
    REQUIRE(router.addRoute(LrRoute(5, "r", LrDestination::File)) == true);
    REQUIRE(router.addRoute(LrRoute(5, "r", LrDestination::File)) == false);
    REQUIRE(router.removeRoute(5) == true);
    REQUIRE(router.routeCount()   == 0u);
    REQUIRE(router.removeRoute(5) == false);
}

TEST_CASE("LoggingRouteV1 incrementMessageCount", "[Editor][S145]") {
    LoggingRouteV1 router;
    router.addRoute(LrRoute(1, "r", LrDestination::Memory));
    router.findRoute(1)->incrementMessageCount();
    router.findRoute(1)->incrementMessageCount();
    REQUIRE(router.findRoute(1)->messageCount() == 2);
}

// ── AIDebugPathV1 ─────────────────────────────────────────────────────────────

TEST_CASE("AiDbgPathType names", "[Editor][S145]") {
    REQUIRE(std::string(aiDbgPathTypeName(AiDbgPathType::NavMesh))  == "NavMesh");
    REQUIRE(std::string(aiDbgPathTypeName(AiDbgPathType::Waypoint)) == "Waypoint");
    REQUIRE(std::string(aiDbgPathTypeName(AiDbgPathType::Smooth))   == "Smooth");
    REQUIRE(std::string(aiDbgPathTypeName(AiDbgPathType::Raw))      == "Raw");
}

TEST_CASE("AiDbgPathStatus names", "[Editor][S145]") {
    REQUIRE(std::string(aiDbgPathStatusName(AiDbgPathStatus::Idle))      == "Idle");
    REQUIRE(std::string(aiDbgPathStatusName(AiDbgPathStatus::Computing)) == "Computing");
    REQUIRE(std::string(aiDbgPathStatusName(AiDbgPathStatus::Valid))     == "Valid");
    REQUIRE(std::string(aiDbgPathStatusName(AiDbgPathStatus::Failed))    == "Failed");
}

TEST_CASE("AiDbgWaypoint constructor", "[Editor][S145]") {
    AiDbgWaypoint wp(1, 1.0f, 2.0f, 3.0f);
    REQUIRE(wp.id()      == 1u);
    REQUIRE(wp.x()       == 1.0f);
    REQUIRE(wp.y()       == 2.0f);
    REQUIRE(wp.z()       == 3.0f);
    REQUIRE(wp.reached() == false);
}

TEST_CASE("AiDbgPath waypoints and reachedCount", "[Editor][S145]") {
    AiDbgPath path(1);
    REQUIRE(path.type()          == AiDbgPathType::NavMesh);
    REQUIRE(path.status()        == AiDbgPathStatus::Idle);
    REQUIRE(path.waypointCount() == 0u);
    AiDbgWaypoint w1(1, 0, 0, 0);
    AiDbgWaypoint w2(2, 1, 0, 0);
    w1.setReached(true);
    path.addWaypoint(w1);
    path.addWaypoint(w2);
    REQUIRE(path.waypointCount() == 2u);
    REQUIRE(path.reachedCount()  == 1u);
}

TEST_CASE("AiDbgPath duplicate waypoint", "[Editor][S145]") {
    AiDbgPath path(1);
    REQUIRE(path.addWaypoint(AiDbgWaypoint(1, 0, 0, 0)) == true);
    REQUIRE(path.addWaypoint(AiDbgWaypoint(1, 0, 0, 0)) == false);
}

TEST_CASE("AiDbgPath clearWaypoints", "[Editor][S145]") {
    AiDbgPath path(1);
    path.addWaypoint(AiDbgWaypoint(1, 0, 0, 0));
    path.clearWaypoints();
    REQUIRE(path.waypointCount() == 0u);
}

TEST_CASE("AIDebugPathV1 add and setStatus", "[Editor][S145]") {
    AIDebugPathV1 debug;
    debug.addPath(AiDbgPath(1));
    REQUIRE(debug.pathCount() == 1u);
    REQUIRE(debug.setStatus(1, AiDbgPathStatus::Valid) == true);
    REQUIRE(debug.findPath(1)->status() == AiDbgPathStatus::Valid);
    REQUIRE(debug.setStatus(99, AiDbgPathStatus::Failed) == false);
}

// ── CodexSnippetMirror ────────────────────────────────────────────────────────

TEST_CASE("CsmLanguage names", "[Editor][S145]") {
    REQUIRE(std::string(csmLanguageName(CsmLanguage::Cpp))    == "Cpp");
    REQUIRE(std::string(csmLanguageName(CsmLanguage::Python)) == "Python");
    REQUIRE(std::string(csmLanguageName(CsmLanguage::CSharp)) == "CSharp");
    REQUIRE(std::string(csmLanguageName(CsmLanguage::Lua))    == "Lua");
    REQUIRE(std::string(csmLanguageName(CsmLanguage::GLSL))   == "GLSL");
}

TEST_CASE("CsmSyncState names", "[Editor][S145]") {
    REQUIRE(std::string(csmSyncStateName(CsmSyncState::Synced))   == "Synced");
    REQUIRE(std::string(csmSyncStateName(CsmSyncState::Modified)) == "Modified");
    REQUIRE(std::string(csmSyncStateName(CsmSyncState::Stale))    == "Stale");
    REQUIRE(std::string(csmSyncStateName(CsmSyncState::Conflict)) == "Conflict");
}

TEST_CASE("CsmSnippet defaults and lineCount", "[Editor][S145]") {
    CsmSnippet s(1, "hello");
    REQUIRE(s.id()        == 1u);
    REQUIRE(s.title()     == "hello");
    REQUIRE(s.language()  == CsmLanguage::Cpp);
    REQUIRE(s.syncState() == CsmSyncState::Synced);
    REQUIRE(s.code()      == "");
    REQUIRE(s.lineCount() == 0);
    s.setCode("int main() {\n  return 0;\n}");
    REQUIRE(s.lineCount() == 3);
}

TEST_CASE("CsmSnippet tags", "[Editor][S145]") {
    CsmSnippet s(1, "sort");
    s.addTag("algorithm");
    s.addTag("performance");
    REQUIRE(s.tags().size() == 2u);
    REQUIRE(s.tags()[0]     == "algorithm");
}

TEST_CASE("CodexSnippetMirror add and filterByLanguage", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CsmSnippet s1(1, "cpp1"); s1.setLanguage(CsmLanguage::Cpp);
    CsmSnippet s2(2, "py1");  s2.setLanguage(CsmLanguage::Python);
    CsmSnippet s3(3, "cpp2"); s3.setLanguage(CsmLanguage::Cpp);
    mirror.addSnippet(s1); mirror.addSnippet(s2); mirror.addSnippet(s3);
    REQUIRE(mirror.snippetCount() == 3u);
    auto cpp = mirror.filterByLanguage(CsmLanguage::Cpp);
    REQUIRE(cpp.size() == 2u);
}

TEST_CASE("CodexSnippetMirror markStale", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    mirror.addSnippet(CsmSnippet(1, "s1"));
    REQUIRE(mirror.markStale(1) == true);
    REQUIRE(mirror.findSnippet(1)->syncState() == CsmSyncState::Stale);
    REQUIRE(mirror.markStale(99) == false);
}
