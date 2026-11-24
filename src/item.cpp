#include "../include/item.hpp"

const ItemDef& get_item_def(ItemType type) 
{
    static const ItemDef defs[] = {
        // nombre, max_stack
        { "Nada",       0 },
        { "Rama",       20 },
        { "Piedra",     30 },
        { "Mora",       50 },
        { "Mango",      10 },
    };
    return defs[(int)type];
}