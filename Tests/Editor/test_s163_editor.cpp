// S163 editor tests: LightmapEditorV1, ReflectionProbeEditorV1, ShadowCasterEditorV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── LightmapEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("LightmapEditorV1 basic", "[Editor][S163]") {
    LightmapEditorV1 lm;
    REQUIRE(lm.objectCount() == 0);
    REQUIRE(lm.contributeCount() == 0);
    REQUIRE_FALSE(lm.baked());
}

TEST_CASE("LightmapEditorV1 objects", "[Editor][S163]") {
    LightmapEditorV1 lm;
    LmpObject o1(1, "floor"); o1.setBakeMode(LmpBakeMode::Baked); o1.setResolution(LmpResolution::R512);
    LmpObject o2(2, "lamp"); o2.setBakeMode(LmpBakeMode::Realtime); o2.setContribute(false);
    REQUIRE(lm.addObject(o1));
    REQUIRE(lm.addObject(o2));
    REQUIRE_FALSE(lm.addObject(o1));
    REQUIRE(lm.objectCount() == 2);
    REQUIRE(lm.contributeCount() == 1);
    REQUIRE(lm.findObject(1)->bakeMode() == LmpBakeMode::Baked);
    REQUIRE(lm.removeObject(2));
    REQUIRE(lm.objectCount() == 1);
}

TEST_CASE("LmpBakeMode names", "[Editor][S163]") {
    REQUIRE(std::string(lmpBakeModeName(LmpBakeMode::Baked))    == "Baked");
    REQUIRE(std::string(lmpBakeModeName(LmpBakeMode::Mixed))    == "Mixed");
    REQUIRE(std::string(lmpBakeModeName(LmpBakeMode::Realtime)) == "Realtime");
}

TEST_CASE("LmpResolution names", "[Editor][S163]") {
    REQUIRE(std::string(lmpResolutionName(LmpResolution::R64))   == "64");
    REQUIRE(std::string(lmpResolutionName(LmpResolution::R128))  == "128");
    REQUIRE(std::string(lmpResolutionName(LmpResolution::R256))  == "256");
    REQUIRE(std::string(lmpResolutionName(LmpResolution::R512))  == "512");
    REQUIRE(std::string(lmpResolutionName(LmpResolution::R1024)) == "1024");
    REQUIRE(std::string(lmpResolutionName(LmpResolution::R2048)) == "2048");
}

// ── ReflectionProbeEditorV1 ───────────────────────────────────────────────

TEST_CASE("ReflectionProbeEditorV1 basic", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    REQUIRE(rp.probeCount() == 0);
    REQUIRE(rp.enabledCount() == 0);
}

TEST_CASE("ReflectionProbeEditorV1 probes", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    RpvProbe p1(1, "lobby"); p1.setRefreshMode(RpvRefreshMode::Baked); p1.setResolution(256);
    RpvProbe p2(2, "outdoor"); p2.setEnabled(false);
    REQUIRE(rp.addProbe(p1));
    REQUIRE(rp.addProbe(p2));
    REQUIRE_FALSE(rp.addProbe(p1));
    REQUIRE(rp.probeCount() == 2);
    REQUIRE(rp.enabledCount() == 1);
    REQUIRE(rp.findProbe(1)->resolution() == 256u);
    REQUIRE(rp.removeProbe(2));
    REQUIRE(rp.probeCount() == 1);
}

TEST_CASE("RpvRefreshMode names", "[Editor][S163]") {
    REQUIRE(std::string(rpvRefreshModeName(RpvRefreshMode::Baked))      == "Baked");
    REQUIRE(std::string(rpvRefreshModeName(RpvRefreshMode::OnAwake))    == "OnAwake");
    REQUIRE(std::string(rpvRefreshModeName(RpvRefreshMode::EveryFrame)) == "EveryFrame");
    REQUIRE(std::string(rpvRefreshModeName(RpvRefreshMode::Custom))     == "Custom");
}

// ── ShadowCasterEditorV1 ──────────────────────────────────────────────────

TEST_CASE("ShadowCasterEditorV1 basic", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    REQUIRE(sc.casterCount() == 0);
    REQUIRE(sc.castingShadowCount() == 0);
}

TEST_CASE("ShadowCasterEditorV1 casters", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    ScvCaster c1(1, "wall"); c1.setQuality(ScvShadowQuality::High); c1.setCastShadow(true);
    ScvCaster c2(2, "glass"); c2.setCastShadow(false);
    REQUIRE(sc.addCaster(c1));
    REQUIRE(sc.addCaster(c2));
    REQUIRE_FALSE(sc.addCaster(c1));
    REQUIRE(sc.casterCount() == 2);
    REQUIRE(sc.castingShadowCount() == 1);
    REQUIRE(sc.findCaster(1)->quality() == ScvShadowQuality::High);
    REQUIRE(sc.removeCaster(2));
    REQUIRE(sc.casterCount() == 1);
}

TEST_CASE("ScvShadowQuality names", "[Editor][S163]") {
    REQUIRE(std::string(scvShadowQualityName(ScvShadowQuality::Low))    == "Low");
    REQUIRE(std::string(scvShadowQualityName(ScvShadowQuality::Medium)) == "Medium");
    REQUIRE(std::string(scvShadowQualityName(ScvShadowQuality::High))   == "High");
    REQUIRE(std::string(scvShadowQualityName(ScvShadowQuality::Ultra))  == "Ultra");
}
