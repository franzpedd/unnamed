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

option(USE_X11 "Enable Linux X11 support" OFF)
option(USE_WAYLAND "Enable Wayland support" OFF)

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
    # vecmath
    Thirdparty/vecmath/vecmath_basic_op.h Thirdparty/vecmath/vecmath_basic_op.c
    Thirdparty/vecmath/vecmath_defines.h
    Thirdparty/vecmath/vecmath_mat_op.h Thirdparty/vecmath/vecmath_mat_op.c
    Thirdparty/vecmath/vecmath_quat_op.h Thirdparty/vecmath/vecmath_quat_op.c
    Thirdparty/vecmath/vecmath_types.h
    Thirdparty/vecmath/vecmath_util.h Thirdparty/vecmath/vecmath_util.c
    Thirdparty/vecmath/vecmath_vec_op.h Thirdparty/vecmath/vecmath_vec_op.c
    Thirdparty/vecmath/vecmath.h

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

if(USE_X11 AND USE_WAYLAND)
    message(FATAL_ERROR "X11 and Wayland cannot both be enabled. Choose one.")
endif()

if(USE_X11)
    target_compile_definitions(CRen PRIVATE CREN_LINUX_X11=1)
    message(STATUS "CRen will use X11")
elseif(USE_WAYLAND)
    target_compile_definitions(CRen PRIVATE CREN_LINUX_WAYLAND=1)
    message(STATUS "CRen will use Wayland")
else()
    message(STATUS "No X11 nor Wayland system selected (use -DUSE_X11=ON or -DUSE_WAYLAND=ON)")
endif()

set_target_properties(CRen PROPERTIES FOLDER "CRen")
set_target_properties(CRen PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CRen>")

# ------------------------------------------------------------------------------------------------------------- dependencies
find_package(Vulkan REQUIRED)
target_link_libraries(CRen PRIVATE Vulkan::Vulkan ctoolbox)

