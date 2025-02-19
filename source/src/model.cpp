#include "head_include.hpp"
#include "model.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader/tiny_obj_loader.h"

Model::Model(const std::string fileDir, const std::string fileName)
{
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

	auto &attrib = reader.GetAttrib();
	auto &shapes = reader.GetShapes();
	auto &materials = reader.GetMaterials();

	// 添加模型的顶点
	for (size_t i = 0; i < attrib.vertices.size(); i += 3)
	{
		glm::vec3 v_pos = glm::vec3(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
		Vertex *v = new Vertex();
		v->pos = v_pos;
		this->vertices.push_back(*v);
	}

	mats.resize(materials.size() + 1);
	for (size_t i = 0; i < materials.size(); i++)
	{
		vec3 diffuse = vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		vec3 specular = vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
		vec3 transmittance = vec3(materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);

		Material mat(materials[i].name,
					 diffuse,
					 diffuse,
					 transmittance,
					 materials[i].shininess,
					 materials[i].ior);

		mats[i + 1] = mat;
	}

	// 将模型数据保存到Model中
	for (int i = 0; i < shapes.size(); i++)
	{
		int mesh_vertex_offset = 0;								// 面片顶点偏移量
		int mesh_num = shapes[i].mesh.num_face_vertices.size(); // 面片数量
		for (int m = 0; m < mesh_num; m++)
		{
			std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
			int each_mesh_vertex_num = shapes[i].mesh.num_face_vertices[m];
			mesh->indices.resize(each_mesh_vertex_num);
			tinyobj::index_t idx;
			// 查看每个面片的顶点
			for (int v = 0; v < each_mesh_vertex_num; v++)
			{
				idx = shapes[i].mesh.indices[mesh_vertex_offset + v];
				mesh->indices[v] = idx.vertex_index; // 面片的的顶点索引
				if (idx.normal_index >= 0 && idx.normal_index < this->vertices.size())
				{
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					this->vertices[idx.normal_index].normal = glm::vec3(nx, ny, nz); // 添加法向量
				}
			}

			// 每个面的材质id
			mesh->mat_id = shapes[i].mesh.material_ids[m];
			this->triangles.push_back(*mesh);
			mesh_vertex_offset += each_mesh_vertex_num;
		}
	}
}

void Model::modelInfo()
{
	// 调试信息
	cout << "# of vertices  : " << (this->vertices.size()) << endl;
	cout << "# of meshes   : " << (this->triangles.size()) << endl;

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