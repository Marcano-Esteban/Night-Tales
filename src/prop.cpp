#include "../include/prop.hpp"

const PropConfig& get_prop_config(PropType type) 
{
    // Índice = orden del enum
    static const PropConfig configs[] = {
        // respawn, harvest, resource, radius
        { 30.0f, 3, "madera", 20.0f },  // TREE
        {  0.0f, 0, "",        5.0f },  // FLOWER
        {  0.0f, 5, "piedra", 25.0f },  // STONE
        { 15.0f, 1, "bayas",  15.0f },  // BERRY_BUSH
    };
    return configs[(int)type];
}