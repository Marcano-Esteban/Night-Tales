#include "../include/save_manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sys/stat.h>

namespace fs = std::filesystem;

SaveManager::SaveManager(const std::string& dir) : saves_dir(dir) 
{
    std::filesystem::create_directories(saves_dir);  // Multiplataforma
}

std::string SaveManager::get_path(const std::string& filename) const 
{
    return saves_dir + filename + ".save";
}

std::string SaveManager::sanitize_name(const std::string& name) 
{
    std::string result;
    for (char c : name) 
    {
        if (std::isalnum((unsigned char)c) || c == '-' || c == '_') 
        {
            result += c;
        } 
        else if (c == ' ') 
        {
            result += '_';
        }
        // Cualquier otro carácter (acentos, símbolos) se descarta
    }
    if (result.empty()) result = "mundo_sin_nombre";
    return result;
}

bool SaveManager::world_name_exists(const std::string& world_name) const 
{
    auto saves = list_saves();
    for (const auto& s : saves) 
    {
        if (s.world_name == world_name) return true;
    }
    return false;
}

bool SaveManager::delete_save(const std::string& filename) 
{
    return std::remove(get_path(filename).c_str()) == 0;
}

std::vector<SaveSlotInfo> SaveManager::list_saves() const 
{
    std::vector<SaveSlotInfo> saves;
    if (!fs::exists(saves_dir)) return saves;

    for (const auto& entry : fs::directory_iterator(saves_dir)) 
    {
        if (entry.path().extension() != ".save") continue;

        SaveSlotInfo info;
        info.filename = entry.path().stem().string();

        std::ifstream file(entry.path());
        if (!file.is_open()) continue;

        std::string line;
        while (std::getline(file, line)) 
        {
            if (line.find("name=") == 0) info.world_name = line.substr(5);
            if (line.find("seed=") == 0) info.seed = std::stoul(line.substr(5));
            if (line.find("last_played=") == 0) info.last_played = std::stoll(line.substr(12));
            if (line.find("game_time=") == 0) info.game_time = std::stof(line.substr(10));
            if (line.find("playtime=") == 0) info.playtime_seconds = std::stof(line.substr(9));
        }
        saves.push_back(info);
    }

    // Ordenar por last_played (más reciente primero)
    std::sort(saves.begin(), saves.end(),
        [](const SaveSlotInfo& a, const SaveSlotInfo& b) {
            return a.last_played > b.last_played;
        });
    return saves;
}

bool SaveManager::save_game(const std::string& filename, const GameSaveData& data) const 
{
    std::ofstream file(get_path(filename));
    if (!file.is_open()) return false;

    file << "[WORLD]\n";
    file << "name=" << data.world_name << "\n";
    file << "seed=" << data.seed << "\n";
    file << "layout=" << data.layout_style << "\n";           
    file << "connection=" << data.connection_style << "\n";
    file << "current_island=" << data.current_island_id << "\n";
    file << "game_time=" << data.time_of_day << "\n";
    file << "playtime=" << data.playtime_seconds << "\n";
    file << "last_played=" << data.last_played << "\n";

    file << "[PLAYER]\n";
    file << "pos_x=" << data.player_position.x << "\n";
    file << "pos_y=" << data.player_position.y << "\n";

    file << "[ISLANDS]\n";
    for (const auto& isla : data.islands) 
    {
        file << "island=" << isla.id << "," << (isla.explorada ? "1" : "0") << ",";
        for (int i = 0; i < isla.deltas.size(); ++i) 
        {
            const auto& d = isla.deltas[i];
            if (i > 0) file << ";";
            file << d.prop_index << ":" << (int)d.state << ":" << d.respawn_at_playtime;
        }
        file << "\n";
    }
    return true;
}

bool SaveManager::load_game(const std::string& filename, GameSaveData& out) const 
{
    std::ifstream file(get_path(filename));
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.find("name=") == 0) out.world_name = line.substr(5);
        if (line.find("seed=") == 0) out.seed = std::stoul(line.substr(5));
        if (line.find("layout=") == 0) out.layout_style = std::stoi(line.substr(7));
        if (line.find("connection=") == 0) out.connection_style = std::stoi(line.substr(11)); 
        if (line.find("current_island=") == 0) out.current_island_id = std::stoi(line.substr(15));
        if (line.find("game_time=") == 0) out.time_of_day = std::stof(line.substr(10));
        if (line.find("playtime=") == 0) out.playtime_seconds = std::stof(line.substr(9));
        if (line.find("pos_x=") == 0) out.player_position.x = std::stof(line.substr(6));
        if (line.find("pos_y=") == 0) out.player_position.y = std::stof(line.substr(6));

        if (line.find("island=") == 0) 
        {
            std::string payload = line.substr(7);
            auto c1 = payload.find(',');
            auto c2 = payload.find(',', c1 + 1);

            IslandSaveData isla;
            isla.id = std::stoi(payload.substr(0, c1));
            isla.explorada = (payload.substr(c1 + 1, c2 - c1 - 1) == "1");

            std::string deltas_str = payload.substr(c2 + 1);
            std::stringstream ss(deltas_str);
            std::string item;
            while (std::getline(ss, item, ';')) 
            {
                if (item.empty()) continue;
                auto d1 = item.find(':');
                auto d2 = item.find(':', d1 + 1);
                PropDelta d;
                d.prop_index = std::stoi(item.substr(0, d1));
                d.state = (PropState)std::stoi(item.substr(d1 + 1, d2 - d1 - 1));
                d.respawn_at_playtime = std::stof(item.substr(d2 + 1));
                isla.deltas.append(d);
            }
            out.islands.append(isla);
        }
    }
    return true;
}