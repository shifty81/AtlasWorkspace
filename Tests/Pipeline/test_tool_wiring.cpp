#include <catch2/catch_test_macros.hpp>
#include "NF/Pipeline/Pipeline.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Helper: create a unique temp directory for each test invocation.
static fs::path makeTempDir(std::string_view name) {
    static std::atomic<uint64_t> s_seq{0};
    auto ts  = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto idx = s_seq.fetch_add(1);
    std::string unique = std::string(name) + "_" +
                         std::to_string(ts)  + "_" +
                         std::to_string(idx);
    auto dir = fs::temp_directory_path() / "nf_toolwiring_tests" / unique;
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir;
}

// Helper: create PipelineDirectories rooted at a temp dir.
static NF::PipelineDirectories makeTempPipeline(std::string_view name) {
    auto root = makeTempDir(name);
    auto dirs = NF::PipelineDirectories::fromRoot(root);
    dirs.ensureCreated();
    return dirs;
}

// ── AIAnalysis event type ─────────────────────────────────────────────────

TEST_CASE("AIAnalysis event type name and fromString", "[Pipeline][ChangeEventType]") {
    REQUIRE(std::string(NF::changeEventTypeName(NF::ChangeEventType::AIAnalysis)) == "AIAnalysis");
    REQUIRE(NF::changeEventTypeFromString("AIAnalysis") == NF::ChangeEventType::AIAnalysis);
}

// ── BlenderGenAdapter ─────────────────────────────────────────────────────

TEST_CASE("BlenderGenAdapter accepts AssetImported only", "[Pipeline][ToolAdapter]") {
    NF::BlenderGenAdapter adapter;
    REQUIRE(std::string(adapter.name()) == "BlenderGenerator");
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::AssetImported));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::ScriptUpdated));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::WorldChanged));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::ContractIssue));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::ReplayExported));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::Unknown));
}

TEST_CASE("BlenderGenAdapter handles AssetImported and emits response", "[Pipeline][ToolAdapter]") {
    auto dirs = makeTempPipeline("blendergen_handle");

    NF::BlenderGenAdapter adapter;
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/ship.glb";
    ev.timestamp = 1000LL;

    REQUIRE(adapter.handleEvent(ev, dirs));
    REQUIRE(adapter.handledCount() == 1);

    // A response .change.json should exist in the changes directory.
    int fileCount = 0;
    for (const auto& entry : fs::directory_iterator(dirs.changes)) {
        if (entry.path().extension() == ".json") ++fileCount;
    }
    REQUIRE(fileCount >= 1);
}

TEST_CASE("BlenderGenAdapter rejects non-AssetImported events", "[Pipeline][ToolAdapter]") {
    auto dirs = makeTempPipeline("blendergen_reject");

    NF::BlenderGenAdapter adapter;
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::ScriptUpdated;
    ev.path      = "scripts/door.graph";
    ev.timestamp = 2000LL;

    REQUIRE_FALSE(adapter.handleEvent(ev, dirs));
    REQUIRE(adapter.handledCount() == 0);
}

// ── ContractScannerAdapter ────────────────────────────────────────────────

TEST_CASE("ContractScannerAdapter accepts ScriptUpdated only", "[Pipeline][ToolAdapter]") {
    NF::ContractScannerAdapter adapter;
    REQUIRE(std::string(adapter.name()) == "ContractScanner");
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::ScriptUpdated));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::AssetImported));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::ReplayExported));
}

TEST_CASE("ContractScannerAdapter handles ScriptUpdated and emits ContractIssue", "[Pipeline][ToolAdapter]") {
    auto dirs = makeTempPipeline("scanner_handle");

    NF::ContractScannerAdapter adapter;
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::ScriptUpdated;
    ev.path      = "src/Foo.cpp";
    ev.timestamp = 3000LL;

    REQUIRE(adapter.handleEvent(ev, dirs));
    REQUIRE(adapter.handledCount() == 1);
}

// ── ReplayMinimizerAdapter ────────────────────────────────────────────────

TEST_CASE("ReplayMinimizerAdapter accepts ReplayExported only", "[Pipeline][ToolAdapter]") {
    NF::ReplayMinimizerAdapter adapter;
    REQUIRE(std::string(adapter.name()) == "ReplayMinimizer");
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::ReplayExported));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::AssetImported));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::ScriptUpdated));
}

