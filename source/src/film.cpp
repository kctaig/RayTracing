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
    constexpr float gamma = 1.0f / 2.2f;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            vec3 color = getPixel(x, y) * invSample;
            color = clamp(color, 0.f, 1.f);
            if (applyGamma) color = pow(color, vec3(gamma));

            const int idx = (y * width + x) * 3;
            out[idx] = static_cast<uint8_t>(color.x * 255.99f);
            out[idx + 1] = static_cast<uint8_t>(color.y * 255.99f);
            out[idx + 2] = static_cast<uint8_t>(color.z * 255.99f);
        }
    }
}

void Film::scale(const float s) {
    width = static_cast<int>(static_cast<float>(width) * s);
    height = static_cast<int>(static_cast<float>(height) * s);
    pixels.resize(static_cast<size_t>(width * height));
}

void Film::saveToFile(const std::string& fileName, int ssp) const {
    const string dir = "../../outputs/" + fileName + "/";
    if (!std::filesystem::exists(dir)) { std::filesystem::create_directories(dir); }
    const string filePath = dir + fileName + "_" + std::to_string(ssp) + ".ppm";
    std::ofstream outfile(filePath, std::ios::binary);
    if (!outfile) { std::cout << "can not open file: " << fileName << std::endl; }
    outfile << "P6\n" << width << ' ' << height << "\n255\n";
    vector<unsigned char> imageData;
    toRGB8(imageData, ssp, true);
    outfile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    outfile.close();
}