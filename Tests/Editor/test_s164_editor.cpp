// S164 editor tests: CinematicEditorV1, SequencerTrackV1, TimelineMarkerV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── CinematicEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("CinematicEditorV1 basic", "[Editor][S164]") {
    CinematicEditorV1 ce;
    REQUIRE(ce.shotCount() == 0);
    REQUIRE(ce.duration() == Catch::Approx(30.0f));
    REQUIRE_FALSE(ce.playing());
}

TEST_CASE("CinematicEditorV1 shots", "[Editor][S164]") {
    CinematicEditorV1 ce;
    CedShot s1(1, "opening"); s1.setShotType(CedShotType::Wide); s1.setStartTime(0.0f); s1.setEndTime(5.0f);
    CedShot s2(2, "closeup"); s2.setShotType(CedShotType::CloseUp); s2.setStartTime(5.0f); s2.setEndTime(8.0f);
    REQUIRE(ce.addShot(s1));
    REQUIRE(ce.addShot(s2));
    REQUIRE_FALSE(ce.addShot(s1));
    REQUIRE(ce.shotCount() == 2);
    REQUIRE(ce.findShot(1)->duration() == Catch::Approx(5.0f));
    ce.setPlaying(true);
    REQUIRE(ce.playing());
    REQUIRE(ce.removeShot(2));
    REQUIRE(ce.shotCount() == 1);
}

TEST_CASE("CedShotType names", "[Editor][S164]") {
    REQUIRE(std::string(cedShotTypeName(CedShotType::Wide))     == "Wide");
    REQUIRE(std::string(cedShotTypeName(CedShotType::Medium))   == "Medium");
    REQUIRE(std::string(cedShotTypeName(CedShotType::CloseUp))  == "CloseUp");
    REQUIRE(std::string(cedShotTypeName(CedShotType::Extreme))  == "Extreme");
    REQUIRE(std::string(cedShotTypeName(CedShotType::Overhead)) == "Overhead");
    REQUIRE(std::string(cedShotTypeName(CedShotType::POV))      == "POV");
}

// ── SequencerTrackV1 ──────────────────────────────────────────────────────

TEST_CASE("SequencerTrackV1 basic", "[Editor][S164]") {
    SequencerTrackV1 st;
    REQUIRE(st.clipCount() == 0);
    REQUIRE(st.trackType() == SqtTrackType::Animation);
    REQUIRE_FALSE(st.muted());
    REQUIRE_FALSE(st.locked());
}

TEST_CASE("SequencerTrackV1 clips", "[Editor][S164]") {
    SequencerTrackV1 st;
    SqtClip c1(1, "walk"); c1.setStartTime(0.0f); c1.setEndTime(2.0f);
    SqtClip c2(2, "run"); c2.setStartTime(2.0f); c2.setEndTime(5.0f);
    REQUIRE(st.addClip(c1));
    REQUIRE(st.addClip(c2));
    REQUIRE_FALSE(st.addClip(c1));
    REQUIRE(st.clipCount() == 2);
    REQUIRE(st.findClip(1)->duration() == Catch::Approx(2.0f));
    st.setMuted(true);
    REQUIRE(st.muted());
    REQUIRE(st.removeClip(2));
    REQUIRE(st.clipCount() == 1);
}

TEST_CASE("SqtTrackType names", "[Editor][S164]") {
    REQUIRE(std::string(sqtTrackTypeName(SqtTrackType::Animation)) == "Animation");
    REQUIRE(std::string(sqtTrackTypeName(SqtTrackType::Audio))     == "Audio");
    REQUIRE(std::string(sqtTrackTypeName(SqtTrackType::Event))     == "Event");
    REQUIRE(std::string(sqtTrackTypeName(SqtTrackType::Camera))    == "Camera");
    REQUIRE(std::string(sqtTrackTypeName(SqtTrackType::Effect))    == "Effect");
    REQUIRE(std::string(sqtTrackTypeName(SqtTrackType::Script))    == "Script");
}

// ── TimelineMarkerV1 ──────────────────────────────────────────────────────

TEST_CASE("TimelineMarkerV1 basic", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    REQUIRE(tm.markerCount() == 0);
}

TEST_CASE("TimelineMarkerV1 markers", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    TlmMarker m1(1, "intro"); m1.setKind(TlmMarkerKind::Cue); m1.setTime(0.0f);
    TlmMarker m2(2, "loop"); m2.setKind(TlmMarkerKind::Loop); m2.setTime(5.0f);
    TlmMarker m3(3, "end"); m3.setKind(TlmMarkerKind::Cut); m3.setTime(10.0f);
    REQUIRE(tm.addMarker(m1));
    REQUIRE(tm.addMarker(m2));
    REQUIRE(tm.addMarker(m3));
    REQUIRE_FALSE(tm.addMarker(m1));
    REQUIRE(tm.markerCount() == 3);
    auto inRange = tm.markersInRange(4.0f, 11.0f);
    REQUIRE(inRange.size() == 2);
    REQUIRE(tm.removeMarker(2));
    REQUIRE(tm.markerCount() == 2);
}

TEST_CASE("TlmMarkerKind names", "[Editor][S164]") {
    REQUIRE(std::string(tlmMarkerKindName(TlmMarkerKind::Cue))    == "Cue");
    REQUIRE(std::string(tlmMarkerKindName(TlmMarkerKind::Loop))   == "Loop");
    REQUIRE(std::string(tlmMarkerKindName(TlmMarkerKind::Cut))    == "Cut");
    REQUIRE(std::string(tlmMarkerKindName(TlmMarkerKind::Sync))   == "Sync");
    REQUIRE(std::string(tlmMarkerKindName(TlmMarkerKind::Custom)) == "Custom");
}
