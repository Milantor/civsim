cmake_minimum_required(VERSION 4.2)
project(civsim)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#libs ops
set(BUILD_EXAMPLES OFF CACHE BOOL "Build raylib examples" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

set(GLFW_BUILD_DOCS ON CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "Build support for Wayland" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "Build support for X11" FORCE)
set(GLFW_INSTALL ON CACHE BOOL "Generate installation target" FORCE)

set(ENTT_BUILD_DOCS ON CACHE BOOL "Enable building with documentation." FORCE)
set(ENTT_INCLUDE_HEADERS ON CACHE BOOL "Add all EnTT headers to EnTT target." FORCE)
set(ENTT_INSTALL ON CACHE BOOL "Install EnTT and related files." FORCE)


add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CIVSIM_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/resources"
)


set(FETCHCONTENT_QUIET OFF)                # show what is being downloaded/built
set(CMAKE_FETCHCONTENT_TIMEOUT 600)        # 10 minute timeout
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
#load libs
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    raylib_cpp
    GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp
    GIT_TAG v5.5.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib_cpp)

FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG v3.16.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(EnTT)

set(FETCHLIBS "raylib" "raylib_cpp" "EnTT::EnTT")

#yeah
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp")
target_sources(${PROJECT_NAME} PRIVATE ${MY_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${FETCHLIBS})