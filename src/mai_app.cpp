#include "mai_app.h"
#include "mai_vk_backend/vk_pipeline.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
  glm::vec3 pos;
  glm::vec2 texCoords;
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} ubo;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

void MaiApp::run() {
  init();
  mainLoop();
};

void MaiApp::init() {
  uint32_t width = 1200, height = 800;
  const char *appName = "Mai Demo";
  renderer = new MAI::MAIRenderer(width, height, appName);

  const aiScene *scene = aiImportFile(RESOURCES_PATH "rubber_duck/scene.gltf",
                                      aiProcess_Triangulate);
  assert(scene);

  const aiMesh *mesh = scene->mMeshes[0];
  vertices.reserve(mesh->mNumVertices);
  indices.reserve(3 * mesh->mNumFaces);
  for (size_t i = 0; i < mesh->mNumVertices; i++) {
    const aiVector3D p = mesh->mVertices[i];
    const aiVector3D t =
        mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f);
    vertices.push_back({
        .pos = glm::vec3(p.x, p.y, p.z),
        .texCoords = glm::vec2(t.x, t.y),
    });
  }
  for (size_t i = 0; i < mesh->mNumFaces; i++)
    for (size_t j = 0; j != 3; j++)
      indices.emplace_back(mesh->mFaces[i].mIndices[j]);

  vertexBuffer = renderer->createBuffer({
      .size = sizeof(vertices[0]) * vertices.size(),
      .data = vertices.data(),
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  });

  indexBuffer = renderer->createBuffer({
      .size = sizeof(indices[0]) * indices.size(),
      .data = indices.data(),
      .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  });

  vert = renderer->createShader(SHADERS_PATH "spvs/main.vspv",
                                VK_SHADER_STAGE_VERTEX_BIT);

  frag = renderer->createShader(SHADERS_PATH "spvs/frag.fspv",
                                VK_SHADER_STAGE_FRAGMENT_BIT);

  const MAI::VertextInput vertInput = {
      .attributes =
          {
              {
                  .binding = 0,
                  .location = 0,
                  .format = VK_FORMAT_R32G32B32_SFLOAT,
                  .offset = 0,
              },
              {
                  .binding = 0,
                  .location = 1,
                  .format = VK_FORMAT_R32G32_SFLOAT,
                  .offset = offsetof(Vertex, texCoords),
              },
          },
      .inputBinding =
          {
              .binding = 0,
              .stride = sizeof(Vertex),
              .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
          },
  };

  pipeline = renderer->createPipeline({
      .vert = vert,
      .frag = frag,
      .vertInput = vertInput,
      .pushConstants =
          {
              .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
              .offset = 0,
              .size = sizeof(UniformBufferObject),
          },
  });
}

void MaiApp::mainLoop() {
  renderer->run([&](uint32_t width, uint32_t height, float aspectRatio,
                    float deltaSeconds) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view =
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    renderer->bindRenderPipeline(pipeline);
    renderer->bindVertexBuffer(0, vertexBuffer);
    renderer->bindIndexBuffer(indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    renderer->updatePushConstant(pipeline, sizeof(ubo), &ubo);
    renderer->cmdDrawIndex(indices.size(), 1, 0);
  });
}

MaiApp::~MaiApp() {
  renderer->waitForDevice();
  delete indexBuffer;
  delete vertexBuffer;
  delete pipeline;
  delete frag;
  delete vert;
  delete renderer;
}
