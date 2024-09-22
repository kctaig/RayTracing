#include "tri_model.hpp"
#include "tiny_obj_loader.h"
#include "head_include.hpp"

#define TINYOBJLOADER_IMPLEMENTATION

bool Tri_Model::make_mesh_by_obj(std::string file_name_dir,std::string file_name){

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = file_name_dir; // Path to material files
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(file_name_dir + "/" +  file_name, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        cout << "TinyObshapesjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // 添加模型的顶点
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        glm::vec3 v_pos = glm::vec3(attrib.vertices[i],attrib.vertices[i + 1],attrib.vertices[i + 2]);
        Vertex* v = new Vertex();
        v->pos = v_pos;
        model.vertices.push_back(*v);
    }

    // 将模型数据保存到Model中
    for (size_t i = 0; i < shapes.size(); i++) {
        size_t mesh_vertex_offset = 0; // 面片顶点偏移量
        size_t mesh_num = shapes[i].mesh.num_face_vertices.size() ; // 面片数量
        for (size_t m = 0; m < mesh_num; m++) {
            std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
            size_t each_mesh_vertex_num = shapes[i].mesh.num_face_vertices[m];
            mesh->indices.resize(each_mesh_vertex_num);
            tinyobj::index_t idx;
            // 查看每个面片的顶点
            for (size_t v = 0; v < each_mesh_vertex_num; v++) {
                idx = shapes[i].mesh.indices[mesh_vertex_offset + v];
                mesh->indices[v] = idx.vertex_index; // 面片的的顶点索引
                if (idx.normal_index >= 0 && idx.normal_index < model.vertices.size()) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    model.vertices[idx.normal_index].normal = glm::vec3(nx, ny, nz); // 添加法向量
                }
            }
            model.meshes.push_back(*mesh);
            mesh_vertex_offset += each_mesh_vertex_num;
        }
    }
    return true;
}

void Tri_Model::print_model_info(){

    // 调试信息
    cout << "# of vertices  : " << (model.vertices.size()) << endl;
    cout << "# of meshes   : " << (model.meshes.size()) << endl;

    // 打印顶点
    cout << "start print vertices: \n";
    for (size_t j = 0; j < model.vertices.size(); j++) {
        Vertex v = model.vertices[j];
        cout << v.pos[0] << " " << v.pos[1] << " " << v.pos[2] << endl;
    }

    // 打印mesh
    cout << "start print meshes: \n";
    for (size_t j = 0; j < model.meshes.size(); j++) {
        cout << "mesh " << j << " : ";
        cout << model.meshes[j].indices[0] << " ";
        cout << model.meshes[j].indices[1] << " ";
        cout << model.meshes[j].indices[2] << endl;
    }
}
