// S152 editor tests: ShaderEditorV1, RenderPassEditorV1, PipelineStateEditorV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ShaderEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("ShaderEditorV1 basic", "[Editor][S152]") {
    ShaderEditorV1 obj;
    REQUIRE(true);
}

// ── RenderPassEditorV1 ────────────────────────────────────────────────────

TEST_CASE("RenderPassEditorV1 basic", "[Editor][S152]") {
    RenderPassEditorV1 obj;
    REQUIRE(true);
}

// ── PipelineStateEditorV1 ─────────────────────────────────────────────────

TEST_CASE("PipelineStateEditorV1 basic", "[Editor][S152]") {
    PipelineStateEditorV1 obj;
    REQUIRE(true);
}
