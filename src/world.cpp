#include "../include/world.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <algorithm> 

// ¿Está la isla en el arreglo?
static bool contains(Designar::DynArray<Island*>& arr, Island* isla) 
{
    for (auto* x : arr) if (x == isla) return true;
    return false;
}

// Distancia entre dos islas
static float distance_between(Island* a, Island* b) 
{
    float dx = b->posicion.x - a->posicion.x;
    float dy = b->posicion.y - a->posicion.y;
    return std::sqrt(dx * dx + dy * dy);
}

World::World() : central(nullptr), current(nullptr), next_island_id(0) {}

World::~World() 
{
    for (auto* isla : all_islands) delete isla;
}

Island* World::create_island(IslandType tipo, int w, int h, unsigned int semilla) 
{
    Island* isla = new Island(next_island_id++, tipo, w, h, semilla, sf::Vector2f(0, 0));
    all_islands.append(isla);
    return isla;
}

// ✅ PASO 2: Genera SOLO la estructura (islas y jerarquía), sin aristas
void World::generate_tree_structure(Island* parent, int max_depth) 
{
    if (parent->profundidad >= max_depth) return;

    int min_h = 0, max_h = 0;
    if (parent->profundidad == 0)      { min_h = 3; max_h = 5; }
    else if (parent->profundidad == 1) { min_h = 1; max_h = 3; }
    else if (parent->profundidad == 2) { min_h = 0; max_h = 2; }
    else return;

    int n = min_h + (rng() % (max_h - min_h + 1));

    for (int i = 0; i < n; ++i) {
        Island* hija = create_island(IslandType::RANDOM, 150, 150, rng());
        parent->add_child(hija);
        // ❌ YA NO se añaden vecinas aquí (se hace en connect_*)
        generate_tree_structure(hija, max_depth);
    }
}

// ✅ PASO 1+2: Layout alrededor del padre con jitter
void World::layout_parent_radial() 
{
    central->posicion = sf::Vector2f(0, 0);
    central->radio = 50.0f;

    std::uniform_real_distribution<float> jitter_angle(-0.2f, 0.2f);
    std::uniform_real_distribution<float> jitter_dist(0.85f, 1.15f);

    std::function<void(Island*)> posicionar = [&](Island* isla) {
        int n = isla->hijas.size();
        if (n == 0) return;

        float paso = 2.0f * 3.14159265f / n;
        for (int i = 0; i < n; ++i) {
            float angulo = i * paso + jitter_angle(rng);
            float distancia = (300.0f + isla->profundidad * 100.0f) * jitter_dist(rng);

            Island* hija = isla->hijas[i];
            hija->posicion = isla->posicion + sf::Vector2f(
                std::cos(angulo) * distancia,
                std::sin(angulo) * distancia
            );
            hija->radio = 50.0f;
            posicionar(hija);
        }
    };
    posicionar(central);
}

// TODO: paso 4
void World::layout_center_radial() 
{
    central->posicion = sf::Vector2f(0, 0);
    central->radio = 50.0f;

    // Radios base por profundidad
    const float radio_base = 400.0f;
    const float radio_crecimiento = 300.0f;

    auto radio_de_profundidad = [&](int depth) {
        return radio_base + depth * radio_crecimiento;
    };

    // Repartición recursiva del sector angular
    std::function<void(Island*, float, float)> posicionar = [&](Island* isla, float angulo_inicio, float angulo_fin) {
        int n = isla->hijas.size();
        if (n == 0) return;

        float ancho = (angulo_fin - angulo_inicio) / n;

        for (int i = 0; i < n; ++i) {
            Island* hija = isla->hijas[i];
            float angulo_centro = angulo_inicio + ancho * (i + 0.5f);
            float radio = radio_de_profundidad(hija->profundidad);

            hija->posicion = sf::Vector2f(
                std::cos(angulo_centro) * radio,
                std::sin(angulo_centro) * radio
            );
            hija->radio = 50.0f;

            // Las hijas de esta hija heredan un sub-sector
            float margen = ancho * 0.4f;
            posicionar(hija, angulo_centro - margen, angulo_centro + margen);
        }
    };

    // La central reparte los 360° entre sus hijas
    posicionar(central, 0.0f, 2.0f * 3.14159265f);
}

