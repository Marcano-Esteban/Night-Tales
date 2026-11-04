# include "../include/terrain_generator.hpp"

terrain_generator::terrain_generator(int width, int height, unsigned int seed)
: noise(seed), _height_map(width, height), _vertices_dirty(true), rng(seed) {}
   
void terrain_generator::generate(float frequency, float seaLevel) 
{
    int octaves = 6;
    float centerX = _height_map.width / 2.0f;
    float centerY = _height_map.height / 2.0f;
    float maxDist = std::min(_height_map.width, _height_map.height) / 2.0f * 0.95f;
    
    const float SEA_LEVEL = 0.35f;

    for (int y = 0; y < _height_map.height; ++y)  
    {
        for (int x = 0; x < _height_map.width; ++x) 
        {
            // 1. Calcular coordenadas normalizadas al centro
            float dx = (x - centerX);
            float dy = (y - centerY);
            float dist = std::sqrt(dx*dx + dy*dy) / maxDist;
            
            // 2. CORTE DURO: Nada de tierra más allá del 90% del radio
            if (dist > 0.90f)
            {
                _height_map.at(x, y) = 0.0f;
                continue;
            }
            
            // 3. DEFORMACIÓN DEL RADIO (Domain Warping)
            // Usamos el ruido con frecuencia muy baja para crear formas alargadas
            float shape_noise_x = (x - centerX) * 0.01f; // Frecuencia baja
            float shape_noise_y = (y - centerY) * 0.01f;
            float shape_val = noise.fractal_brownian_motion(shape_noise_x, shape_noise_y, 8); // Solo 2 octavas para formas grandes
            
            // shape_val está entre -1 y 1, lo mapeamos a un multiplicador de radio
            // 0.6 significa que el radio puede variar entre 0.4 y 1.6 veces su valor original
            float radius_modifier = 1.0f + shape_val * 0.8f; 
            
            // Si el modificador es negativo (imposible en la práctica con 0.6, pero por seguridad)
            radius_modifier = std::max(0.3f, radius_modifier);
            
            // 4. Nueva distancia deformada
            float warped_dist = dist / radius_modifier;
            
            // 5. Máscara radial basada en la distancia deformada
            float radial_mask = 1.0f - warped_dist;
            radial_mask = std::max(0.0f, std::min(1.0f, radial_mask));
            radial_mask = radial_mask * radial_mask * (3 - 2 * radial_mask);
            
            // 6. Ruido para el terreno (montañas, valles)
            float nx = (x - centerX) * frequency;
            float ny = (y - centerY) * frequency;
            float noise_val = noise.fractal_brownian_motion(nx, ny, octaves);
            
            // 7. Combinar
            float height = radial_mask * 0.7f + (noise_val * 0.3f);
            height *= radial_mask; // Falloff extra en los bordes
            
            // 8. Asegurar rango
            height = std::max(0.0f, std::min(1.0f, height));
            
            _height_map.at(x, y) = height;
        }
    }
    _vertices_dirty = true;
}

sf::Color terrain_generator::get_color(float height) 
{
    if (height < 0.35f) 
    {
        float depth = (0.35f - height) / 0.35f;
        if (depth > 0.5f) 
        {
            return sf::Color(20, 60, 140);
        } 
        else 
        {
            return sf::Color(60, 130, 200);
        }
    } 
    else if (height < 0.42f) 
    {
        return sf::Color(194, 178, 128);
    } 
    else if (height < 0.55f) 
    {
        return sf::Color(120, 190, 80);
    } 
    else if (height < 0.65f) 
    {
        return sf::Color(80, 160, 60);
    } 
    else if (height < 0.75f) 
    {
        return sf::Color(150, 130, 90);
    } 
    else if (height < 0.85f) 
    {
        return sf::Color(180, 160, 130);
    } 
    else 
    {
        return sf::Color(240, 240, 255);
    }
}
    
