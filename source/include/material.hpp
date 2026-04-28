#pragma once

#include "utils.hpp"

class Texture {
  public:
    Texture() = default;
    Texture(const string& path, const string& texName);

    ~Texture();

    vec3 value(const vec2& uv) const;
    bool valid() const;

  private:
    float* data{nullptr};
    int width{0};
    int height{0};
    int channels{0};
    bool assumeSrgb{true};
};

template <typename T>
struct MaterialParam {
    MaterialParam() = default;
    explicit MaterialParam(const T& v) : value(v) {}

    T value{0};
    shared_ptr<Texture> texture;
    bool useTexture{false};

    T sample(const vec2& uv) const;
};

template <>
inline vec3 MaterialParam<vec3>::sample(const vec2& uv) const {
    if (useTexture && texture && texture->valid()) { return texture->value(uv); }
    return value;
}

template <>
inline float MaterialParam<float>::sample(const vec2& uv) const {
    if (useTexture && texture && texture->valid()) { return texture->value(uv).x; }
    return value;
}

class Material {
  public:
    Material() = default;

    Material(vec3 kd, vec3 ks, vec3 tr, float ns, float ni)
        : baseColor(kd), specular(ks), transmission(tr), roughness(ns), ior(ni) {}

    void evaluate(const vec2& uv) const;

    MaterialParam<vec3> baseColor{};
    MaterialParam<vec3> specular{};
    MaterialParam<vec3> transmission{};
    MaterialParam<vec3> emission{};
    MaterialParam<float> metallic{};
    MaterialParam<float> roughness{0};
    MaterialParam<float> sheen{0};
    MaterialParam<float> ior{0};

    // Auxiliary maps kept for future shading extensions.
    shared_ptr<Texture> ambientTex;
    shared_ptr<Texture> specularHighlightTex;
    shared_ptr<Texture> bumpTex;
    shared_ptr<Texture> displacementTex;
    shared_ptr<Texture> alphaTex;
    shared_ptr<Texture> reflectionTex;
    shared_ptr<Texture> normalTex;
};
