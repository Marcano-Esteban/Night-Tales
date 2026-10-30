#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Carga una fuente (assets/ primero, sistema después)
// Devuelve una REFERENCIA a la fuente estática
sf::Font& load_font();

// Crea un texto con la fuente dada
sf::Text create_info_text(sf::Font& font, const std::string& message);

// Crea un texto con posición personalizada
sf::Text create_text(sf::Font& font, const std::string& message, unsigned int size, sf::Vector2f position);
                     