// TODO: paso 5
void World::layout_random_scatter() 
{
    central->posicion = sf::Vector2f(0, 0);
    central->radio = 50.0f;

    std::uniform_real_distribution<float> ang_dist(0.0f, 2.0f * 3.14159265f);
    std::uniform_real_distribution<float> dist_dist(200.0f, 1400.0f);

    // Distancia mínima entre islas (para no solaparse)
    const float MIN_SEPARATION = 180.0f;
    const int MAX_INTENTOS = 80;

    // Recorrer todas las islas del árbol y colocar cada una al azar
    for (auto* isla : all_islands) {
        if (isla == central) continue;

        bool colocado = false;
        for (int intento = 0; intento < MAX_INTENTOS; ++intento) {
            float ang = ang_dist(rng);
            float dist = dist_dist(rng);
            sf::Vector2f pos(std::cos(ang) * dist, std::sin(ang) * dist);

            // ¿Se solapa con otra isla?
            bool overlap = false;
            for (auto* otra : all_islands) {
                if (otra == isla) continue;
                if (otra == central) continue; // La central ya está
                float dx = pos.x - otra->posicion.x;
                float dy = pos.y - otra->posicion.y;
                float d = std::sqrt(dx * dx + dy * dy);
                if (d < MIN_SEPARATION) { overlap = true; break; }
            }
            // También chequear contra la central
            float dxc = pos.x - central->posicion.x;
            float dyc = pos.y - central->posicion.y;
            if (std::sqrt(dxc*dxc + dyc*dyc) < MIN_SEPARATION) overlap = true;

            if (!overlap) {
                isla->posicion = pos;
                isla->radio = 50.0f;
                colocado = true;
                break;
            }
        }

        // Si no encontró hueco tras 80 intentos, la coloca igual
        if (!colocado) {
            float ang = ang_dist(rng);
            float dist = dist_dist(rng);
            isla->posicion = sf::Vector2f(std::cos(ang) * dist, std::sin(ang) * dist);
            isla->radio = 50.0f;
        }
    }
}

// ✅ PASO 2: Añade aristas padre-hijo (lo que antes estaba en generate_tree_structure)
void World::connect_tree_only() 
{
    for (auto* isla : all_islands) {
        for (auto* hija : isla->hijas) {
            Conexion c1;
            c1.destino = hija;
            c1.tipo = ConnectionType::CHAIN;
            c1.longitud = 0.0f;
            c1.estado_intacta = true;
            isla->add_neighbor(hija, c1);

            Conexion c2;
            c2.destino = isla;
            c2.tipo = ConnectionType::CHAIN;
            c2.longitud = 0.0f;
            c2.estado_intacta = true;
            hija->add_neighbor(isla, c2);
        }
    }
}

// TODO: paso 4
void World::add_shortcuts(float max_distance, float probability) {
    // Vacío por ahora
}

// TODO: paso 4
void World::add_ring_connections(float probability) 
{
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    // Recorrer cada profundidad
    for (int depth = 1; depth <= 3; ++depth) {
        // 1. Recopilar islas de este anillo
        Designar::DynArray<Island*> group;
        for (auto* isla : all_islands) {
            if (isla->profundidad == depth) group.append(isla);
        }
        if (group.size() < 2) continue;

        // 2. Ordenar por ángulo desde el centro
        std::sort(group.begin(), group.end(), [](Island* a, Island* b) {
            return std::atan2(a->posicion.y, a->posicion.x) <
                   std::atan2(b->posicion.y, b->posicion.x);
        });

        // 3. Conectar adyacentes (anillo cerrado)
        for (int i = 0; i < group.size(); ++i) {
            Island* a = group[i];
            Island* b = group[(i + 1) % group.size()];
            if (a == b) continue;

            // ¿Probabilidad?
            if (dist01(rng) >= probability) continue;

            // ¿Ya están conectadas? (evitar duplicados del árbol)
            bool already = false;
            for (const auto& con : a->vecinas) {
                if (con.destino == b) { already = true; break; }
            }
            if (already) continue;

            // Distancia entre ambas
            float dx = b->posicion.x - a->posicion.x;
            float dy = b->posicion.y - a->posicion.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            // ✅ Añadir conexión bidireccional
            Conexion c1;
            c1.destino = b;
            c1.tipo = ConnectionType::CHAIN;
            c1.longitud = distance;
            c1.estado_intacta = true;
            a->add_neighbor(b, c1);

            Conexion c2;
            c2.destino = a;
            c2.tipo = ConnectionType::CHAIN;
            c2.longitud = distance;
            c2.estado_intacta = true;
            b->add_neighbor(a, c2);
        }
    }
}

