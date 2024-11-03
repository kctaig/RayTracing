#pragma once
#include "head_include.hpp"
#include <filesystem>

class Film
{
public:
    Film() {}

    Film(size_t w, size_t h) : width(w), height(h) { pixels.resize(w * h);}
    void saveToFile(const std::filesystem::path &fileName);
    vec3 getPixel(size_t x, size_t y) { return this->pixels[y * width + x]; }
    void setPixel(size_t x, size_t y, const glm::vec3 &color) { pixels[y * width + x] = color; }

    size_t width, height;
    vector<vec3> pixels;
};
