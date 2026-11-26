#include "../include/game.hpp"
#include "../include/utilyties.hpp"
#include <cmath>
#include <ctime>
#include <iostream>

Game::Game() 
{
    fade_rect.setSize(sf::Vector2f(1280, 720));
    fade_rect.setFillColor(sf::Color(0, 0, 0, 0));
}

void Game::init(sf::Font& f) 
{
    font = &f;
    info_text = create_info_text(f, "");
    
    minimap_view.setViewport(sf::FloatRect(sf::Vector2f(0.75f, 0.0f), sf::Vector2f(0.25f, 0.25f)));
    minimap_view.setSize(sf::Vector2f(3000.0f, 3000.0f));
    minimap_view.setCenter(sf::Vector2f(0, 0));
}

void Game::start_new_game(const std::string& world_name, unsigned int seed, int player_count,
                        LayoutStyle layout, ConnectionStyle connection,CharacterType character)
{
    current_world_name = world_name;
    current_seed = seed;
    current_layout = layout;
    current_connection = connection;
    current_character = character;
    
    world.create_world(seed, layout, connection);
    current_island = world.get_current_island();
    current_island->explorada = true;
    
    players.clear();
    sf::Vector2f spawn = current_island->terrain.find_spawn_point(50.0f);
    for (int i = 0; i < player_count; ++i) 
    {
        player p(spawn.x, spawn.y, character);  
        players.append(p);
    }
    
    camera.setSize(sf::Vector2f(1280, 720));
    camera.setCenter(players[0].get_position());
    time_of_day = 0.25f;
    playtime = 0.0f;
    is_transitioning = false;
    show_big_map = false;
}

void Game::load_from_save(const GameSaveData& data) 
{
    current_world_name = data.world_name;
    current_seed = data.seed;
    current_layout = (LayoutStyle)data.layout_style;
    current_connection = (ConnectionStyle)data.connection_style;
    current_character = (CharacterType)data.character_type;
    
    // Reconstruir con los mismos estilos
    world.create_world(data.seed, current_layout, current_connection);
    
    // Aplicar deltas a las islas
    for (const auto& isla_save : data.islands) 
    {
        for (auto* isla : world.get_all_islands()) 
        {
            if (isla->id == isla_save.id) 
            {
                isla->explorada = isla_save.explorada;
                isla->prop_deltas = isla_save.deltas;
                break;
            }
        }
    }
    
    // Viajar a la isla guardada (NO quedarse en la central)
    Island* target = nullptr;
    for (auto* isla : world.get_all_islands()) 
    {
        if (isla->id == data.current_island_id) 
        {
            target = isla;
            break;
        }
    }
    if (!target) target = world.get_current_island(); // Fallback
    
    world.travel_to(target);
    current_island = target;
    current_island->explorada = true;
    
    players.clear();
    player p(data.player_position.x, data.player_position.y, current_character);
    players.append(p);
    
    camera.setSize(sf::Vector2f(1280, 720));
    camera.setCenter(p.get_position());
    time_of_day = data.time_of_day;
    playtime = data.playtime_seconds;
    is_transitioning = false;
    show_big_map = false;
}

GameSaveData Game::get_save_data() const 
{
    GameSaveData data;
    data.world_name = current_world_name;
    data.seed = current_seed;
    data.current_island_id = world.get_current_id();
    data.time_of_day = time_of_day;
    data.playtime_seconds = playtime;
    data.last_played = std::time(nullptr);
    data.player_position = players[0].get_position();
    data.character_type = (int)current_character;
    
    for (auto* isla : world.get_all_islands()) 
    {
        IslandSaveData isl;
        isl.id = isla->id;
        isl.explorada = isla->explorada;
        // Si la isla está cargada, extraer deltas de props actuales
        if (isla->is_loaded) 
        {
            for (int i = 0; i < isla->props.size(); ++i) 
            {
                if (isla->props[i].state != PropState::AVAILABLE) 
                {
                    PropDelta d;
                    d.prop_index = i;
                    d.state = isla->props[i].state;
                    d.respawn_at_playtime = isla->props[i].respawn_at_playtime;
                    isl.deltas.append(d);
                }
            }
        } 
        else 
        {
            isl.deltas = isla->prop_deltas;
        }
        data.islands.append(isl);
    }
    return data;
}

sf::Vector2f Game::get_player_position() const 
{
    return players[0].get_position();
}

