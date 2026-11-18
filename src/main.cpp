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
    world.create_world(56789); 
    
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

    // === Variables para el Minimapa ===
    sf::View minimap_view;
    minimap_view.setViewport(sf::FloatRect(sf::Vector2f(0.8f, 0.0f), sf::Vector2f(0.2f, 0.2f)));; // Arriba a la derecha
    minimap_view.setSize(sf::Vector2f(2000.0f, 2000.0f)); // Tamaño del mundo visible
    minimap_view.setCenter(sf::Vector2f(0, 0)); // Centrado en la isla central
    
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
            jugador.update(dt, current->terrain.get_height_map(), TILE_SIZE, current->props);
            
            for (const auto& portal : current->vecinas) 
            {
                sf::Vector2f diff = jugador.get_position() - portal.position;
                float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                
                if (dist < jugador.get_radius() + portal.radius) 
                {
                    target_island = portal.destino; 
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

        // Actualizar timers de la isla actual
        current->update_props(dt);
        // Dibujar
        current->terrain.draw(window, TILE_SIZE);
        current->draw_props(window, TILE_SIZE);   // ← Cambio

        for (const auto& portal : current->vecinas) 
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

        // ============================================
        // DIBUJAR MINI-MAPA
        // ============================================
        window.setView(window.getDefaultView()); // Volver a la UI
        window.setView(minimap_view);            // Usar la vista del minimapa

        // 1. Dibujar las conexiones (líneas)
        for (auto* isla : world.get_all_islands()) // Necesitaremos hacer 'all_islands' público o un getter
        {
            for (const auto& conexion : isla->vecinas)
            {
                sf::Vertex linea[] = {
                    sf::Vertex{isla->posicion, sf::Color(200, 200, 200, 100)},
                    sf::Vertex{conexion.destino->posicion, sf::Color(200, 200, 200, 100)}
                };
                window.draw(linea, 2, sf::PrimitiveType::Lines);
            }
        }

        // 2. Dibujar las islas (círculos)
        for (auto* isla : world.get_all_islands())
        {
            sf::CircleShape punto(isla->radio * 0.5f); // Hacerlos más pequeños para el mapa
            punto.setOrigin(sf::Vector2f(isla->radio * 0.5f, isla->radio * 0.5f));
            punto.setPosition(isla->posicion);

            if (isla == current) 
            {
                punto.setFillColor(sf::Color::Yellow); // Isla actual
            } 
            else if (isla->explorada) 
            {
                punto.setFillColor(sf::Color::White);  // Ya la visitamos
            } 
            else 
            {
                punto.setFillColor(sf::Color(100, 100, 100)); // No visitada
            }
            
            window.draw(punto);
        }

        // 3. Dibujar un borde al minimapa
        sf::RectangleShape borde(sf::Vector2f(1280, 720)); // Tamaño de la ventana
        borde.setFillColor(sf::Color::Transparent);
        borde.setOutlineColor(sf::Color::White);
        borde.setOutlineThickness(2.0f);
        
        window.draw(borde);
        
        // Volver a la vista de la UI para el resto de cosas
        window.setView(window.getDefaultView());
        
        window.display();
    }
    return 0;
}