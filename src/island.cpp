#include "../include/island.hpp"

Island::Island(int id, IslandType type, int width, int height, unsigned int seed, sf::Vector2f pos)
    : id(id), type(type), terrain(width, height, seed), world_position(pos), generated(false)
{
     // empty
}

void Island::generate() 
{
    if (generated) return; 

    terrain.generate();// Generamos el terreno
    terrain.rebuild_vertices(50.0f);// Reconstruimos los vértices con el TILE_SIZE que uses (50.0f)
    terrain.generate_props(50.0f);// Generamos los árboles y flores
    generated = true;
}

void Island::add_portal(sf::Vector2f pos, int target_id, sf::Vector2f spawn_pos) 
{
    portals.push_back({pos, 60.0f, target_id, spawn_pos}); // Radio de 60 unidades
}