void Game::update(float dt) 
{
    if (is_transitioning) 
    {
        transition_timer += dt;
        if (transition_timer < 1.0f) 
        {
            fade_rect.setFillColor(sf::Color(0, 0, 0, (std::uint8_t)(255 * transition_timer)));
        } 
        else if (transition_timer < 2.0f) 
        {
            if (target_island != current_island) 
            {
                fade_rect.setFillColor(sf::Color(0, 0, 0, 255));
                world.travel_to(target_island);
                current_island = target_island;
                players[0].set_position(target_spawn);
                camera.setCenter(players[0].get_position());
            }
        } 
        else if (transition_timer < 3.0f) 
        {
            float fp = (transition_timer - 2.0f) / 1.0f;
            fade_rect.setFillColor(sf::Color(0, 0, 0, (std::uint8_t)(255 * (1.0f - fp))));
        } 
        else 
        {
            is_transitioning = false;
        }
        return;
    }
    
    playtime += dt;
    time_of_day += dt / day_duration_seconds;
    if (time_of_day > 1.0f) time_of_day -= 1.0f;
    
    current_island->update_props(playtime, day_duration_seconds);
    
    for (auto& p : players) 
    {
        p.update(dt, playtime, current_island->terrain.get_height_map(), 50.0f, current_island->props);
    }
    
    
    camera.setCenter(players[0].get_position());
    camera.setSize(sf::Vector2f(1280.0f * zoom, 720.0f * zoom));
}

void Game::draw(sf::RenderWindow& window) // ===============================================
{
    window.clear(sf::Color(135, 206, 235));
    
    window.setView(camera);
    current_island->terrain.draw(window, 50.0f);
    current_island->draw_props(window, 50.0f);
    
    for (const auto& conexion : current_island->vecinas) 
    {
        sf::CircleShape portal(conexion.radius);
        portal.setOrigin(sf::Vector2f(conexion.radius, conexion.radius));
        portal.setPosition(conexion.position);
        portal.setFillColor(sf::Color(160, 32, 240, 150));
        portal.setOutlineColor(sf::Color::White);
        portal.setOutlineThickness(4.0f);
        window.draw(portal);
    }
    
    for (auto& p : players) p.draw(window);

    draw_day_night_overlay(window);
    
    window.setView(window.getDefaultView());
    
    if (show_big_map) 
    {
        draw_map(window, true);
    } 
    else 
    {
        draw_map(window, false);
    }
    
    if (is_transitioning) window.draw(fade_rect);
}

void Game::draw_map(sf::RenderWindow& window, bool fullscreen) // =============================
{
    // Guardar la vista anterior y usar una apropiada
    sf::View map_view;
    if (fullscreen) 
    {
        map_view.setViewport(sf::FloatRect(sf::Vector2f(0.1f, 0.1f), sf::Vector2f(0.8f, 0.8f)));
    } 
    else 
    {
        map_view.setViewport(sf::FloatRect(sf::Vector2f(0.75f, 0.0f), sf::Vector2f(0.25f, 0.25f)));
    }
    map_view.setSize(sf::Vector2f(3000.0f, 3000.0f));
    map_view.setCenter(sf::Vector2f(0, 0));
    window.setView(map_view);
    
    // Fondo
    sf::RectangleShape bg(sf::Vector2f(2000, 2000));
    bg.setPosition(sf::Vector2f(-1000, -1000));
    bg.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(bg);
    
    // Conexiones
    for (auto* isla : world.get_all_islands()) 
    {
        for (const auto& conexion : isla->vecinas) 
        {
            sf::Vertex linea[] = {
                sf::Vertex{isla->posicion, sf::Color(200, 200, 200, 150)},
                sf::Vertex{conexion.destino->posicion, sf::Color(200, 200, 200, 150)}
            };
            window.draw(linea, 2, sf::PrimitiveType::Lines);
        }
    }
    
    // Islas
    for (auto* isla : world.get_all_islands()) 
    {
        sf::CircleShape punto(isla->radio * 0.5f);
        punto.setOrigin(sf::Vector2f(isla->radio * 0.5f, isla->radio * 0.5f));
        punto.setPosition(isla->posicion);
        if (isla == current_island) punto.setFillColor(sf::Color::Yellow);
        else if (isla->explorada) punto.setFillColor(sf::Color::White);
        else punto.setFillColor(sf::Color(100, 100, 100));
        window.draw(punto);
    }
    
    window.setView(window.getDefaultView());
}

