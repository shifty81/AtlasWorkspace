#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>

// Include all new tool module headers to verify compilation
#include "Tools/ConsoleToolModule.h"
#include "Tools/ProfilerToolModule.h"
#include "Tools/ECSInspectorToolModule.h"
#include "Tools/MaterialEditorToolModule.h"
#include "Tools/MeshViewerToolModule.h"
#include "Tools/PrefabEditorToolModule.h"
#include "Tools/NetInspectorToolModule.h"
#include "Tools/AIDebuggerToolModule.h"
#include "Tools/WorldGraphToolModule.h"
#include "Tools/SceneGraphToolModule.h"
#include "Tools/RuleGraphEditorToolModule.h"
#include "Tools/CIDashboardToolModule.h"
#include "Tools/JobTraceToolModule.h"
#include "Tools/ProofViewerToolModule.h"
#include "Tools/DesyncVisualizerToolModule.h"
#include "Tools/StateHashDiffToolModule.h"
#include "Tools/DataBrowserToolModule.h"
#include "Tools/PhysicsTunerToolModule.h"

// Include existing tool infrastructure
#include "Tools/IEditorToolModule.h"

// ── Tool module Name() tests ─────────────────────────────────────

TEST_CASE("ConsoleToolModule Name", "[Editor][Tools]") {
    atlas::editor::ConsoleToolModule tool;
    REQUIRE(tool.Name() == "Console");
}

TEST_CASE("ProfilerToolModule Name", "[Editor][Tools]") {
    atlas::editor::ProfilerToolModule tool;
    REQUIRE(tool.Name() == "Profiler");
}

TEST_CASE("ECSInspectorToolModule Name", "[Editor][Tools]") {
    atlas::editor::ECSInspectorToolModule tool;
    REQUIRE(tool.Name() == "ECS Inspector");
}

TEST_CASE("MaterialEditorToolModule Name", "[Editor][Tools]") {
    atlas::editor::MaterialEditorToolModule tool;
    REQUIRE(tool.Name() == "Material Editor");
}

TEST_CASE("MeshViewerToolModule Name", "[Editor][Tools]") {
    atlas::editor::MeshViewerToolModule tool;
    REQUIRE(tool.Name() == "Mesh Viewer");
}

TEST_CASE("PrefabEditorToolModule Name", "[Editor][Tools]") {
    atlas::editor::PrefabEditorToolModule tool;
    REQUIRE(tool.Name() == "Prefab Editor");
}

TEST_CASE("NetInspectorToolModule Name", "[Editor][Tools]") {
    atlas::editor::NetInspectorToolModule tool;
    REQUIRE(tool.Name() == "Net Inspector");
}

TEST_CASE("AIDebuggerToolModule Name", "[Editor][Tools]") {
    atlas::editor::AIDebuggerToolModule tool;
    REQUIRE(tool.Name() == "AI Debugger");
}

TEST_CASE("WorldGraphToolModule Name", "[Editor][Tools]") {
    atlas::editor::WorldGraphToolModule tool;
    REQUIRE(tool.Name() == "World Graph");
}

TEST_CASE("SceneGraphToolModule Name", "[Editor][Tools]") {
    atlas::editor::SceneGraphToolModule tool;
    REQUIRE(tool.Name() == "Scene Graph");
}

TEST_CASE("RuleGraphEditorToolModule Name", "[Editor][Tools]") {
    atlas::editor::RuleGraphEditorToolModule tool;
    REQUIRE(tool.Name() == "Rule Graph Editor");
}

TEST_CASE("CIDashboardToolModule Name", "[Editor][Tools]") {
    atlas::editor::CIDashboardToolModule tool;
    REQUIRE(tool.Name() == "CI Dashboard");
}

TEST_CASE("JobTraceToolModule Name", "[Editor][Tools]") {
    atlas::editor::JobTraceToolModule tool;
    REQUIRE(tool.Name() == "Job Trace");
}

TEST_CASE("ProofViewerToolModule Name", "[Editor][Tools]") {
    atlas::editor::ProofViewerToolModule tool;
    REQUIRE(tool.Name() == "Proof Viewer");
}

TEST_CASE("DesyncVisualizerToolModule Name", "[Editor][Tools]") {
    atlas::editor::DesyncVisualizerToolModule tool;
    REQUIRE(tool.Name() == "Desync Visualizer");
}

