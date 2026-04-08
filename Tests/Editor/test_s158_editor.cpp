// S158 editor tests: CurveLibraryV1, SplineEditorV1, GradientEditorV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── CurveLibraryV1 ────────────────────────────────────────────────────────

TEST_CASE("CurveLibraryV1 basic", "[Editor][S158]") {
    CurveLibraryV1 cl;
    REQUIRE(cl.keyframeCount() == 0);
    REQUIRE(cl.curveType() == ClbCurveType::Linear);
    REQUIRE_FALSE(cl.looping());
}

TEST_CASE("CurveLibraryV1 keyframes", "[Editor][S158]") {
    CurveLibraryV1 cl;
    ClbKeyframe k1(1, 0.0f, 0.0f);
    ClbKeyframe k2(2, 0.5f, 1.0f);
    ClbKeyframe k3(3, 1.0f, 0.0f);
    REQUIRE(cl.addKeyframe(k1));
    REQUIRE(cl.addKeyframe(k2));
    REQUIRE(cl.addKeyframe(k3));
    REQUIRE_FALSE(cl.addKeyframe(k1));
    REQUIRE(cl.keyframeCount() == 3);
    REQUIRE(cl.findKeyframe(2) != nullptr);
    REQUIRE(cl.removeKeyframe(2));
    REQUIRE(cl.keyframeCount() == 2);
}

TEST_CASE("ClbCurveType names", "[Editor][S158]") {
    REQUIRE(std::string(clbCurveTypeName(ClbCurveType::Linear))  == "Linear");
    REQUIRE(std::string(clbCurveTypeName(ClbCurveType::Bezier))  == "Bezier");
    REQUIRE(std::string(clbCurveTypeName(ClbCurveType::Hermite)) == "Hermite");
    REQUIRE(std::string(clbCurveTypeName(ClbCurveType::BSpline)) == "BSpline");
    REQUIRE(std::string(clbCurveTypeName(ClbCurveType::Step))    == "Step");
}

// ── SplineEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("SplineEditorV1 basic", "[Editor][S158]") {
    SplineEditorV1 se;
    REQUIRE(se.pointCount() == 0);
    REQUIRE(se.interp() == SplInterp::CatmullRom);
    REQUIRE_FALSE(se.closed());
}

TEST_CASE("SplineEditorV1 points", "[Editor][S158]") {
    SplineEditorV1 se;
    SplPoint p1(1, 0.0f, 0.0f, 0.0f);
    SplPoint p2(2, 1.0f, 0.0f, 0.0f);
    REQUIRE(se.addPoint(p1));
    REQUIRE(se.addPoint(p2));
    REQUIRE_FALSE(se.addPoint(p1));
    REQUIRE(se.pointCount() == 2);
    REQUIRE(se.findPoint(2) != nullptr);
    se.setClosed(true);
    REQUIRE(se.closed());
    REQUIRE(se.removePoint(2));
    REQUIRE(se.pointCount() == 1);
}

TEST_CASE("SplInterp names", "[Editor][S158]") {
    REQUIRE(std::string(splInterpName(SplInterp::CatmullRom)) == "CatmullRom");
    REQUIRE(std::string(splInterpName(SplInterp::Bezier))     == "Bezier");
    REQUIRE(std::string(splInterpName(SplInterp::Linear))     == "Linear");
    REQUIRE(std::string(splInterpName(SplInterp::BSpline))    == "BSpline");
}

// ── GradientEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("GradientEditorV1 basic", "[Editor][S158]") {
    GradientEditorV1 ge;
    REQUIRE(ge.stopCount() == 0);
    REQUIRE(ge.blendMode() == GrdBlendMode::Linear);
    REQUIRE_FALSE(ge.hdr());
}

TEST_CASE("GradientEditorV1 stops", "[Editor][S158]") {
    GradientEditorV1 ge;
    GrdStop s1(1, 0.0f); s1.setR(0.0f); s1.setG(0.0f); s1.setB(0.0f); s1.setA(1.0f);
    GrdStop s2(2, 1.0f); s2.setR(1.0f); s2.setG(1.0f); s2.setB(1.0f); s2.setA(1.0f);
    REQUIRE(ge.addStop(s1));
    REQUIRE(ge.addStop(s2));
    REQUIRE_FALSE(ge.addStop(s1));
    REQUIRE(ge.stopCount() == 2);
    REQUIRE(ge.findStop(1) != nullptr);
    REQUIRE(ge.removeStop(1));
    REQUIRE(ge.stopCount() == 1);
}

TEST_CASE("GrdBlendMode names", "[Editor][S158]") {
    REQUIRE(std::string(grdBlendModeName(GrdBlendMode::Linear))   == "Linear");
    REQUIRE(std::string(grdBlendModeName(GrdBlendMode::Smooth))   == "Smooth");
    REQUIRE(std::string(grdBlendModeName(GrdBlendMode::Constant)) == "Constant");
    REQUIRE(std::string(grdBlendModeName(GrdBlendMode::Ease))     == "Ease");
}
