#include "../include/noise_generator.hpp"

noise_generator::noise_generator(unsigned int seed) : rng(seed), permutation(512, 0)
{ 
    for (int i = 0; i < 256; ++i)
    { 
        permutation[i] = i;
    }

    std::shuffle(permutation.begin(), permutation.begin() + 256, rng);

    for (int i = 0; i < 256; ++i)
    { 
        permutation[i + 256] = permutation[i];
    }
}
    
float noise_generator::perlin_noise(float x, float y) 
{
    // ----------------------------------------
    // Posición relativa dentro de la celda (0.0 a 1.0)
    float local_x = x - std::floor(x);      // Si x = 3.7 → local_x = 0.7 (70% hacia la derecha)
    float local_y = y - std::floor(y);   // Si y = 2.3 → local_y = 0.3 (30% hacia abajo)
    
    // ----------------------------------------
    // Encontrar la celda que contiene el punto
    int x0 = (int)std::floor(x) & 255;     // Esquina izquierda
    int y0 = (int)std::floor(y) & 255;   // Esquina superior
    int x1 = x0 + 1;                       // Esquina derecha
    int y1 = y0 + 1;                       // Esquina inferior
    
    // ----------------------------------------
    // Suavizar las coordenadas locales con FADE
    float u = fade_function(local_x);  // Suavizado horizontal
    float v = fade_function(local_y);  // Suavizado vertical
    
    // ----------------------------------------
    // Obtener los índices de las 4 esquinas
    int hash_x0_y0 = permutation[x0] + y0;  // Esquina a (sup-izq) // La tabla de permutación "desordena" las coordenadas
    int hash_x1_y0 = permutation[x1] + y0;  // Esquina b (sup-der) // para que el ruido sea pseudoaleatorio,
    int hash_x0_y1 = permutation[x0] + y1;  // Esquina c (inf-izq) // & 255 asegura que el índice esté entre 0 y 255
    int hash_x1_y1 = permutation[x1] + y1;  // Esquina d (inf-der)
    
    // ----------------------------------------
    // Calcular el gradiente en cada esquina
    float dot_a = gradient(permutation[hash_x0_y0], local_x,     local_y);     // Cada esquina tiene una "dirección" (flecha) aleatoria
    float dot_b = gradient(permutation[hash_x1_y0], local_x - 1, local_y);     // grad() calcula el producto punto entre esa dirección
    float dot_c = gradient(permutation[hash_x0_y1], local_x,     local_y - 1); // y el vector desde la esquina hasta el punto
    float dot_d = gradient(permutation[hash_x1_y1], local_x - 1, local_y - 1);
    
    // ----------------------------------------
    // Interpolar los valores de las 4 esquinas
    float interp_top = linear_interpolation(u, dot_a, dot_b);  // Fila superior: a → b
    float interp_bottom = linear_interpolation(u, dot_c, dot_d);  // Fila inferior: c → d
    
    // Luego mezclamos verticalmente (entre las dos filas)
    float value = linear_interpolation(v, interp_top, interp_bottom);
    
    return value;  // Valor entre -1.0 y 1.0 aproximadamente
}
    
float noise_generator::fractal_brownian_motion(float x, float y, int octaves = 6) 
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;        
    for (int i = 0; i < octaves; ++i) 
    {
        total += perlin_noise(x * frequency, y * frequency) * amplitude;

        maxValue += amplitude; // Acumular la amplitud máxima (para normalizar)            
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    // Normalizar a 0.0 - 1.0
    return total / maxValue;
}
    
float noise_generator::fade_function(float t) 
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}
    
float noise_generator::linear_interpolation(float t, float a, float b) 
{
    return a + t * (b - a);
}
    
float noise_generator::gradient(int hash, float x, float y) 
{
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

