#include "material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <algorithm>
#include <cctype>

#include "stb_image.h"

namespace {
inline float srgbToLinear(float c) {
    c = glm::clamp(c, 0.0f, 1.0f);
    if (c <= 0.04045f) return c / 12.92f;
    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}
}  // namespace

Texture::Texture(const string& path, const string& texName) {
    data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
    if (!data)
        std::cerr << "Failed to load texture: " << path << std::endl;
    else
        std::cout << "Successfully Loaded texture: " << texName << std::endl;
}

Texture::~Texture() { stbi_image_free(data); }

vec3 Texture::value(const vec2& texCoord) const {
    const float u = glm::clamp(texCoord.x, 0.f, 1.f);
    const float v = glm::clamp(texCoord.y, 0.f, 1.f);
    const int x = static_cast<int>(u * width);
    const int y = static_cast<int>(v * height);
    int offset = ((height - y) * width + x) * channels;
    offset = glm::clamp(offset, 0, width * height * channels - 3);
    return vec3(data[offset], data[offset + 1], data[offset + 2]);
}

vec3 Material::getDiffuse(const vec2& texCoord) const {
    return useTexture ? texture->value(texCoord) : diffuse;
}
