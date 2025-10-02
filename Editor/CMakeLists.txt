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
project(Editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

# ------------------------------------------------------------------------------------------------------------- project
set(SOURCES
    Source/Core/Editor.h Source/Core/Editor.cpp
    Source/main.cpp
)
group_sources(${SOURCES})

add_executable(Editor ${SOURCES})
target_include_directories(Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)

set_target_properties(Editor PROPERTIES FOLDER "Projects")
set_target_properties(Editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Editor>")

# ------------------------------------------------------------------------------------------------------------- dependencies
target_link_libraries(Editor PRIVATE CRen Engine)
target_link_directories(Editor PUBLIC ${CMAKE_SOURCE_DIR}/Bin)