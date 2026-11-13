#pragma once
#include "island.hpp"
#include <mtreenode.hpp> 
#include <map.hpp>

class World 
{
private:
    Designar::MTreeNode<Island*>* island_tree;
    Designar::TreeMap<int, Island*> island_index; // Para búsquedas rápidas por IDs
    Island* current_island;

public:
    World();
    ~World();

    void create_test_world(unsigned int seed);
    void travel_to(Island* target_island); 
    
    Island* get_current_island();
    int get_current_id() const;
};