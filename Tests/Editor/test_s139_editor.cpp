// S139 editor tests: TypographySystem, IconographySpec, FontRegistryEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── TypographySystem ──────────────────────────────────────────────────────────

TEST_CASE("TypoScale names", "[Editor][S139]") {
    REQUIRE(std::string(typoScaleName(TypoScale::Caption))  == "Caption");
    REQUIRE(std::string(typoScaleName(TypoScale::Body))     == "Body");
    REQUIRE(std::string(typoScaleName(TypoScale::Subtitle)) == "Subtitle");
    REQUIRE(std::string(typoScaleName(TypoScale::Title))    == "Title");
    REQUIRE(std::string(typoScaleName(TypoScale::Display))  == "Display");
}

TEST_CASE("TypoWeight names", "[Editor][S139]") {
    REQUIRE(std::string(typoWeightName(TypoWeight::Thin))    == "Thin");
    REQUIRE(std::string(typoWeightName(TypoWeight::Light))   == "Light");
    REQUIRE(std::string(typoWeightName(TypoWeight::Regular)) == "Regular");
    REQUIRE(std::string(typoWeightName(TypoWeight::Medium))  == "Medium");
    REQUIRE(std::string(typoWeightName(TypoWeight::Bold))    == "Bold");
    REQUIRE(std::string(typoWeightName(TypoWeight::Black))   == "Black");
}

TEST_CASE("TypoStyle defaults", "[Editor][S139]") {
    TypoStyle s(1, "body-style", TypoScale::Body, TypoWeight::Regular);
    REQUIRE(s.id()            == 1u);
    REQUIRE(s.name()          == "body-style");
    REQUIRE(s.scale()         == TypoScale::Body);
    REQUIRE(s.weight()        == TypoWeight::Regular);
    REQUIRE(s.sizeOverride()  == 0.0f);
    REQUIRE(s.lineHeight()    == 1.2f);
    REQUIRE(s.letterSpacing() == 0.0f);
    REQUIRE(s.enabled()       == true);
}

TEST_CASE("TypoStyle setters", "[Editor][S139]") {
    TypoStyle s(2, "display-style", TypoScale::Display, TypoWeight::Bold);
    s.setSizeOverride(18.0f);
    s.setLineHeight(1.5f);
    s.setLetterSpacing(0.05f);
    s.setEnabled(false);
    REQUIRE(s.sizeOverride()  == 18.0f);
    REQUIRE(s.lineHeight()    == 1.5f);
    REQUIRE(s.letterSpacing() == 0.05f);
    REQUIRE(s.enabled()       == false);
}

TEST_CASE("TypographySystem add and count", "[Editor][S139]") {
    TypographySystem sys;
    REQUIRE(sys.styleCount() == 0u);
    sys.addStyle(TypoStyle(1, "s1", TypoScale::Body, TypoWeight::Regular));
    sys.addStyle(TypoStyle(2, "s2", TypoScale::Title, TypoWeight::Bold));
    REQUIRE(sys.styleCount() == 2u);
}

TEST_CASE("TypographySystem duplicate prevention", "[Editor][S139]") {
    TypographySystem sys;
    bool ok1 = sys.addStyle(TypoStyle(10, "s", TypoScale::Caption, TypoWeight::Thin));
    bool ok2 = sys.addStyle(TypoStyle(10, "s", TypoScale::Caption, TypoWeight::Thin));
    REQUIRE(ok1 == true);
    REQUIRE(ok2 == false);
    REQUIRE(sys.styleCount() == 1u);
}

TEST_CASE("TypographySystem find and remove", "[Editor][S139]") {
    TypographySystem sys;
    sys.addStyle(TypoStyle(5, "subtitle", TypoScale::Subtitle, TypoWeight::Medium));
    auto* found = sys.findStyle(5);
    REQUIRE(found != nullptr);
    REQUIRE(found->name() == "subtitle");
    REQUIRE(sys.removeStyle(5) == true);
    REQUIRE(sys.findStyle(5) == nullptr);
    REQUIRE(sys.removeStyle(5) == false);
}

TEST_CASE("TypographySystem find missing", "[Editor][S139]") {
    TypographySystem sys;
    REQUIRE(sys.findStyle(99) == nullptr);
}

// ── IconographySpec ───────────────────────────────────────────────────────────

TEST_CASE("IconSize pixel values", "[Editor][S139]") {
    REQUIRE(iconSpecSizePx(IconSpecSize::XS) == 16);
    REQUIRE(iconSpecSizePx(IconSpecSize::SM) == 24);
    REQUIRE(iconSpecSizePx(IconSpecSize::MD) == 32);
    REQUIRE(iconSpecSizePx(IconSpecSize::LG) == 48);
    REQUIRE(iconSpecSizePx(IconSpecSize::XL) == 64);
}

TEST_CASE("IconStyle names", "[Editor][S139]") {
    REQUIRE(std::string(iconStyleName(IconStyle::Filled))   == "Filled");
    REQUIRE(std::string(iconStyleName(IconStyle::Outlined))  == "Outlined");
    REQUIRE(std::string(iconStyleName(IconStyle::TwoTone))   == "TwoTone");
    REQUIRE(std::string(iconStyleName(IconStyle::Rounded))   == "Rounded");
    REQUIRE(std::string(iconStyleName(IconStyle::Sharp))     == "Sharp");
}

