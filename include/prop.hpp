#pragma once
#include <SFML/Graphics.hpp>

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
    float respawn_timer;
};

struct Prop 
{
    sf::Vector2f position;
    PropType type;
    float collision_radius;
    sf::Color color;

    // ✅ Estado dinámico
    PropState state = PropState::AVAILABLE;
    float respawn_timer = 0.0f;
};

// ✅ Configuración por tipo (la "tabla" que permite extensibilidad)
struct PropConfig 
{
    float respawn_time;       // 0.0 = no renovable
    int harvest_amount;       // Cuánto recurso da
    const char* resource;     // "madera", "piedra", "baya"
    float render_radius;
};

const PropConfig& get_prop_config(PropType type);