#include "camera.hpp"
#include <ext/matrix_transform.hpp>
#include <ext/matrix_clip_space.hpp>

Ray Camera::rayCasting(const shared_ptr<Film>filmPtr, const ivec2& pixelCoord) const
{
	float width = static_cast<float>(filmPtr->width);
	float height = static_cast<float>(filmPtr->height);
	float aspect = width / height;

	vec3 forward = normalize(lookat - eye);
	vec3 right = normalize(cross(forward, up));
	vec3 up = cross(right, forward);

	/*
	* 屏幕坐标系统以屏幕的左上角为原点，坐标为 [0,0]。在这个系统中，x 轴的正方向是向右，y 轴的正方向是向下。
	* 视口坐标系统则以视口的中心为原点，坐标为 [0,0]。在这个系统中，x 轴的正方向同样向右，但 y 轴的正方向是向上。
	* 因此，在将屏幕坐标转换为视口坐标时，需要进行以下转换：
	* 1. 首先，将屏幕坐标的 y 轴方向反转，以匹配视口坐标系统中 y 轴向上的方向。
	* 2. 其次，将屏幕坐标的原点从左上角平移到视口的中心。这涉及到从屏幕坐标中减去半个屏幕的宽度和高度。
	* 这样，屏幕上的像素遍历（从左到右，从上到下）就可以转换为视口坐标系统中的遍历（从中心向右，从中心向上）。
	*/

	float viewportH = std::tan(glm::radians(fovy) * 0.5f) * 2.f;
	float viweportW = viewportH * aspect;
	float screenX = (static_cast<float>(pixelCoord.x) + genRandomFloat()) / width ;
	float screenY = (static_cast<float>(pixelCoord.y) + genRandomFloat()) / height ;

	float viewportX = (screenX - 0.5f) * viweportW;
	float viewportY = ((1.0 - screenY) - 0.5f) * viewportH;
	vec3 viewportCoord = eye 
		+ forward 
		+ viewportX * right
		+ viewportY * up ;
	return Ray(eye, normalize(viewportCoord - eye));

	/*
	// also works
	// 计算投影矩阵
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
	*/
}