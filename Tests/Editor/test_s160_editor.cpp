// S160 editor tests: TerrainBrushV1, FoliagePainterV1, RoadToolV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── TerrainBrushV1 ────────────────────────────────────────────────────────

TEST_CASE("TerrainBrushV1 basic", "[Editor][S160]") {
    TerrainBrushV1 tb;
    REQUIRE(tb.brushCount() == 0);
    REQUIRE(tb.activeBrush() == 0);
}

TEST_CASE("TerrainBrushV1 brushes", "[Editor][S160]") {
    TerrainBrushV1 tb;
    TbrBrush b1(1, "raise"); b1.setMode(TbrBrushMode::Raise); b1.setRadius(3.0f);
    TbrBrush b2(2, "smooth"); b2.setMode(TbrBrushMode::Smooth);
    REQUIRE(tb.addBrush(b1));
    REQUIRE(tb.addBrush(b2));
    REQUIRE_FALSE(tb.addBrush(b1));
    REQUIRE(tb.brushCount() == 2);
    tb.setActiveBrush(1);
    REQUIRE(tb.activeBrush() == 1);
    REQUIRE(tb.findBrush(1)->radius() == Catch::Approx(3.0f));
    REQUIRE(tb.removeBrush(2));
    REQUIRE(tb.brushCount() == 1);
}

TEST_CASE("TbrBrushMode names", "[Editor][S160]") {
    REQUIRE(std::string(tbrBrushModeName(TbrBrushMode::Raise))   == "Raise");
    REQUIRE(std::string(tbrBrushModeName(TbrBrushMode::Lower))   == "Lower");
    REQUIRE(std::string(tbrBrushModeName(TbrBrushMode::Smooth))  == "Smooth");
    REQUIRE(std::string(tbrBrushModeName(TbrBrushMode::Flatten)) == "Flatten");
    REQUIRE(std::string(tbrBrushModeName(TbrBrushMode::Paint))   == "Paint");
    REQUIRE(std::string(tbrBrushModeName(TbrBrushMode::Stamp))   == "Stamp");
}

// ── FoliagePainterV1 ──────────────────────────────────────────────────────

TEST_CASE("FoliagePainterV1 basic", "[Editor][S160]") {
    FoliagePainterV1 fp;
    REQUIRE(fp.typeCount() == 0);
    REQUIRE(fp.brushRadius() == Catch::Approx(5.0f));
}

TEST_CASE("FoliagePainterV1 types", "[Editor][S160]") {
    FoliagePainterV1 fp;
    FpvFoliageType t1(1, "tree"); t1.setDensity(2.0f); t1.setMode(FpvPlacementMode::Scatter);
    FpvFoliageType t2(2, "grass"); t2.setEnabled(false);
    REQUIRE(fp.addType(t1));
    REQUIRE(fp.addType(t2));
    REQUIRE_FALSE(fp.addType(t1));
    REQUIRE(fp.typeCount() == 2);
    REQUIRE(fp.findType(1)->density() == Catch::Approx(2.0f));
    fp.setBrushRadius(10.0f);
    REQUIRE(fp.brushRadius() == Catch::Approx(10.0f));
    REQUIRE(fp.removeType(2));
    REQUIRE(fp.typeCount() == 1);
}

TEST_CASE("FpvPlacementMode names", "[Editor][S160]") {
    REQUIRE(std::string(fpvPlacementModeName(FpvPlacementMode::Scatter)) == "Scatter");
    REQUIRE(std::string(fpvPlacementModeName(FpvPlacementMode::Align))   == "Align");
    REQUIRE(std::string(fpvPlacementModeName(FpvPlacementMode::Cluster)) == "Cluster");
    REQUIRE(std::string(fpvPlacementModeName(FpvPlacementMode::Grid))    == "Grid");
}

// ── RoadToolV1 ────────────────────────────────────────────────────────────

TEST_CASE("RoadToolV1 basic", "[Editor][S160]") {
    RoadToolV1 rt;
    REQUIRE(rt.segmentCount() == 0);
    REQUIRE(rt.snapping());
}

TEST_CASE("RoadToolV1 segments", "[Editor][S160]") {
    RoadToolV1 rt;
    RtlSegment s1(1); s1.setSurface(RtlRoadSurface::Asphalt); s1.setWidth(4.0f);
    RtlSegment s2(2); s2.setSurface(RtlRoadSurface::Gravel);
    REQUIRE(rt.addSegment(s1));
    REQUIRE(rt.addSegment(s2));
    REQUIRE_FALSE(rt.addSegment(s1));
    REQUIRE(rt.segmentCount() == 2);
    REQUIRE(rt.findSegment(1)->width() == Catch::Approx(4.0f));
    rt.setSnapping(false);
    REQUIRE_FALSE(rt.snapping());
    REQUIRE(rt.removeSegment(2));
    REQUIRE(rt.segmentCount() == 1);
}

TEST_CASE("RtlRoadSurface names", "[Editor][S160]") {
    REQUIRE(std::string(rtlRoadSurfaceName(RtlRoadSurface::Asphalt))     == "Asphalt");
    REQUIRE(std::string(rtlRoadSurfaceName(RtlRoadSurface::Gravel))      == "Gravel");
    REQUIRE(std::string(rtlRoadSurfaceName(RtlRoadSurface::Dirt))        == "Dirt");
    REQUIRE(std::string(rtlRoadSurfaceName(RtlRoadSurface::Cobblestone)) == "Cobblestone");
    REQUIRE(std::string(rtlRoadSurfaceName(RtlRoadSurface::Sand))        == "Sand");
}
