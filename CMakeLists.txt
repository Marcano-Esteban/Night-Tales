cmake_minimum_required(VERSION 3.10)
project(TerrainGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Crear ejecutable
add_executable(TerrainGenerator main.cpp)

# Enlazar SFML
target_link_libraries(TerrainGenerator
    sfml-graphics
    sfml-window
    sfml-system
)

# En CachyOS/Arch, a veces necesitas estas banderas
target_compile_options(TerrainGenerator PRIVATE -Wall -Wextra -O2)