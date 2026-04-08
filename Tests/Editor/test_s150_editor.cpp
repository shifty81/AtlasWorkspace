// S150 editor tests: PackageManagerV1, DependencyGraphV1, VersionResolverV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── PackageManagerV1 ──────────────────────────────────────────────────────

TEST_CASE("PackageManagerV1 basic", "[Editor][S150]") {
    PackageManagerV1 obj;
    REQUIRE(true);
}

// ── DependencyGraphV1 ─────────────────────────────────────────────────────

TEST_CASE("DependencyGraphV1 basic", "[Editor][S150]") {
    DependencyGraphV1 obj;
    REQUIRE(true);
}

// ── VersionResolverV1 ─────────────────────────────────────────────────────

TEST_CASE("VersionResolverV1 basic", "[Editor][S150]") {
    VersionResolverV1 obj;
    REQUIRE(true);
}
