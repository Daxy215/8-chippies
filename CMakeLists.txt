cmake_minimum_required(VERSION 3.20)
project(8_chippy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

find_package(OpenGL REQUIRED)
find_package(glfw3 QUIET)

if (NOT glfw3_FOUND)
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif ()

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
        miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio.git
        GIT_TAG master
)
FetchContent_MakeAvailable(miniaudio)

add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui SYSTEM PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC OpenGL::GL glfw)

add_executable(8_chippy
        main.cpp
        src/chip8/Audio.cpp
        src/chip8/Cpu.cpp
        src/chip8/Gpu.cpp
        src/chip8/Input.cpp
        src/chip8/Interconnect.cpp
        src/chip8/Platform.cpp
        src/chip8/Timer.cpp
)

target_include_directories(8_chippy PRIVATE src)
target_include_directories(8_chippy SYSTEM PRIVATE ${miniaudio_SOURCE_DIR})
target_link_libraries(8_chippy PRIVATE imgui)

if (WIN32)
    target_link_libraries(8_chippy PRIVATE comdlg32)
endif ()
