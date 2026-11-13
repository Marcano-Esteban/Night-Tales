#include "../include/island.hpp"

Island::Island(int id, IslandType type, int width, int height, unsigned int seed, sf::Vector2f pos)
    : id(id), type(type), terrain(width, height, seed), world_position(pos), generated(false), is_loaded(false) 
{
    // empty
}

void Island::generate() 
{
    if (generated) return; 

    terrain.generate();// Generamos el terreno
    terrain.rebuild_vertices(50.0f);// Reconstruimos los vértices con el TILE_SIZE que uses (50.0f)
    terrain.generate_props(50.0f);// Generamos los árboles y flores
    generated = true;
}

void Island::load() 
{
    if (is_loaded) return;
    if (!generated) generate();
    is_loaded = true;
}

void Island::unload() 
{
    if (!is_loaded) return;
    terrain.clear(); // Liberamos memoria
    is_loaded = false;
}

/** Nota: Para que unload libere memoria de verdad,
terrain_generator necesitaría un método clear()
que haga _vertices.clear() y _props.clear().*/
void Island::add_portal(sf::Vector2f pos, Island* target, sf::Vector2f spawn_pos) 
{
    Portal nuevo_portal;
    nuevo_portal.position = pos;
    nuevo_portal.radius = 60.0f;
    nuevo_portal.target_island = target;
    nuevo_portal.spawn_position = spawn_pos;
    
    portals.append(nuevo_portal); 
}