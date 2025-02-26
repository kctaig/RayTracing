#include "film.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

void Film::reset(int width, int height)
{
	this->width = width;
	this->height = height;
	this->pixels.resize(width * height);
}

void Film::saveToFile(const std::string fileName, int spp)
{
	// 输出 二进制 文件
	std::ofstream outfile(fileName, std::ios::binary);
	if (!outfile)
	{
		std::cout << "can not open file: " << fileName << std::endl;
	}
	outfile << "P6\n"
		<< width << ' ' << height << "\n255\n";
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			glm::vec3& color = static_cast <float>(1.0) / static_cast<float>(spp) * getPixel(x, y);
			glm::ivec3 pixelColor = glm::clamp(color * 255.f, 0.f, 255.f);
			outfile << static_cast<uint8_t>(pixelColor.x)
				<< static_cast<uint8_t>(pixelColor.y)
				<< static_cast<uint8_t>(pixelColor.z);
		}
	}
	outfile.close();
}