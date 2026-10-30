#include "../include/player.hpp"

player::player(float x, float y) : position(x, y), speed(200.0f), radius(10.0f), can_walk(false)  
{
    // Configurar el círculo
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f(radius, radius));  // Centro del círculo
    shape.setFillColor(sf::Color(255, 200, 50));    // Amarillo
    shape.setOutlineColor(sf::Color(150, 100, 0));  // Borde marrón
    shape.setOutlineThickness(2.0f);
}

// Comprueba si el jugador puede estar en (x, y)
bool player::is_valid_position(float x, float y, const height_map& map, float tile_size, const std::vector<Prop>& props)
{
    // 1. Verificar colisión con el agua/cielo
    int tile_x = static_cast<int>(x / tile_size + map.width / 2.0f);
    int tile_y = static_cast<int>(y / tile_size + map.height / 2.0f);
    
    if (tile_x < 0 || tile_x >= map.width || tile_y < 0 || tile_y >= map.height)
    {
        return false; // Fuera del mapa
    }
    
    float height = map.at(tile_x, tile_y);
    if (height <= 0.35f)
    {
        return false; // Es agua/cielo
    }

    // 2. Verificar colisión con árboles
    for (const auto& prop : props)
    {
        if (prop.type == PropType::TREE)
        {
            // Centro de colisión del árbol (un poco más arriba de la base)
            float tree_center_y = prop.position.y - (tile_size * 0.35f);
            
            float dx = x - prop.position.x;
            float dy = y - tree_center_y;
            float distance = std::sqrt(dx*dx + dy*dy);
            
            if (distance < radius + prop.collision_radius)
            {
                return false; // Choca con un árbol
            }
        }
    }

    return true; // La posición es válida
}

void player::update(float deltaTime, const height_map& map, float tile_size, const std::vector<Prop>& props)
{
    // 1. LEER INPUT (WASD)
    sf::Vector2f direction(0.0f, 0.0f);
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) direction.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) direction.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) direction.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) direction.x += 1.0f;
    
    // Normalizar el vector (para que la diagonal no sea más rápida)
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    // 2. Intentar mover en X
    float new_x = position.x + direction.x * speed * deltaTime;
    if (is_valid_position(new_x, position.y, map, tile_size, props))
    {
        position.x = new_x;
    }

    // 3. Intentar mover en Y
    float new_y = position.y + direction.y * speed * deltaTime;
    if (is_valid_position(position.x, new_y, map, tile_size, props))
    {
        position.y = new_y;
    }

    // 4. Actualizar el estado para la UI (can_walk)
    int tile_x = static_cast<int>(position.x / tile_size + map.width / 2.0f);
    int tile_y = static_cast<int>(position.y / tile_size + map.height / 2.0f);

    if (tile_x >= 0 && tile_x < map.width && tile_y >= 0 && tile_y < map.height)
    {
        can_walk = (map.at(tile_x, tile_y) > 0.35f);
    }
    else
    {
        can_walk = false;
    }

    // 5. ACTUALIZAR POSICIÓN VISUAL
    shape.setPosition(position);
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

sf::Vector2f player::get_position() const 
{ 
    return position; 
}

float player::get_radius() const 
{ 
    return radius; 
}

bool player::can_walk_here() const 
{ 
    return can_walk; 
}