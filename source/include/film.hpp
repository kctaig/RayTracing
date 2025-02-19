#pragma once

#include "head_include.hpp"

class Film
{
public:

	// 用于测试的分辨率
	Film() : width(100), height(100) {
		pixels.resize(width * height);
	}

	Film(int w, int h) : width(w), height(h) { pixels.resize(w * h); }
	void saveToFile(const std::string fileName);
	void reset(int width, int height);
	vec3 getPixel(int x, int y) const { return pixels[y * width + x]; }
	void setPixel(int x, int y, const glm::vec3 color) { pixels[y * width + x] = color; }

	int width, height;
private:
	vector<vec3> pixels;
};