#include <camera.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

Ray Camera::generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offsets) const
{
	float width = static_cast<float>(filmPtr->width);
	float height = static_cast<float>(filmPtr->height);

	vec3 forward = normalize(lookat - eye);
	vec3 right = normalize(cross(forward, up));
	vec3 up = cross(right, forward);

	// 计算投影矩阵
	float aspect = width / height;
	mat4 projection = glm::perspective(glm::radians(fovy), aspect, 0.1f, 100.0f);
	mat4 view = glm::lookAt(eye, lookat, up);
	// 计算屏幕坐标
	float x = (2.0f * (pixelCoord.x + 0.5f) / float(width)) - 1.0f;
	float y = 1.0f - (2.0f * (pixelCoord.y + 0.5f) / float(height));

	// 根据透视投影计算反向光线
	glm::vec4 rayClip(x, y, -1.0f, 1.0f);
	glm::vec4 rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

	// 转换到世界空间
	vec3 rayDir = normalize(glm::vec3(glm::inverse(view) * rayEye));

	// 返回光线
	return Ray{eye, rayDir};
}