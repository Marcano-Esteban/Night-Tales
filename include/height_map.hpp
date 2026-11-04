# pragma once

# include <array.hpp>
// MAPA DE ALTURAS
class height_map 
{
public:
    int width;
    int height;
    Designar::DynArray<Designar::DynArray<float>> data;
    
    height_map(int w, int h);
    float& at(int x, int y);
    const float& at(int x, int y) const;
};