#pragma once
#include "terrain_generator.hpp"
#include "prop.hpp" 

enum class IslandType { CENTRAL, VILLAGE, TEMPLE, RANDOM };
enum class ConnectionType { CHAIN, PORTAL };

class Island;

struct Anchor 
{
    sf::Vector2f position;
};

struct Conexion 
{
    Island* destino;
    ConnectionType tipo;
    float longitud;             // Para CADENA
    bool estado_intacta;        // Para CADENA
    int id;                     // Para PORTAL
    bool bidireccional;         // Para PORTAL
    sf::Vector2f position;      // Dónde está la conexión en esta isla
    float radius;               // Radio de interacción
    sf::Vector2f spawn_position;// Dónde aparece el jugador al cruzar
};

class Island 
{
public:
    
    int id;                     // ── Identidad ─────────────────────────────
    unsigned int semilla;
    int profundidad;
    IslandType tipo;

    sf::Vector2f posicion;      // ── Posición en el cielo ──────────────────
    float radio;

    Island* padre;              // ── Jerarquía del árbol ───────────────────
    Designar::DynArray<Island*> hijas;

    Designar::DynArray<Conexion> vecinas;// ── Grafo de navegación ───────────────────

    terrain_generator terrain;  // ── Terreno (carga/descarga) ──────────────
    bool generated;
    bool is_loaded;

    bool explorada;             // ── Estado de juego ───────────────────────
    bool modificada;
    Designar::DynArray<Anchor> anchors;

    Island(int id, IslandType tipo, int width, int height, unsigned int semilla, sf::Vector2f pos);

    Designar::DynArray<Prop> props;           // Solo cuando is_loaded
    Designar::DynArray<PropDelta> prop_deltas; // Siempre (persiste)
    
    void load();
    void unload();
    void update_props(float dt);
    void draw_props(sf::RenderWindow& window, float tile_size);
    
    void add_child(Island* child);
    void add_neighbor(Island* neighbor, const Conexion& conexion);
};