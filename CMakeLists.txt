cmake_minimum_required(VERSION 3.14)
project(NeonFluxGarden LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(neon_flux_garden src/main.cpp)

target_link_libraries(neon_flux_garden PRIVATE sfml-graphics sfml-window sfml-system)

target_compile_options(neon_flux_garden PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

if (MSVC)
    target_compile_options(neon_flux_garden PRIVATE /W4)
endif()

add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_BINARY_DIR}/resources)

add_dependencies(neon_flux_garden copy_resources)
