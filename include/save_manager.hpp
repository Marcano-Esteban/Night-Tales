#pragma once
#include <array.hpp>
#include <string>
#include <vector>
#include <ctime>
#include <SFML/System/Vector2.hpp>
#include "prop.hpp"

struct IslandSaveData 
{
    int id;
    bool explorada;
    Designar::DynArray<PropDelta> deltas;
};

struct GameSaveData 
{
    std::string world_name;
    unsigned int seed = 0;
    int current_island_id = 0;
    float time_of_day = 0.25f;
    float playtime_seconds = 0.0f;
    long long last_played = 0;
    sf::Vector2f player_position = {0.0f, 0.0f};
    Designar::DynArray<IslandSaveData> islands;
    int layout_style = 0;
    int connection_style = 0;
    int character_type = 0; 
};

struct SaveSlotInfo 
{
    std::string filename;      // Sin ".save" (ej: "Mi_Mundo")
    std::string world_name;    // Como lo escribió el jugador ("Mi Mundo")
    long long last_played = 0;
    float game_time = 0.0f;
    float playtime_seconds = 0.0f;
    unsigned int seed = 0;
};

class SaveManager 
{
public:
    SaveManager(const std::string& dir = "saves/");

    std::vector<SaveSlotInfo> list_saves() const;
    bool world_name_exists(const std::string& world_name) const;
    bool delete_save(const std::string& filename);
    std::string get_path(const std::string& filename) const;

    bool save_game(const std::string& filename, const GameSaveData& data) const;
    bool load_game(const std::string& filename, GameSaveData& out_data) const;

    // Convierte "Mi Mundo" → "Mi_Mundo"
    static std::string sanitize_name(const std::string& name);

private:
    std::string saves_dir;
};