TEST_CASE("ReplayMinimizerAdapter handles ReplayExported", "[Pipeline][ToolAdapter]") {
    auto dirs = makeTempPipeline("replay_handle");

    NF::ReplayMinimizerAdapter adapter;
    NF::ChangeEvent ev;
    ev.tool      = "GameRuntime";
    ev.eventType = NF::ChangeEventType::ReplayExported;
    ev.path      = "replays/run42.replay.json";
    ev.timestamp = 4000LL;

    REQUIRE(adapter.handleEvent(ev, dirs));
    REQUIRE(adapter.handledCount() == 1);
}

// ── SwissAgentAdapter ─────────────────────────────────────────────────────

TEST_CASE("SwissAgentAdapter accepts all non-Unknown event types", "[Pipeline][ToolAdapter]") {
    NF::SwissAgentAdapter adapter;
    REQUIRE(std::string(adapter.name()) == "SwissAgent");
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::AssetImported));
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::WorldChanged));
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::ScriptUpdated));
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::AnimationExported));
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::ContractIssue));
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::ReplayExported));
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::AIAnalysis));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::Unknown));
}

TEST_CASE("SwissAgentAdapter handles event and emits AIAnalysis", "[Pipeline][ToolAdapter]") {
    auto dirs = makeTempPipeline("swiss_handle");

    NF::SwissAgentAdapter adapter;
    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Bar.cpp";
    ev.timestamp = 5000LL;

    REQUIRE(adapter.handleEvent(ev, dirs));
    REQUIRE(adapter.handledCount() == 1);
}

// ── AtlasAIAdapter ────────────────────────────────────────────────────────

TEST_CASE("AtlasAIAdapter accepts ContractIssue and WorldChanged", "[Pipeline][ToolAdapter]") {
    NF::AtlasAIAdapter adapter;
    REQUIRE(std::string(adapter.name()) == "AtlasAI");
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::ContractIssue));
    REQUIRE(adapter.acceptsEvent(NF::ChangeEventType::WorldChanged));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::AssetImported));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::ScriptUpdated));
    REQUIRE_FALSE(adapter.acceptsEvent(NF::ChangeEventType::ReplayExported));
}

TEST_CASE("AtlasAIAdapter handles WorldChanged and emits AIAnalysis", "[Pipeline][ToolAdapter]") {
    auto dirs = makeTempPipeline("atlas_ai_handle");

    NF::AtlasAIAdapter adapter;
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::WorldChanged;
    ev.path      = "worlds/sector03.json";
    ev.timestamp = 6000LL;

    REQUIRE(adapter.handleEvent(ev, dirs));
    REQUIRE(adapter.handledCount() == 1);
}

// ── ToolRegistry ──────────────────────────────────────────────────────────

TEST_CASE("ToolRegistry registers and queries tools", "[Pipeline][ToolRegistry]") {
    NF::ToolRegistry registry;
    REQUIRE(registry.toolCount() == 0);

    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::SwissAgentAdapter>());
    REQUIRE(registry.toolCount() == 3);

    REQUIRE(registry.tool(0) != nullptr);
    REQUIRE(std::string(registry.tool(0)->name()) == "BlenderGenerator");
    REQUIRE(std::string(registry.tool(1)->name()) == "ContractScanner");
    REQUIRE(std::string(registry.tool(2)->name()) == "SwissAgent");
    REQUIRE(registry.tool(3) == nullptr); // out-of-bounds
}

