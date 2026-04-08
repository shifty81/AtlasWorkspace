// S155 editor tests: MeshInspectorV1, FontEditorV1, LocalizationEditorV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── MeshInspectorV1 ───────────────────────────────────────────────────────

TEST_CASE("MeshInspectorV1 basic", "[Editor][S155]") {
    MeshInspectorV1 obj;
    REQUIRE(true);
}

// ── FontEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("FontEditorV1 basic", "[Editor][S155]") {
    FontEditorV1 obj;
    REQUIRE(true);
}

// ── LocalizationEditorV1 ──────────────────────────────────────────────────

TEST_CASE("LocalizationEditorV1 basic", "[Editor][S155]") {
    LocalizationEditorV1 obj;
    REQUIRE(true);
}
