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
		std::filesystem::create_directories(dir);
	}
	string filePath = dir + fileName + "_" + std::to_string(ssp) + ".ppm";
	std::ofstream outfile(filePath, std::ios::binary);
	if (!outfile)
	{
		std::cout << "can not open file: " << fileName << std::endl;
	}
	outfile << "P6\n" << width << ' ' << height << "\n255\n";
	const float gamma = 1.0f / 2.2f;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			glm::vec3 color = getPixel(x, y) / static_cast<float>(ssp);
			// check the color value
			//if (color.x < 0.f || color.y < 0.f || color.z < 0.f) color = vec3(0, 0, 1);
			//if (color.x > 1.f || color.y > 1.f || color.z > 1.f) color = vec3(1, 0, 0);
			//if (isNAN(color)) color = vec3(0, 1, 0);
			color = glm::pow(glm::clamp(color, 0.f, 1.f), glm::vec3(gamma)) * 255.99f;
			outfile << static_cast<uint8_t>(color.x)
					<< static_cast<uint8_t>(color.y)
					<< static_cast<uint8_t>(color.z);
		}
	}
	outfile.close();
}