TEST_CASE("ToolRegistry dispatch routes events to matching adapters", "[Pipeline][ToolRegistry]") {
    auto dirs = makeTempPipeline("registry_dispatch");

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::SwissAgentAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());

    // AssetImported → BlenderGen + SwissAgent = 2 handlers
    NF::ChangeEvent ev1;
    ev1.tool      = "Editor";
    ev1.eventType = NF::ChangeEventType::AssetImported;
    ev1.path      = "assets/cube.glb";
    ev1.timestamp = 100LL;
    REQUIRE(registry.dispatch(ev1, dirs) == 2);

    // ScriptUpdated → ContractScanner + SwissAgent = 2 handlers
    NF::ChangeEvent ev2;
    ev2.tool      = "Editor";
    ev2.eventType = NF::ChangeEventType::ScriptUpdated;
    ev2.path      = "scripts/door.graph";
    ev2.timestamp = 200LL;
    REQUIRE(registry.dispatch(ev2, dirs) == 2);

    // ContractIssue → SwissAgent + AtlasAI = 2 handlers
    NF::ChangeEvent ev3;
    ev3.tool      = "ContractScanner";
    ev3.eventType = NF::ChangeEventType::ContractIssue;
    ev3.path      = "src/Baz.cpp";
    ev3.timestamp = 300LL;
    REQUIRE(registry.dispatch(ev3, dirs) == 2);

    // WorldChanged → SwissAgent + AtlasAI = 2 handlers
    NF::ChangeEvent ev4;
    ev4.tool      = "Editor";
    ev4.eventType = NF::ChangeEventType::WorldChanged;
    ev4.path      = "worlds/sector01.json";
    ev4.timestamp = 400LL;
    REQUIRE(registry.dispatch(ev4, dirs) == 2);

    // ReplayExported → ReplayMinimizer + SwissAgent = 2 handlers
    NF::ChangeEvent ev5;
    ev5.tool      = "GameRuntime";
    ev5.eventType = NF::ChangeEventType::ReplayExported;
    ev5.path      = "replays/run1.replay.json";
    ev5.timestamp = 500LL;
    REQUIRE(registry.dispatch(ev5, dirs) == 2);

    // Unknown → nobody handles it
    NF::ChangeEvent ev6;
    ev6.tool      = "Editor";
    ev6.eventType = NF::ChangeEventType::Unknown;
    ev6.path      = "unknown/thing";
    ev6.timestamp = 600LL;
    REQUIRE(registry.dispatch(ev6, dirs) == 0);
}

TEST_CASE("ToolRegistry dispatch accumulates handledCount per tool", "[Pipeline][ToolRegistry]") {
    auto dirs = makeTempPipeline("registry_counts");

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::SwissAgentAdapter>());

    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "a.glb";
    ev.timestamp = 100LL;

    registry.dispatch(ev, dirs);
    registry.dispatch(ev, dirs);

    REQUIRE(registry.tool(0)->handledCount() == 2); // BlenderGen
    REQUIRE(registry.tool(1)->handledCount() == 2); // SwissAgent (accepts all)
}

TEST_CASE("ToolRegistry attach wires watcher to dispatch", "[Pipeline][ToolRegistry]") {
    auto dirs = makeTempPipeline("registry_attach");

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::SwissAgentAdapter>());

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);

    // Write a .change.json and poll — the registry should dispatch it.
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "models/ship.glb";
    ev.timestamp = 7777LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    int found = watcher.poll();
    REQUIRE(found >= 1);

    // Both tools should have been notified.
    REQUIRE(registry.tool(0)->handledCount() >= 1); // BlenderGen
    REQUIRE(registry.tool(1)->handledCount() >= 1); // SwissAgent
}

TEST_CASE("ToolRegistry with all five tools handles full event matrix", "[Pipeline][ToolRegistry]") {
    auto dirs = makeTempPipeline("registry_full");

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::SwissAgentAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());
    REQUIRE(registry.toolCount() == 5);

    // AIAnalysis → only SwissAgent (AtlasAI doesn't accept AIAnalysis)
    NF::ChangeEvent evAI;
    evAI.tool      = "SwissAgent";
    evAI.eventType = NF::ChangeEventType::AIAnalysis;
    evAI.path      = "analysis/result.json";
    evAI.timestamp = 800LL;
    REQUIRE(registry.dispatch(evAI, dirs) == 1); // SwissAgent only

    // AnimationExported → SwissAgent only
    NF::ChangeEvent evAnim;
    evAnim.tool      = "BlenderGenerator";
    evAnim.eventType = NF::ChangeEventType::AnimationExported;
    evAnim.path      = "anims/walk.glb";
    evAnim.timestamp = 900LL;
    REQUIRE(registry.dispatch(evAnim, dirs) == 1); // SwissAgent only
}
