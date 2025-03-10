#include "head_include.hpp"
#include "model.hpp"
#include "bvh.hpp"
#include <chrono>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader/tiny_obj_loader.h"

bool Mesh::intersection(const Ray& ray, PayLoad& payload) const {
	bool isHit = false;
	vec3 v0 = vertices[0].pos;
	vec3 v1 = vertices[1].pos;
	vec3 v2 = vertices[2].pos;
	vec3 E1 = v1 - v0;
	vec3 E2 = v2 - v0;
	vec3 N = cross(E1, E2);
	// 检查光线是否与三角形平行
	if (dot(N, -ray.getDir()) < EPSILON) return isHit;
	vec3 T = ray.getOrigin() - v0;
	vec3 D = normalize(ray.getDir());
	vec3 P = cross(D, E2);
	vec3 Q = cross(T, E1);
	float p_e1 = dot(P, E1);
	if (fabs(p_e1) < EPSILON) return isHit;
	float t = dot(Q, E2) / p_e1;
	float u = dot(P, T) / p_e1;
	float v = dot(Q, D) / p_e1;

	if (t >= 0.f && t < payload.t &&  u >= 0 && v >= 0 && 1 - u - v >= 0)
	{
		isHit = true;
		payload.t = t;
		payload.hitPos = ray.at(t);
		payload.uv = { u, v };
		vec3 normal =
			vertices[0].normal * (1.f - u - v) +
			vertices[1].normal * u +
			vertices[2].normal * v;
		payload.normal = normalize(normal);
		payload.matPtr = matPtr;
	}
	return isHit;
}

void Model::loadFromFile(const string fileDir, const string fileName)
{
	auto start = std::chrono::high_resolution_clock::now();
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = fileDir; // Path to material files
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(fileDir + "/" + fileName + ".obj", reader_config))
	{
		if (!reader.Error().empty())
		{
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty())
	{
		cout << "TinyObshapesjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	matPtrs.resize(materials.size());
	for (size_t i = 0; i < materials.size(); i++)
	{
		vec3 diffuse = vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		vec3 specular = vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
		vec3 transmittance = vec3(materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);

		shared_ptr<Material> newMat = std::make_shared<Material>(materials[i].name,
			diffuse,
			specular,
			transmittance,
			materials[i].shininess,
			materials[i].ior);

		// light material
		for (int j = 0; j < lightPtrs.size(); j++)
		{
			if (lightPtrs[j]->getMatName() == newMat->matName)
			{
				newMat->lightPtr = lightPtrs[j];
				break;
			}
		}
		matPtrs[i] = newMat;
	}

	for (int i = 0; i < shapes.size(); i++)
	{
		int mesh_vertex_offset = 0;								// 面片顶点偏移量
		int mesh_num = static_cast<int>(shapes[i].mesh.num_face_vertices.size()); // 面片数量
		for (int m = 0; m < mesh_num; m++)
		{
			std::shared_ptr<Mesh> meshPtr = std::make_shared<Mesh>();
			int each_mesh_vertex_num = shapes[i].mesh.num_face_vertices[m];
			meshPtr->vertices.resize(each_mesh_vertex_num);
			tinyobj::index_t idx;
			// 查看每个面片的顶点
			for (int v = 0; v < each_mesh_vertex_num; v++)
			{
				idx = shapes[i].mesh.indices[mesh_vertex_offset + v];

				// add vertex
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				meshPtr->vertices[v].pos = glm::vec3(vx, vy, vz);

				// add normal
				tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
				tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
				tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
				meshPtr->vertices[v].normal = glm::vec3(nx, ny, nz);

				// todo: add texture
			}
			// bbox
			meshPtr->bboxPtr = std::make_shared<BBox>(meshPtr->vertices);
			// material
			int matId = shapes[i].mesh.material_ids[m];
			meshPtr->matPtr = matPtrs[matId];

			//提取光源
			shared_ptr<Light>lightPtr = meshPtr->matPtr->lightPtr;
			if (lightPtr != nullptr)
			{
				lightPtr->getMeshPtrs().push_back(meshPtr);
				// 计算光源的面积
				vec3 v0 = meshPtr->vertices[0].pos;
				vec3 v1 = meshPtr->vertices[1].pos;
				vec3 v2 = meshPtr->vertices[2].pos;
				vec3 E1 = v1 - v0;
				vec3 E2 = v2 - v0;
				lightPtr->addArea(length(cross(E1, E2)) / 2.0f);
			}

			meshPtrs.push_back(meshPtr);
			mesh_vertex_offset += each_mesh_vertex_num;
		}
	}
	auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start);
	cout << "Vertices: " << attrib.vertices.size() << endl;
	cout << "Faces: " << meshPtrs.size() << endl;
	cout << "Model Build Time: " << duration.count() << " seconds" << endl;
}