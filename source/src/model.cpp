#include "model.hpp"

#include <algorithm>

#include "bbox.hpp"
#include "material.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

bool Mesh::intersection(const Ray& ray, PayLoad& payload) const {
    bool isHit = false;
    const vec3& v0 = vertices[0].pos;
    const vec3& v1 = vertices[1].pos;
    const vec3& v2 = vertices[2].pos;
    vec3 E1 = v1 - v0;
    vec3 E2 = v2 - v0;
    // check the ray is parallel to the triangle
    if (vec3 N = cross(E1, E2); fabs(dot(N, -ray.getDir())) < EPSILON) return isHit;
    vec3 T = ray.getOrigin() - v0;
    vec3 D = normalize(ray.getDir());
    vec3 P = cross(D, E2);
    vec3 Q = cross(T, E1);
    float p_e1 = dot(P, E1);
    if (fabs(p_e1) < EPSILON) return isHit;
    float t = dot(Q, E2) / p_e1;
    float u = dot(P, T) / p_e1;
    float v = dot(Q, D) / p_e1;

    if (t > 0.01f && t < payload.t && u >= 0 && v >= 0 && 1 - u - v >= 0) {
        isHit = true;
        payload.t = t;
        payload.pos = ray.at(t);
        payload.uv = {u, v};
        // geometric normal for robust side tests (reflection/refraction/offset)
        vec3 gn = normalize(cross(E1, E2));
        payload.frontFace = dot(gn, D) < 0.f;
        payload.geoNormal = payload.frontFace ? gn : -gn;

        // shading normal for BRDF evaluation
        vec3 sn = normalize(calculateNormal({u, v}));
        payload.normal = dot(sn, payload.geoNormal) < 0.f ? -sn : sn;
    }
    return isHit;
}

float Mesh::calculateArea() const {
    const vec3& v0 = vertices[0].pos;
    const vec3& v1 = vertices[1].pos;
    const vec3& v2 = vertices[2].pos;
    const vec3 E1 = v1 - v0;
    const vec3 E2 = v2 - v0;
    return length(cross(E1, E2)) / 2.0f;
}

vec2 Mesh::getTexCoord(const vec2& uv) const {
    return vertices[0].uv * (1.f - uv.x - uv.y) + vertices[1].uv * uv.x + vertices[2].uv * uv.y;
}

vec3 Mesh::calculateNormal(const vec2& uv) const {
    return vertices[0].normal * (1.f - uv.x - uv.y) + vertices[1].normal * uv.x +
           vertices[2].normal * uv.y;
}

