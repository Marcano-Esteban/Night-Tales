#include "../include/world.hpp"
#include <cmath>
#include <iostream>

World::World() : central(nullptr), current(nullptr), next_island_id(0) {}

World::~World() 
{
    for (auto* isla : all_islands) 
    {
        delete isla;
    }
}

Island* World::create_island(IslandType tipo, int w, int h, unsigned int semilla) 
{
    Island* isla = new Island(next_island_id++, tipo, w, h, semilla, sf::Vector2f(0, 0));
    all_islands.append(isla);
    return isla;
}

// Genera recursivamente el árbol N-ario
void World::generate_tree_structure(Island* parent, int max_depth) 
{
    if (parent->profundidad >= max_depth) return;

    // Reglas de ramificación
    int min_hijas = 0, max_hijas = 0;
    if (parent->profundidad == 0) 
    { 
        min_hijas = 3;
        max_hijas = 5; 
    }
    else if (parent->profundidad == 1) 
    { 
        min_hijas = 1;
        max_hijas = 3; 
    }
    else if (parent->profundidad == 2) 
    { 
        min_hijas = 0; 
        max_hijas = 2; 
    }
    else return;

    int num_hijas = min_hijas + (rng() % (max_hijas - min_hijas + 1));

    for (int i = 0; i < num_hijas; ++i) 
    {
        unsigned int nueva_semilla = rng();
        Island* hija = create_island(IslandType::RANDOM, 150, 150, nueva_semilla);
        parent->add_child(hija);
        
        // Conectar vecinos (padre <-> hija)
        Conexion c1;
        c1.destino = hija;
        c1.tipo = ConnectionType::CHAIN;
        c1.longitud = 200.0f;
        c1.estado_intacta = true;
        parent->add_neighbor(hija, c1);

        Conexion c2;
        c2.destino = parent;
        c2.tipo = ConnectionType::CHAIN;
        c2.longitud = 200.0f;
        c2.estado_intacta = true;
        hija->add_neighbor(parent, c2);

        // Recursión
        generate_tree_structure(hija, max_depth);
    }
}

// Layout radial (trigonometría)
void World::layout_radial() 
{
    // 1. Posicionar la central en el centro (0,0)
    central->posicion = sf::Vector2f(0, 0);

    // 2. Recorrer el árbol asignando posiciones
    // Para simplificar, recorreremos recursivamente
    std::function<void(Island*, float)> posicionar = [&](Island* isla, float radio_anillo) {
        int num_hijas = isla->hijas.size();
        if (num_hijas == 0) return;

        // Distribuir las hijas en un círculo alrededor del padre
        float angulo_inicial = 0.0f;
        float paso_angulo = 2.0f * 3.14159265f / num_hijas;

        for (int i = 0; i < num_hijas; ++i) 
        {
            float angulo = angulo_inicial + i * paso_angulo;
            
            // Distancia desde el padre (puede variar según profundidad)
            float distancia = 300.0f + (isla->profundidad * 100.0f);

            sf::Vector2f offset(
                std::cos(angulo) * distancia,
                std::sin(angulo) * distancia
            );

            Island* hija = isla->hijas[i];
            hija->posicion = isla->posicion + offset;
            hija->radio = 50.0f; // Radio visual

            // Recursión con un radio de anillo mayor
            posicionar(hija, radio_anillo + 1.0f);
        }
    };

    posicionar(central, 1.0f);
}

void World::create_world(unsigned int seed, WorldStyle style) 
{
    rng.seed(seed);
    next_island_id = 0;

    // 1. Crear la isla central
    central = create_island(IslandType::CENTRAL, 200, 200, seed);
    central->load();
    current = central;
    central->explorada = true;

    // 2. Generar el árbol según el estilo
    if (style == WorldStyle::ARBOL_ATAJOS || style == WorldStyle::ARBOL_PURO) 
    {
        generate_tree_structure(central, 3); // Profundidad máxima 3
        layout_radial();
        // (Los atajos se implementarán después)
    } 
    else if (style == WorldStyle::ARCHIPIELAGO) 
    {
        // TODO: Implementar Poisson disk sampling
    }
    // ... otros estilos ...

    // 3. Conectar portales físicos (usando los bordes del terreno)
    const float TILE_SIZE = 50.0f;
    for (auto* isla : all_islands) 
    {
        // Para cada vecina, crear un portal en el borde
        for (auto& conexion : isla->vecinas) 
        {
            // Cargar temporalmente la isla para calcular el borde
            isla->load();
            conexion.destino->load();

            // Calcular dirección (hacia dónde está la otra isla)
            sf::Vector2f dir = conexion.destino->posicion - isla->posicion;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) 
            {
                dir.x /= len;
                dir.y /= len;
            }

            // Calcular borde en esa dirección
            sf::Vector2f borde = isla->terrain.find_edge(dir.x, dir.y, TILE_SIZE);
            conexion.position = borde;
            conexion.radius = 60.0f;
            
            // El spawn es un poco adentro del borde de la otra isla
            sf::Vector2f borde_destino = conexion.destino->terrain.find_edge(-dir.x, -dir.y, TILE_SIZE);
            conexion.spawn_position = borde_destino + (dir * 100.0f);

            isla->unload();
            conexion.destino->unload();
        }
    }

    // Cargar solo la central al final
    central->load();
    current = central;
}

void World::travel_to(Island* target_island) 
{
    if (current) current->unload();
    target_island->load();
    current = target_island;
}

const Designar::DynArray<Island*>& World::get_all_islands() const
{
    return all_islands;
}