#include "../include/world.hpp"

World::World() : current_island_id(0) {}

void World::create_test_world(unsigned int seed) 
{
    islands.emplace(0, Island(0, IslandType::CENTRAL, 200, 200, seed, sf::Vector2f(0, 0)));
    islands.emplace(1, Island(1, IslandType::VILLAGE, 200, 200, seed + 100, sf::Vector2f(-500, 0)));
    islands.emplace(2, Island(2, IslandType::TEMPLE, 200, 200, seed + 200, sf::Vector2f(500, 0)));
    
    islands.at(0).connections = {1, 2};
    islands.at(1).connections = {0};
    islands.at(2).connections = {0};

    islands.at(0).generate();
    islands.at(1).generate();
    islands.at(2).generate();

    const float TILE_SIZE = 50.0f; // Para calcular las coordenadas

    // 1. Isla Central -> Isla Izquierda
    sf::Vector2f edge_0_left = islands.at(0).terrain.find_edge(-1.0f, 0.0f, TILE_SIZE);
    sf::Vector2f edge_1_right = islands.at(1).terrain.find_edge(1.0f, 0.0f, TILE_SIZE);
    
    // El portal en la isla 0 se pone en el borde izquierdo.
    // El spawn en la isla 1 es un poco adentro del borde derecho (restamos 100 unidades)
    islands.at(0).add_portal(
        edge_0_left, 
        1, 
        edge_1_right - sf::Vector2f(100.0f, 0.0f) 
    );

    // 2. Isla Central -> Isla Derecha
    sf::Vector2f edge_0_right = islands.at(0).terrain.find_edge(1.0f, 0.0f, TILE_SIZE);
    sf::Vector2f edge_2_left = islands.at(2).terrain.find_edge(-1.0f, 0.0f, TILE_SIZE);
    
    islands.at(0).add_portal(
        edge_0_right, 
        2, 
        edge_2_left + sf::Vector2f(100.0f, 0.0f) 
    );

    // 3. Isla Izquierda -> Central (vuelta)
    islands.at(1).add_portal(
        edge_1_right, 
        0, 
        edge_0_left + sf::Vector2f(100.0f, 0.0f) 
    );

    // 4. Isla Derecha -> Central (vuelta)
    islands.at(2).add_portal(
        edge_2_left, 
        0, 
        edge_0_right - sf::Vector2f(100.0f, 0.0f) 
    );

    travel_to(0);
}

void World::travel_to(int island_id) 
{
    if (islands.find(island_id) != islands.end()) 
    {
        current_island_id = island_id;
        // Genera la isla (si no está generada aún)
        islands.at(current_island_id).generate(); 
    }
}

Island& World::get_current_island() 
{
    return islands.at(current_island_id);
}

int World::get_current_id() const 
{
    return current_island_id;
}
