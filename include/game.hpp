#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <cstdint>
#include <string>
#include "../include/world.hpp"
#include "../include/player.hpp"
#include "../include/save_manager.hpp"

class Game 
{
private:
    World world;
    Designar::DynArray<player> players;
    Island* current_island = nullptr;
    
    sf::View camera;
    float zoom = 1.0f;
    
    bool is_transitioning = false;
    float transition_timer = 0.0f;
    Island* target_island = nullptr;
    sf::Vector2f target_spawn;
    
    float time_of_day = 0.25f;
    float day_duration_seconds = 600.0f;
    float playtime = 0.0f;
    
    unsigned int current_seed = 0;
    std::string current_world_name;
    
    sf::RectangleShape fade_rect;
    std::optional<sf::Text> info_text;
    sf::Font* font = nullptr;
    
    // Minimapa y mapa grande
    sf::View minimap_view;
    bool show_big_map = false;
    
    void draw_map(sf::RenderWindow& window, bool fullscreen);
    Prop* find_interactable_prop();
    bool  find_interactable_portal(Island** out_target, sf::Vector2f* out_spawn);

    void draw_inventory_hud(sf::RenderWindow& window);
    void draw_stats_hud(sf::RenderWindow& window);
    void draw_interaction_prompt(sf::RenderWindow& window);

    LayoutStyle current_layout = LayoutStyle::PARENT_RADIAL;
    ConnectionStyle current_connection = ConnectionStyle::TREE_SHORTCUTS;

    CharacterType current_character = CharacterType::MANGA;
    void draw_clock(sf::RenderWindow& window);

public:
    Game();
    void init(sf::Font& f);
    
    void start_new_game(const std::string& world_name, unsigned int seed, int player_count,
                        LayoutStyle layout, ConnectionStyle connection, CharacterType character);
    void load_from_save(const GameSaveData& data);
    GameSaveData get_save_data() const;
    
    void update(float dt);
    void draw_hud(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void draw_day_night_overlay(sf::RenderWindow& window);
    void handle_interact();
    float get_playtime() const { return playtime; }
    
    void toggle_big_map() { show_big_map = !show_big_map; }
    bool is_big_map_open() const { return show_big_map; }
    
    float get_time_of_day() const { return time_of_day; }
    unsigned int get_seed() const { return current_seed; }
    std::string get_world_name() const { return current_world_name; }
    CharacterType get_character() const { return current_character; }
    sf::Vector2f get_player_position() const;
    const PlayerStats& get_stats() const;
};