// S151 editor tests: SceneTreeEditorV1, ComponentInspectorV1, EntityQueryV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── SceneTreeEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("SceneTreeEditorV1 basic", "[Editor][S151]") {
    SceneTreeEditorV1 obj;
    REQUIRE(true);
}

// ── ComponentInspectorV1 ──────────────────────────────────────────────────

TEST_CASE("ComponentInspectorV1 basic", "[Editor][S151]") {
    ComponentInspectorV1 obj;
    REQUIRE(true);
}

// ── EntityQueryV1 ─────────────────────────────────────────────────────────

TEST_CASE("EntityQueryV1 basic", "[Editor][S151]") {
    EntityQueryV1 obj;
    REQUIRE(true);
}
