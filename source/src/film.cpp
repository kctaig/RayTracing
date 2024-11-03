#include "film.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

void Film::saveToFile(const std::filesystem::path &fileName)
{
    // 输出 二进制 文件
    std::ofstream outfile(fileName, std::ios::binary);
    if (!outfile)
    {
        std::cout << "can not open file: " << fileName << std::endl;
    }
    outfile << "P6\n"
            << width << ' ' << height << "\n255\n";
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const glm::vec3 &color = getPixel(x, y);
            glm::ivec3 color_i = glm::clamp(color * 255.f, 0.f, 255.f);
            outfile << static_cast<uint8_t>(color_i.x)
                    << static_cast<uint8_t>(color_i.y)
                    << static_cast<uint8_t>(color_i.z);
        }
    }
}