void terrain_generator::rebuild_vertices(float tile_size)
{
    _vertices.clear();
    _vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    _vertices.resize(_height_map.width * _height_map.height * 6);
    
    // Offset para centrar el terreno
    float totalWidth = _height_map.width * tile_size;
    float totalHeight = _height_map.height * tile_size;
    float offsetX = -totalWidth / 2.0f;
    float offsetY = -totalHeight / 2.0f;
    
    int index = 0; 
    
    for (int y = 0; y < _height_map.height; ++y) 
    {
        for (int x = 0; x < _height_map.width; ++x) 
        {
            float height = _height_map.at(x, y);
            sf::Color color = get_color(height);

            // Pintar zonas de teletransporte
            if (height > 0.35f) 
            {
                if (x < 15) // Borde izquierdo
                {
                    color = sf::Color(160, 32, 240); // Púrpura mágico
                }
                else if (x > _height_map.width - 15) // Borde derecho
                {
                    color = sf::Color(255, 20, 147); // Rosa intenso
                }
            }
            
            // Coordenadas del tile
            float left   = x * tile_size + offsetX;
            float top    = y * tile_size + offsetY;
            float right  = left + tile_size;
            float bottom = top + tile_size;
            
            // Generar los 2 triángulos
            // Triángulo 1 (sup-izq, sup-der, inf-izq)
            _vertices[index + 0] = sf::Vertex{sf::Vector2f(left,  top),    color};
            _vertices[index + 1] = sf::Vertex{sf::Vector2f(right, top),    color};
            _vertices[index + 2] = sf::Vertex{sf::Vector2f(left,  bottom), color};
            
            // Triángulo 2 (sup-der, inf-der, inf-izq)
            _vertices[index + 3] = sf::Vertex{sf::Vector2f(right, top),    color};
            _vertices[index + 4] = sf::Vertex{sf::Vector2f(right, bottom), color};
            _vertices[index + 5] = sf::Vertex{sf::Vector2f(left,  bottom), color};
            
            index += 6;
        }
    }
    
    _vertices_dirty = false;
}

sf::Vector2f terrain_generator::find_spawn_point(float tile_size) const
{
    // Buscar el tile más alto en el centro del mapa (dentro de un radio)
    int center_x = _height_map.width / 2;
    int center_y = _height_map.height / 2;
    int search_radius = 50;  // Buscar en un radio de 50 tiles
    
    int best_x = center_x;
    int best_y = center_y;
    float best_height = -1.0f;
    
    for (int y = center_y - search_radius; y < center_y + search_radius; ++y)
    {
        for (int x = center_x - search_radius; x < center_x + search_radius; ++x)
        {
            if (x < 0 || x >= _height_map.width || y < 0 || y >= _height_map.height) continue;
                
            float h = _height_map.at(x, y);
            if (h > best_height)
            {
                best_height = h;
                best_x = x;
                best_y = y;
            }
        }
    }
    
    // Convertir coordenadas del mapa a coordenadas del mundo
    // El centro del mapa es (0, 0) en el mundo
    float world_x = (best_x - _height_map.width / 2.0f) * tile_size;
    float world_y = (best_y - _height_map.height / 2.0f) * tile_size;
    
    return sf::Vector2f(world_x, world_y);
}

void terrain_generator::generate_props(float tile_size)
{
    _props.clear();
    
    // Distribución uniforme para probabilidades (0.0 a 1.0)
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < _height_map.height; ++y) 
    {
        for (int x = 0; x < _height_map.width; ++x) 
        {
            float height = _height_map.at(x, y);
            
            // Coordenadas del mundo (centradas)
            float world_x = (x - _height_map.width / 2.0f) * tile_size;
            float world_y = (y - _height_map.height / 2.0f) * tile_size;

            // ÁRBOLES: Solo en pasto/bosque (altura entre 0.42 y 0.75) 2% de probabilidad por tile
            if (height > 0.42f && height < 0.75f && dist(rng) < 0.02f) 
            {
                // Un poco de aleatoriedad en la posición para que no se vean cuadriculados
                float offset_x = (dist(rng) - 0.5f) * tile_size;
                float offset_y = (dist(rng) - 0.5f) * tile_size;
                
                _props.append({
                    sf::Vector2f(world_x + offset_x, world_y + offset_y),
                    PropType::TREE,
                    tile_size * 0.3f, // Radio de colisión (30% del tamaño del tile)
                    sf::Color::White
                });
            }
 
            // FLORES: En pasto (altura entre 0.42 y 0.65) 3% de probabilidad
            else if (height > 0.42f && height < 0.65f && dist(rng) < 0.03f)
            {
                float offset_x = (dist(rng) - 0.5f) * tile_size;
                float offset_y = (dist(rng) - 0.5f) * tile_size;

                sf::Color flower_color;
                int color_choice = rand() % 3; 
                if (color_choice == 0) flower_color = sf::Color::Yellow;
                else if (color_choice == 1) flower_color = sf::Color(255, 105, 180); // Rosa
                else flower_color = sf::Color::White;
                
                _props.append({
                    sf::Vector2f(world_x + offset_x, world_y + offset_y),
                    PropType::FLOWER,
                    0.0f, // Sin colisión
                    flower_color
                });
            }
        }
    }
}

