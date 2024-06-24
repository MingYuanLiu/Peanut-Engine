#pragma once
#include <assimp/mesh.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include <assimp/postprocess.h>

namespace peanut
{
    class Mesh 
    {
    public:
        // vertex
        struct Vertex 
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec2 texcoord;
        };
        // index
        struct Index 
        {
            uint32_t v1, v2, v3;
        };

    public:
        Mesh() {};
        explicit Mesh(const aiMesh* mesh);
        ~Mesh() {}

        Mesh(const Mesh& mesh) = default;

        static std::shared_ptr<Mesh> ReadFromFile(const std::string& filename);
        static std::shared_ptr<Mesh> ReadFromString(const std::string& data);

        const std::vector<Vertex>& vertices() const { return vertices_; }
        const std::vector<Index>& indexes() const { return indices_; }

    private:
        std::vector<Vertex> vertices_;
        std::vector<Index> indices_;

        static constexpr uint32_t kImportFlags =
            aiProcess_CalcTangentSpace | aiProcess_Triangulate |
            aiProcess_SortByPType | aiProcess_PreTransformVertices |
            aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
            aiProcess_Debone | aiProcess_ValidateDataStructure;
};
}  // namespace peanut