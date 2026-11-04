#pragma once
#include "island.hpp"
#include <map.hpp>

class World 
{
private:
    Designar::TreeMap<int, Island> islands;
    int current_island_id;

public:
    World();
    
    // Crea las islas iniciales (por ahora, 3 islas fijas para probar)
    void create_test_world(unsigned int seed);
    
    // Cambia a otra isla
    void travel_to(int island_id);
    
    // Obtener la isla actual
    Island& get_current_island();
    int get_current_id() const;
};