set(VCPKG_TARGET_TRIPLET x64-windows)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Felix/Projects/vcpkg/scripts/buildsystems/vcpkg.cmake")
cmake_minimum_required(VERSION 3.16)

project (hello)

# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(hello main.c render2d.c)
target_link_libraries(hello PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_image OpenGL::GL GLEW::GLEW)
