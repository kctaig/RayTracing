#include "head_include.hpp"
#include "model.hpp"
#include "bvh.hpp"
#include <chrono>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader/tiny_obj_loader.h"

PayLoad Mesh::intersection(const Ray& ray, const shared_ptr<Model>modelPtr) const {
	PayLoad payload{};
	vec3 v0 = modelPtr->vertices[indices[0]].pos;
	vec3 v1 = modelPtr->vertices[indices[1]].pos;
	vec3 v2 = modelPtr->vertices[indices[2]].pos;
	vec3 E1 = v1 - v0;
	vec3 E2 = v2 - v0;
	vec3 T = ray.getOrigin() - v0;
	vec3 D = normalize(ray.getDir());
	vec3 P = cross(D, E2);
	vec3 Q = cross(T, E1);
	float p_e1 = dot(P, E1);
	float t = dot(Q, E2) / p_e1;
	float u = dot(P, T) / p_e1;
	float v = dot(Q, D) / p_e1;

	if (t >= 0.f && u >= 0 && v >= 0 && 1 - u - v >= 0)
	{
		payload.t = t;
		payload.hitPos = ray.at(t);
		payload.uv = { u, v };
		vec3 normal = (
			modelPtr->vertices[indices[0]].normal +
			modelPtr->vertices[indices[1]].normal +
			modelPtr->vertices[indices[2]].normal) / 3.0f;
		payload.normal = normalize(normal);
		payload.matId = matId;
	}
	return payload;
}

Model::Model(const std::string fileDir, const std::string fileName, vector<Light>& lights)
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

	// 添加模型的顶点
	for (size_t i = 0; i < attrib.vertices.size(); i += 3)
	{
		glm::vec3 v_pos = glm::vec3(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
		vertices.push_back(Vertex(v_pos));
	}

	// 添加材质
	modelMats.resize(materials.size());
	for (size_t i = 0; i < materials.size(); i++)
	{
		vec3 diffuse = vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		vec3 specular = vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
		vec3 transmittance = vec3(materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);

		Material newMat = Material(materials[i].name,
			diffuse,
			diffuse,
			transmittance,
			materials[i].shininess,
			materials[i].ior);

		// 光源材质id
		for (int j = 0; j < lights.size(); j++)
		{
			if (lights[j].matName == newMat.matName)
			{
				newMat.lightId = j;
				break;
			}
		}
		modelMats[i] = newMat;
	}

	// 将模型数据保存到Model中
	for (int i = 0; i < shapes.size(); i++)
	{
		int mesh_vertex_offset = 0;								// 面片顶点偏移量
		int mesh_num = shapes[i].mesh.num_face_vertices.size(); // 面片数量
		for (int m = 0; m < mesh_num; m++)
		{
			std::shared_ptr<Mesh> meshPtr = std::make_shared<Mesh>();
			int each_mesh_vertex_num = shapes[i].mesh.num_face_vertices[m];
			meshPtr->indices.resize(each_mesh_vertex_num);
			tinyobj::index_t idx;
			// 查看每个面片的顶点
			for (int v = 0; v < each_mesh_vertex_num; v++)
			{
				idx = shapes[i].mesh.indices[mesh_vertex_offset + v];
				meshPtr->indices[v] = idx.vertex_index; // 面片的的顶点索引
				if (idx.normal_index >= 0 && idx.normal_index < this->vertices.size())
				{
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					vertices[idx.normal_index].normal = glm::vec3(nx, ny, nz); // 添加法向量
				}

				// todo: 添加纹理坐标
			}
			// 每个面的包围盒
			meshPtr->bboxPtr = std::make_shared<BBox>(vertices, meshPtr->indices);

			// 每个面的材质id
			meshPtr->matId = shapes[i].mesh.material_ids[m];
			// 提取光源
			int lightId = modelMats[meshPtr->matId].lightId;
			if (lightId >= 0)
			{
				lights[lightId].meshPtrs.push_back(meshPtr);
			}
			meshPtrs.push_back(meshPtr);
			mesh_vertex_offset += each_mesh_vertex_num;
		}
	}
	auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start);
	cout << "Model Build Time: " << duration.count() << " seconds" << endl;
	modelInfo();
}

void Model::modelInfo()
{
	// 调试信息
	cout << "Number of Vertices  : " << vertices.size() << endl;
	cout << "Number of Meshes   : " << meshPtrs.size() << endl;

	//// 打印顶点
	// cout << "start print vertices: \n";
	// for (size_t j = 0; j < this->vertices.size(); j++)
	//{
	//     Vertex v = this->vertices[j];
	//     cout << v.pos[0] << " " << v.pos[1] << " " << v.pos[2] << endl;
	// }

	// 打印mesh
	/*cout << "start print meshes: \n";
	for (size_t j = 0; j < this->triangles.size(); j++)
	{
		cout << "mesh " << j << " : ";
		cout << this->triangles[j].indices[0] << " ";
		cout << this->triangles[j].indices[1] << " ";
		cout << this->triangles[j].indices[2] << endl;
	}*/
}