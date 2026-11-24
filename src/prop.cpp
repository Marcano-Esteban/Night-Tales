#include "../include/prop.hpp"

const PropConfig& get_prop_config(PropType type) 
{
    // Índice = orden del enum
    static const PropConfig configs[] = {
        // respawn, harvest, resource, radius
        { 2.0f, 3, "madera", 20.0f },  // TREE
        {  0.0f, 0, "",        5.0f },  // FLOWER
        {  0.0f, 5, "piedra", 25.0f },  // STONE
        { 2.0f, 1, "bayas",  15.0f },  // BERRY_BUSH
    };
    return configs[(int)type];
}

ItemType prop_type_to_item(PropType type) 
{
    switch (type) 
    {
        case PropType::TREE:       return ItemType::WOOD_BRANCH;
        case PropType::STONE:      return ItemType::STONE;
        case PropType::BERRY_BUSH: return ItemType::BERRY;
        default:                   return ItemType::NONE;
    }
}