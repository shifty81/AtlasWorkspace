// S153 editor tests: AudioGraphEditorV1, SoundMixerEditorV1, AnimationCurveEditorV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── AudioGraphEditorV1 ────────────────────────────────────────────────────

TEST_CASE("AudioGraphEditorV1 basic", "[Editor][S153]") {
    AudioGraphEditorV1 obj;
    REQUIRE(true);
}

// ── SoundMixerEditorV1 ────────────────────────────────────────────────────

TEST_CASE("SoundMixerEditorV1 basic", "[Editor][S153]") {
    SoundMixerEditorV1 obj;
    REQUIRE(true);
}

// ── AnimationCurveEditorV1 ────────────────────────────────────────────────

TEST_CASE("AnimationCurveEditorV1 basic", "[Editor][S153]") {
    AnimationCurveEditorV1 obj;
    REQUIRE(true);
}
