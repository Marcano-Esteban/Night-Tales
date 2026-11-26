#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <cctype>
#include "../include/game.hpp"
#include "../include/save_manager.hpp"
#include "../include/utilyties.hpp"
#include "../include/character.hpp"

std::string layout_to_string(LayoutStyle l) 
{
    switch (l) 
    {
        case LayoutStyle::PARENT_RADIAL:  return "Alrededor del padre";
        case LayoutStyle::CENTER_RADIAL:  return "Anillos desde el centro";
        case LayoutStyle::RANDOM_SCATTER: return "Aleatorio (archipielago)";
    }
    return "?";
}

std::string connection_to_string(ConnectionStyle c) 
{
    switch (c) 
    {
        case ConnectionStyle::TREE_ONLY:      return "Solo arbol";
        case ConnectionStyle::TREE_SHORTCUTS: return "Arbol + atajos";
        case ConnectionStyle::RINGS:          return "Anillos + puentes";
        case ConnectionStyle::PROXIMITY:      return "Proximidad (BFS)";
    }
    return "?";
}

enum class AppState 
{
    MAIN_MENU,
    NEW_GAME_MENU,
    CHARACTER_SELECT,
    LOAD_GAME_MENU,
    PLAYING,
    PAUSED
};

int main() 
{
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280, 720)), "Night Tales");
    window.setFramerateLimit(60);
    
    sf::Font& font = load_font();
    Game game;
    game.init(font);
    
    SaveManager save_mgr("saves/");
    AppState state = AppState::MAIN_MENU;
    
    // Estado de los menús
    int char_select_index = 0;  // 0=Manga, 1=Kaiser
    int selected_option = 0;
    int load_selected = 0;
    std::string world_name_input;
    std::string seed_input;
    std::string error_message;
    bool typing_name = true;
    
    // Save actual
    std::string current_save_filename;
    
    std::vector<SaveSlotInfo> load_saves_cache;
    sf::Clock clock;

    LayoutStyle selected_layout = LayoutStyle::PARENT_RADIAL;
    ConnectionStyle selected_connection = ConnectionStyle::TREE_SHORTCUTS;

    while (window.isOpen()) 
    {
        float dt = clock.restart().asSeconds();
        
        while (auto event = window.pollEvent()) 
        {
            if (event->is<sf::Event::Closed>()) window.close();
            
            if (state == AppState::MAIN_MENU) 
            {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) 
                {
                    if (key->code == sf::Keyboard::Key::Up && selected_option > 0) selected_option--;
                    if (key->code == sf::Keyboard::Key::Down && selected_option < 3) selected_option++;
                    if (key->code == sf::Keyboard::Key::Enter) 
                    {
                        if (selected_option == 0) 
                        {
                            state = AppState::NEW_GAME_MENU;
                            world_name_input = "";
                            seed_input = "";
                            error_message = "";
                            typing_name = true;
                        } 
                        else if (selected_option == 1) 
                        {
                            load_saves_cache = save_mgr.list_saves();
                            load_selected = 0;
                            state = AppState::LOAD_GAME_MENU;
                        } 
                        else if (selected_option == 3) 
                        {
                            window.close();
                        }
                    }
                }
            }
            else if (state == AppState::NEW_GAME_MENU) 
            {
                if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) 
                {
                    if (textEvent->unicode == 8) 
                    {
                        if (typing_name && !world_name_input.empty()) world_name_input.pop_back();
                        else if (!typing_name && !seed_input.empty()) seed_input.pop_back();
                        error_message = "";
                    } 
                    else if (textEvent->unicode == 13) 
                    {
                        if (typing_name) 
                        {
                            if (world_name_input.empty()) 
                            {
                                error_message = "El nombre no puede estar vacio";
                            } 
                            else if (save_mgr.world_name_exists(world_name_input)) 
                            {
                                error_message = "Ya existe un mundo con ese nombre";
                            } 
                            else 
                            {
                                typing_name = false;
                                error_message = "";
                            }
                        } 
                        else 
                        {
                            current_save_filename = SaveManager::sanitize_name(world_name_input); // Sanitizar nombre y guardar

                            char_select_index = 0;
                            state = AppState::CHARACTER_SELECT;
                        }
                    } 
                    else if (textEvent->unicode >= 32 && textEvent->unicode < 128) 
                    {
                        if (typing_name) world_name_input += (char)textEvent->unicode;
                        else if (isdigit(textEvent->unicode)) seed_input += (char)textEvent->unicode;
                        error_message = "";
                    }
                }
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) 
                {
                    if (key->code == sf::Keyboard::Key::Escape) 
                    {
                        state = AppState::MAIN_MENU;
                    }
                    // ✅ Ciclar layouts (3 opciones)
                    if (key->code == sf::Keyboard::Key::L) 
                    {
                        int l = (int)selected_layout;
                        l = (l + 1) % 3;
                        selected_layout = (LayoutStyle)l;
                    }
                    // ✅ Ciclar conexiones (4 opciones)
                    if (key->code == sf::Keyboard::Key::C) 
                    {
                        int c = (int)selected_connection;
                        c = (c + 1) % 4;
                        selected_connection = (ConnectionStyle)c;
                    }
                }
            }
            else if (state == AppState::LOAD_GAME_MENU) 
            {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) 
                {
                    if (key->code == sf::Keyboard::Key::Escape) 
                    {
                        state = AppState::MAIN_MENU;
                    }
                    if (key->code == sf::Keyboard::Key::Up && load_selected > 0) 
                    {
                        load_selected--;
                    }
                    if (key->code == sf::Keyboard::Key::Down && load_selected < (int)load_saves_cache.size() - 1) 
                    {
                        load_selected++;
                    }
                    if (key->code == sf::Keyboard::Key::Delete && !load_saves_cache.empty()) 
                    {
                        std::string to_delete = load_saves_cache[load_selected].filename;
                        save_mgr.delete_save(to_delete);
                        load_saves_cache = save_mgr.list_saves();
                        if (load_selected >= (int)load_saves_cache.size() && load_selected > 0) 
                        {
                            load_selected--;
                        }
                    }
                    if (key->code == sf::Keyboard::Key::Enter && !load_saves_cache.empty()) 
                    {
                        current_save_filename = load_saves_cache[load_selected].filename;
                        GameSaveData data;
                        if (save_mgr.load_game(current_save_filename, data)) 
                        {
                            game.load_from_save(data);
                            state = AppState::PLAYING;
                        }
                    }
                }
            }
            else if (state == AppState::PLAYING) 
            {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) 
                {
                    if (key->code == sf::Keyboard::Key::Escape) state = AppState::PAUSED;
                    if (key->code == sf::Keyboard::Key::M) game.toggle_big_map();
                    if (key->code == sf::Keyboard::Key::E) game.handle_interact();
                    if (key->code == sf::Keyboard::Key::F) game.handle_eat();   
                }
            }
            else if (state == AppState::PAUSED) 
            {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) 
                {
                    if (key->code == sf::Keyboard::Key::Up && selected_option > 0) selected_option--;
                    if (key->code == sf::Keyboard::Key::Down && selected_option < 3) selected_option++;
                    if (key->code == sf::Keyboard::Key::Escape) state = AppState::PLAYING;
                    if (key->code == sf::Keyboard::Key::Enter) 
                    {
                        if (selected_option == 0) 
                        {
                            state = AppState::PLAYING;
                        } 
                        else if (selected_option == 1) 
                        {
                            save_mgr.save_game(current_save_filename, game.get_save_data());
                        } 
                        else if (selected_option == 2) 
                        {
                            save_mgr.save_game(current_save_filename, game.get_save_data());
                            state = AppState::MAIN_MENU;
                        } 
                        else if (selected_option == 3) 
                        {
                            state = AppState::MAIN_MENU;
                        }
                    }
                }
            }
            else if (state == AppState::CHARACTER_SELECT) 
            {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) 
                {
                    if (key->code == sf::Keyboard::Key::Left) char_select_index = 0;
                    if (key->code == sf::Keyboard::Key::Right) char_select_index = 1;
                    if (key->code == sf::Keyboard::Key::Escape) state = AppState::NEW_GAME_MENU;
                    if (key->code == sf::Keyboard::Key::Enter) 
                    {
                        CharacterType chosen = (CharacterType)char_select_index;
                        unsigned int final_seed = seed_input.empty() ? (unsigned int)std::time(nullptr) : std::stoul(seed_input);
                       
                        game.start_new_game(world_name_input, final_seed, 1,
                                selected_layout, selected_connection, chosen);

                        save_mgr.save_game(current_save_filename, game.get_save_data());
                        state = AppState::PLAYING;
                    }
                }
            }
        }
        
        // Actualizar
        if (state == AppState::PLAYING) game.update(dt);
        
        // Dibujar
        window.clear(sf::Color(30, 30, 40));
        
        if (state == AppState::PLAYING) 
        {
            game.draw(window);
            game.draw_hud(window);
        } 
        else if (state == AppState::PAUSED) 
        {
            game.draw(window);
            window.setView(window.getDefaultView());
            sf::RectangleShape overlay(sf::Vector2f(1280, 720));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
            
            sf::Text pause_text(font, "", 24);
            pause_text.setPosition(sf::Vector2f(100, 100));
            std::string t = "=== PAUSA ===\n\n";
            t += (selected_option == 0 ? "> " : "  ") + std::string("Continuar\n");
            t += (selected_option == 1 ? "> " : "  ") + std::string("Guardar\n");
            t += (selected_option == 2 ? "> " : "  ") + std::string("Guardar y Salir\n");
            t += (selected_option == 3 ? "> " : "  ") + std::string("Salir sin Guardar\n");
            pause_text.setString(t);
            window.draw(pause_text);
        } 
        else 
        {
            sf::Text menu_text(font, "", 24);
            menu_text.setPosition(sf::Vector2f(100, 100));
            
            if (state == AppState::MAIN_MENU) 
            {
                std::string m = "=== NIGHT TALES ===\n\n";
                m += (selected_option == 0 ? "> " : "  ") + std::string("Nueva Partida\n");
                m += (selected_option == 1 ? "> " : "  ") + std::string("Cargar Partida\n");
                m += (selected_option == 2 ? "> " : "  ") + std::string("Opciones\n");
                m += (selected_option == 3 ? "> " : "  ") + std::string("Salir\n");
                menu_text.setString(m);
            } 
            else if (state == AppState::NEW_GAME_MENU) 
            {
                std::string m = "=== NUEVA PARTIDA ===\n\n";
                m += "Nombre: " + world_name_input + (typing_name ? "_" : "") + "\n\n";
                m += "Semilla: " + seed_input + (!typing_name ? "_" : "") + "\n\n";
                m += "Layout [L]: " + layout_to_string(selected_layout) + "\n";
                m += "Conexion [C]: " + connection_to_string(selected_connection) + "\n\n";
                if (!error_message.empty()) m += "⚠ " + error_message + "\n\n";
                m += "ENTER para continuar, ESC para volver";
                menu_text.setString(m);
                window.draw(menu_text);
            } 
            else if (state == AppState::LOAD_GAME_MENU) 
            {
                std::string m = "=== CARGAR PARTIDA ===\n\n";
                if (load_saves_cache.empty()) 
                {
                    m += "No hay partidas guardadas\n";
                } 
                else 
                {
                    for (int i = 0; i < (int)load_saves_cache.size(); ++i) 
                    {
                        const auto& s = load_saves_cache[i];
                        m += (i == load_selected ? "> " : "  ") + s.world_name;
                        m += "  [Seed: " + std::to_string(s.seed) + "]\n";
                    }
                }
                m += "\n↑↓ navegar, ENTER cargar, DEL borrar, ESC volver";
                menu_text.setString(m);
            }
            else if (state == AppState::CHARACTER_SELECT) 
            {
                sf::Text title(font, "=== ELIGE TU PERSONAJE ===", 28);
                title.setPosition(sf::Vector2f(400, 50));
                window.draw(title);
    
                // Dos cartas
                for (int i = 0; i < 2; ++i) 
                {
                    CharacterType type = (CharacterType)i;
                    const auto& def = get_character_def(type);
        
                    float x = 200.0f + i * 500.0f;
                    float y = 150.0f;
        
                    // Fondo de la carta
                    sf::RectangleShape card(sf::Vector2f(400, 450));
                    card.setPosition(sf::Vector2f(x, y));
                    card.setFillColor(i == char_select_index ? sf::Color(50, 50, 80) : sf::Color(30, 30, 40));
                    card.setOutlineColor(i == char_select_index ? sf::Color::Yellow : sf::Color(100, 100, 100));
                    card.setOutlineThickness(4.0f);
                    window.draw(card);
        
                    // Nombre
                    sf::Text name(font, def.name, 32);
                    name.setFillColor(def.color);
                    name.setPosition(sf::Vector2f(x + 20, y + 20));
                    window.draw(name);
        
                    // Descripción
                    sf::Text desc(font, def.description, 14);
                    desc.setFillColor(sf::Color(220, 220, 220));
                    desc.setPosition(sf::Vector2f(x + 20, y + 70));
                    window.draw(desc);
        
                    // Stats
                    std::string stats_str = 
                    "Vida: " + std::to_string((int)def.max_health) + " HP\n\n" +
                    "Cordura: " + std::to_string((int)def.max_sanity) + "\n\n" +
                    "Velocidad: x" + std::to_string(def.speed_mult).substr(0, 4) + "\n\n" +
                    "Escalada: " + (def.climb_speed_mult >= 2.0f ? "Rapida (x2)" : "Normal") + "\n\n" +
                    "Recoleccion: +" + std::to_string(def.harvest_bonus) + "\n\n" +
                    "Inventario: " + std::to_string(def.inventory_slots) + " slots\n\n" +
                    "Habilidad: " + std::string(get_ability_description(def.ability));
        
                    sf::Text stats_text(font, stats_str, 16);
                    stats_text.setFillColor(sf::Color::White);
                    stats_text.setPosition(sf::Vector2f(x + 20, y + 150));
                    window.draw(stats_text);
                }
    
                sf::Text hint(font, "← → para elegir, ENTER para confirmar, ESC para volver", 16);
                hint.setPosition(sf::Vector2f(400, 650));
                window.draw(hint);
            }
            window.draw(menu_text);
        }
        
        window.display();
    }
    return 0;
}