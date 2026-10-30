    #include "../include/height_map.hpp"

    
    height_map::height_map(int w, int h) : width(w), height(h) 
    {
        data.resize(h, std::vector<float>(w, 0.0f));
    }
    
    float& height_map::at(int x, int y)
    { 
        return data[y][x]; 
    }
    
    const float& height_map::at(int x, int y) const 
    { 
        return data[y][x]; 
    }
