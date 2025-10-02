# ------------------------------------------------------------------------------------------------------------- group files within same directory-tree inside visual studio
function(group_sources)
    foreach(file IN LISTS ARGN)
        get_filename_component(dir "${file}" DIRECTORY)
        if(dir STREQUAL "")
            set(group "root")
        else()
            string(REPLACE "/" "\\" group "${dir}")
        endif()
        source_group("${group}" FILES "${file}")
    endforeach()
endfunction()

# ------------------------------------------------------------------------------------------------------------- configuration
cmake_minimum_required(VERSION 3.22.1)
project(CRen LANGUAGES C)
include(FetchContent)

set(CMAKE_C_STANDARD 11) 
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

# ------------------------------------------------------------------------------------------------------------- project
set(SOURCES
    # memm
    Thirdparty/memm/memm.h Thirdparty/memm/memm.c
    # stb
    Thirdparty/stb/stb_image_write.h Thirdparty/stb/stb_image.h Thirdparty/stb/stb_impl.c

    # project
    Source/Vulkan/crenvk_buffer.h Source/Vulkan/crenvk_buffer.c
    Source/Vulkan/crenvk_core.h Source/Vulkan/crenvk_core.c
    Source/Vulkan/crenvk_context.h Source/Vulkan/crenvk_context.c
    Source/Vulkan/crenvk_pipeline.h Source/Vulkan/crenvk_pipeline.c
    Source/Vulkan/crenvk_renderphase.h Source/Vulkan/crenvk_renderphase.c
    #
    Source/cren_callbacks.h
    Source/cren_camera.h Source/cren_camera.c
    Source/cren_context.h Source/cren_context.c
    Source/cren_defines.h
    Source/cren_error.h Source/cren_error.c
    Source/cren_platform.h Source/cren_platform.c
    Source/cren_types.h
    Source/cren.h
)
group_sources(${SOURCES})

add_library(CRen SHARED ${SOURCES})
target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty)

target_compile_definitions(CRen PRIVATE CREN_BUILD_WITH_VULKAN=1)
target_compile_definitions(CRen PRIVATE CREN_SHARED_LIBRARY=1 CREN_BUILDING_DLL=1)
target_compile_definitions(CRen PRIVATE MEMM_BUILD_SHARED=1 MEMM_EXPORTS=1) # memm shared library

set_target_properties(CRen PROPERTIES FOLDER "CRen")
set_target_properties(CRen PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CRen>")

# ------------------------------------------------------------------------------------------------------------- dependencies
find_package(Vulkan REQUIRED)
target_link_libraries(CRen PRIVATE Vulkan::Vulkan ctoolbox vecmath)