TEST_CASE("IconEntry constructor", "[Editor][S139]") {
    IconEntry e(1, "home", "navigation", IconSpecSize::MD, IconStyle::Filled, 0xE88A);
    REQUIRE(e.id()       == 1u);
    REQUIRE(e.name()     == "home");
    REQUIRE(e.category() == "navigation");
    REQUIRE(e.size()     == IconSpecSize::MD);
    REQUIRE(e.style()    == IconStyle::Filled);
    REQUIRE(e.unicode()  == 0xE88Au);
}

TEST_CASE("IconographySpec add and remove", "[Editor][S139]") {
    IconographySpec spec;
    spec.addIcon(IconEntry(1, "home", "nav", IconSpecSize::SM, IconStyle::Filled, 0xE001));
    spec.addIcon(IconEntry(2, "star", "action", IconSpecSize::MD, IconStyle::Outlined, 0xE002));
    REQUIRE(spec.iconCount() == 2u);
    REQUIRE(spec.removeIcon(1) == true);
    REQUIRE(spec.iconCount() == 1u);
    REQUIRE(spec.removeIcon(99) == false);
}

TEST_CASE("IconographySpec filterBySize", "[Editor][S139]") {
    IconographySpec spec;
    spec.addIcon(IconEntry(1, "a", "x", IconSpecSize::SM, IconStyle::Filled, 1));
    spec.addIcon(IconEntry(2, "b", "x", IconSpecSize::SM, IconStyle::Outlined, 2));
    spec.addIcon(IconEntry(3, "c", "x", IconSpecSize::MD, IconStyle::Filled, 3));
    auto sm = spec.filterBySize(IconSpecSize::SM);
    REQUIRE(sm.size() == 2u);
    auto md = spec.filterBySize(IconSpecSize::MD);
    REQUIRE(md.size() == 1u);
}

// ── FontRegistryEditor ────────────────────────────────────────────────────────

TEST_CASE("FontFormat names", "[Editor][S139]") {
    REQUIRE(std::string(fontFormatName(FontFormat::TTF))   == "TTF");
    REQUIRE(std::string(fontFormatName(FontFormat::OTF))   == "OTF");
    REQUIRE(std::string(fontFormatName(FontFormat::WOFF))  == "WOFF");
    REQUIRE(std::string(fontFormatName(FontFormat::WOFF2)) == "WOFF2");
}

TEST_CASE("FontCategory names", "[Editor][S139]") {
    REQUIRE(std::string(fontCategoryName(FontCategory::Serif))       == "Serif");
    REQUIRE(std::string(fontCategoryName(FontCategory::SansSerif))   == "SansSerif");
    REQUIRE(std::string(fontCategoryName(FontCategory::Monospace))   == "Monospace");
    REQUIRE(std::string(fontCategoryName(FontCategory::Display))     == "Display");
    REQUIRE(std::string(fontCategoryName(FontCategory::Handwriting)) == "Handwriting");
}

TEST_CASE("FontRecord defaults", "[Editor][S139]") {
    FontRecord r(1, "Roboto", FontFormat::TTF, FontCategory::SansSerif, "/fonts/roboto.ttf");
    REQUIRE(r.id()         == 1u);
    REQUIRE(r.familyName() == "Roboto");
    REQUIRE(r.format()     == FontFormat::TTF);
    REQUIRE(r.category()   == FontCategory::SansSerif);
    REQUIRE(r.filePath()   == "/fonts/roboto.ttf");
    REQUIRE(r.weight()     == 400);
    REQUIRE(r.italic()     == false);
    REQUIRE(r.loaded()     == false);
}

TEST_CASE("FontRecord setters", "[Editor][S139]") {
    FontRecord r(2, "Consolas", FontFormat::OTF, FontCategory::Monospace, "/fonts/consolas.otf");
    r.setWeight(700);
    r.setItalic(true);
    r.setLoaded(true);
    REQUIRE(r.weight() == 700);
    REQUIRE(r.italic() == true);
    REQUIRE(r.loaded() == true);
}

TEST_CASE("FontRegistryEditor register and filter", "[Editor][S139]") {
    FontRegistryEditor reg;
    reg.registerFont(FontRecord(1, "Roboto", FontFormat::TTF, FontCategory::SansSerif, "a.ttf"));
    reg.registerFont(FontRecord(2, "Georgia", FontFormat::OTF, FontCategory::Serif, "b.otf"));
    reg.registerFont(FontRecord(3, "Courier", FontFormat::TTF, FontCategory::Monospace, "c.ttf"));
    REQUIRE(reg.recordCount() == 3u);
    auto sans = reg.filterByCategory(FontCategory::SansSerif);
    REQUIRE(sans.size() == 1u);
    REQUIRE(reg.unregisterFont(2) == true);
    REQUIRE(reg.recordCount() == 2u);
}
