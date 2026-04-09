# ── Atlas Engine + Workspace test suite ──────────────────────────

# Guard: if tests are somehow added but online deps are disabled and
# Catch2 is not already available, fail early with a clear message.
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    if(NOT ATLAS_ENABLE_ONLINE_DEPS)
        message(FATAL_ERROR
            "Tests requested but Catch2 was not found locally and "
            "ATLAS_ENABLE_ONLINE_DEPS is OFF. Either:\n"
            "  - Set ATLAS_ENABLE_ONLINE_DEPS=ON to fetch from GitHub, or\n"
            "  - Vendor Catch2 locally under ThirdParty/Catch2, or\n"
            "  - Set ATLAS_BUILD_TESTS=OFF to skip tests.")
    endif()

    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# On MSVC, force serialised PDB writes so parallel builds don't corrupt Catch2d.pdb
if(MSVC)
    target_compile_options(Catch2 PRIVATE /FS)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# ── Route all test executables into bin/Tests/ ────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Tests)
foreach(CFG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CFG} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CFG}/Tests)
endforeach()

# Core tests
add_executable(NF_CoreTests
    Core/test_core.cpp
    catch_main.cpp
)
target_link_libraries(NF_CoreTests PRIVATE NF::Core Catch2::Catch2)
catch_discover_tests(NF_CoreTests)

# Engine tests
add_executable(NF_EngineTests
    Engine/test_engine.cpp
    catch_main.cpp
)
target_link_libraries(NF_EngineTests PRIVATE NF::Engine NF::Core Catch2::Catch2)
catch_discover_tests(NF_EngineTests)

# Physics tests
add_executable(NF_PhysicsTests
    Physics/test_physics.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhysicsTests PRIVATE NF::Physics NF::Core Catch2::Catch2)
catch_discover_tests(NF_PhysicsTests)

# Audio tests
add_executable(NF_AudioTests
    Audio/test_audio.cpp
    catch_main.cpp
)
target_link_libraries(NF_AudioTests PRIVATE NF::Audio NF::Core Catch2::Catch2)
catch_discover_tests(NF_AudioTests)

# Animation tests
add_executable(NF_AnimationTests
    Animation/test_animation.cpp
    Animation/test_s5_animation.cpp
    catch_main.cpp
)
target_link_libraries(NF_AnimationTests PRIVATE NF::Animation NF::Core Catch2::Catch2)
catch_discover_tests(NF_AnimationTests)

# Input tests
add_executable(NF_InputTests
    Input/test_input.cpp
    catch_main.cpp
)
target_link_libraries(NF_InputTests PRIVATE NF::Input NF::Core Catch2::Catch2)
catch_discover_tests(NF_InputTests)

# Networking tests
add_executable(NF_NetworkingTests
    Networking/test_networking.cpp
    catch_main.cpp
)
target_link_libraries(NF_NetworkingTests PRIVATE NF::Networking NF::Engine NF::Core Catch2::Catch2)
catch_discover_tests(NF_NetworkingTests)

# GraphVM tests
add_executable(NF_GraphVMTests
    GraphVM/test_graphvm.cpp
    catch_main.cpp
)
target_link_libraries(NF_GraphVMTests PRIVATE NF::GraphVM NF::Core Catch2::Catch2)
catch_discover_tests(NF_GraphVMTests)

# Renderer tests
add_executable(NF_RendererTests
    Renderer/test_renderer.cpp
    catch_main.cpp
)
target_link_libraries(NF_RendererTests PRIVATE NF::Renderer NF::Core Catch2::Catch2)
catch_discover_tests(NF_RendererTests)

# AI tests
add_executable(NF_AITests
    AI/test_ai.cpp
    catch_main.cpp
)
target_link_libraries(NF_AITests PRIVATE NF::AI NF::Engine NF::Core NF::GraphVM Catch2::Catch2)
catch_discover_tests(NF_AITests)

# Editor tests
add_executable(NF_EditorTests
    Editor/test_editor.cpp
    Editor/test_m2_editor.cpp
    Editor/test_m3_editor.cpp
    Editor/test_m4_editor.cpp
    Editor/test_m1c_editor.cpp
    Editor/test_tool_modules.cpp
    Editor/test_workspace_shell.cpp
    Editor/test_asset_editor_tool.cpp
    catch_main.cpp
)
target_include_directories(NF_EditorTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Editor/src
)
target_link_libraries(NF_EditorTests PRIVATE
    NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_EditorTests)

# Pipeline tests
add_executable(NF_PipelineTests
    Pipeline/test_pipeline.cpp
    Pipeline/test_tool_wiring.cpp
    Pipeline/test_blender_bridge.cpp
    Pipeline/test_workspace_broker.cpp
    Pipeline/test_arbiter_reasoner.cpp
    Pipeline/test_full_suite.cpp
    catch_main.cpp
)
target_link_libraries(NF_PipelineTests PRIVATE NF::Pipeline NF::Core Catch2::Catch2)
catch_discover_tests(NF_PipelineTests)

# UI tests
add_executable(NF_UITests
    UI/test_ui.cpp
    UI/test_atlasui.cpp
    UI/test_m1_editor.cpp
    UI/test_widgets_v2.cpp
    catch_main.cpp
)
target_link_libraries(NF_UITests PRIVATE NF::UI NF::Core NF::Input Catch2::Catch2)
catch_discover_tests(NF_UITests)

# Workspace wiring tests (DrawListDispatcher, InputBridge, new widgets)
add_executable(NF_WorkspaceWiringTests
    UI/test_workspace_wiring.cpp
    catch_main.cpp
)
target_link_libraries(NF_WorkspaceWiringTests PRIVATE NF::Editor NF::UI NF::Core NF::Input Catch2::Catch2)
catch_discover_tests(NF_WorkspaceWiringTests)
