# ── Atlas Engine + Workspace test suite ──────────────────────────

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

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
    Editor/test_s4_editor.cpp
    Editor/test_s6_editor.cpp
    Editor/test_s7_editor.cpp
    Editor/test_s8_editor.cpp
    Editor/test_s9_editor.cpp
    Editor/test_s10_editor.cpp
    Editor/test_s11_editor.cpp
    Editor/test_s12_editor.cpp
    Editor/test_s13_editor.cpp
    Editor/test_s14_editor.cpp
    Editor/test_s15_editor.cpp
    Editor/test_s16_editor.cpp
    Editor/test_s17_editor.cpp
    Editor/test_s18_editor.cpp
    Editor/test_s19_editor.cpp
    Editor/test_s20_editor.cpp
    Editor/test_s21_editor.cpp
    Editor/test_s22_editor.cpp
    Editor/test_s23_editor.cpp
    Editor/test_s24_editor.cpp
    Editor/test_s25_editor.cpp
    Editor/test_s26_editor.cpp
    Editor/test_s27_editor.cpp
    Editor/test_s28_editor.cpp
    Editor/test_s29_editor.cpp
    Editor/test_s30_editor.cpp
    Editor/test_s31_editor.cpp
    Editor/test_s32_editor.cpp
    Editor/test_s33_editor.cpp
    Editor/test_s34_editor.cpp
    Editor/test_s35_editor.cpp
    Editor/test_s36_editor.cpp
    Editor/test_s37_editor.cpp
    Editor/test_s38_editor.cpp
    Editor/test_s39_editor.cpp
    Editor/test_s40_editor.cpp
    Editor/test_s41_editor.cpp
    Editor/test_s42_editor.cpp
    Editor/test_s43_editor.cpp
    Editor/test_s44_editor.cpp
    Editor/test_s45_editor.cpp
    Editor/test_s46_editor.cpp
    Editor/test_s47_editor.cpp
    Editor/test_s48_editor.cpp
    Editor/test_s49_editor.cpp
    Editor/test_s50_editor.cpp
    Editor/test_s51_editor.cpp
    Editor/test_s52_editor.cpp
    Editor/test_s53_editor.cpp
    Editor/test_s54_editor.cpp
    Editor/test_s55_editor.cpp
    Editor/test_s56_editor.cpp
    Editor/test_s57_editor.cpp
    Editor/test_s58_editor.cpp
    Editor/test_s59_editor.cpp
    Editor/test_s60_editor.cpp
    Editor/test_s61_editor.cpp
    Editor/test_s62_editor.cpp
    Editor/test_s63_editor.cpp
    Editor/test_s64_editor.cpp
    Editor/test_s65_editor.cpp
    Editor/test_s66_editor.cpp
    Editor/test_s67_editor.cpp
    Editor/test_s68_editor.cpp
    Editor/test_s69_editor.cpp
    Editor/test_s70_editor.cpp
    Editor/test_s71_editor.cpp
    Editor/test_s72_editor.cpp
    Editor/test_s73_editor.cpp
    Editor/test_s74_editor.cpp
    Editor/test_s75_editor.cpp
    Editor/test_s76_editor.cpp
    Editor/test_s77_editor.cpp
    Editor/test_s78_editor.cpp
    Editor/test_s79_editor.cpp
    Editor/test_s80_editor.cpp
    Editor/test_s81_editor.cpp
    Editor/test_s82_editor.cpp
    Editor/test_s83_editor.cpp
    Editor/test_s84_editor.cpp
    Editor/test_s85_editor.cpp
    Editor/test_s86_editor.cpp
    Editor/test_s87_editor.cpp
    Editor/test_s88_editor.cpp
    Editor/test_s89_editor.cpp
    Editor/test_s90_editor.cpp
    Editor/test_s91_editor.cpp
    Editor/test_s92_editor.cpp
    Editor/test_s93_editor.cpp
    Editor/test_s94_editor.cpp
    Editor/test_s95_editor.cpp
    Editor/test_s96_editor.cpp
    Editor/test_s97_editor.cpp
    Editor/test_s98_editor.cpp
    Editor/test_s99_editor.cpp
    Editor/test_s100_editor.cpp
    Editor/test_s101_editor.cpp
    Editor/test_s102_editor.cpp
    Editor/test_s103_editor.cpp
    Editor/test_s104_editor.cpp
    Editor/test_s105_editor.cpp
    Editor/test_s106_editor.cpp
    Editor/test_s107_editor.cpp
    Editor/test_s108_editor.cpp
    Editor/test_s109_editor.cpp
    Editor/test_s110_editor.cpp
    Editor/test_s111_editor.cpp
    Editor/test_s112_editor.cpp
    Editor/test_s113_editor.cpp
    Editor/test_s114_editor.cpp
    Editor/test_s115_editor.cpp
    Editor/test_s116_editor.cpp
    Editor/test_s117_editor.cpp
    Editor/test_s118_editor.cpp
    Editor/test_s119_editor.cpp
    Editor/test_s120_editor.cpp
    Editor/test_s121_editor.cpp
    Editor/test_s122_editor.cpp
    Editor/test_s123_editor.cpp
    Editor/test_m1c_editor.cpp
    Editor/test_tool_modules.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/ConsoleToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/ProfilerToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/ECSInspectorToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/MaterialEditorToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/MeshViewerToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/PrefabEditorToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/AIDebuggerToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/WorldGraphToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/SceneGraphToolModule.cpp
    ${CMAKE_SOURCE_DIR}/Source/Editor/src/Tools/PhysicsTunerToolModule.cpp
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
