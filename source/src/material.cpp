#include "head_include.hpp"
#include "material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const string& path, const string& texName)
{
	data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	if (!data) std::cerr << "Failed to load texture: " << path << std::endl;
	else std::cout << "Successfully Loaded texture: " << texName << std::endl;
}

vec3 Texture::value(const vec2 &texCoord) const
{
	float u = glm::clamp(texCoord.x, 0.f, 1.f);
	float v = glm::clamp(texCoord.y, 0.f, 1.f);
	int x = static_cast<int>(u * width);
	int y = static_cast<int>(v * height);
	int offset = (y * width + x) * channels;
	offset = glm::clamp(offset, 0, width * height * channels - 3);
	float r = data[offset];
	float g = data[offset + 1];
	float b = data[offset + 2];
	return vec3(r, g, b);
}

vec3 Material::getDiffuse(const vec2& texcoord) const
{
	return useTexture ? texturePtr->value(texcoord) : diffuse;
}
