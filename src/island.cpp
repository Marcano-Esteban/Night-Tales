#include "../include/island.hpp"

Island::Island(int id, IslandType tipo, int width, int height, unsigned int semilla, sf::Vector2f pos)
    : id(id), tipo(tipo), semilla(semilla), profundidad(0), posicion(pos), radio(50.0f),
      padre(nullptr), terrain(width, height, semilla), generated(false), is_loaded(false),
      explorada(false), modificada(false) {}

void Island::load() 
{
    if (is_loaded) return;
    if (!generated) 
    {
        terrain.generate();
        generated = true;
    }
    terrain.generate_props(50.0f, this->props);
    for (const auto& delta : prop_deltas) 
    {
        if (delta.prop_index < props.size()) 
        {
            props[delta.prop_index].state = delta.state;
            props[delta.prop_index].respawn_timer = delta.respawn_timer;
        }
    }
    terrain.rebuild_vertices(50.0f);
    is_loaded = true;
}

void Island::unload() 
{
    if (!is_loaded) return;
    prop_deltas.clear();
    
    for (int i = 0; i < props.size(); ++i) 
    {
        if (props[i].state != PropState::AVAILABLE) 
        {
            prop_deltas.append({i, props[i].state, props[i].respawn_timer});
        }
    }
    props.clear();
    terrain.clear();
    is_loaded = false;
}

void Island::update_props(float dt) 
{
    if (!is_loaded) return;   // Solo actualiza la isla activa
    for (auto& prop : props) 
    {
        if (prop.state != PropState::DEPLETED) continue;
        prop.respawn_timer -= dt;
        if (prop.respawn_timer <= 0.0f) 
        {
            prop.state = PropState::AVAILABLE;
            prop.respawn_timer = 0.0f;
        }
    }
}

void Island::draw_props(sf::RenderWindow& window, float tile_size) 
{
    terrain.draw_props(window, props, tile_size);
}

void Island::add_child(Island* child) 
{
    hijas.append(child);
    child->padre = this;
    child->profundidad = profundidad + 1;
}

void Island::add_neighbor(Island* neighbor, const Conexion& conexion) 
{
    vecinas.append(conexion);
}