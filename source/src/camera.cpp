#include <camera.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

void Camera::generateMatrix()
{
	auto w = film->width;
	auto h = film->height;
	auto aspectRatio = static_cast<float>(w) / static_cast<float>(h);
	//camFromWorld = glm::lookAt(pos, viewpoint, up);
	clipFromCam = glm::perspective(
		glm::radians(vfov),
		aspectRatio,
		1.f,
		200.f
	);
}

Ray Camera::generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offsets) const
{
    auto ndc = (static_cast<glm::vec2>(pixelCoord) + offsets) / glm::vec2(film->width, film->height);
    ndc.y = 1.f - ndc.y;
    ndc = ndc * 2.f - 1.f;
	auto worldCoord = glm::inverse(clipFromCam) * glm::vec4(ndc, 1, 1.f);
	//auto worldCoord = glm::inverse(camFromWorld) * glm::inverse(clipFromCam) * glm::vec4(ndc, 0, 1.f);

	//cout << "world: " << worldCoord.x << " " << worldCoord.y << " " << worldCoord.z << endl;

	auto rayDir = normalize((vec3(worldCoord) - pos));
    return Ray(pos, rayDir);
}
