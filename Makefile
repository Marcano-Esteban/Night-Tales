CXX = g++
#CXXFLAGS = -std=c++17 -Wall -O2 -march=native 
TARGET = TerrainGenerator
CXXFLAGS = -std=c++17 -O3 -march=native
SOURCES = src/main.cpp src/noise_generator.cpp src/terrain_generator.cpp src/height_map.cpp src/utilyties.cpp src/player.cpp src/island.cpp src/world.cpp
DESIGNAR_INCLUDE = -I./DeSiGNAR/include

# Intentar usar pkg-config primero
PKG_CONFIG := $(shell command -v pkg-config 2> /dev/null)
ifdef PKG_CONFIG
    SFML_FLAGS := $(shell pkg-config --cflags --libs sfml-all 2>/dev/null || echo "")
endif

# Si pkg-config falla, usar rutas por defecto
ifeq ($(SFML_FLAGS),)
    SFML_FLAGS = -I/usr/include -L/usr/lib -lsfml-graphics -lsfml-window -lsfml-system
    $(warning "pkg-config no encontrado o SFML no encontrado, usando rutas por defecto")
endif

all: TerrainGenerator

TerrainGenerator: src/main.cpp
	g++ $(CXXFLAGS) $(DESIGNAR_INCLUDE) $(SOURCES) -o TerrainGenerator $(SFML_FLAGS)

# all: $(TARGET)

# $(TARGET): $(SOURCES)
#	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(SFML_FLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

rebuild: clean all

help:
	@echo "Comandos:"
	@echo "  make      - Compilar"
	@echo "  make run  - Compilar y ejecutar"
	@echo "  make clean - Limpiar"
	@echo "  make rebuild - Recompilar"

.PHONY: all clean run rebuild help