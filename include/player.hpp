#pragma once

#include <SFML/Graphics.hpp>
#include "height_map.hpp"
#include "../include/terrain_generator.hpp" 

class player 
{
private:
    sf::Vector2f position;      // Posición en el mundo
    sf::CircleShape shape;      // Forma visual (círculo)
    float speed;                // Velocidad (píxeles por segundo)
    float radius;               // Radio del círculo
    bool can_walk;              // Para saber cuándo está sobre agua/cielo
    bool is_valid_position(float x, float y, const height_map& map, float tile_size, const std::vector<Prop>& props);
    
public:
    player(float x, float y);
    
    // Actualizar posición según input y colisiones
    void update(float deltaTime, const height_map& map, float tile_size, const std::vector<Prop>& props);
    void set_position(sf::Vector2f new_pos);
    // Dibujar en la ventana
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f get_position() const;
    float get_radius() const;
    bool can_walk_here() const;
};