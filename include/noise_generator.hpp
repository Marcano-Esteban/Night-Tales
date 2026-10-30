# pragma once

#include <vector>
#include <random>
#include <algorithm>


class noise_generator 
{
private:
    std::mt19937 rng;
    std::vector<int> permutation;

    float fade_function(float t);
    float linear_interpolation(float t, float a, float b);
    float gradient(int hash, float x, float y);
    float perlin_noise(float x, float y); 

public:
    noise_generator(unsigned int seed = 42);
    float fractal_brownian_motion(float x, float y, int octaves); //Fractal Brownian Motion
    

};
 