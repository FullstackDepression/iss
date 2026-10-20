cmake_minimum_required(VERSION 3.14)
project(iss LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SGP4: github.com/dnwrnr/sgp4 (Apache-2.0), сабмодуль.
# Собираю только libsgp4, без её тестов и примеров.
file(GLOB SGP4_SOURCES ${CMAKE_SOURCE_DIR}/sgp4/libsgp4/*.cpp)
add_library(sgp4 STATIC ${SGP4_SOURCES})
target_include_directories(sgp4 PUBLIC ${CMAKE_SOURCE_DIR}/sgp4)

add_executable(iss main.cpp is_visible.cpp)
target_link_libraries(iss PRIVATE sgp4)
target_compile_options(iss PRIVATE -Wall -Wextra -g -fsanitize=address,undefined)
target_link_options(iss PRIVATE -fsanitize=address,undefined)
