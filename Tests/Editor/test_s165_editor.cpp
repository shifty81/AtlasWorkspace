// S165 editor tests: AssetTagEditorV1, AssetDependencyV1, AssetMigratorV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── AssetTagEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("AssetTagEditorV1 basic", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    REQUIRE(ate.tagCount() == 0);
}

TEST_CASE("AssetTagEditorV1 tags", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    AtgTag t1(1, "texture"); t1.setCategory(AtgTagCategory::Type);
    AtgTag t2(2, "wip"); t2.setCategory(AtgTagCategory::Status);
    AtgTag t3(3, "mobile"); t3.setCategory(AtgTagCategory::Platform);
    REQUIRE(ate.addTag(t1));
    REQUIRE(ate.addTag(t2));
    REQUIRE(ate.addTag(t3));
    REQUIRE_FALSE(ate.addTag(t1));
    REQUIRE(ate.tagCount() == 3);
    REQUIRE(ate.findTag(1) != nullptr);
    auto typeOnly = ate.filterByCategory(AtgTagCategory::Type);
    REQUIRE(typeOnly.size() == 1);
    REQUIRE(ate.removeTag(3));
    REQUIRE(ate.tagCount() == 2);
}

TEST_CASE("AtgTagCategory names", "[Editor][S165]") {
    REQUIRE(std::string(atgTagCategoryName(AtgTagCategory::Type))     == "Type");
    REQUIRE(std::string(atgTagCategoryName(AtgTagCategory::Status))   == "Status");
    REQUIRE(std::string(atgTagCategoryName(AtgTagCategory::Platform)) == "Platform");
    REQUIRE(std::string(atgTagCategoryName(AtgTagCategory::Pipeline)) == "Pipeline");
    REQUIRE(std::string(atgTagCategoryName(AtgTagCategory::Custom))   == "Custom");
}

// ── AssetDependencyV1 ─────────────────────────────────────────────────────

TEST_CASE("AssetDependencyV1 basic", "[Editor][S165]") {
    AssetDependencyV1 ad;
    REQUIRE(ad.depCount() == 0);
    REQUIRE(ad.hardDepCount() == 0);
}

TEST_CASE("AssetDependencyV1 deps", "[Editor][S165]") {
    AssetDependencyV1 ad;
    AdvDep d1(1, 10, 20); d1.setType(AdvDepType::Hard);
    AdvDep d2(2, 10, 30); d2.setType(AdvDepType::Soft);
    AdvDep d3(3, 11, 20); d3.setType(AdvDepType::Optional);
    REQUIRE(ad.addDep(d1));
    REQUIRE(ad.addDep(d2));
    REQUIRE(ad.addDep(d3));
    REQUIRE_FALSE(ad.addDep(d1));
    REQUIRE(ad.depCount() == 3);
    REQUIRE(ad.hardDepCount() == 1);
    auto from10 = ad.depsFrom(10);
    REQUIRE(from10.size() == 2);
    REQUIRE(ad.removeDep(3));
    REQUIRE(ad.depCount() == 2);
}

TEST_CASE("AdvDepType names", "[Editor][S165]") {
    REQUIRE(std::string(advDepTypeName(AdvDepType::Hard))      == "Hard");
    REQUIRE(std::string(advDepTypeName(AdvDepType::Soft))      == "Soft");
    REQUIRE(std::string(advDepTypeName(AdvDepType::Optional))  == "Optional");
    REQUIRE(std::string(advDepTypeName(AdvDepType::Generated)) == "Generated");
}

// ── AssetMigratorV1 ───────────────────────────────────────────────────────

TEST_CASE("AssetMigratorV1 basic", "[Editor][S165]") {
    AssetMigratorV1 am;
    REQUIRE(am.taskCount() == 0);
    REQUIRE(am.doneCount() == 0);
    REQUIRE(am.failedCount() == 0);
}

TEST_CASE("AssetMigratorV1 tasks", "[Editor][S165]") {
    AssetMigratorV1 am;
    AmvTask t1(1, "assets/tex.png"); t1.setStatus(AmvMigrateStatus::Done); t1.setTargetVersion(2);
    AmvTask t2(2, "assets/mesh.obj"); t2.setStatus(AmvMigrateStatus::Failed);
    AmvTask t3(3, "assets/mat.json"); // Pending
    REQUIRE(am.addTask(t1));
    REQUIRE(am.addTask(t2));
    REQUIRE(am.addTask(t3));
    REQUIRE_FALSE(am.addTask(t1));
    REQUIRE(am.taskCount() == 3);
    REQUIRE(am.doneCount() == 1);
    REQUIRE(am.failedCount() == 1);
    REQUIRE(am.findTask(1)->targetVersion() == 2u);
    REQUIRE(am.removeTask(2));
    REQUIRE(am.taskCount() == 2);
}

TEST_CASE("AmvMigrateStatus names", "[Editor][S165]") {
    REQUIRE(std::string(amvMigrateStatusName(AmvMigrateStatus::Pending))    == "Pending");
    REQUIRE(std::string(amvMigrateStatusName(AmvMigrateStatus::InProgress)) == "InProgress");
    REQUIRE(std::string(amvMigrateStatusName(AmvMigrateStatus::Done))       == "Done");
    REQUIRE(std::string(amvMigrateStatusName(AmvMigrateStatus::Failed))     == "Failed");
    REQUIRE(std::string(amvMigrateStatusName(AmvMigrateStatus::Skipped))    == "Skipped");
}
