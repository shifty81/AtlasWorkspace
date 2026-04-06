cmake_minimum_required(VERSION 3.20)

# Must be set before project() to take effect via CMP0091
cmake_policy(SET CMP0091 NEW)

project(NovaForge VERSION 0.1.0 LANGUAGES CXX)

# Enforce a consistent MSVC runtime library across all targets to avoid LNK4098
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── CMake helper modules ──────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Apply Windows-specific compile definitions when targeting Windows.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(WindowsToolchain)
endif()

# ── Global compile settings ───────────────────────────────────────
# Applied per-target via a common interface library to avoid polluting
# FetchContent dependencies with our warning flags.
add_library(NF_CompileOptions INTERFACE)
target_compile_options(NF_CompileOptions INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /Zc:__cplusplus /utf-8 /MP>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Output directories — applications land in bin/, tests in bin/Tests/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-config output directories (Visual Studio / multi-config generators)
foreach(CFG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CFG} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CFG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CFG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CFG})
endforeach()

# ── Third-party packages ──────────────────────────────────────────
# ImGui is banned — the project uses NF::UI (Atlas UI) exclusively.
# See ATLAS_CORE_CONTRACT.md §6.

# ── Build mode ────────────────────────────────────────────────────
option(NF_STANDALONE "Standalone build (all targets enabled by default)" ON)
option(NF_HOSTED     "Workspace-hosted build (targets disabled by default, Workspace controls)" OFF)

# In hosted mode, disable executables by default (Workspace controls them)
if(NF_HOSTED AND NOT NF_STANDALONE)
    set(_NF_DEFAULT_BUILD_TARGETS OFF)
else()
    set(_NF_DEFAULT_BUILD_TARGETS ON)
endif()

# Build options
option(NF_BUILD_EDITOR  "Build NovaForge Editor"             ${_NF_DEFAULT_BUILD_TARGETS})
option(NF_BUILD_GAME    "Build NovaForge Game Runtime"       ${_NF_DEFAULT_BUILD_TARGETS})
option(NF_BUILD_SERVER  "Build NovaForge Dedicated Server"   ${_NF_DEFAULT_BUILD_TARGETS})
option(NF_BUILD_TESTS   "Build test suite"                   OFF)

# Core modules (static libraries)
add_subdirectory(Source/Core)
add_subdirectory(Source/Engine)
add_subdirectory(Source/Renderer)
add_subdirectory(Source/Physics)
add_subdirectory(Source/Audio)
add_subdirectory(Source/Animation)
add_subdirectory(Source/Input)
add_subdirectory(Source/Networking)
add_subdirectory(Source/UI)
add_subdirectory(Source/Game)

# Expanded modules (active in tempnovaforge)
add_subdirectory(Source/GraphVM)   # Phase 5
add_subdirectory(Source/World)     # Phase 7
add_subdirectory(Source/AI)        # AtlasAI broker systems
add_subdirectory(Source/Pipeline)  # S0 Pipeline Core

# Expanded modules (stubs if not yet present)
# add_subdirectory(Source/GraphVM)   # Phase 5
# add_subdirectory(Source/World)     # Phase 7
# add_subdirectory(Source/AI)        # Pending AtlasAI merge

if(NF_BUILD_EDITOR)
    add_subdirectory(Source/Editor)
endif()

# Executables
add_subdirectory(Source/Programs)

if(NF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

message(STATUS "")
message(STATUS "=== NovaForge ${PROJECT_VERSION} ===")
message(STATUS "  Standalone:    ${NF_STANDALONE}")
message(STATUS "  Hosted:        ${NF_HOSTED}")
message(STATUS "  Build Editor:  ${NF_BUILD_EDITOR}")
message(STATUS "  Build Game:    ${NF_BUILD_GAME}")
message(STATUS "  Build Server:  ${NF_BUILD_SERVER}")
message(STATUS "  Build Tests:   ${NF_BUILD_TESTS}")
message(STATUS "")