// TODO: paso 5
void World::connect_proximity(float max_distance, float probability) 
{
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    // ============ 1. Conectar pares cercanos ============
    for (int i = 0; i < all_islands.size(); ++i) {
        for (int j = i + 1; j < all_islands.size(); ++j) {
            Island* a = all_islands[i];
            Island* b = all_islands[j];

            float d = distance_between(a, b);
            if (d > max_distance) continue;
            if (dist01(rng) >= probability) continue;

            Conexion c1;
            c1.destino = b;
            c1.tipo = ConnectionType::CHAIN;
            c1.longitud = d;
            c1.estado_intacta = true;
            a->add_neighbor(b, c1);

            Conexion c2;
            c2.destino = a;
            c2.tipo = ConnectionType::CHAIN;
            c2.longitud = d;
            c2.estado_intacta = true;
            b->add_neighbor(a, c2);
        }
    }

    // ============ 2. BFS para saber qué islas son alcanzables ============
    Designar::DynArray<Island*> visitadas;
    Designar::DynArray<Island*> cola;
    cola.append(central);

    int head = 0;
    while (head < cola.size()) {
        Island* actual = cola[head++];
        if (contains(visitadas, actual)) continue;
        visitadas.append(actual);

        for (const auto& con : actual->vecinas) {
            if (!contains(visitadas, con.destino)) {
                cola.append(con.destino);
            }
        }
    }

    // ============ 3. Reconectar islas huérfanas ============
    // Recorremos todas las islas. Si no están en visitadas, las conectamos
    // a la isla ya-alcanzable más cercana, y añadimos al BFS.
    for (auto* isla : all_islands) {
        if (contains(visitadas, isla)) continue;

        Island* nearest = nullptr;
        float min_d = 1e9f;
        for (auto* v : visitadas) {
            float d = distance_between(v, isla);
            if (d < min_d) { min_d = d; nearest = v; }
        }
        if (!nearest) continue;

        Conexion c1;
        c1.destino = nearest;
        c1.tipo = ConnectionType::CHAIN;
        c1.longitud = min_d;
        c1.estado_intacta = true;
        isla->add_neighbor(nearest, c1);

        Conexion c2;
        c2.destino = isla;
        c2.tipo = ConnectionType::CHAIN;
        c2.longitud = min_d;
        c2.estado_intacta = true;
        nearest->add_neighbor(isla, c2);

        visitadas.append(isla);
    }
}

void World::create_world(unsigned int seed, LayoutStyle layout, ConnectionStyle connection) {
    rng.seed(seed);
    next_island_id = 0;

    // 1. Crear isla central
    central = create_island(IslandType::CENTRAL, 200, 200, seed);
    central->explorada = true;

    // 2. Generar estructura (islas y jerarquía)
    generate_tree_structure(central, 3);

    // 3. Aplicar layout
    switch (layout) 
    {
        case LayoutStyle::PARENT_RADIAL:  layout_parent_radial();  break;
        case LayoutStyle::CENTER_RADIAL:  layout_center_radial();  break;
        case LayoutStyle::RANDOM_SCATTER: layout_random_scatter(); break;
    }

    // 4. Aplicar conexiones
    switch (connection) 
    {
        case ConnectionStyle::TREE_ONLY:
            connect_tree_only();
            break;
        case ConnectionStyle::TREE_SHORTCUTS:
            connect_tree_only();
            add_shortcuts(700.0f, 0.35f); 
            break;
        case ConnectionStyle::RINGS:
            connect_tree_only();
            add_ring_connections(0.4f); 
            break;
        case ConnectionStyle::PROXIMITY:
            connect_proximity(600.0f, 0.5f); 
            break;
    }

    // 5. Conectar portales físicos (bucle existente, sin cambios)
    const float TILE_SIZE = 50.0f;
    for (auto* isla : all_islands) {
        for (auto& conexion : isla->vecinas) {
            isla->load();
            conexion.destino->load();

            sf::Vector2f dir = conexion.destino->posicion - isla->posicion;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) { dir.x /= len; dir.y /= len; }

            sf::Vector2f borde = isla->terrain.find_edge(dir.x, dir.y, TILE_SIZE);
            conexion.position = borde;
            conexion.radius = 60.0f;

            sf::Vector2f borde_destino = conexion.destino->terrain.find_edge(-dir.x, -dir.y, TILE_SIZE);
            conexion.spawn_position = borde_destino + (dir * 100.0f);

            isla->unload();
            conexion.destino->unload();
        }
    }

    central->load();
    current = central;
}

void World::travel_to(Island* target_island) {
    if (current) current->unload();
    target_island->load();
    current = target_island;
}

const Designar::DynArray<Island*>& World::get_all_islands() const {
    return all_islands;
}