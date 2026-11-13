#include "../include/world.hpp"

World::World() : current_island(nullptr) {}

World::~World() 
{
    // Nota: Aquí deberías liberar la memoria de todas las islas. 
}

void World::create_test_world(unsigned int seed) 
{
    // 1. Crear Isla Central
    Island* central = new Island(0, IslandType::CENTRAL, 200, 200, seed, sf::Vector2f(0, 0));
    island_tree = new Designar::MTreeNode<Island*>(central); // ✅ Nodo raíz
    
    // 2. Crear Isla Aldea
    Island* _village = new Island(1, IslandType::VILLAGE, 150, 150, seed + 100, sf::Vector2f(-500, 0));
    auto village = new Designar::MTreeNode<Island*>(_village);
    island_tree->insert_child(village);

    // 3. Crear Isla Templo
    Island* _temple = new Island(2, IslandType::TEMPLE, 150, 150, seed + 200, sf::Vector2f(500, 0));
    auto temple = new Designar::MTreeNode<Island*>(_temple);
    island_tree->insert_child(temple); 

    // Generar y cargar SOLO la central al inicio
    central->load();
    current_island = central;
    
    const float TILE_SIZE = 50.0f;

    // 4. Conectar portales
    // Central -> Aldea
    sf::Vector2f edge_central_left = central->terrain.find_edge(-1.0f, 0.0f, TILE_SIZE);
    _village->load();
    sf::Vector2f edge_village_right = _village->terrain.find_edge(1.0f, 0.0f, TILE_SIZE);
    central->add_portal(edge_central_left, _village, edge_village_right - sf::Vector2f(100.0f, 0.0f));
    _village->unload(); // Descargamos la aldea inmediatamente

    // Central -> Templo
    sf::Vector2f edge_central_right = central->terrain.find_edge(1.0f, 0.0f, TILE_SIZE);
    _temple->load();
    sf::Vector2f edge_temple_left = _temple->terrain.find_edge(-1.0f, 0.0f, TILE_SIZE);
    central->add_portal(edge_central_right, _temple, edge_temple_left + sf::Vector2f(100.0f, 0.0f));
    _temple->unload();

    // Aldea -> Central
    _village->load();
    _village->add_portal(edge_village_right, central, edge_central_left + sf::Vector2f(100.0f, 0.0f));
    _village->unload();

    // Templo -> Central
    _temple->load();
    _temple->add_portal(edge_temple_left, central, edge_central_right - sf::Vector2f(100.0f, 0.0f));
    _temple->unload();
}

void World::travel_to(Island* target_island) 
{
    if (current_island) 
    {
        current_island->unload(); // Descargamos la isla actual
    }
    target_island->load(); // Cargamos la nueva
    current_island = target_island;
}

Island* World::get_current_island() 
{ 
    return current_island; 
}

int World::get_current_id() const 
{ 
    return current_island ? current_island->id : -1; 
}