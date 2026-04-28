#include "material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <algorithm>
#include <cctype>

#include "stb_image.h"

Texture::Texture(const string& path, const string& texName) {
    data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    assumeSrgb = (ext != ".exr" && ext != ".hdr");
    if (!data)
        std::cerr << "Failed to load texture: " << path << std::endl;
    else
        std::cout << "Successfully Loaded texture: " << texName << std::endl;
}

Texture::~Texture() { stbi_image_free(data); }

bool Texture::valid() const { return data != nullptr && width > 0 && height > 0 && channels >= 3; }

vec3 Texture::value(const vec2& texCoord) const {
    if (!valid()) return vec3(0.0f);

    const float u = glm::clamp(texCoord.x, 0.f, 1.f);
    const float v = glm::clamp(texCoord.y, 0.f, 1.f);
    const int x = glm::clamp(static_cast<int>(u * (width - 1)), 0, width - 1);
    const int y = glm::clamp(static_cast<int>(v * (height - 1)), 0, height - 1);

    int offset = ((height - 1 - y) * width + x) * channels;
    offset = glm::clamp(offset, 0, width * height * channels - 3);

    vec3 c(data[offset], data[offset + 1], data[offset + 2]);
    if (assumeSrgb) { c = vec3(srgbToLinear(c.x), srgbToLinear(c.y), srgbToLinear(c.z)); }

    return c;
}

void Material::evaluate(const vec2& uv) const {
    baseColor.sample(uv);
    metallic.sample(uv);
    roughness.sample(uv);
    specular.sample(uv);
    transmission.sample(uv);
    emission.sample(uv);
    ior.sample(uv);
}
