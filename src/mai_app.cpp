#include "mai_app.h"
#include "mai_vk_backend/vk_pipeline.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 texCoords;
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} ubo;

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
};
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
};

void MaiApp::run() {
  init();
  mainLoop();
};

void MaiApp::init() {
  uint32_t width = 1200, height = 800;
  const char *appName = "Mai Demo";

  MAI::TextureModule image =
      renderer->createTexture(RESOURCES_PATH "statue-1275469_1280.jpg");

  renderer = new MAI::MAIRenderer(width, height, appName);
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
                  .format = VK_FORMAT_R32G32_SFLOAT,
                  .offset = 0,
              },
              {
                  .binding = 0,
                  .location = 1,
                  .format = VK_FORMAT_R32G32B32_SFLOAT,
                  .offset = offsetof(Vertex, color),
              },
              {
                  .binding = 0,
                  .location = 2,
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

  VkDescriptorSetLayout descriptorSet = renderer->createDescriptorSetLayout({
      {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      },
      {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
  });

  uniformBuffer = renderer->createBuffer({
      .size = sizeof(ubo),
      .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  });

  sets = renderer->createDescriptorSets(
      descriptorSet, uniformBuffer->getUniformBuffers(), sizeof(ubo));

  pipeline = renderer->createPipeline({
      .vert = vert,
      .frag = frag,
      .descriptorSetLayout = descriptorSet,
      .vertInput = vertInput,
  });

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

  renderer->destroyDescriptorSetLayout(descriptorSet);
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

    renderer->updateBuffer(uniformBuffer, &ubo, sizeof(ubo));

    renderer->bindRenderPipeline(pipeline);
    renderer->bindVertexBuffer(0, vertexBuffer);
    renderer->bindIndexBuffer(indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    renderer->bindDescriptorSet(pipeline, sets);
    renderer->cmdDrawIndex(indices.size(), 1, 0);
  });
}

MaiApp::~MaiApp() {
  renderer->waitForDevice();
  delete uniformBuffer;
  delete indexBuffer;
  delete vertexBuffer;
  delete pipeline;
  delete frag;
  delete vert;
  delete renderer;
}
