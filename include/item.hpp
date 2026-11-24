#pragma once

enum class ItemType 
{
    NONE = 0,
    WOOD_BRANCH,    // Rama (del árbol)
    STONE,          // Piedra
    BERRY,          // Mora
    MANGO,          // Mango
    ITEM_COUNT      // ← Truco para saber cuántos hay
};

struct ItemDef 
{
    const char* name;
    int max_stack;   // 0 = no apilable
};

const ItemDef& get_item_def(ItemType type);