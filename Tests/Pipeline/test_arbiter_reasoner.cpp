#include <catch2/catch_test_macros.hpp>
#include "NF/Pipeline/Pipeline.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static fs::path makeTempDir(std::string_view name) {
    static std::atomic<uint64_t> s_seq{0};
    auto ts  = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto idx = s_seq.fetch_add(1);
    std::string unique = std::string(name) + "_" +
                         std::to_string(ts)  + "_" +
                         std::to_string(idx);
    auto dir = fs::temp_directory_path() / "nf_arbiter_tests" / unique;
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir;
}

static NF::PipelineDirectories makeTempPipeline(std::string_view name) {
    auto root = makeTempDir(name);
    auto dirs = NF::PipelineDirectories::fromRoot(root);
    dirs.ensureCreated();
    return dirs;
}

// ── RuleSeverity ──────────────────────────────────────────────────────────

TEST_CASE("RuleSeverity name strings", "[Pipeline][ArbiterReasoner]") {
    REQUIRE(std::string(NF::ruleSeverityName(NF::RuleSeverity::Info))     == "Info");
    REQUIRE(std::string(NF::ruleSeverityName(NF::RuleSeverity::Warning))  == "Warning");
    REQUIRE(std::string(NF::ruleSeverityName(NF::RuleSeverity::Error))    == "Error");
    REQUIRE(std::string(NF::ruleSeverityName(NF::RuleSeverity::Critical)) == "Critical");
}

// ── AB-1: Rule management ─────────────────────────────────────────────────

TEST_CASE("ArbiterReasoner constructs with empty state", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;
    REQUIRE(reasoner.ruleCount() == 0);
    REQUIRE(reasoner.violationCount() == 0);
    REQUIRE(reasoner.eventsProcessed() == 0);
    REQUIRE(reasoner.passesGate());
}

TEST_CASE("ArbiterReasoner adds and finds rules", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    NF::ArbiterRule rule;
    rule.id          = "R001";
    rule.description = "Test rule";
    rule.severity    = NF::RuleSeverity::Warning;
    rule.eventType   = NF::ChangeEventType::ContractIssue;
    rule.pathPattern = "*.cpp";
    rule.suggestion  = "Fix it";

    reasoner.addRule(rule);
    REQUIRE(reasoner.ruleCount() == 1);

    auto* found = reasoner.findRule("R001");
    REQUIRE(found != nullptr);
    REQUIRE(found->description == "Test rule");
    REQUIRE(found->severity == NF::RuleSeverity::Warning);

    REQUIRE(reasoner.findRule("nonexistent") == nullptr);
}

TEST_CASE("ArbiterReasoner loads rules from JSON", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    std::string json = R"([
        {
            "id": "R100",
            "description": "No raw pointers in headers",
            "severity": "Error",
            "event_type": "ContractIssue",
            "path_pattern": "*.h",
            "suggestion": "Use smart pointers"
        },
        {
            "id": "R101",
            "description": "Script needs validation",
            "severity": "Warning",
            "event_type": "ScriptUpdated",
            "path_pattern": "*.graph",
            "suggestion": "Run validator"
        }
    ])";

    size_t loaded = reasoner.loadRulesFromJson(json);
    REQUIRE(loaded == 2);
    REQUIRE(reasoner.ruleCount() == 2);

    auto* r100 = reasoner.findRule("R100");
    REQUIRE(r100 != nullptr);
    REQUIRE(r100->severity == NF::RuleSeverity::Error);
    REQUIRE(r100->eventType == NF::ChangeEventType::ContractIssue);

    auto* r101 = reasoner.findRule("R101");
    REQUIRE(r101 != nullptr);
    REQUIRE(r101->severity == NF::RuleSeverity::Warning);
}

// ── AB-2: Rule evaluation ─────────────────────────────────────────────────