TEST_CASE("StateHashDiffToolModule Name", "[Editor][Tools]") {
    atlas::editor::StateHashDiffToolModule tool;
    REQUIRE(tool.Name() == "State Hash Diff");
}

TEST_CASE("DataBrowserToolModule Name", "[Editor][Tools]") {
    atlas::editor::DataBrowserToolModule tool;
    REQUIRE(tool.Name() == "Data Browser");
}

TEST_CASE("PhysicsTunerToolModule Name", "[Editor][Tools]") {
    atlas::editor::PhysicsTunerToolModule tool;
    REQUIRE(tool.Name() == "Physics Tuner");
}

// ── IEditorToolModule interface compliance ───────────────────────

TEST_CASE("Tool modules implement IEditorToolModule interface", "[Editor][Tools]") {
    // Verify each tool can be used through the IEditorToolModule interface
    atlas::editor::ConsoleToolModule console;
    atlas::editor::ProfilerToolModule profiler;
    atlas::editor::ECSInspectorToolModule ecsInspector;

    atlas::editor::IEditorToolModule* tools[] = {
        &console, &profiler, &ecsInspector
    };

    for (auto* tool : tools) {
        REQUIRE_FALSE(tool->Name().empty());

        // These should not throw
        tool->OnRegister();
        tool->RegisterPanels();
        tool->RegisterMenus();
        tool->RegisterModes();
        tool->Update(0.016f);
        tool->Render();
        REQUIRE_FALSE(tool->HandleInput(0, false));
        tool->OnUnregister();
    }
}

TEST_CASE("All 18 tool modules have unique names", "[Editor][Tools]") {
    atlas::editor::ConsoleToolModule console;
    atlas::editor::ProfilerToolModule profiler;
    atlas::editor::ECSInspectorToolModule ecsInspector;
    atlas::editor::MaterialEditorToolModule materialEditor;
    atlas::editor::MeshViewerToolModule meshViewer;
    atlas::editor::PrefabEditorToolModule prefabEditor;
    atlas::editor::NetInspectorToolModule netInspector;
    atlas::editor::AIDebuggerToolModule aiDebugger;
    atlas::editor::WorldGraphToolModule worldGraph;
    atlas::editor::SceneGraphToolModule sceneGraph;
    atlas::editor::RuleGraphEditorToolModule ruleGraphEditor;
    atlas::editor::CIDashboardToolModule ciDashboard;
    atlas::editor::JobTraceToolModule jobTrace;
    atlas::editor::ProofViewerToolModule proofViewer;
    atlas::editor::DesyncVisualizerToolModule desyncVisualizer;
    atlas::editor::StateHashDiffToolModule stateHashDiff;
    atlas::editor::DataBrowserToolModule dataBrowser;
    atlas::editor::PhysicsTunerToolModule physicsTuner;

    std::set<std::string> names;
    atlas::tools::IToolModule* tools[] = {
        &console, &profiler, &ecsInspector, &materialEditor,
        &meshViewer, &prefabEditor, &netInspector, &aiDebugger,
        &worldGraph, &sceneGraph, &ruleGraphEditor, &ciDashboard,
        &jobTrace, &proofViewer, &desyncVisualizer, &stateHashDiff,
        &dataBrowser, &physicsTuner
    };

    for (auto* tool : tools) {
        auto result = names.insert(tool->Name());
        INFO("Duplicate tool name: " << tool->Name());
        REQUIRE(result.second); // insertion succeeded = name was unique
    }

    REQUIRE(names.size() == 18);
}

TEST_CASE("All tool modules survive full lifecycle", "[Editor][Tools]") {
    atlas::editor::MaterialEditorToolModule materialEditor;
    atlas::editor::WorldGraphToolModule worldGraph;
    atlas::editor::CIDashboardToolModule ciDashboard;
    atlas::editor::PhysicsTunerToolModule physicsTuner;

    atlas::editor::IEditorToolModule* tools[] = {
        &materialEditor, &worldGraph, &ciDashboard, &physicsTuner
    };

    for (auto* tool : tools) {
        // Full lifecycle: register → panels/menus/modes → use → unregister
        tool->OnRegister();
        tool->RegisterPanels();
        tool->RegisterMenus();
        tool->RegisterModes();

        for (int i = 0; i < 3; ++i) {
            tool->Update(0.016f);
            tool->Render();
        }

        REQUIRE_FALSE(tool->HandleInput(42, true));
        REQUIRE_FALSE(tool->HandleInput(42, false));

        tool->OnUnregister();
    }
}
