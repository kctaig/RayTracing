#include <camera.hpp>

Ray Camera::gen_primary_ray(float x, float y, float aspect_ratio) const
{
    float v_half_height = tan(deg2rad(vfov / 2)) * focal_length;
    float v_half_width = v_half_height * aspect_ratio;
    float v_x = (2.0 * x - 1) * v_half_width;
    float v_y = (2.0 * y - 1) * v_half_height;
    vec3 ray_dir = vec3(v_x, v_y, pos.z - focal_length) - pos;
    return Ray(pos, ray_dir);
}
