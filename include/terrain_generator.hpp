#pragma once

#include <SFML/Graphics.hpp>
#include "noise_generator.hpp"
#include "../include/height_map.hpp"

// Estructura para un objeto del mapa
enum class PropType { TREE, FLOWER };

struct Prop 
{
    sf::Vector2f position;
    PropType type;
    float collision_radius; // 0.0 si no tiene colisión
    sf::Color color; 
};


class terrain_generator 
{
private:
    noise_generator noise;
    height_map _height_map;

    sf::VertexArray _vertices;
    bool _vertices_dirty;
    
    Designar::DynArray<Prop> _props;
    std::mt19937 rng;
    
public:
    terrain_generator(int width, int height, unsigned int seed = 42);

    void generate(float frequency = 0.05f, float seaLevel = 0.04f);
    void rebuild_vertices(float tile_size);
    void draw(sf::RenderWindow& window, float tileSize);

    void generate_props(float tile_size);
    void draw_props(sf::RenderWindow& window, float tile_size);
    const Designar::DynArray<Prop>& get_props() const;

    sf::Color get_color(float height);
    const height_map& get_height_map() const;

    sf::Vector2f find_spawn_point(float tile_size) const;
    sf::Vector2f find_edge(float dir_x, float dir_y, float tile_size) const; // Busca el borde de la isla en una dirección dada (ej. -1, 0 = izquierda)
};
