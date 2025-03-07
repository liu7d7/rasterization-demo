cmake_minimum_required(VERSION 3.22)
project(rasterization C)

include(C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_C_STANDARD 23)

add_subdirectory(lib/glfw)

add_executable(rasterization main.c lib/glad/glad.c lib/glad/glad.h lib/glad/khrplatform.h stbtt_impl.c)

add_compile_definitions(TRACY_ENABLE=1)

find_package(assimp CONFIG REQUIRED)

target_link_libraries(rasterization PRIVATE glfw assimp::assimp)
