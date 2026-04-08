// S140 editor tests: FileIntakePipeline, DropTargetHandler, AssetImportQueue
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── FileIntakePipeline ────────────────────────────────────────────────────────

TEST_CASE("IntakeStage names", "[Editor][S140]") {
    REQUIRE(std::string(intakeStageName(IntakeStage::Validate))    == "Validate");
    REQUIRE(std::string(intakeStageName(IntakeStage::Fingerprint)) == "Fingerprint");
    REQUIRE(std::string(intakeStageName(IntakeStage::Convert))     == "Convert");
    REQUIRE(std::string(intakeStageName(IntakeStage::Register))    == "Register");
    REQUIRE(std::string(intakeStageName(IntakeStage::Done))        == "Done");
}

TEST_CASE("IntakeResult names", "[Editor][S140]") {
    REQUIRE(std::string(intakeResultName(IntakeResult::Ok))        == "Ok");
    REQUIRE(std::string(intakeResultName(IntakeResult::Skipped))   == "Skipped");
    REQUIRE(std::string(intakeResultName(IntakeResult::Failed))    == "Failed");
    REQUIRE(std::string(intakeResultName(IntakeResult::Duplicate)) == "Duplicate");
}

TEST_CASE("IntakeFile defaults", "[Editor][S140]") {
    IntakeFile f(1, "/assets/tex.png");
    REQUIRE(f.id()        == 1u);
    REQUIRE(f.path()      == "/assets/tex.png");
    REQUIRE(f.stage()     == IntakeStage::Validate);
    REQUIRE(f.result()    == IntakeResult::Ok);
    REQUIRE(f.error()     == "");
    REQUIRE(f.sizeBytes() == 0u);
}

TEST_CASE("IntakeFile setters", "[Editor][S140]") {
    IntakeFile f(2, "/assets/mesh.fbx");
    f.setStage(IntakeStage::Convert);
    f.setResult(IntakeResult::Failed);
    f.setError("unsupported format");
    f.setSizeBytes(102400);
    REQUIRE(f.stage()     == IntakeStage::Convert);
    REQUIRE(f.result()    == IntakeResult::Failed);
    REQUIRE(f.error()     == "unsupported format");
    REQUIRE(f.sizeBytes() == 102400u);
}

TEST_CASE("FileIntakePipeline enqueue and count", "[Editor][S140]") {
    FileIntakePipeline pipeline;
    REQUIRE(pipeline.fileCount() == 0u);
    pipeline.enqueue(IntakeFile(1, "a.png"));
    pipeline.enqueue(IntakeFile(2, "b.png"));
    REQUIRE(pipeline.fileCount() == 2u);
    REQUIRE(pipeline.enqueue(IntakeFile(1, "dup.png")) == false);
}

TEST_CASE("FileIntakePipeline pendingCount", "[Editor][S140]") {
    FileIntakePipeline pipeline;
    pipeline.enqueue(IntakeFile(1, "a.png"));
    pipeline.enqueue(IntakeFile(2, "b.png"));
    auto* f2 = pipeline.findFile(2);
    f2->setStage(IntakeStage::Done);
    REQUIRE(pipeline.pendingCount() == 1u);
}

TEST_CASE("FileIntakePipeline advance", "[Editor][S140]") {
    FileIntakePipeline pipeline;
    pipeline.enqueue(IntakeFile(1, "x.png"));
    REQUIRE(pipeline.findFile(1)->stage() == IntakeStage::Validate);
    REQUIRE(pipeline.advance(1) == true);
    REQUIRE(pipeline.findFile(1)->stage() == IntakeStage::Fingerprint);
    REQUIRE(pipeline.advance(99) == false);
}

TEST_CASE("FileIntakePipeline advance at Done", "[Editor][S140]") {
    FileIntakePipeline pipeline;
    pipeline.enqueue(IntakeFile(3, "z.png"));
    pipeline.findFile(3)->setStage(IntakeStage::Done);
    REQUIRE(pipeline.advance(3) == false);
}

TEST_CASE("FileIntakePipeline dequeue", "[Editor][S140]") {
    FileIntakePipeline pipeline;
    pipeline.enqueue(IntakeFile(10, "f.png"));
    REQUIRE(pipeline.dequeue(10) == true);
    REQUIRE(pipeline.fileCount() == 0u);
    REQUIRE(pipeline.dequeue(10) == false);
}

// ── DropTargetHandler ─────────────────────────────────────────────────────────

TEST_CASE("DropAction names", "[Editor][S140]") {
    REQUIRE(std::string(dropActionName(DropAction::Copy)) == "Copy");
    REQUIRE(std::string(dropActionName(DropAction::Move)) == "Move");
    REQUIRE(std::string(dropActionName(DropAction::Link)) == "Link");
    REQUIRE(std::string(dropActionName(DropAction::Ask))  == "Ask");
}

