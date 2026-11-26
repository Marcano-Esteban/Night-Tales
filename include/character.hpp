#pragma once
#include <SFML/Graphics.hpp>

enum class CharacterType { MANGA = 0, KAISER = 1 };

enum class UniqueAbility {
    NONE,
    NIGHT_VISION,    // Manga: ve en la oscuridad
    BREAK_ROCKS      // Káiser: rompe rocas grandes
};

struct CharacterDef {
    const char* name;
    const char* description;
    float speed_mult;
    float max_health;
    float max_sanity;
    float hunger_decay;
    float climb_speed_mult;
    int harvest_bonus;
    int inventory_slots;
    sf::Color color;
    UniqueAbility ability;
};

const CharacterDef& get_character_def(CharacterType type);
const char* get_ability_description(UniqueAbility ability);