#pragma once
#include "terrain_generator.hpp"

enum class IslandType 
{
    CENTRAL,    // Donde está el portal
    VILLAGE,    // Aldea (fija)
    TEMPLE,     // Templo (fijo)
    RANDOM      // Generada al azar
};

struct Portal 
{
    sf::Vector2f position;      // Centro del portal en coordenadas del mundo
    float radius;               // Radio de colisión
    int target_island_id;       // A qué isla lleva
    sf::Vector2f spawn_position;// Dónde aparece el jugador en la isla destino
};

class Island 
{
public:
    int id;
    IslandType type;
    terrain_generator terrain;  
    sf::Vector2f world_position; // Coordenadas del mundo para saber dónde conectarla (solo para el mapa mental)
    std::vector<int> connections;// IDs de las islas a las que se conecta
    std::vector<Portal> portals;
    bool generated;

    Island(int id, IslandType type, int width, int height, unsigned int seed, sf::Vector2f pos);
    
    void generate();
    void add_portal(sf::Vector2f pos, int target_id, sf::Vector2f spawn_pos);
};