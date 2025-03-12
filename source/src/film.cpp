#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "film.hpp"

void Film::reset(int width, int height)
{
	this->width = width;
	this->height = height;
	this->pixels.resize(static_cast<size_t>(width * height));
}

void Film::scale(float s)
{
	width = static_cast<int>(static_cast<float>(width) * s);
	height = static_cast<int>(static_cast<float>(height) * s);
	pixels.resize(static_cast<size_t> (width * height));
}

void Film::saveToFile(const std::string fileName, int ssp) const
{
	string dir = "../../output/" + fileName + "/";
	if (!std::filesystem::exists(dir))
	{
		std::filesystem::create_directory(dir);
	}
	string filePath = dir + fileName + "_" + std::to_string(ssp) + ".ppm";
	std::ofstream outfile(filePath, std::ios::binary);
	if (!outfile)
	{
		std::cout << "can not open file: " << fileName << std::endl;
	}
	outfile << "P6\n"
		<< width << ' ' << height << "\n255\n";
	const float gamma = 1.f / 2.2f;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			glm::vec3& color = static_cast <float>(1.0) / static_cast<float>(ssp) * getPixel(x, y);
			// 如果颜色出现负值，说明计算有误
			//if (color.x < 0.f || color.y<0.f || color.z <0.f) color = vec3(0,1,0);
			//if (color.x > 1.f || color.y > 1.f || color.z > 1.f) color = vec3(1, 0, 0);
			//if (color.x != color.x || color.y != color.y || color.z != color.z) color = vec3(0, 1, 0);
			color = glm::clamp(color, 0.f, 1.f);
			color = glm::pow(color, glm::vec3(gamma));
			glm::ivec3 pixelColor = glm::clamp(color * 255.f, 0.f, 255.f);
			outfile << static_cast<uint8_t>(pixelColor.x)
				<< static_cast<uint8_t>(pixelColor.y)
				<< static_cast<uint8_t>(pixelColor.z);
		}
	}
	outfile.close();
}