TEST_CASE("ArbiterReasoner evaluates matching rules", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    NF::ArbiterRule rule;
    rule.id          = "R001";
    rule.description = "Contract violation in source";
    rule.severity    = NF::RuleSeverity::Error;
    rule.eventType   = NF::ChangeEventType::ContractIssue;
    rule.pathPattern = "*.cpp";
    rule.suggestion  = "Review the file";
    reasoner.addRule(rule);

    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Foo.cpp";
    ev.timestamp = 1000LL;

    auto violations = reasoner.evaluate(ev);
    REQUIRE(violations.size() == 1);
    REQUIRE(violations[0].ruleId == "R001");
    REQUIRE(violations[0].severity == NF::RuleSeverity::Error);
    REQUIRE(violations[0].path == "src/Foo.cpp");
    REQUIRE(violations[0].suggestion == "Review the file");
}

TEST_CASE("ArbiterReasoner skips non-matching event types", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    NF::ArbiterRule rule;
    rule.id        = "R001";
    rule.eventType = NF::ChangeEventType::ContractIssue;
    rule.pathPattern = "*.cpp";
    reasoner.addRule(rule);

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "src/Foo.cpp";

    auto violations = reasoner.evaluate(ev);
    REQUIRE(violations.empty());
}

TEST_CASE("ArbiterReasoner skips non-matching paths", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    NF::ArbiterRule rule;
    rule.id          = "R001";
    rule.eventType   = NF::ChangeEventType::ContractIssue;
    rule.pathPattern = "*.h";
    reasoner.addRule(rule);

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Foo.cpp";

    auto violations = reasoner.evaluate(ev);
    REQUIRE(violations.empty());
}

TEST_CASE("ArbiterReasoner matches wildcard paths", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    NF::ArbiterRule rule;
    rule.id          = "R010";
    rule.eventType   = NF::ChangeEventType::WorldChanged;
    rule.pathPattern = "worlds/*";
    reasoner.addRule(rule);

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::WorldChanged;
    ev.path      = "worlds/sector01.json";

    auto violations = reasoner.evaluate(ev);
    REQUIRE(violations.size() == 1);
}

TEST_CASE("ArbiterReasoner rule with empty path matches all paths", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    NF::ArbiterRule rule;
    rule.id        = "R099";
    rule.eventType = NF::ChangeEventType::ScriptUpdated;
    // pathPattern is empty — matches everything
    reasoner.addRule(rule);

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::ScriptUpdated;
    ev.path      = "any/path/at/all.txt";

    auto violations = reasoner.evaluate(ev);
    REQUIRE(violations.size() == 1);
}

TEST_CASE("ArbiterReasoner evaluates multiple rules simultaneously", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;

    NF::ArbiterRule r1;
    r1.id        = "R001";
    r1.eventType = NF::ChangeEventType::ContractIssue;
    r1.pathPattern = "*.cpp";
    reasoner.addRule(r1);

    NF::ArbiterRule r2;
    r2.id        = "R002";
    r2.eventType = NF::ChangeEventType::ContractIssue;
    // empty path — matches all
    reasoner.addRule(r2);

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Bar.cpp";

    auto violations = reasoner.evaluate(ev);
    REQUIRE(violations.size() == 2);
}

// ── AB-3: Default rules ───────────────────────────────────────────────────

TEST_CASE("ArbiterReasoner loads default game balance rules", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();
    REQUIRE(reasoner.ruleCount() >= 8);

    // Spot-check a few default rules.
    REQUIRE(reasoner.findRule("R001") != nullptr);
    REQUIRE(reasoner.findRule("R010") != nullptr);
    REQUIRE(reasoner.findRule("R020") != nullptr);
    REQUIRE(reasoner.findRule("R030") != nullptr);
}

TEST_CASE("Default rules trigger on matching events", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Entity.cpp";

    auto violations = reasoner.evaluate(ev);
    REQUIRE(violations.size() >= 1);  // R001 should match
}

// ── AB-4: Violation tracking ──────────────────────────────────────────────

TEST_CASE("ArbiterReasoner processEvent tracks violations", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_process");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Main.cpp";
    ev.timestamp = 1000LL;

    size_t count = reasoner.processEvent(ev, dirs);
    REQUIRE(count >= 1);
    REQUIRE(reasoner.violationCount() >= 1);
    REQUIRE(reasoner.eventsProcessed() == 1);

    auto pathViolations = reasoner.violationsForPath("src/Main.cpp");
    REQUIRE(pathViolations.size() >= 1);
}

