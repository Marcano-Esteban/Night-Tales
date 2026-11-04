#include "../include/world.hpp"

World::World() : current_island_id(0) {}

void World::create_test_world(unsigned int seed) 
{
    islands.insert(0, Island(0, IslandType::CENTRAL, 200, 200, seed, sf::Vector2f(0, 0)));
    islands.insert(1, Island(1, IslandType::VILLAGE, 200, 200, seed + 100, sf::Vector2f(-500, 0)));
    islands.insert(2, Island(2, IslandType::TEMPLE, 200, 200, seed + 200, sf::Vector2f(500, 0)));
    
    islands.find(0).connections = {1, 2};
    islands.find(1).connections = {0};
    islands.find(2).connections = {0};

    islands.find(0).generate();
    islands.find(1).generate();
    islands.find(2).generate();

    const float TILE_SIZE = 50.0f; // Para calcular las coordenadas

    // 1. Isla Central -> Isla Izquierda
    sf::Vector2f edge_0_left = islands.find(0).terrain.find_edge(-1.0f, 0.0f, TILE_SIZE);
    sf::Vector2f edge_1_right = islands.find(1).terrain.find_edge(1.0f, 0.0f, TILE_SIZE);
    
    // El portal en la isla 0 se pone en el borde izquierdo.
    // El spawn en la isla 1 es un poco adentro del borde derecho (restamos 100 unidades)
    islands.find(0).add_portal(edge_0_left, 1, edge_1_right - sf::Vector2f(100.0f, 0.0f));

    // 2. Isla Central -> Isla Derecha
    sf::Vector2f edge_0_right = islands.find(0).terrain.find_edge(1.0f, 0.0f, TILE_SIZE);
    sf::Vector2f edge_2_left = islands.find(2).terrain.find_edge(-1.0f, 0.0f, TILE_SIZE);
    
    islands.find(0).add_portal(edge_0_right, 2, edge_2_left + sf::Vector2f(100.0f, 0.0f));

    // 3. Isla Izquierda -> Central (vuelta)
    islands.find(1).add_portal(edge_1_right, 0, edge_0_left + sf::Vector2f(100.0f, 0.0f));

    // 4. Isla Derecha -> Central (vuelta)
    islands.find(2).add_portal(edge_2_left, 0, edge_0_right - sf::Vector2f(100.0f, 0.0f));

    travel_to(0);
}

void World::travel_to(int island_id) 
{
    try 
    {
        Island& island = islands.find(island_id);
        current_island_id = island_id;
        island.generate();
    } 
    catch (...) 
    {
        // La isla no existe (no debería pasar en el flujo normal)
    }
}

Island& World::get_current_island() 
{
    return islands.find(current_island_id);
}

int World::get_current_id() const 
{
    return current_island_id;
}
