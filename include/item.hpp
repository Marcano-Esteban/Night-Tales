#pragma once

enum class ItemType 
{
    NONE = 0,
    WOOD_BRANCH,    // Rama
    STONE,          // Piedra
    BERRY,          // Mora
    MANGO,          // Mango 
    LIANA,          // Liana 
    DREAM_FLOWER,   // Flor de sueño
    ITEM_COUNT
};

struct ItemDef 
{
    const char* name;
    int max_stack;   // 0 = no apilable
    float hunger_restore;
    float sanity_restore;
    bool is_edible() const { return hunger_restore > 0.0f || sanity_restore > 0.0f; }
};

const ItemDef& get_item_def(ItemType type);