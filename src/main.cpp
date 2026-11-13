#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include "../include/world.hpp"
#include "../include/utilyties.hpp"
#include "../include/player.hpp"  

enum class GameState { PLAYING, TRANSITIONING };

int main() 
{
    const float TILE_SIZE = 50.0f;
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280, 720)), "Islas Flotantes");
    window.setFramerateLimit(60);
    
    World world;
    world.create_test_world(12345); 
    
    Island* current = world.get_current_island();
    sf::Vector2f spawn = current->terrain.find_spawn_point(TILE_SIZE);
    player jugador(spawn.x, spawn.y);
    
    GameState state = GameState::PLAYING;
    float transition_timer = 0.0f;
    Island* target_island = nullptr;
    sf::Vector2f target_spawn;
    
    bool running = true;
    sf::Font& font = load_font();
    sf::Text info = create_info_text(font, "=== ISLA ===");
    sf::RectangleShape fade_rect(sf::Vector2f(1280, 720));
    fade_rect.setFillColor(sf::Color(0, 0, 0, 0));

    sf::View camera;
    camera.setSize(sf::Vector2f(1280, 720));
    camera.setCenter(jugador.get_position());
    float zoom = 1.0f;

    sf::Clock clock;
    
    while (window.isOpen()) 
    {
        while (auto event = window.pollEvent()) 
        {
            if (event->is <sf::Event::Closed>()) window.close();
            
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseWheelScrolled>()) 
            {
                zoom -= mouseEvent->delta * 0.5f;
                zoom = std::max(0.5f, std::min(20.0f, zoom));
            }

            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) 
            {
                if (keyEvent->code == sf::Keyboard::Key::Escape) running = false;
            }
        }

        if (!running) break;
        float dt = clock.restart().asSeconds();

        if (state == GameState::PLAYING) 
        {
            jugador.update(dt, current->terrain.get_height_map(), TILE_SIZE, current->terrain.get_props());
            
            for (const auto& portal : current->portals) 
            {
                sf::Vector2f diff = jugador.get_position() - portal.position;
                float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                
                if (dist < jugador.get_radius() + portal.radius) 
                {
                    target_island = portal.target_island; 
                    target_spawn = portal.spawn_position;
                    state = GameState::TRANSITIONING;
                    transition_timer = 0.0f;
                    break;
                }
            }
        } 
        else if (state == GameState::TRANSITIONING) 
        {
            transition_timer += dt;
            
            if (transition_timer < 1.0f) 
            {
                fade_rect.setFillColor(sf::Color(0, 0, 0, static_cast<int>(255 * transition_timer)));
            }
            else if (transition_timer >= 1.0f && transition_timer < 2.0f) 
            {
                if (target_island != current) 
                {
                    fade_rect.setFillColor(sf::Color(0, 0, 0, 255));
                    world.travel_to(target_island);
                    current = target_island;
                    jugador.set_position(target_spawn);
                    camera.setCenter(jugador.get_position());
                }
            }
            else if (transition_timer >= 2.0f) 
            {
                float fade_progress = (transition_timer - 2.0f) / 1.0f;
                fade_rect.setFillColor(sf::Color(0, 0, 0, static_cast<int>(255 * (1.0f - fade_progress))));
                if (transition_timer >= 3.0f) 
                {
                    state = GameState::PLAYING;
                }
            }
        }

        camera.setCenter(jugador.get_position());
        camera.setSize(sf::Vector2f(1280.0f * zoom, 720.0f * zoom));
        
        window.clear(sf::Color(135, 206, 235));

        window.setView(camera);
        current->terrain.draw(window, TILE_SIZE); 
        current->terrain.draw_props(window, TILE_SIZE); 
        
        for (const auto& portal : current->portals) 
        {
            sf::CircleShape portal_shape(portal.radius);
            portal_shape.setOrigin(sf::Vector2f(portal.radius, portal.radius));
            portal_shape.setPosition(portal.position);
            portal_shape.setFillColor(sf::Color(160, 32, 240, 150));
            portal_shape.setOutlineColor(sf::Color::White);
            portal_shape.setOutlineThickness(4.0f);
            window.draw(portal_shape);
        }
        jugador.draw(window); 

        window.setView(window.getDefaultView());
        if (state == GameState::TRANSITIONING) 
        {
            window.draw(fade_rect);
        }

        if (state == GameState::PLAYING) 
        {
            std::string infoText = "Isla Actual: " + std::to_string(world.get_current_id());
            info.setString(infoText);
            window.draw(info);
        }
        
        window.display();
    }
    return 0;
}