void terrain_generator::draw_props(sf::RenderWindow& window, float tile_size)
{
    // Obtenemos la vista actual para no dibujar lo que está fuera de pantalla (Culling)
    sf::FloatRect view_bounds = window.getView().getViewport();
    sf::Vector2f view_center = window.getView().getCenter();
    sf::Vector2f view_size = window.getView().getSize();
    
    float left = view_center.x - view_size.x / 2.0f;
    float right = view_center.x + view_size.x / 2.0f;
    float top = view_center.y - view_size.y / 2.0f;
    float bottom = view_center.y + view_size.y / 2.0f;
    float margin = 50.0f; // Margen extra para no verlos aparecer de golpe

    sf::CircleShape canopy;
    sf::RectangleShape trunk;
    sf::CircleShape flower;

    for (const auto& prop : _props)
    {
        // Culling: Si está fuera de la pantalla, no lo dibujamos (ahorra mucha CPU)
        if (prop.position.x < left - margin || prop.position.x > right + margin || prop.position.y < top - margin || prop.position.y > bottom + margin)
        {
            continue;
        }

        if (prop.type == PropType::TREE)
        {
            trunk.setSize(sf::Vector2f(tile_size * 0.2f, tile_size * 0.7f));
            trunk.setFillColor(sf::Color(101, 67, 33)); // Marrón
            trunk.setOrigin(sf::Vector2f(tile_size * 0.1f, tile_size * 0.7f)); // Origen abajo
            trunk.setPosition(prop.position);
            window.draw(trunk);

            float canopy_radius = tile_size * 0.4f;
            canopy.setRadius(canopy_radius);
            canopy.setOrigin(sf::Vector2f(canopy_radius, canopy_radius));
            canopy.setFillColor(sf::Color(34, 139, 34)); // Verde bosque

            canopy.setPosition(sf::Vector2f(prop.position.x, prop.position.y - tile_size * 0.6f));
            window.draw(canopy);
        }
        else if (prop.type == PropType::FLOWER)
        {
            float flower_radius = tile_size * 0.08f;
            flower.setRadius(flower_radius);
            flower.setOrigin(sf::Vector2f(flower_radius, flower_radius));
            flower.setPosition(prop.position);
            
            // Colores aleatorios para las flores (amarillo, rosa, blanco)
            flower.setFillColor(prop.color);
            window.draw(flower);
        }
    }
}

sf::Vector2f terrain_generator::find_edge(float dir_x, float dir_y, float tile_size) const
{
    // Normalizar la dirección
    float length = std::sqrt(dir_x * dir_x + dir_y * dir_y);
    if (length > 0.0f) 
    {
        dir_x /= length;
        dir_y /= length;
    }

    float centerX = _height_map.width / 2.0f;
    float centerY = _height_map.height / 2.0f;
    
    float current_x = centerX;
    float current_y = centerY;
    
    // Avanzamos en pasos pequeños (0.5 tiles) hasta encontrar agua
    float step = 0.5f; 
    
    while (true)  
    {
        current_x += dir_x * step;
        current_y += dir_y * step;
        
        int tile_x = static_cast<int>(current_x);
        int tile_y = static_cast<int>(current_y);
        
        // Si nos salimos del mapa, nos detenemos
        if (tile_x < 0 || tile_x >= _height_map.width || tile_y < 0 || tile_y >= _height_map.height)     
        {
            break;
        }
        
        // Si encontramos cielo/agua, retrocedemos un paso (para quedarnos en tierra)
        if (_height_map.at(tile_x, tile_y) <= 0.35f) 
        {
            current_x -= dir_x * step;
            current_y -= dir_y * step;
            break;
        }
    }
    
    // Convertir a coordenadas del mundo
    float world_x = (current_x - _height_map.width / 2.0f) * tile_size;
    float world_y = (current_y - _height_map.height / 2.0f) * tile_size;
    
    return sf::Vector2f(world_x, world_y);
}

void terrain_generator::draw(sf::RenderWindow& window, float tileSize) 
{
    // Si el tamaño del tile cambió, reconstruir los vértices
    if (_vertices_dirty)
    {
        rebuild_vertices(tileSize);
    }
    window.draw(_vertices);
}
    
const height_map& terrain_generator::get_height_map() const { return _height_map; }

const Designar::DynArray<Prop>& terrain_generator::get_props() const { return _props; }