void Game::draw_day_night_overlay(sf::RenderWindow& window) //==================================
{
    bool has_night_vision = (current_character == CharacterType::MANGA);
    window.setView(camera); // Usar la misma vista que el mundo
    
    sf::Color overlay_color = sf::Color::Transparent;
    float alpha = 0.0f;
    
    // time_of_day: 0.0 = amanecer, 0.25 = mediodía, 0.5 = atardecer, 0.75 = medianoche
    if (time_of_day >= 0.0f && time_of_day < 0.2f) 
    {
        // Amanecer: naranja → transparente
        float t = time_of_day / 0.2f;
        overlay_color = sf::Color(255, 140, 0);
        alpha = 120.0f * (1.0f - t);
    } 
    else if (time_of_day >= 0.2f && time_of_day < 0.4f) 
    {
        // Día: transparente
        alpha = 0.0f;
    } 
    else if (time_of_day >= 0.4f && time_of_day < 0.6f) 
    {
        // Atardecer: transparente → naranja
        float t = (time_of_day - 0.4f) / 0.2f;
        overlay_color = sf::Color(255, 100, 0);
        alpha = 120.0f * t;
    } 
    else if (time_of_day >= 0.6f && time_of_day < 0.7f) 
    {
        // Noche temprana: naranja → azul oscuro
        float t = (time_of_day - 0.6f) / 0.1f;
        overlay_color = sf::Color(
            255 * (1.0f - t),
            100 * (1.0f - t),
            0 + (30 * t)
        );
        alpha = 120.0f + 60.0f * t; // De 120 a 180
    } 
    else if (time_of_day >= 0.7f && time_of_day < 0.9f) 
    {
        // Noche profunda: azul oscuro
        overlay_color = sf::Color(0, 0, 30);
        alpha = 180.0f;
    } 
    else 
    {
        // Amanecer temprano: azul → naranja
        float t = (time_of_day - 0.9f) / 0.1f;
        overlay_color = sf::Color(
            0 + (255 * t),
            0 + (140 * t),
            30 + (0 * t)
        );
        alpha = 180.0f * (1.0f - t);
    }
    
    if (has_night_vision && time_of_day >= 0.6f) 
    {
        alpha *= 0.25f;  // Reducir al 25%
    }
    overlay_color.a = static_cast<std::uint8_t>(alpha);
    
    // Rectángulo que cubre toda la pantalla (en coordenadas de mundo)
    sf::RectangleShape overlay;
    sf::Vector2f view_size = camera.getSize();
    sf::Vector2f view_center = camera.getCenter();
    overlay.setSize(view_size);
    overlay.setPosition(sf::Vector2f(
        view_center.x - view_size.x / 2.0f,
        view_center.y - view_size.y / 2.0f
    ));
    overlay.setFillColor(overlay_color);
    window.draw(overlay);
}

Prop* Game::find_interactable_prop() 
{
    if (players[0].is_busy()) return nullptr;
    Prop* best = nullptr;
    float min_dist = 1e9f;
    for (auto& prop : current_island->props) 
    {
        if (prop.state == PropState::GONE) continue;
        if (prop_type_to_item(prop.type) == ItemType::NONE) continue;

        // Los 3 tipos de árbol son interactuables en cualquier estado
        bool is_tree = (prop.type == PropType::TREE || prop.type == PropType::MANGO_TREE || prop.type == PropType::LIANA_TREE);           
        if (!is_tree && prop.state != PropState::AVAILABLE) continue;

        float dx = players[0].get_position().x - prop.position.x;
        float dy = players[0].get_position().y - prop.position.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist < players[0].get_radius() + prop.collision_radius && dist < min_dist) 
        {
            best = &prop;
            min_dist = dist;
        }
    }
    return best;
}

bool Game::find_interactable_portal(Island** out_target, sf::Vector2f* out_spawn) 
{
    if (players[0].is_busy()) return false;
    for (const auto& conexion : current_island->vecinas) 
    {
        float dx = players[0].get_position().x - conexion.position.x;
        float dy = players[0].get_position().y - conexion.position.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist < players[0].get_radius() + conexion.radius) 
        {
            *out_target = conexion.destino;
            *out_spawn = conexion.spawn_position;
            return true;
        }
    }
    return false;
}

