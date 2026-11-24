#include "../include/inventory.hpp"

Inventory::Inventory(int size) : max_slots(size) 
{
    slots = Designar::DynArray<InventorySlot>(size, InventorySlot{});
}

int Inventory::add_item(ItemType type, int quantity) 
{
    if (type == ItemType::NONE || quantity <= 0) return 0;
    int max_stack = get_item_def(type).max_stack;
    if (max_stack <= 0) max_stack = 1;

    int remaining = quantity;

    // 1. Llenar stacks existentes del mismo tipo
    for (auto& slot : slots) 
    {
        if (remaining <= 0) break;
        if (slot.type != type) continue;
        int space = max_stack - slot.quantity;
        if (space <= 0) continue;
        int to_add = std::min(space, remaining);
        slot.quantity += to_add;
        remaining -= to_add;
    }

    // 2. Usar slots vacíos
    for (auto& slot : slots) 
    {
        if (remaining <= 0) break;
        if (!slot.is_empty()) continue;
        int to_add = std::min(max_stack, remaining);
        slot.type = type;
        slot.quantity = to_add;
        remaining -= to_add;
    }

    return quantity - remaining;
}

int Inventory::remove_item(ItemType type, int quantity) 
{
    int remaining = quantity;
    for (auto& slot : slots) 
    {
        if (remaining <= 0) break;
        if (slot.type != type) continue;
        int to_remove = std::min(slot.quantity, remaining);
        slot.quantity -= to_remove;
        remaining -= to_remove;
        if (slot.quantity <= 0) 
        {
            slot.type = ItemType::NONE;
            slot.quantity = 0;
        }
    }
    return quantity - remaining;
}

int Inventory::count_item(ItemType type) const 
{
    int total = 0;
    for (const auto& slot : slots) 
    {
        if (slot.type == type) total += slot.quantity;
    }
    return total;
}

int Inventory::free_space_for(ItemType type, int quantity) const 
{
    int max_stack = get_item_def(type).max_stack;
    if (max_stack <= 0) max_stack = 1;
    int space = 0;
    for (const auto& slot : slots) 
    {
        if (slot.is_empty()) space += max_stack;
        else if (slot.type == type) space += (max_stack - slot.quantity);
    }
    return std::min(space, quantity);
}

int Inventory::used_slots() const 
{
    int count = 0;
    for (const auto& slot : slots) 
    {
        if (!slot.is_empty()) count++;
    }
    return count;
}

void Inventory::resize(int new_size) 
{
    Designar::DynArray<InventorySlot> new_slots(new_size, InventorySlot{});
    int copy_count = std::min(new_size, max_slots);
    for (int i = 0; i < copy_count; ++i) 
    {
        new_slots[i] = slots[i];
    }
    slots = new_slots;
    max_slots = new_size;
}