#include "runtime/functions/assets/mesh.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>

#include "runtime/core/base/logger.h"

namespace peanut
{
struct LogStream : public Assimp::LogStream
{
    static void initialize()
    {
      if (Assimp::DefaultLogger::isNullLogger())
      {
        Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
        Assimp::DefaultLogger::get()->attachStream(
            new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
      }
    }

    void write(const char* message) override
    {
      PEANUT_LOG_ERROR("Assimp Error: {0}", message);
    }
};

Mesh::Mesh(const aiMesh* mesh)
{
    assert(mesh->HasPositions());
    assert(mesh->HasNormals());

    vertices_.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < vertices_.capacity(); ++i)
      {
      Vertex vertex;
      vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                         mesh->mVertices[i].z};
      vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                       mesh->mNormals[i].z};
      if (mesh->HasTangentsAndBitangents()) {
        vertex.tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y,
                          mesh->mTangents[i].z};
      }
      if (mesh->HasTextureCoords(0)) {
        vertex.texcoord = {mesh->mTextureCoords[0][i].x,
                           mesh->mTextureCoords[0][i].y};
      }
      vertices_.push_back(vertex);
    }

    indices_.reserve(mesh->mNumFaces);
    for (size_t i = 0; i < indices_.capacity(); ++i) {
      assert(mesh->mFaces[i].mNumIndices == 3);
      indices_.push_back({mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1],
                        mesh->mFaces[i].mIndices[2]});
    }
}

std::shared_ptr<Mesh> Mesh::ReadFromFile(const std::string& filename) 
{
    LogStream::initialize();

    PEANUT_LOG_INFO("Loading mesh from file: {0}", filename.c_str());

    std::shared_ptr<Mesh> mesh;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename.c_str(), kImportFlags);
    if (scene && scene->HasMeshes()) 
    {
        mesh = std::make_shared<Mesh>(scene->mMeshes[0]);
    } else 
    {
        PEANUT_LOG_ERROR("File {0} not include mesh", filename.c_str());
    }

    return mesh;
}

std::shared_ptr<Mesh> Mesh::ReadFromString(const std::string& data) {
  LogStream::initialize();

  PEANUT_LOG_INFO("Loading mesh from memory");

  std::shared_ptr<Mesh> mesh;
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFileFromMemory(
      data.c_str(), data.length(), kImportFlags, "nff");
  if (scene && scene->HasMeshes()) {
    mesh = std::make_shared<Mesh>(scene->mMeshes[0]);
  } else {
    PEANUT_LOG_ERROR("Memory data not include mesh");
  }

  return mesh;
}
}  // namespace peanut