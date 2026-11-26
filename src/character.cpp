#include "../include/character.hpp"

const CharacterDef& get_character_def(CharacterType type) {
    static const CharacterDef defs[] = {
        {   // MANGA 🌸 - Ágil
            "Manga",
            "Agil y rapido. Escala arboles al doble de velocidad y puede ver en la oscuridad.",
            1.25f,      // speed_mult
            80.0f,      // max_health
            110.0f,     // max_sanity
            0.30f,      // hunger_decay
            2.0f,       // climb_speed_mult (el doble de rapido)
            0,          // harvest_bonus (base: 3)
            6,          // inventory_slots
            sf::Color(255, 220, 50),  // Amarillo
            UniqueAbility::NIGHT_VISION
        },
        {   // KAISER ⚔️ - Fuerte
            "Kaiser",
            "Fuerte y resistente. Aguanta mas dano, recolecta mas y rompe rocas grandes.",
            0.9f,       // speed_mult
            120.0f,     // max_health
            90.0f,      // max_sanity
            0.35f,      // hunger_decay
            1.0f,       // climb_speed_mult
            2,          // harvest_bonus (+2 = 5 ramas)
            8,          // inventory_slots
            sf::Color(80, 140, 240),  // Azul
            UniqueAbility::BREAK_ROCKS
        }
    };
    return defs[(int)type];
}

const char* get_ability_description(UniqueAbility ability) {
    switch (ability) {
        case UniqueAbility::NIGHT_VISION: return "Vision nocturna";
        case UniqueAbility::BREAK_ROCKS:  return "Rompe rocas grandes";
        default: return "";
    }
}