TEST_CASE("ArbiterReasoner processEvent emits AIAnalysis response", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_emit");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Test.cpp";
    ev.timestamp = 2000LL;

    reasoner.processEvent(ev, dirs);

    // Check that a .change.json response was written.
    int fileCount = 0;
    for (const auto& entry : fs::directory_iterator(dirs.changes)) {
        if (entry.path().extension() == ".json") ++fileCount;
    }
    REQUIRE(fileCount >= 1);
}

TEST_CASE("ArbiterReasoner processEvent with no matching rules produces no violations", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_nomatch");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::Unknown;
    ev.path      = "unknown/thing";

    size_t count = reasoner.processEvent(ev, dirs);
    REQUIRE(count == 0);
    REQUIRE(reasoner.eventsProcessed() == 1);
}

// ── AB-5: CI gate ─────────────────────────────────────────────────────────

TEST_CASE("ArbiterReasoner gate passes with no violations", "[Pipeline][ArbiterReasoner]") {
    NF::ArbiterReasoner reasoner;
    REQUIRE(reasoner.passesGate());
}

TEST_CASE("ArbiterReasoner gate fails with Error violations", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_gate");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    // R001 is severity Error, matches ContractIssue on *.cpp
    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Bug.cpp";
    ev.timestamp = 3000LL;

    reasoner.processEvent(ev, dirs);
    REQUIRE_FALSE(reasoner.passesGate());
}

TEST_CASE("ArbiterReasoner gate passes with only Info/Warning violations", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_gate_pass");
    NF::ArbiterReasoner reasoner;

    // Add only a Warning-severity rule.
    NF::ArbiterRule rule;
    rule.id        = "W001";
    rule.severity  = NF::RuleSeverity::Warning;
    rule.eventType = NF::ChangeEventType::AssetImported;
    reasoner.addRule(rule);

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/box.glb";
    ev.timestamp = 4000LL;

    reasoner.processEvent(ev, dirs);
    REQUIRE(reasoner.violationCount() == 1);
    REQUIRE(reasoner.passesGate());  // Warning doesn't fail the gate
}

TEST_CASE("ArbiterReasoner summary includes violation counts", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_summary");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ChangeEvent ev;
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/X.cpp";
    ev.timestamp = 5000LL;

    reasoner.processEvent(ev, dirs);

    auto s = reasoner.summary();
    REQUIRE(s.find("ArbiterAI:") != std::string::npos);
    REQUIRE(s.find("FAIL") != std::string::npos);  // Error violations cause FAIL
}

// ── Pipeline integration ──────────────────────────────────────────────────

TEST_CASE("ArbiterReasoner attaches to PipelineWatcher", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_attach");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::PipelineWatcher watcher(dirs.changes);
    reasoner.attachToWatcher(watcher, dirs);

    // Write a ContractIssue event.
    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/System.cpp";
    ev.timestamp = 6000LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    REQUIRE(reasoner.eventsProcessed() >= 1);
    REQUIRE(reasoner.violationCount() >= 1);
}

TEST_CASE("ArbiterReasoner skips own events to avoid feedback loops", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_noloop");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::PipelineWatcher watcher(dirs.changes);
    reasoner.attachToWatcher(watcher, dirs);

    // Write an ArbiterAI event (should be skipped).
    NF::ChangeEvent ev;
    ev.tool      = "ArbiterAI";
    ev.eventType = NF::ChangeEventType::AIAnalysis;
    ev.path      = "analysis/result.json";
    ev.timestamp = 7000LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    REQUIRE(reasoner.eventsProcessed() == 0);
}

TEST_CASE("ArbiterReasoner end-to-end with ToolRegistry", "[Pipeline][ArbiterReasoner]") {
    auto dirs = makeTempPipeline("arbiter_e2e");
    NF::ArbiterReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::ArbiterAdapter>());

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);
    reasoner.attachToWatcher(watcher, dirs);

    // Write a WorldChanged event.
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::WorldChanged;
    ev.path      = "worlds/sector03.json";
    ev.timestamp = 8000LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    // Both the adapter and reasoner should have processed the event.
    REQUIRE(registry.tool(0)->handledCount() >= 1);
    REQUIRE(reasoner.eventsProcessed() >= 1);
    REQUIRE(reasoner.violationCount() >= 1);  // R010 or R011 should match
}
