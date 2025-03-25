#include "camera.hpp"
#include <ext/matrix_transform.hpp>
#include <ext/matrix_clip_space.hpp>

Ray Camera::rayCasting(const shared_ptr<Film>filmPtr, const ivec2& pixelCoord) const
{
	float width = static_cast<float>(filmPtr->width);
	float height = static_cast<float>(filmPtr->height);

	//float h = std::tan(fovy * M_PI / 180.f * 0.5f) * 2.f;
	//float aspect = width / height;
	//vec3 forward = normalize(lookat - eye);
	//vec3 right = normalize(cross(forward, up));
	//vec3 ver = up * h;
	//vec3 hor = right * h * aspect;
	//float u = (static_cast<float>(pixelCoord.x) + genRandomFloat()) / width;
	//float v = (static_cast<float>(pixelCoord.y) + genRandomFloat()) / height;
	//vec3 tar = eye + forward + (u - 0.5f) * hor + (v - 0.5f) * ver;
	//return Ray(eye, normalize(tar - eye));

	vec3 forward = normalize(lookat - eye);
	vec3 right = normalize(cross(forward, up));
	vec3 up = cross(right, forward);
	// 计算投影矩阵
	float aspect = width / height;
	mat4 projection = glm::perspective(glm::radians(fovy), aspect, 0.01f, 1000.0f);
	mat4 view = glm::lookAt(eye, lookat, up);
	// 添加扰动
	float disturbed_x = static_cast<float>(pixelCoord.x) + genRandomFloat();
	disturbed_x = std::max(0.f, std::min(width - 1.f, disturbed_x));
	float disturbed_y = static_cast<float>(pixelCoord.y) + genRandomFloat();
	disturbed_y = std::max(0.f, std::min(height - 1.f, disturbed_y));
	// 计算屏幕坐标
	float screenX = (2.0f * disturbed_x / width) - 1.0f;
	float screenY = 1.0f - (2.0f * disturbed_y / height);
	// 根据透视投影计算反向光线
	vec4 rayClip(screenX, screenY, -1.0f, 1.0f);
	vec4 rayEye = glm::inverse(projection) * rayClip;
	rayEye = vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
	// 转换到世界空间
	vec3 rayDir = normalize(vec3(glm::inverse(view) * rayEye));
	return Ray{ eye, rayDir };
}