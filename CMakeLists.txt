# ------------------------------------------------------------------------------------------------------------- configuration
cmake_minimum_required(VERSION 3.22.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTools") 

# ------------------------------------------------------------------------------------------------------------- options
option(BUILD_PROJECTS "Build the projects as well as CRen" ON)

# ------------------------------------------------------------------------------------------------------------- projects
project(Solution VERSION 1.0 LANGUAGES C)

include(Thirdparty/CMakeLists.txt)
add_subdirectory(CRen)
add_subdirectory(CRen/Thirdparty/ctoolbox)

if(BUILD_PROJECTS)
    add_subdirectory(Engine)
    add_subdirectory(Editor)
endif()