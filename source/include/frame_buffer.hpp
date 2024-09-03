#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

class Frame_Buffer {
public:
    Frame_Buffer(size_t width, size_t height);
    // 将像素存储到图片中
    void save_to_file(const std::filesystem::path& filename);
    size_t get_width() const { return this->width; } // 不能在 const 成员函数中修改类成员变量
    size_t get_height() const { return this->height; }
    // 获取像素颜色
    glm::vec3 get_pixel(size_t x, size_t y) { return this->pixels[y * width + x]; }
    // 设置像素颜色
    void set_pixel(size_t x, size_t y, const glm::vec3& color) { pixels[y * width + x] = color; }

private:
    size_t width, height;
    std::vector<glm::vec3> pixels; // 帧缓冲中的像素
};