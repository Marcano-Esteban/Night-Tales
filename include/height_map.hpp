# pragma once

# include <vector>
// 2. MAPA DE ALTURAS
class height_map 
{
public:
    int width;
    int height;
    std::vector<std::vector<float>> data;
    
    height_map(int w, int h);
    float& at(int x, int y);
    const float& at(int x, int y) const;
};