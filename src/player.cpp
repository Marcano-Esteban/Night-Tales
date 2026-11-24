#include "../include/player.hpp"
#include <cmath>
#include <algorithm>

player::player(float x, float y) : position(x, y), speed(200.0f), radius(10.0f), can_walk(false), inventory(6)
    
{
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f(radius, radius));
    shape.setFillColor(sf::Color(255, 200, 50));
    shape.setOutlineColor(sf::Color(150, 100, 0));
    shape.setOutlineThickness(2.0f);
}

bool player::is_valid_position(float x, float y, const height_map& map,float tile_size, const Designar::DynArray<Prop>& props)
{
    int tile_x = static_cast<int>(x / tile_size + map.width / 2.0f);
    int tile_y = static_cast<int>(y / tile_size + map.height / 2.0f);

    if (tile_x < 0 || tile_x >= map.width || tile_y < 0 || tile_y >= map.height) return false;
        
    if (map.at(tile_x, tile_y) <= 0.35f) return false;

    for (const auto& prop : props) 
    {
        if (prop.type == PropType::TREE && prop.state == PropState::AVAILABLE) 
        {
            float tree_center_y = prop.position.y - (tile_size * 0.35f);
            float dx = x - prop.position.x;
            float dy = y - tree_center_y;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist < radius + prop.collision_radius) return false;
        }
    }
    return true;
}

void player::update(float dt, float playtime, const height_map& map, float tile_size, Designar::DynArray<Prop>& props)      
{
    // ===== ESTADOS DE ESCALADA =====
    if (state == PlayerState::CLIMBING) 
    {
        climb_timer += dt;
        float progress = std::min(1.0f, climb_timer / CLIMB_UP_DURATION);
        int alpha = (int)(255 * (1.0f - progress));
        shape.setFillColor(sf::Color(255, 200, 50, alpha));

        if (climb_timer >= CLIMB_UP_DURATION) 
        {
            state = PlayerState::AT_TOP;
            climb_timer = 0.0f;
            shape.setFillColor(sf::Color(255, 200, 50, 0));
        }
        shape.setPosition(position);
        return;
    }

    if (state == PlayerState::AT_TOP) 
    {
        climb_timer += dt;
        if (climb_timer >= CLIMB_TOP_DURATION) 
        {
            if (climbing_target && climbing_target->state == PropState::AVAILABLE) // Recolectar
            {
                const auto& cfg = get_prop_config(climbing_target->type);
                ItemType item = prop_type_to_item(climbing_target->type);

                int added = inventory.add_item(item, cfg.harvest_amount);
                if (added > 0) 
                {
                    if (cfg.respawn_days > 0.0f) 
                    {
                        climbing_target->state = PropState::DEPLETED;
                        climbing_target->respawn_at_playtime = playtime + cfg.respawn_days * 600.0f; // 1 día = 600s   
                    } 
                    else 
                    {
                        climbing_target->state = PropState::GONE;
                    }
                }
            }
            state = PlayerState::DESCENDING;
            climb_timer = 0.0f;
        }
        shape.setPosition(position);
        return;
    }

    if (state == PlayerState::DESCENDING) 
    {
        climb_timer += dt;
        float progress = std::min(1.0f, climb_timer / CLIMB_DOWN_DURATION);
        int alpha = (int)(255 * progress);
        shape.setFillColor(sf::Color(255, 200, 50, alpha));

        if (climb_timer >= CLIMB_DOWN_DURATION) 
        {
            state = PlayerState::NORMAL;
            climbing_target = nullptr;
            shape.setFillColor(sf::Color(255, 200, 50, 255));
        }
        shape.setPosition(position);
        return;
    }

    // ===== ESTADO NORMAL =====
    stats.hunger = std::max(0.0f, stats.hunger - hunger_decay_per_second * dt); // Hambre decae con el tiempo
    if (stats.hunger <= 0.0f) 
    {
        stats.health = std::max(0.0f, stats.health - 1.0f * dt);
    }

    // Movimiento
    sf::Vector2f direction(0.0f, 0.0f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) direction.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) direction.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) direction.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) direction.x += 1.0f;

    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.0f) 
    { 
        direction.x /= len; 
        direction.y /= len; 
    }

    float nx = position.x + direction.x * speed * dt;
    if (is_valid_position(nx, position.y, map, tile_size, props)) position.x = nx;

    float ny = position.y + direction.y * speed * dt;
    if (is_valid_position(position.x, ny, map, tile_size, props)) position.y = ny;

    int tx = static_cast<int>(position.x / tile_size + map.width / 2.0f);
    int ty = static_cast<int>(position.y / tile_size + map.height / 2.0f);
    if (tx >= 0 && tx < map.width && ty >= 0 && ty < map.height)
    {
        can_walk = (map.at(tx, ty) > 0.35f);
    }
    else
    {
        can_walk = false;
    }

    shape.setPosition(position);
}

bool player::try_climb(Prop* tree) 
{
    if (state != PlayerState::NORMAL) return false;
    if (!tree || tree->type != PropType::TREE) return false;
    if (tree->state == PropState::GONE) return false;


    // Solo verificar si cave en el inventario si el árbol TIENE ramas
    if (tree->state == PropState::AVAILABLE) 
    {
        const auto& cfg = get_prop_config(tree->type);
        ItemType item = prop_type_to_item(tree->type);

        if (inventory.free_space_for(item, cfg.harvest_amount) < cfg.harvest_amount) return false;
    }
        
    climbing_target = tree;
    state = PlayerState::CLIMBING;
    climb_timer = 0.0f;
    return true;
}

void player::set_position(sf::Vector2f new_pos) 
{
    position = new_pos;
    shape.setPosition(position);
}

void player::draw(sf::RenderWindow& window) 
{
    window.draw(shape);
}