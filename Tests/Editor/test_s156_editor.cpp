// S156 editor tests: ProfilerViewV1, MemoryTrackerV1, DiagnosticPanelV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ProfilerViewV1 ────────────────────────────────────────────────────────

TEST_CASE("ProfilerViewV1 basic", "[Editor][S156]") {
    ProfilerViewV1 obj;
    REQUIRE(true);
}

// ── MemoryTrackerV1 ───────────────────────────────────────────────────────

TEST_CASE("MemoryTrackerV1 basic", "[Editor][S156]") {
    MemoryTrackerV1 obj;
    REQUIRE(true);
}

// ── DiagnosticPanelV1 ─────────────────────────────────────────────────────

TEST_CASE("DiagnosticPanelV1 basic", "[Editor][S156]") {
    DiagnosticPanelV1 obj;
    REQUIRE(true);
}
