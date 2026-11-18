#pragma once

#include <SFML/Graphics.hpp>
#include "noise_generator.hpp"
#include "../include/height_map.hpp"
#include "../include/prop.hpp"

class terrain_generator 
{
private:
    noise_generator noise;
    height_map _height_map;

    sf::VertexArray _vertices;
    bool _vertices_dirty;
    
    std::mt19937 rng;
    
public:
    terrain_generator(int width, int height, unsigned int seed = 42);

    void clear();
    void generate(float frequency = 0.05f, float seaLevel = 0.04f);
    void rebuild_vertices(float tile_size);
    void draw(sf::RenderWindow& window, float tileSize);

    void generate_props(float tile_size, Designar::DynArray<Prop>& out);
    void draw_props(sf::RenderWindow& window, const Designar::DynArray<Prop>& props, float tile_size);

    sf::Color get_color(float height);
    const height_map& get_height_map() const;

    sf::Vector2f find_spawn_point(float tile_size) const;
    sf::Vector2f find_edge(float dir_x, float dir_y, float tile_size) const; // Busca el borde de la isla en una dirección dada (ej. -1, 0 = izquierda)
};
