#include "../include/utilyties.hpp"
#include <iostream>
#include <vector>

// ============================================
// Carga la fuente (assets/ primero, sistema después)
sf::Font& load_font()
{
    static sf::Font font;
    static bool loaded = false;

    if (!loaded)
    {
        // Lista de rutas ordenadas por prioridad
        std::vector<std::string> font_paths = {
            // 1. Assets local (portable)
            "assets/fonts/default.ttf",
            "../assets/fonts/default.ttf",
            "../../assets/fonts/default.ttf",
            
            // 2. Sistema Linux (CachyOS, Arch, Ubuntu, etc.)
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/noto/NotoSans-Regular.ttf",
            
            // 3. Windows
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            
            // 4. macOS
            "/System/Library/Fonts/Helvetica.ttc",
            "/Library/Fonts/Arial.ttf"
        };

        for (const auto& path : font_paths)
        {
            if (font.openFromFile(path))
            {
                loaded = true;
                std::cout << "Fuente cargada: " << path << std::endl;
                break;
            }
        }

        if (!loaded)
        {
            std::cout << "⚠️  No se pudo cargar ninguna fuente." << std::endl;
            std::cout << "   Crea la carpeta assets/fonts/ y pon una fuente default.ttf" << std::endl;
        }
    }

    return font;
}

// ============================================
// Crea un texto con la fuente dada
sf::Text create_info_text(sf::Font& font, const std::string& message)
{
    sf::Text info(font, message, 18);
    info.setFillColor(sf::Color::White);
    info.setPosition(sf::Vector2f(10, 10));
    return info;
}

// ============================================
// Crea un texto con posición personalizada
sf::Text create_text(sf::Font& font, const std::string& message, unsigned int size, sf::Vector2f position)
{                  
    sf::Text text(font, message, size);
    text.setFillColor(sf::Color::White);
    text.setPosition(position);
    return text;
}