// S162 editor tests: NetworkSyncEditorV1, RpcInspectorV1, LagCompEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── NetworkSyncEditorV1 ───────────────────────────────────────────────────

TEST_CASE("NetworkSyncEditorV1 basic", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    REQUIRE(ns.varCount() == 0);
    REQUIRE(ns.interpolatedCount() == 0);
}

TEST_CASE("NetworkSyncEditorV1 vars", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    NsvVar v1(1, "position"); v1.setMode(NsvSyncMode::Unreliable); v1.setInterpolated(true);
    NsvVar v2(2, "health"); v2.setMode(NsvSyncMode::Reliable); v2.setRate(10.0f);
    REQUIRE(ns.addVar(v1));
    REQUIRE(ns.addVar(v2));
    REQUIRE_FALSE(ns.addVar(v1));
    REQUIRE(ns.varCount() == 2);
    REQUIRE(ns.interpolatedCount() == 1);
    REQUIRE(ns.findVar(2)->rate() == Catch::Approx(10.0f));
    REQUIRE(ns.removeVar(1));
    REQUIRE(ns.varCount() == 1);
}

TEST_CASE("NsvSyncMode names", "[Editor][S162]") {
    REQUIRE(std::string(nsvSyncModeName(NsvSyncMode::Reliable))            == "Reliable");
    REQUIRE(std::string(nsvSyncModeName(NsvSyncMode::Unreliable))          == "Unreliable");
    REQUIRE(std::string(nsvSyncModeName(NsvSyncMode::ReliableOrdered))     == "ReliableOrdered");
    REQUIRE(std::string(nsvSyncModeName(NsvSyncMode::UnreliableSequenced)) == "UnreliableSequenced");
}

// ── RpcInspectorV1 ────────────────────────────────────────────────────────

TEST_CASE("RpcInspectorV1 basic", "[Editor][S162]") {
    RpcInspectorV1 ri;
    REQUIRE(ri.rpcCount() == 0);
    REQUIRE(ri.bufferedCount() == 0);
}

TEST_CASE("RpcInspectorV1 RPCs", "[Editor][S162]") {
    RpcInspectorV1 ri;
    RpiRpc r1(1, "TakeDamage"); r1.setTarget(RpiCallTarget::Server);
    RpiRpc r2(2, "SpawnEffect"); r2.setTarget(RpiCallTarget::AllClients); r2.setBuffered(true);
    REQUIRE(ri.addRpc(r1));
    REQUIRE(ri.addRpc(r2));
    REQUIRE_FALSE(ri.addRpc(r1));
    REQUIRE(ri.rpcCount() == 2);
    REQUIRE(ri.bufferedCount() == 1);
    REQUIRE(ri.findRpc(1) != nullptr);
    REQUIRE(ri.removeRpc(2));
    REQUIRE(ri.rpcCount() == 1);
}

TEST_CASE("RpiCallTarget names", "[Editor][S162]") {
    REQUIRE(std::string(rpiCallTargetName(RpiCallTarget::Server))     == "Server");
    REQUIRE(std::string(rpiCallTargetName(RpiCallTarget::AllClients)) == "AllClients");
    REQUIRE(std::string(rpiCallTargetName(RpiCallTarget::Owner))      == "Owner");
    REQUIRE(std::string(rpiCallTargetName(RpiCallTarget::Others))     == "Others");
}

// ── LagCompEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("LagCompEditorV1 basic", "[Editor][S162]") {
    LagCompEditorV1 lc;
    REQUIRE(lc.entryCount() == 0);
    REQUIRE(lc.enabledCount() == 0);
}

TEST_CASE("LagCompEditorV1 entries", "[Editor][S162]") {
    LagCompEditorV1 lc;
    LcvEntry e1(1, "player"); e1.setStrategy(LcvCompStrategy::Rewind); e1.setMaxLag(0.3f);
    LcvEntry e2(2, "projectile"); e2.setEnabled(false);
    REQUIRE(lc.addEntry(e1));
    REQUIRE(lc.addEntry(e2));
    REQUIRE_FALSE(lc.addEntry(e1));
    REQUIRE(lc.entryCount() == 2);
    REQUIRE(lc.enabledCount() == 1);
    REQUIRE(lc.findEntry(1)->maxLag() == Catch::Approx(0.3f));
    REQUIRE(lc.removeEntry(2));
    REQUIRE(lc.entryCount() == 1);
}

TEST_CASE("LcvCompStrategy names", "[Editor][S162]") {
    REQUIRE(std::string(lcvCompStrategyName(LcvCompStrategy::Rewind))      == "Rewind");
    REQUIRE(std::string(lcvCompStrategyName(LcvCompStrategy::Extrapolate)) == "Extrapolate");
    REQUIRE(std::string(lcvCompStrategyName(LcvCompStrategy::Interpolate)) == "Interpolate");
    REQUIRE(std::string(lcvCompStrategyName(LcvCompStrategy::Predictive))  == "Predictive");
}