void Game::handle_interact() 
{
    if (players[0].is_busy() || is_transitioning) return;

    // 1. Prioridad: props
    Prop* target_prop = find_interactable_prop();
    if (target_prop) 
    {
        // Los 3 tipos de árbol se escalan
        bool is_tree = (target_prop->type == PropType::TREE || target_prop->type == PropType::MANGO_TREE ||  target_prop->type == PropType::LIANA_TREE);
                        
                       
                    
        if (is_tree) 
        {
            players[0].try_climb(target_prop);
        } 
        else if (target_prop->type == PropType::STONE || target_prop->type == PropType::LARGE_ROCK ||
                    target_prop->type == PropType::BERRY_BUSH || target_prop->type == PropType::DREAM_FLOWER) 
                 
        {
            const auto& cfg = get_prop_config(target_prop->type);
            ItemType item = prop_type_to_item(target_prop->type);
        
            int amount = cfg.harvest_amount;
            if (current_character == CharacterType::KAISER && (target_prop->type == PropType::STONE || target_prop->type == PropType::LARGE_ROCK))
                amount *= 2;
        
            int added = players[0].get_inventory().add_item(item, amount);
            if (added > 0) 
            {
                if (cfg.respawn_days > 0.0f) 
                {
                    target_prop->state = PropState::DEPLETED;
                    target_prop->respawn_at_playtime = playtime + cfg.respawn_days * day_duration_seconds;
                } 
                else 
                {
                    target_prop->state = PropState::GONE;
                }
            }
        }
        return;  // ← Si hubo prop, no pasa a portales
    }

    // 2. Si NO hubo prop, comprobar portales
    Island* target = nullptr;
    sf::Vector2f spawn;
    if (find_interactable_portal(&target, &spawn)) 
    {
        target_island = target;
        target_spawn = spawn;
        is_transitioning = true;
        transition_timer = 0.0f;
        players[0].reduce_sanity(10.0f);
    }
}

void Game::draw_clock(sf::RenderWindow& window) 
{
    window.setView(window.getDefaultView());
    
    const float clock_radius = 80.0f;
    const sf::Vector2f clock_center(1280.0f - 100.0f, 720.0f - 100.0f);
    
    // Fondo circular
    sf::CircleShape bg(clock_radius);
    bg.setOrigin(sf::Vector2f(clock_radius, clock_radius));
    bg.setPosition(clock_center);
    bg.setFillColor(sf::Color(20, 20, 30, 220));
    bg.setOutlineColor(sf::Color(200, 200, 200, 200));
    bg.setOutlineThickness(3.0f);
    window.draw(bg);
    
    // Marcas de las horas (cada 3h = 8 marcas)
    for (int i = 0; i < 8; ++i) 
    {
        float angle = i * (360.0f / 8.0f) * 3.14159265f / 180.0f - 3.14159265f / 2.0f;
        sf::RectangleShape mark(sf::Vector2f(3.0f, (i % 2 == 0) ? 12.0f : 6.0f));
        mark.setOrigin(sf::Vector2f(1.5f, (i % 2 == 0) ? 12.0f : 6.0f));
        mark.setPosition(clock_center);
        mark.setRotation(sf::radians(angle + 3.14159265f / 2.0f));
        mark.setFillColor(sf::Color(200, 200, 200));
        // Mover la marca hacia el borde
        mark.move(sf::Vector2f(std::cos(angle) * (clock_radius - 12.0f),
                                std::sin(angle) * (clock_radius - 12.0f)));
        window.draw(mark);
    }
    
    // Manecilla (apunta al "sol" en el cielo)
    // 0.0 = amanecer (izq), 0.25 = mediodía (arriba), 0.5 = atardecer (der), 0.75 = medianoche (abajo)
    float angle_rad = 3.14159265f + time_of_day * 2.0f * 3.14159265f;
    sf::RectangleShape hand(sf::Vector2f(clock_radius * 0.7f, 3.0f));
    hand.setOrigin(sf::Vector2f(0.0f, 1.5f));
    hand.setPosition(clock_center);
    hand.setRotation(sf::radians(angle_rad));
    hand.setFillColor(sf::Color(255, 220, 100));
    window.draw(hand);
    
    // Sol o luna en el centro
    sf::CircleShape celestial(12.0f);
    celestial.setOrigin(sf::Vector2f(12.0f, 12.0f));
    celestial.setPosition(clock_center);
    bool is_day = (time_of_day >= 0.05f && time_of_day < 0.55f);
    celestial.setFillColor(is_day ? sf::Color(255, 220, 100) : sf::Color(200, 200, 220));
    window.draw(celestial);
    
    // Texto con hora (HH:MM)
    // Convertir time_of_day (0.0=6:00) a hora del día
    float hour_float = (time_of_day * 24.0f + 6.0f);
    if (hour_float >= 24.0f) hour_float -= 24.0f;
    int hour = (int)hour_float;
    int minute = (int)((hour_float - hour) * 60.0f);
    
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", hour, minute);
    
    sf::Text time_text(*font, time_str, 14);
    time_text.setFillColor(sf::Color::White);
    time_text.setOutlineColor(sf::Color::Black);
    time_text.setOutlineThickness(2.0f);
    sf::FloatRect tb = time_text.getLocalBounds();
    time_text.setOrigin(sf::Vector2f(tb.size.x / 2.0f, 0));
    time_text.setPosition(sf::Vector2f(clock_center.x, clock_center.y + clock_radius + 5));
    window.draw(time_text);
    
    // Contador de días
    int day_count = (int)playtime / (int)day_duration_seconds;
    sf::Text day_text(*font, "Dia " + std::to_string(day_count + 1), 12);
    day_text.setFillColor(sf::Color(200, 200, 200));
    day_text.setOutlineColor(sf::Color::Black);
    day_text.setOutlineThickness(2.0f);
    sf::FloatRect db = day_text.getLocalBounds();
    day_text.setOrigin(sf::Vector2f(db.size.x / 2.0f, 0));
    day_text.setPosition(sf::Vector2f(clock_center.x, clock_center.y - clock_radius - 20));
    window.draw(day_text);
}

