#pragma once
#include <SFML/Graphics.hpp>
#include "height_map.hpp"
#include "terrain_generator.hpp"
#include "inventory.hpp"

enum class PlayerState 
{
    NORMAL,
    CLIMBING,     // Subiendo al árbol
    AT_TOP,       // Arriba (recolectando)
    DESCENDING,   // Bajando
};

struct PlayerStats 
{
    float health = 100.0f;
    float hunger = 100.0f;
    float sanity = 100.0f;
    float max_health = 100.0f;
    float max_hunger = 100.0f;
    float max_sanity = 100.0f;
};

class player 
{
private:
    sf::Vector2f position;
    sf::CircleShape shape;
    float speed;
    float radius;
    bool can_walk;

    PlayerStats stats;
    float hunger_decay_per_second = 0.3f;

    Inventory inventory;
    PlayerState state = PlayerState::NORMAL;

    // Escalada
    float climb_timer = 0.0f;
    Prop* climbing_target = nullptr;
    static constexpr float CLIMB_UP_DURATION = 1.5f;
    static constexpr float CLIMB_TOP_DURATION = 0.8f;
    static constexpr float CLIMB_DOWN_DURATION = 1.5f;

    bool is_valid_position(float x, float y, const height_map& map, float tile_size, const Designar::DynArray<Prop>& props);

                           
public:
    player(float x, float y);

    // ✅ Nueva firma: recibe props no-const y playtime
    void update(float dt, float playtime, const height_map& map, float tile_size,Designar::DynArray<Prop>& props);
     
    void set_position(sf::Vector2f new_pos);
    void draw(sf::RenderWindow& window);

    // Getters
    sf::Vector2f get_position() const { return position; }
    float get_radius() const { return radius; }
    bool can_walk_here() const { return can_walk; }
    const PlayerStats& get_stats() const { return stats; }
    Inventory& get_inventory() { return inventory; }
    const Inventory& get_inventory() const { return inventory; }
    bool is_busy() const { return state != PlayerState::NORMAL; }

    // Acciones
    bool try_climb(Prop* tree);
    void damage(float amount) { stats.health = std::max(0.0f, stats.health - amount); }
    void reduce_sanity(float amount) { stats.sanity = std::max(0.0f, stats.sanity - amount); }
};