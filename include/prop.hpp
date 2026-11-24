#pragma once
#include <SFML/Graphics.hpp>
#include "item.hpp"

enum class PropType 
{
    TREE,           // Madera, renovable
    FLOWER,         // Decorativo, no recolectable
    STONE,          // Piedra, no renovable
    BERRY_BUSH,     // Bayas, renovable
    // ← AÑADIR MÁS AQUÍ
};

enum class PropState 
{
    AVAILABLE,      // Disponible para recolectar
    DEPLETED,       // Recolectado, esperando respawn
    GONE            // Recolectado permanentemente (no renovable)
};

struct PropDelta 
{
    int prop_index;         // Índice en el arreglo original
    PropState state;
    float respawn_at_playtime;
};

struct Prop 
{
    sf::Vector2f position;
    PropType type;
    float collision_radius;
    sf::Color color;

    PropState state = PropState::AVAILABLE;
    float respawn_at_playtime = 0.0f; 
};

// Configuración por tipo (la "tabla" que permite extensibilidad)
struct PropConfig 
{
    float respawn_days;       // 0.0 = no renovable
    int harvest_amount;       // Cuánto recurso da
    const char* resource;     // "madera", "piedra", "baya"
    float render_radius;
};

const PropConfig& get_prop_config(PropType type);
ItemType prop_type_to_item(PropType type);  