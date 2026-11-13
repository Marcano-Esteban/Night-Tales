#pragma once
#include "terrain_generator.hpp"

enum class IslandType 
{
    CENTRAL,
    VILLAGE,
    TEMPLE,
    RANDOM
};

class Island;

struct Portal 
{
    sf::Vector2f position;      // Centro del portal en coordenadas del mundo
    float radius;               // Radio de colisión
    Island* target_island;      // A qué isla lleva
    sf::Vector2f spawn_position;
}; 

class Island 
{
public:
    int id;
    IslandType type;
    terrain_generator terrain;  
    sf::Vector2f world_position; // Coordenadas del mundo para saber dónde conectarla (solo para el mapa mental)
    Designar::DynArray<int> connections;// IDs de las islas a las que se conecta
    Designar::DynArray<Portal> portals;
    bool generated;
    bool is_loaded = false;
    
    Island(int id, IslandType type, int width, int height, unsigned int seed, sf::Vector2f pos);

    void load();   // Genera el terreno y props
    void unload(); // Libera la memoria (vértices y props)
    void generate();
    void add_portal(sf::Vector2f pos, Island* target, sf::Vector2f spawn_pos);
};