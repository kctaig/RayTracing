#include <camera.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

//void Camera::generateMatrix()
//{
//	auto w = film->width;
//	auto h = film->height;
//	auto aspectRatio = static_cast<float>(w) / static_cast<float>(h);
//	camFromWorld = glm::lookAt(eye, lookat, up);
//	clipFromCam = glm::perspective(
//		glm::radians(fovy),
//		aspectRatio,
//		1.f,
//		200.f
//	);
//}

Ray Camera::generateRay(const glm::ivec2& pixelCoord, const glm::vec2& offsets) const
{
	//auto ndc = (static_cast<glm::vec2>(pixelCoord) + offsets) / glm::vec2(film->width, film->height);
	//ndc.y = 1.f - ndc.y;
	//ndc = ndc * 2.f - 1.f;
	//auto worldCoord = glm::inverse(clipFromCam) * glm::vec4(ndc, 1, 1.f);
	//auto worldCoord = glm::inverse(camFromWorld) * glm::inverse(clipFromCam) * glm::vec4(ndc, 0, 1.f);

	//cout << "world: " << worldCoord.x << " " << worldCoord.y << " " << worldCoord.z << endl;

	//float fovy_rad = fovy * M_PI / 180.0;
	float focal_length = film->height / (2.0 * tan(fovy / 2.0));

	vec3 view = normalize(lookat - eye);
	vec3 right = normalize(glm::cross(up, view));
	vec3 up = glm::cross(view, right);

	float plane_height = 2.0 * focal_length * tan(fovy / 2.0);
	float plane_width = plane_height * (float)film->width / film->height;

	float x = (2.0 * pixelCoord.x / film->width - 1.0) * plane_width / 2.0;
	float y = (2.0 * pixelCoord.y / film->height - 1.0) * plane_height / 2.0;

	vec3 ray_dir = view + x * right + y * up;

	return Ray(eye, ray_dir);
}