void Game::draw_stats_hud(sf::RenderWindow& window) 
{
    const auto& stats = players[0].get_stats();
    
    auto draw_bar = [&](float y, float fill, float max, sf::Color bg, sf::Color fg) {
        sf::RectangleShape bg_rect(sf::Vector2f(200, 20));
        bg_rect.setPosition(sf::Vector2f(20, y));
        bg_rect.setFillColor(bg);
        bg_rect.setOutlineColor(sf::Color(0, 0, 0, 200));
        bg_rect.setOutlineThickness(1.0f);
        window.draw(bg_rect);
        
        float ratio = (max > 0) ? (fill / max) : 0.0f;
        sf::RectangleShape bar(sf::Vector2f(200 * ratio, 20));
        bar.setPosition(sf::Vector2f(20, y));
        bar.setFillColor(fg);
        window.draw(bar);
        
        // Texto con el valor
        sf::Text label(*font, std::to_string((int)fill) + "/" + std::to_string((int)max), 14);
        label.setFillColor(sf::Color::White);
        label.setOutlineColor(sf::Color::Black);
        label.setOutlineThickness(1.0f);
        label.setPosition(sf::Vector2f(25, y + 2));
        window.draw(label);
    };
    
    draw_bar(20, stats.health, stats.max_health, sf::Color(50, 0, 0), sf::Color(200, 30, 30));
    draw_bar(50, stats.hunger, stats.max_hunger, sf::Color(50, 30, 0), sf::Color(255, 140, 0));
    draw_bar(80, stats.sanity, stats.max_sanity, sf::Color(30, 0, 50), sf::Color(160, 50, 200));
}

