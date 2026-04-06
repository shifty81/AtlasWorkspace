cmake_minimum_required(VERSION 3.20)

# Must be set before project() to take effect via CMP0091
cmake_policy(SET CMP0091 NEW)

project(AtlasWorkspace VERSION 0.1.0 LANGUAGES CXX)

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

# ── Build options ─────────────────────────────────────────────────
option(ATLAS_BUILD_TESTS "Build workspace test suite" OFF)

# ── Workspace platform modules (static libraries) ─────────────────
add_subdirectory(Source/Core)
add_subdirectory(Source/Renderer)
add_subdirectory(Source/UI)
add_subdirectory(Source/GraphVM)
add_subdirectory(Source/AI)
add_subdirectory(Source/Pipeline)
add_subdirectory(Source/Editor)

if(ATLAS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

message(STATUS "")
message(STATUS "=== AtlasWorkspace ${PROJECT_VERSION} ===")
message(STATUS "  Build Tests:   ${ATLAS_BUILD_TESTS}")
message(STATUS "")
