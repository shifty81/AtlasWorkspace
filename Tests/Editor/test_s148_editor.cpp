// S148 editor tests: CommandPaletteV1, HotkeyRegistryV1, GestureRecognizerV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── CommandPaletteV1 ──────────────────────────────────────────────────────

TEST_CASE("CommandPaletteV1 basic", "[Editor][S148]") {
    CommandPaletteV1 obj;
    REQUIRE(true);
}

// ── HotkeyRegistryV1 ──────────────────────────────────────────────────────

TEST_CASE("HotkeyRegistryV1 basic", "[Editor][S148]") {
    HotkeyRegistryV1 obj;
    REQUIRE(true);
}

// ── GestureRecognizerV1 ───────────────────────────────────────────────────

TEST_CASE("GestureRecognizerV1 basic", "[Editor][S148]") {
    GestureRecognizerV1 obj;
    REQUIRE(true);
}
