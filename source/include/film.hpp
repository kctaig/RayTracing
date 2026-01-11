#pragma once

#include "head_include.hpp"

class Film {
  public:
    Film() = default;
    Film(const int w, const int h) : width(w), height(h) { pixels.resize(w * h); }

    void saveToFile(const std::string& fileName, int cur_sample = 1) const;
    void reset(int width, int height);
    void scale(float s);
    size_t index(const int x, const int y) const { return static_cast<size_t>(y * width + x); }
    vec3 getPixel(const int x, const int y) const { return pixels[index(x, y)]; }
    void addToPixel(const int x, const int y, const vec3 color) { pixels[index(x, y)] += color; }
    void setPixel(const int x, const int y, const vec3 color) { pixels[index(x, y)] = color; }

    int width{0}, height{0};

  private:
    vector<vec3> pixels;
};