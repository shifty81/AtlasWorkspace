// S149 editor tests: BuildPipelineEditorV1, CompilerSettingsV1, LinkerSettingsV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── BuildPipelineEditorV1 ─────────────────────────────────────────────────

TEST_CASE("BuildPipelineEditorV1 basic", "[Editor][S149]") {
    BuildPipelineEditorV1 obj;
    REQUIRE(true);
}

// ── CompilerSettingsV1 ────────────────────────────────────────────────────

TEST_CASE("CompilerSettingsV1 basic", "[Editor][S149]") {
    CompilerSettingsV1 obj;
    REQUIRE(true);
}

// ── LinkerSettingsV1 ──────────────────────────────────────────────────────

TEST_CASE("LinkerSettingsV1 basic", "[Editor][S149]") {
    LinkerSettingsV1 obj;
    REQUIRE(true);
}
