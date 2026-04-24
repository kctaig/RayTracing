#include "film.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

void Film::reset(const int w, const int h) {
    this->width = w;
    this->height = h;
    this->pixels.assign(static_cast<size_t>(w * h), vec3(0.0f));
}

void Film::clear() { std::fill(pixels.begin(), pixels.end(), vec3(0.0f)); }

void Film::toRGB8(vector<unsigned char>& out, int sampleCount, bool applyGamma) const {
    out.resize(static_cast<size_t>(width * height * 3));
    const float invSample = 1.0f / static_cast<float>(std::max(1, sampleCount));

    auto toSrgb = [](float linear) {
        linear = std::max(0.0f, linear);
        if (linear <= 0.0031308f) return 12.92f * linear;
        return 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            vec3 color = getPixel(x, y) * invSample;

            if (applyGamma) {
                // Compress HDR range first, then convert linear RGB to sRGB.
                color = color / (vec3(1.0f) + color);
                color = vec3(toSrgb(color.x), toSrgb(color.y), toSrgb(color.z));
            }

            const int idx = (y * width + x) * 3;
            const float r = std::min(255.99f, std::max(0.0f, color.x * 255.99f));
            const float g = std::min(255.99f, std::max(0.0f, color.y * 255.99f));
            const float b = std::min(255.99f, std::max(0.0f, color.z * 255.99f));
            out[idx] = static_cast<uint8_t>(r);
            out[idx + 1] = static_cast<uint8_t>(g);
            out[idx + 2] = static_cast<uint8_t>(b);
        }
    }
}

void Film::scale(const float s) {
    width = static_cast<int>(static_cast<float>(width) * s);
    height = static_cast<int>(static_cast<float>(height) * s);
    pixels.resize(static_cast<size_t>(width * height));
}

void Film::saveToFile(const std::string& fileName, int ssp) const {
    const fs::path projectRoot = fs::current_path();
    const string dir = (projectRoot / "outputs" / fileName).string() + "/";
    if (!fs::exists(dir)) fs::create_directories(dir);
    const string filePath = dir + fileName + "_" + std::to_string(ssp) + ".ppm";
    std::ofstream outfile(filePath, std::ios::binary);
    if (!outfile) { std::cout << "ERROR: Can not open file: " << fileName << std::endl; }
    outfile << "P6\n" << width << ' ' << height << "\n255\n";
    vector<unsigned char> imageData;
    toRGB8(imageData, ssp, true);
    outfile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    outfile.close();
}