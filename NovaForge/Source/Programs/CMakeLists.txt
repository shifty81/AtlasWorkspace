# ── Executables ────────────────────────────────────────────────────

# Editor
if(NF_BUILD_EDITOR)
    add_executable(NovaForgeEditor NovaForgeEditor/main.cpp)
    target_link_libraries(NovaForgeEditor PRIVATE
        NF::NovaForgeAdapter
        NF::Editor NF::Game NF::World NF::AI NF::GraphVM
        NF::Renderer NF::Physics NF::Audio NF::Animation
        NF::Input NF::Networking NF::UI NF::Engine NF::Core
        NF_CompileOptions
    )
endif()

# Game Client
if(NF_BUILD_GAME)
    add_executable(NovaForgeGame NovaForgeGame/main.cpp)
    target_link_libraries(NovaForgeGame PRIVATE
        NF::Game NF::World NF::GraphVM
        NF::Renderer NF::Physics NF::Audio NF::Animation
        NF::Input NF::Networking NF::UI NF::Engine NF::Core
        NF_CompileOptions
    )
endif()

# Dedicated Server
if(NF_BUILD_SERVER)
    add_executable(NovaForgeServer NovaForgeServer/main.cpp)
    target_link_libraries(NovaForgeServer PRIVATE
        NF::Game NF::World NF::AI NF::GraphVM
        NF::Networking NF::Physics NF::Engine NF::Core
        NF_CompileOptions
    )
endif()
