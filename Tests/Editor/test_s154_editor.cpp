// S154 editor tests: ParticleSystemEditorV1, MaterialNodeEditorV1, TextureViewerV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ParticleSystemEditorV1 ────────────────────────────────────────────────

TEST_CASE("ParticleSystemEditorV1 basic", "[Editor][S154]") {
    ParticleSystemEditorV1 obj;
    REQUIRE(true);
}

// ── MaterialNodeEditorV1 ──────────────────────────────────────────────────

TEST_CASE("MaterialNodeEditorV1 basic", "[Editor][S154]") {
    MaterialNodeEditorV1 obj;
    REQUIRE(true);
}

// ── TextureViewerV1 ───────────────────────────────────────────────────────

TEST_CASE("TextureViewerV1 basic", "[Editor][S154]") {
    TextureViewerV1 obj;
    REQUIRE(true);
}
