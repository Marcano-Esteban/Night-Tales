#pragma once
#include <array.hpp>
#include <algorithm>
#include "item.hpp"

struct InventorySlot 
{
    ItemType type = ItemType::NONE;
    int quantity = 0;
    bool is_empty() const { return type == ItemType::NONE || quantity <= 0; }
};

class Inventory 
{
private:
    Designar::DynArray<InventorySlot> slots;
    int max_slots;

public:
    Inventory(int size = 6);

    // Añade items. Devuelve cuántos se añadieron realmente (0 si lleno).
    int add_item(ItemType type, int quantity);

    // Quita items. Devuelve cuántos se quitaron.
    int remove_item(ItemType type, int quantity);

    int count_item(ItemType type) const;
    int free_space_for(ItemType type, int quantity) const;

    int used_slots() const;
    int total_slots() const { return max_slots; }
    const Designar::DynArray<InventorySlot>& get_slots() const { return slots; }

    // ✅ Preparado para ampliar (mochila, cofre, etc.)
    void resize(int new_size);
};