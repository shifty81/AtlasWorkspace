# NF::NovaForgeAdapter — NovaForge project adapter for Atlas Workspace
#
# INTERFACE library: header-only. All implementation lives in
# NovaForgeAdapter.h (inline). Future .cpp files can be added here.
#
# Consumers link this target to gain access to NovaForge::NovaForgeAdapter
# and its transitive dependency on NF::Workspace (IGameProjectAdapter, etc.).

add_library(NF_NovaForgeAdapter INTERFACE)
target_include_directories(NF_NovaForgeAdapter INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(NF_NovaForgeAdapter INTERFACE NF::Workspace)
add_library(NF::NovaForgeAdapter ALIAS NF_NovaForgeAdapter)