void Game::draw_inventory_hud(sf::RenderWindow& window) 
{
    const auto& inv = players[0].get_inventory();
    const auto& slots = inv.get_slots();
    
    const float slot_size = 60.0f;
    const float spacing = 8.0f;
    const int slot_count = slots.size();
    const float total_width = slot_count * slot_size + (slot_count - 1) * spacing;
    const float start_x = (1280.0f - total_width) / 2.0f;
    const float y = 720.0f - slot_size - 20.0f;
    
    for (int i = 0; i < slot_count; ++i) 
    {
        // Fondo del slot
        sf::RectangleShape slot_bg(sf::Vector2f(slot_size, slot_size));
        slot_bg.setPosition(sf::Vector2f(start_x + i * (slot_size + spacing), y));
        slot_bg.setFillColor(sf::Color(0, 0, 0, 180));
        slot_bg.setOutlineColor(sf::Color(200, 200, 200));
        slot_bg.setOutlineThickness(2.0f);
        window.draw(slot_bg);
        
        if (slots[i].is_empty()) continue;
        
        // Color por tipo de item
        sf::Color item_color;
        switch (slots[i].type) 
        {
            case ItemType::WOOD_BRANCH: item_color = sf::Color(139, 90, 43); break;
            case ItemType::STONE:       item_color = sf::Color(150, 150, 150); break;
            case ItemType::BERRY:       item_color = sf::Color(200, 50, 100); break;
            case ItemType::MANGO:       item_color = sf::Color(255, 200, 50); break;
            case ItemType::LIANA:        item_color = sf::Color(120, 180, 80); break;
            case ItemType::DREAM_FLOWER: item_color = sf::Color(200, 100, 255); break;
            default:                    item_color = sf::Color::White; break;
        }
        
        float sx = start_x + i * (slot_size + spacing);
        
        // Icono
        sf::CircleShape icon(slot_size * 0.3f);
        icon.setOrigin(sf::Vector2f(slot_size * 0.3f, slot_size * 0.3f));
        icon.setPosition(sf::Vector2f(sx + slot_size * 0.5f, y + slot_size * 0.4f));
        icon.setFillColor(item_color);
        icon.setOutlineColor(sf::Color::Black);
        icon.setOutlineThickness(1.0f);
        window.draw(icon);
        
        // Cantidad
        sf::Text qty(*font, std::to_string(slots[i].quantity), 16);
        qty.setFillColor(sf::Color::White);
        qty.setOutlineColor(sf::Color::Black);
        qty.setOutlineThickness(2.0f);
        qty.setPosition(sf::Vector2f(sx + slot_size * 0.6f, y + slot_size * 0.55f));
        window.draw(qty);
    }
}

void Game::draw_interaction_prompt(sf::RenderWindow& window) 
{
    if (players[0].is_busy() || is_transitioning) return;
    
    Prop* target_prop = find_interactable_prop();
    Island* target_portal = nullptr;
    sf::Vector2f portal_spawn;
    bool has_portal = find_interactable_portal(&target_portal, &portal_spawn);
    
    if (!target_prop && !has_portal) return;
    
    std::string prompt_text = "[E] ";
    if (target_prop) 
    {
        switch (target_prop->type) 
        {
            case PropType::TREE:
                prompt_text += (target_prop->state == PropState::AVAILABLE) ? "Escalar arbol" : "Escalar (sin ramas)";
                break;
            case PropType::MANGO_TREE:
                prompt_text += (target_prop->state == PropState::AVAILABLE) ? "Escalar mango" : "Escalar (sin frutas)";
                break;
            case PropType::LIANA_TREE:
                prompt_text += (target_prop->state == PropState::AVAILABLE) ? "Escalar lianas" : "Escalar (sin lianas)";
                break;
            case PropType::STONE:
                prompt_text += "Recoger piedra";
                break;
            case PropType::LARGE_ROCK:
                prompt_text += "Romper roca";
                break;
            case PropType::BERRY_BUSH:
                prompt_text += "Recoger moras";
                break;
            case PropType::DREAM_FLOWER:
                prompt_text += "Recoger flor de sueno";
                break;
            default:
                prompt_text += "Interactuar";
                break;
        }
    } 
    else if (has_portal) 
    {
        prompt_text += "Viajar a isla " + std::to_string(target_portal->id);
    }
    
    // Dibujar en el mundo, encima del jugador
    window.setView(camera);
    
    sf::Text prompt(*font, prompt_text, 18);
    prompt.setFillColor(sf::Color::White);
    prompt.setOutlineColor(sf::Color::Black);
    prompt.setOutlineThickness(3.0f);
    
    sf::Vector2f player_pos = players[0].get_position();
    sf::FloatRect bounds = prompt.getLocalBounds();
    prompt.setOrigin(sf::Vector2f(bounds.size.x / 2.0f, bounds.size.y));
    prompt.setPosition(sf::Vector2f(player_pos.x, player_pos.y - 30.0f));
    window.draw(prompt);
}

void Game::handle_eat() 
{
    if (players[0].is_busy() || is_transitioning) return;
    players[0].eat_first_food();
}

void Game::draw_hud(sf::RenderWindow& window) 
{
    window.setView(window.getDefaultView());
    draw_stats_hud(window);
    draw_inventory_hud(window);
    draw_interaction_prompt(window);
    draw_clock(window);
}

const PlayerStats& Game::get_stats() const { return players[0].get_stats(); }