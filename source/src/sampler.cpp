#include "sampler.hpp"

vec3 Sampler::weightSamplingOnMesh() const {
    float r1 = genRandomFloat(), r2 = genRandomFloat();
    if (r1 + r2 > 1.f) {
        r1 = 1.f - r1;
        r2 = 1.f - r2;
    }
    return vec3{1.f - r1 - r2, r1, r2};
}