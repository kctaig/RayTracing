#include "frame_buffer.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdint>

// image plane frame buffer 构造函数
Frame_Buffer::Frame_Buffer(size_t width, size_t height)
    : width(width), height(height) {
    pixels.resize(width * height);
}

void Frame_Buffer::save_to_file(const std::filesystem::path& filename) {
    // 输出 二进制 文件
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        std::cout << "can not open file: " << filename << std::endl;
    }
    outfile << "P6\n"
         << width << ' ' << height << "\n255\n";
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            const glm::vec3& color = get_pixel(x, y);
            glm::ivec3 color_i = glm::clamp(color * 255.f, 0.f, 255.f);
            outfile << static_cast<uint8_t>(color_i.x) << static_cast<uint8_t>(color_i.y) << static_cast<uint8_t>(color_i.z);
        }
    }
}
