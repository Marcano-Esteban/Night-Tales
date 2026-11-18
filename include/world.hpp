#pragma once
#include "island.hpp"
#include <array.hpp>
#include <random>

enum class WorldStyle {
    ARBOL_ATAJOS,
    ARCHIPIELAGO,
    ANILLOS,
    REJILLA,
    ARBOL_PURO
};

class World {
private:
    Designar::DynArray<Island*> all_islands;
    Island* central;
    Island* current;
    int next_island_id;
    std::mt19937 rng;

    // Métodos internos de generación
    void generate_tree_structure(Island* parent, int max_depth);
    void layout_radial();

public:
    World();
    ~World();
    
    void create_world(unsigned int seed, WorldStyle style = WorldStyle::ARBOL_ATAJOS);
    void travel_to(Island* target_island);
    
    Island* get_current_island() 
    { 
        return current; 
    }
    
    int get_current_id() const 
    { 
        return current ? current->id : -1; 
    }

    const Designar::DynArray<Island*>& get_all_islands() const;
    
    Island* create_island(IslandType tipo, int w, int h, unsigned int semilla);
};