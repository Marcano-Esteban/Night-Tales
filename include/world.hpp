#pragma once
#include "island.hpp"
#include <array.hpp>
#include <random>

enum class LayoutStyle 
{
    PARENT_RADIAL,     // Alrededor del padre (con jitter)
    CENTER_RADIAL,     // Anillos concéntricos desde el centro
    RANDOM_SCATTER     // Archipiélago aleatorio
};

enum class ConnectionStyle 
{
    TREE_ONLY,         // Solo padre-hijo
    TREE_SHORTCUTS,    // Árbol + atajos
    RINGS,             // Anillos + puentes
    PROXIMITY,         // Proximidad + BFS (archipiélago)
    REGILLA
};

class World {
private:
    Designar::DynArray<Island*> all_islands;
    Island* central;
    Island* current;
    int next_island_id;
    std::mt19937 rng;

    // === Generación de estructura (islas y jerarquía) ===
    void generate_tree_structure(Island* parent, int max_depth);

    // === Layouts (asignan posiciones) ===
    void layout_parent_radial();
    void layout_center_radial();       // TODO: paso 4
    void layout_random_scatter();      // TODO: paso 5

    // === Conexiones (rellenan vecinas) ===
    void connect_tree_only();
    void add_shortcuts(float max_distance, float probability);   // TODO: paso 4
    void add_ring_connections(float probability);                 // TODO: paso 4
    void connect_proximity(float max_distance, float probability);// TODO: paso 5

public:
    World();
    ~World();
    
    void create_world(unsigned int seed, LayoutStyle layout = LayoutStyle::CENTER_RADIAL, ConnectionStyle connection = ConnectionStyle::RINGS);
    void travel_to(Island* target_island);

    Island* create_island(IslandType tipo, int w, int h, unsigned int semilla);
    const Designar::DynArray<Island*>& get_all_islands() const;
    Island* get_current_island() { return current; } 
    int get_current_id() const { return current ? current->id : -1; }
};