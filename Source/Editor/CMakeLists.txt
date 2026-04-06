add_library(NF_Editor STATIC src/Editor.cpp)
target_include_directories(NF_Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Editor PUBLIC NF::Core NF::Renderer NF::UI NF::GraphVM NF::Pipeline PRIVATE NF_CompileOptions)
add_library(NF::Editor ALIAS NF_Editor)
