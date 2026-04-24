#include "model.hpp"

#include "bbox.hpp"
#include "head_include.hpp"
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
        payload.hitPos = ray.at(t);
        payload.uv = {u, v};
        // geometric normal for robust side tests (reflection/refraction/offset)
        vec3 gn = normalize(cross(E1, E2));
        payload.frontFace = dot(gn, D) < 0.f;
        payload.geomNormal = payload.frontFace ? gn : -gn;

        // shading normal for BRDF evaluation
        vec3 sn = normalize(getNormal({u, v}));
        payload.normal = dot(sn, payload.geomNormal) < 0.f ? -sn : sn;
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

vec3 Mesh::getNormal(const vec2& uv) const {
    return vertices[0].normal * (1.f - uv.x - uv.y) + vertices[1].normal * uv.x +
           vertices[2].normal * uv.y;
}

void Model::loadFromFile(const string& fileDir, const string& fileName) {
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

    materials.resize(mats.size());
    for (size_t i = 0; i < mats.size(); i++) {
        auto diffuse = vec3(mats[i].diffuse[0], mats[i].diffuse[1], mats[i].diffuse[2]);
        auto specular = vec3(mats[i].specular[0], mats[i].specular[1], mats[i].specular[2]);
        auto transmittance =
            vec3(mats[i].transmittance[0], mats[i].transmittance[1], mats[i].transmittance[2]);

        auto newMat = std::make_shared<Material>(
            mats[i].name, diffuse, specular, transmittance, mats[i].shininess, mats[i].ior
        );

        // texture
        if (!mats[i].diffuse_texname.empty()) {
            newMat->useTexture = true;
            string texturePath = fileDir + "/" + mats[i].diffuse_texname;
            auto texturePtr = std::make_shared<Texture>(texturePath, mats[i].diffuse_texname);
            newMat->texture = texturePtr;
        }

        // light material
        for (int j = 0; j < lights.size(); j++) {
            if (lights[j]->getMatName() == newMat->matName) {
                newMat->light = lights[j];
                break;
            }
        }
        materials[i] = newMat;
    }

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
                tinyobj::real_t nx = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 0];
                tinyobj::real_t ny = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 1];
                tinyobj::real_t nz = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 2];
                mesh->vertices[v].normal = vec3(nx, ny, nz);

                // add texture
                tinyobj::real_t tx =
                    attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 0];
                tinyobj::real_t ty =
                    attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 1];
                mesh->vertices[v].uv = vec2(tx, ty);
            }

            // bbox
            mesh->bboxPtr = std::make_shared<BBox>(mesh->vertices);

            // material
            int matId = shapes[i].mesh.material_ids[m];
            mesh->material = materials[matId];

            // light
            shared_ptr<Light> light = mesh->material->light;
            if (light) {
                light->getMeshes().push_back(mesh);
                light->addArea(mesh->calculateArea());
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