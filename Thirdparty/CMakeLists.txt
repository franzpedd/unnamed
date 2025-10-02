# ------------------------------------------------------------------------------------------------------------- dependencies
find_package(Git REQUIRED)

# ------------------------------------------------------------------------------------------------------------- dependency: memm
set(MEMM_DIR ${CMAKE_SOURCE_DIR}/CRen/Thirdparty/memm)
if(NOT EXISTS ${MEMM_DIR})
    execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/franzpedd/memm.git ${MEMM_DIR})
endif()

# ------------------------------------------------------------------------------------------------------------- dependency: vecmath
set(VECMATH_DIR ${CMAKE_SOURCE_DIR}/CRen/Thirdparty/vecmath)
if(NOT EXISTS ${VECMATH_DIR})
    execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/franzpedd/vecmath.git ${VECMATH_DIR})
endif()

set(CMAKE_FOLDER "Thirdparty/vecmath") 
    add_subdirectory(${CMAKE_SOURCE_DIR}/CRen/Thirdparty/vecmath)
unset(CMAKE_FOLDER)

# ------------------------------------------------------------------------------------------------------------- dependency: ctoolbox
set(CTOOLBOX_DIR ${CMAKE_SOURCE_DIR}/CRen/Thirdparty/ctoolbox)
if(NOT EXISTS ${CTOOLBOX_DIR})
    execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/franzpedd/ctoolbox.git ${CTOOLBOX_DIR})
endif()

set(CMAKE_FOLDER "Thirdparty/ctoolbox") 
    add_subdirectory(${CMAKE_SOURCE_DIR}/CRen/Thirdparty/ctoolbox)
unset(CMAKE_FOLDER)

# ------------------------------------------------------------------------------------------------------------- thirdparty dependency: Dear ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/Engine/Thirdparty/imgui)
if(NOT EXISTS ${IMGUI_DIR})
    execute_process(COMMAND ${GIT_EXECUTABLE} clone --branch docking https://github.com/ocornut/imgui.git  ${IMGUI_DIR})
endif()

# ------------------------------------------------------------------------------------------------------------- thirdparty dependecy: SDL
set(SDL_DIR ${CMAKE_SOURCE_DIR}/Engine/Thirdparty/sdl)
if(NOT EXISTS ${SDL_DIR})
    execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/libsdl-org/SDL.git ${SDL_DIR})
endif()

# adding sdl into a separate path, since it is it's own project
set(CMAKE_FOLDER "Thirdparty/SDL") 
    add_subdirectory(${CMAKE_SOURCE_DIR}/Engine/Thirdparty/sdl)
unset(CMAKE_FOLDER)

