#include "../include/item.hpp"

const ItemDef& get_item_def(ItemType type) 
{
    static const ItemDef defs[] = {
        // nombre, max_stack, hambre, cordura
        { "Nada",           0,  0.0f,  0.0f },
        { "Rama",          20,  0.0f,  0.0f },
        { "Piedra",        30,  0.0f,  0.0f },
        { "Mora",          50, 15.0f,  5.0f },   // Comestible
        { "Mango",         10, 30.0f, 10.0f },   // Comestible
        { "Liana",         15,  0.0f,  0.0f },
        { "Flor de sueño",  5,  0.0f,  0.0f },
    };
    return defs[(int)type];
}