TEST_CASE("DropZone names", "[Editor][S140]") {
    REQUIRE(std::string(dropZoneName(DropZone::AssetFolder))    == "AssetFolder");
    REQUIRE(std::string(dropZoneName(DropZone::SceneView))      == "SceneView");
    REQUIRE(std::string(dropZoneName(DropZone::ContentBrowser)) == "ContentBrowser");
    REQUIRE(std::string(dropZoneName(DropZone::Inspector))      == "Inspector");
}

TEST_CASE("DropTargetHandler beginDrop creates active session", "[Editor][S140]") {
    DropTargetHandler handler;
    REQUIRE(handler.beginDrop(1, DropZone::SceneView) == true);
    auto* s = handler.findSession(1);
    REQUIRE(s != nullptr);
    REQUIRE(s->active() == true);
    REQUIRE(s->zone()   == DropZone::SceneView);
    REQUIRE(s->action() == DropAction::Copy);
}

TEST_CASE("DropTargetHandler duplicate beginDrop", "[Editor][S140]") {
    DropTargetHandler handler;
    handler.beginDrop(1, DropZone::Inspector);
    REQUIRE(handler.beginDrop(1, DropZone::Inspector) == false);
}

TEST_CASE("DropTargetHandler endDrop and activeSessions", "[Editor][S140]") {
    DropTargetHandler handler;
    handler.beginDrop(1, DropZone::AssetFolder);
    handler.beginDrop(2, DropZone::ContentBrowser);
    REQUIRE(handler.activeSessions() == 2u);
    REQUIRE(handler.endDrop(1) == true);
    REQUIRE(handler.activeSessions() == 1u);
    REQUIRE(handler.endDrop(99) == false);
}

TEST_CASE("DropTargetHandler addPath", "[Editor][S140]") {
    DropTargetHandler handler;
    handler.beginDrop(1, DropZone::SceneView);
    REQUIRE(handler.addPath(1, "/assets/model.fbx") == true);
    REQUIRE(handler.addPath(1, "/assets/tex.png")   == true);
    REQUIRE(handler.findSession(1)->pathCount()      == 2u);
    REQUIRE(handler.addPath(99, "/x") == false);
}

// ── AssetImportQueue ──────────────────────────────────────────────────────────

TEST_CASE("ImportPriority names", "[Editor][S140]") {
    REQUIRE(std::string(importPriorityName(ImportPriority::Low))      == "Low");
    REQUIRE(std::string(importPriorityName(ImportPriority::Normal))   == "Normal");
    REQUIRE(std::string(importPriorityName(ImportPriority::High))     == "High");
    REQUIRE(std::string(importPriorityName(ImportPriority::Critical)) == "Critical");
}

TEST_CASE("ImportStatus names", "[Editor][S140]") {
    REQUIRE(std::string(importStatusName(ImportStatus::Waiting))    == "Waiting");
    REQUIRE(std::string(importStatusName(ImportStatus::Processing)) == "Processing");
    REQUIRE(std::string(importStatusName(ImportStatus::Complete))   == "Complete");
    REQUIRE(std::string(importStatusName(ImportStatus::Error))      == "Error");
}

TEST_CASE("ImportJob defaults", "[Editor][S140]") {
    ImportJob j(1, "/assets/rock.fbx");
    REQUIRE(j.id()        == 1u);
    REQUIRE(j.assetPath() == "/assets/rock.fbx");
    REQUIRE(j.priority()  == ImportPriority::Normal);
    REQUIRE(j.status()    == ImportStatus::Waiting);
    REQUIRE(j.progress()  == 0.0f);
    REQUIRE(j.message()   == "");
}

TEST_CASE("AssetImportQueue enqueue and waitingCount", "[Editor][S140]") {
    AssetImportQueue queue;
    queue.enqueue(ImportJob(1, "a.fbx"));
    queue.enqueue(ImportJob(2, "b.png"));
    REQUIRE(queue.jobCount()     == 2u);
    REQUIRE(queue.waitingCount() == 2u);
    queue.setStatus(1, ImportStatus::Processing);
    REQUIRE(queue.waitingCount() == 1u);
}

TEST_CASE("AssetImportQueue setProgress and cancel", "[Editor][S140]") {
    AssetImportQueue queue;
    queue.enqueue(ImportJob(5, "c.wav"));
    REQUIRE(queue.setProgress(5, 0.75f) == true);
    REQUIRE(queue.findJob(5)->progress() == 0.75f);
    REQUIRE(queue.cancel(5) == true);
    REQUIRE(queue.jobCount() == 0u);
    REQUIRE(queue.cancel(5) == false);
}
