#include <SFML/Window/Keyboard.hpp>
#include <algorithm>

#include "../include/world.hpp"
#include "../include/utilyties.hpp"
#include "../include/player.hpp"  

int main() 
{
    const float TILE_SIZE = 50.0f; 
    bool is_transitioning = false;

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280, 720)), "Islas Flotantes");
    window.setFramerateLimit(60);
    
    // 1. Crear el mundo (genera las 3 islas)
    World world;
    world.create_test_world(12345); 

    // 2. Crear al jugador solo una vez
    Island& start_island = world.get_current_island();
    sf::Vector2f spawn = start_island.terrain.find_spawn_point(TILE_SIZE);
    player jugador(spawn.x, spawn.y);
    
    bool showInfo = true;
    bool running = true;
    
    sf::Font& font = load_font();
    sf::Text info = create_info_text(font, "=== ISLA ===");

    sf::View camera;
    camera.setSize(sf::Vector2f(1280, 720));
    camera.setCenter(jugador.get_position());
    float zoom = 1.0f;

    sf::Clock clock;
    int frameCount = 0;
    float fpsTimer = 0.0f;
    int fps = 0;
    
    // Puntero a la isla actual
    Island* current = &world.get_current_island();

    while (window.isOpen()) 
    {
        while (auto event = window.pollEvent()) 
        {
            if (event->is<sf::Event::Closed>()) window.close();
            
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseWheelScrolled>()) 
            {
                zoom -= mouseEvent->delta * 0.5f;
                zoom = std::max(0.5f, std::min(20.0f, zoom));
            }
            
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) 
            {
                if (keyEvent->code == sf::Keyboard::Key::H) showInfo = !showInfo;
                if (keyEvent->code == sf::Keyboard::Key::Escape) running = false;
            }
        }

        if (!running) break;

        // Calcular FPS
        frameCount++;
        float dt = clock.restart().asSeconds();
        fpsTimer += dt;
        if (fpsTimer >= 1.0f) 
        { 
            fps = frameCount; frameCount = 0; fpsTimer = 0.0f; 
        }

        // 3. Actualizar jugador
        current = &world.get_current_island();
        jugador.update(dt, current->terrain.get_height_map(), TILE_SIZE, current->terrain.get_props());
        
        // 4. Lógica de transición por PORTALES
        bool portal_triggered = false;
        for (const auto& portal : current->portals) 
        {
            // Calcular distancia entre el jugador y el centro del portal
            sf::Vector2f diff = jugador.get_position() - portal.position;
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            
            // Si el jugador toca el portal...
            if (dist < jugador.get_radius() + portal.radius) 
            {
                world.travel_to(portal.target_island_id);
                jugador.set_position(portal.spawn_position);
                current = &world.get_current_island(); // Actualizamos el puntero
                portal_triggered = true;
                break; // Salimos del bucle
            }
        }

        if (is_transitioning) 
        {
            camera.setCenter(jugador.get_position());
            is_transitioning = false;
        }

        camera.setCenter(jugador.get_position());
        camera.setSize(sf::Vector2f(1280.0f * zoom, 720.0f * zoom));
        
        window.clear(sf::Color(135, 206, 235));

        window.setView(camera);
        current->terrain.draw(window, TILE_SIZE); 
        current->terrain.draw_props(window, TILE_SIZE); 

        // DIBUJAR PORTALES
        for (const auto& portal : current->portals) 
        {
            sf::CircleShape portal_shape(portal.radius);
            portal_shape.setOrigin(sf::Vector2f(portal.radius, portal.radius));
            portal_shape.setPosition(portal.position);
            portal_shape.setFillColor(sf::Color(160, 32, 240, 150)); // Púrpura semitransparente
            portal_shape.setOutlineColor(sf::Color::White);
            portal_shape.setOutlineThickness(4.0f);
            window.draw(portal_shape);
        }

        jugador.draw(window); 

        window.setView(window.getDefaultView());

        if (showInfo) 
        {
            std::string infoText = 
                "=== ISLAS FLOTANTES ===\n"
                "WASD: Mover\n"
                "Ve a los bordes para viajar\n"
                "\n"
                "Isla Actual: " + std::to_string(world.get_current_id()) +
                "\nFPS: " + std::to_string(fps);
            
            info.setString(infoText);
            
            sf::RectangleShape background;
            background.setSize(sf::Vector2f(320, 200));
            background.setFillColor(sf::Color(0, 0, 0, 180));
            background.setPosition(sf::Vector2f(5, 5));
            window.draw(background);
            window.draw(info);
        }
        
        window.display();
    }
    return 0;
}