#include "../include/prop.hpp"

const PropConfig& get_prop_config(PropType type) 
{
    // Índice = orden del enum
    static const PropConfig configs[] = {
        // días, harvest, resource, radius
        { 2.0f, 3, "rama",          20.0f },  // TREE
        { 4.5f, 5, "mango",         25.0f },  // MANGO_TREE (respawn 4-5 días)
        { 3.0f, 2, "liana",         22.0f },  // LIANA_TREE
        { 0.0f, 0, "",               5.0f },  // FLOWER (decorativa)
        { 0.0f, 1, "flor de sueno",  6.0f },  // DREAM_FLOWER (no renovable)
        { 0.0f, 5, "piedra",        25.0f },  // STONE (no renovable)
        { 0.0f, 10, "piedra",       35.0f },  // LARGE_ROCK (da muchas)
        { 2.0f, 1, "mora",          15.0f },  // BERRY_BUSH
    };
    return configs[(int)type];
}

ItemType prop_type_to_item(PropType type) 
{
    switch (type) 
    {
        case PropType::TREE:         return ItemType::WOOD_BRANCH;
        case PropType::MANGO_TREE:   return ItemType::MANGO;
        case PropType::LIANA_TREE:   return ItemType::LIANA;
        case PropType::STONE:        return ItemType::STONE;
        case PropType::LARGE_ROCK:   return ItemType::STONE;
        case PropType::BERRY_BUSH:   return ItemType::BERRY;
        case PropType::DREAM_FLOWER: return ItemType::DREAM_FLOWER;
        default:                     return ItemType::NONE;
    }
}