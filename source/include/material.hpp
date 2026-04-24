#pragma once

#include "light.hpp"

class Texture {
  public:
    Texture() = default;
    ~Texture();
    Texture(const string& path, const string& texName);
    vec3 value(const vec2& texCoord) const;
    bool valid() const;

    float* data{nullptr};
    int width{0};
    int height{0};
    int channels{0};
    bool assumeSrgb{true};

class Material {
  public:
    Material() = default;

    Material(string name, vec3 kd, vec3 ks, vec3 tr, float ns, float ni)
        : matName(name),
          diffuse(kd),
          specular(ks),
          transmittance(tr),
          shininess(ns),
          refraIndex(ni) {}

    vec3 getDiffuse(const vec2& texCoord) const;

    string matName;

    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    float shininess;
    float refraIndex;

    shared_ptr<Light> light;

    bool useTexture = false;
    shared_ptr<Texture> texture;
};