void Model::loadFromFile(const string& fileDir, const string& fileName) {
    meshes.clear();
    materials.clear();

    auto start = std::chrono::high_resolution_clock::now();

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = fileDir;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(fileDir + "/" + fileName + ".obj", reader_config)) {
        if (!reader.Error().empty()) { std::cerr << "TinyObjReader: " << reader.Error(); }
        exit(1);
    }

    if (!reader.Warning().empty()) { cout << "TinyObjReader: " << reader.Warning(); }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& mats = reader.GetMaterials();

    // material setup
    materials.resize(mats.size());
    vector<int> emissiveMatIds;
    for (size_t i = 0; i < mats.size(); i++) {
        const vec3 diffuse(mats[i].diffuse[0], mats[i].diffuse[1], mats[i].diffuse[2]);
        const vec3 specular(mats[i].specular[0], mats[i].specular[1], mats[i].specular[2]);
        const vec3 transmission(
            mats[i].transmittance[0], mats[i].transmittance[1], mats[i].transmittance[2]
        );

        auto newMat = std::make_shared<Material>(
            diffuse, specular, transmission, mats[i].shininess, mats[i].ior
        );

        newMat->emission.value =
            vec3(mats[i].emission[0], mats[i].emission[1], mats[i].emission[2]);
        if (glm::length(newMat->emission.value) > MIN_LIGHTING) {
            emissiveMatIds.push_back(static_cast<int>(i));
        }
        newMat->metallic.value = mats[i].metallic;
        // Prefer MTL PBR roughness when present, fallback to legacy Ns if roughness is absent.
        newMat->roughness.value =
            mats[i].roughness > 0.0f
                ? mats[i].roughness
                : glm::clamp(std::sqrt(2.0f / (mats[i].shininess + 2.0f)), 0.0f, 1.0f);
        newMat->sheen.value = mats[i].sheen;

        auto tryLoad = [&](const string& texName) -> shared_ptr<Texture> {
            if (texName.empty()) return nullptr;
            const string texturePath = fileDir + "/" + texName;
            auto texturePtr = std::make_shared<Texture>(texturePath, texName);
            if (!texturePtr->valid()) return nullptr;
            return texturePtr;
        };

        auto bindVecParam = [&](const string& texName, MaterialParam<vec3>& param) {
            auto tex = tryLoad(texName);
            if (tex) {
                param.useTexture = true;
                param.texture = tex;
            }
        };

        auto bindFloatParam = [&](const string& texName, MaterialParam<float>& param) {
            auto tex = tryLoad(texName);
            if (tex) {
                param.useTexture = true;
                param.texture = tex;
            }
        };

        // Core uber-shader params.
        bindVecParam(mats[i].diffuse_texname, newMat->baseColor);      // map_Kd
        bindVecParam(mats[i].specular_texname, newMat->specular);      // map_Ks
        bindVecParam(mats[i].emissive_texname, newMat->emission);      // map_Ke
        bindFloatParam(mats[i].metallic_texname, newMat->metallic);    // map_Pm
        bindFloatParam(mats[i].roughness_texname, newMat->roughness);  // map_Pr
        bindFloatParam(mats[i].sheen_texname, newMat->sheen);          // map_Ps

        // MTL has no standard transmittance texture; reuse alpha map as transmission mask.
        bindVecParam(mats[i].alpha_texname, newMat->transmission);  // map_d

        // Auxiliary maps (kept for future BRDF/normal mapping).
        newMat->ambientTex = tryLoad(mats[i].ambient_texname);                       // map_Ka
        newMat->specularHighlightTex = tryLoad(mats[i].specular_highlight_texname);  // map_Ns
        newMat->bumpTex = tryLoad(mats[i].bump_texname);                  // bump/map_Bump
        newMat->displacementTex = tryLoad(mats[i].displacement_texname);  // disp
        newMat->alphaTex = tryLoad(mats[i].alpha_texname);                // map_d
        newMat->reflectionTex = tryLoad(mats[i].reflection_texname);      // refl
        newMat->normalTex = tryLoad(mats[i].normal_texname);              // norm

        materials[i] = newMat;
    }

    vector<shared_ptr<Light>> lightByMatId(materials.size(), nullptr);
    if (!lights.empty() && !emissiveMatIds.empty()) {
        for (size_t i = 0; i < emissiveMatIds.size(); ++i) {
            const size_t lightIdx = std::min(i, lights.size() - 1);
            lightByMatId[emissiveMatIds[i]] = lights[lightIdx];
        }
    }

    // vertex
    for (int i = 0; i < shapes.size(); i++) {
        int mesh_vertex_offset = 0;
        int mesh_num = static_cast<int>(shapes[i].mesh.num_face_vertices.size());
        // check mesh
        for (int m = 0; m < mesh_num; m++) {
            // auto mesh = new Mesh();
            auto mesh = std::make_shared<Mesh>();
            int each_mesh_vertex_num = shapes[i].mesh.num_face_vertices[m];
            mesh->vertices.resize(each_mesh_vertex_num);
            tinyobj::index_t idx;
            // check the vertex of each face
            for (int v = 0; v < each_mesh_vertex_num; v++) {
                idx = shapes[i].mesh.indices[mesh_vertex_offset + v];

                // add vertex
                tinyobj::real_t vx = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2];
                mesh->vertices[v].pos = vec3(vx, vy, vz);

                // add normal
                if (idx.normal_index >= 0 &&
                    3 * static_cast<size_t>(idx.normal_index) + 2 < attrib.normals.size()) {
                    tinyobj::real_t nx =
                        attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 0];
                    tinyobj::real_t ny =
                        attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 1];
                    tinyobj::real_t nz =
                        attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 2];
                    mesh->vertices[v].normal = vec3(nx, ny, nz);
                } else {
                    mesh->vertices[v].normal = vec3(0.0f, 1.0f, 0.0f);
                }

                // add texture
                if (idx.texcoord_index >= 0 &&
                    2 * static_cast<size_t>(idx.texcoord_index) + 1 < attrib.texcoords.size()) {
                    tinyobj::real_t tx =
                        attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 0];
                    tinyobj::real_t ty =
                        attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 1];
                    mesh->vertices[v].uv = vec2(tx, ty);
                } else {
                    mesh->vertices[v].uv = vec2(0.0f);
                }
            }

            // bbox
            mesh->bboxPtr = std::make_shared<BBox>(mesh->vertices);

            // material
            const int matId = shapes[i].mesh.material_ids[m];
            if (matId >= 0 && matId < static_cast<int>(materials.size())) {
                mesh->material = materials[matId];

                shared_ptr<Light> assignedLight = lightByMatId[matId];
                if (assignedLight) {
                    mesh->light = assignedLight;
                    assignedLight->getMeshes().push_back(mesh);
                    assignedLight->addArea(mesh->calculateArea());
                }
            }

            meshes.push_back(mesh);
            mesh_vertex_offset += each_mesh_vertex_num;
        }
    }
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start
    );
    cout << "Model Vertices: " << attrib.vertices.size() << endl
         << "Model Faces: " << meshes.size() << endl
         << "Model Import Time: " << duration.count() << " seconds" << endl;
}

shared_ptr<Light> Model::randomSelectLight() const {
    vector<float> areaPreSum(lights.size());
    for (int i = 0; i < lights.size(); i++) {
        areaPreSum[i] = lights[i]->getArea();
        if (i > 0) areaPreSum[i] += areaPreSum[i - 1];
    }
    const float randomValue = genRandomFloat() * areaPreSum.back();
    int lightIdx = 0;
    while (lightIdx < lights.size() && randomValue > areaPreSum[lightIdx]) { lightIdx++; }
    return lights[lightIdx];
}

float Model::calculateLightsArea() const {
    float lightsArea = 0.f;
    for (const auto& light : lights) { lightsArea += light->getArea(); }
    return lightsArea;
}
