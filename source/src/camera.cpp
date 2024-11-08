#include <camera.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

Ray Camera::generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offsets) const
{
	// the ratio of width to height
	auto aspectRatio = static_cast<float>(film->width) / static_cast<float>(film->height);
	// ndc coordinates
    auto ndc = (static_cast<glm::vec2>(pixelCoord) + offsets) / glm::vec2(film->width - 1, film->height - 1);
    ndc.y = 1.f - ndc.y;
    ndc = ndc * 2.f - 1.f;
	ndc.x *= aspectRatio;
	ndc *= tan(glm::radians(vfov / 2));
	// set the view plane is  where z = -1
	auto worldCoord =  glm::vec4(ndc, -1, 1.f);
	// from camera space to world space
	worldCoord = glm::inverse(glm::lookAt(pos, viewPoint, up)) * worldCoord;
	auto rayDir = glm::normalize(glm::vec3(worldCoord)-pos);
    return Ray(pos, rayDir);
}
