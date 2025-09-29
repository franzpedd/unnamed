# ------------------------------------------------------------------------------------------------------------- group files within same directory-tree inside visual studio
function(group_sources)
    foreach(file IN LISTS ARGN)
        get_filename_component(dir "${file}" DIRECTORY) # get the directory of the file relative to CMakeLists.txt

        if(dir STREQUAL "")
            set(group "root")  # group root files under "root"
        else()
            string(REPLACE "/" "\\" group "${dir}")  # replace "/" with "\\" for visual vtudio
        endif()

        source_group("${group}" FILES "${file}")
    endforeach()
endfunction()

# ------------------------------------------------------------------------------------------------------------- configuration
cmake_minimum_required(VERSION 3.22.1)
project(Editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

# ------------------------------------------------------------------------------------------------------------- project
set(SOURCES
    # imguizmo
    Thirdparty/imguizmo/imguizmo.h Thirdparty/imguizmo/imguizmo.cpp
    # imgui
    Thirdparty/imgui/imconfig.h
    Thirdparty/imgui/imgui_demo.cpp
    Thirdparty/imgui/imgui_draw.cpp
    Thirdparty/imgui/imgui_internal.h
    Thirdparty/imgui/imgui_tables.cpp
    Thirdparty/imgui/imgui_widgets.cpp
    Thirdparty/imgui/imgui.cpp
    Thirdparty/imgui/imgui.h
    Thirdparty/imgui/imstb_rectpack.h
    Thirdparty/imgui/imstb_textedit.h
    Thirdparty/imgui/imstb_truetype.h
    Thirdparty/imgui/backends/imgui_impl_sdl3.h # Thirdparty/imgui/backends/imgui_impl_sdl3.cpp
    Thirdparty/imgui/backends/imgui_impl_vulkan.h # Thirdparty/imgui/backends/imgui_impl_vulkan.cpp
    
    # project
    Source/Core/Application.h Source/Core/Application.cpp
    Source/Core/Defines.h
    Source/Core/Input.h
    Source/Core/Renderer.h Source/Core/Renderer.cpp
    Source/Core/Window.h Source/Core/Window.cpp
    #
    Source/UI/GUI.h Source/UI/GUI.cpp
    Source/UI/Icons.h
    Source/UI/Widget.h Source/UI/Widget.cpp
    #
    Source/Util/Memory.h
    Source/Util/Stack.h
    #
    Source/Cosmos.h
)
group_sources(${SOURCES})

add_library(Engine SHARED ${SOURCES})
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/imgui)

target_compile_definitions(Engine PRIVATE CREN_BUILD_WITH_VULKAN=1)
target_compile_definitions(Engine PRIVATE COSMOS_EXPORT)

set_target_properties(Engine PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Engine>")
set_target_properties(Engine PROPERTIES FOLDER "Projects")

# ------------------------------------------------------------------------------------------------------------- dependencies
find_package(Vulkan REQUIRED)

target_link_libraries(Engine PRIVATE CRen ctoolbox)
target_link_libraries(Engine PRIVATE SDL3::SDL3)
target_link_libraries(Engine PRIVATE Vulkan::Vulkan)

# must copy the SDL3 into the build folder
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3::SDL3>"
        "$<TARGET_FILE_DIR:Engine>"
    VERBATIM
)