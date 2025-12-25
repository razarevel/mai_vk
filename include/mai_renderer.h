#pragma once

#include "mai_vk_backend/vk_buffer.h"
#include "mai_vk_backend/vk_cmd.h"
#include "mai_vk_backend/vk_context.h"
#include "mai_vk_backend/vk_descriptor.h"
#include "mai_vk_backend/vk_pipeline.h"
#include "mai_vk_backend/vk_render.h"
#include "mai_vk_backend/vk_shader.h"
#include "mai_vk_backend/vk_swapchain.h"
#include "mai_vk_backend/vk_sync.h"
#include <functional>

namespace MAI {

struct TextureModule {
  int texWidth, texHeight, texComp;
  VkDeviceSize imageSize;
};

using DrawFrameFunc = std::function<void(
    uint32_t width, uint32_t height, float aspectRatio, float deltaSeconds)>;

struct MAIRenderer {
  GLFWwindow *window = nullptr;
  VKContext *vkContext;
  VKSwapchain *vkSwapchain;
  VKSync *vkSyncObj;
  VKCmd *vkCmd;
  VKDescriptor *vkDescriptor;
  VKRender *vkRender;

  MAIRenderer(uint32_t width, uint32_t height, const char *appName);
  ~MAIRenderer();

  void run(DrawFrameFunc drawFrame);

  VKShader *createShader(const char *filename, VkShaderStageFlagBits stage);
  VKPipeline *createPipeline(PipelineInfo info);
  VKbuffer *createBuffer(BufferInfo info);
  VkDescriptorSetLayout createDescriptorSetLayout(
      std::vector<VkDescriptorSetLayoutBinding> uboLayouts);

  std::vector<VkDescriptorSet>
  createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
                       const std::vector<VkBuffer> &uniformBuffers,
                       size_t bufferSize);

  void bindRenderPipeline(VKPipeline *pipeline);
  void bindVertexBuffer(uint32_t firstBinding, VKbuffer *buffer,
                        uint32_t offset = 0);
  void bindIndexBuffer(VKbuffer *buffer, VkDeviceSize offset,
                       VkIndexType indexType);
  void bindDescriptorSet(VKPipeline *pipeline,
                         const std::vector<VkDescriptorSet> &sets);

  void cmdDraw(uint32_t vertexCount, uint32_t instanceCount,
               uint32_t firstIndex = 0, uint32_t firstIntance = 0);

  void cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount,
                    uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                    uint32_t firstInstance = 0);
  void updateBuffer(VKbuffer *buffer, void *data, size_t size);

  void waitForDevice() { vkContext->waitForDevice(); }

  void destroyDescriptorSetLayout(VkDescriptorSetLayout layout);

  TextureModule createTexture(const char *filename);

private:
  GLFWwindow *initWindow(uint32_t width, uint32_t height, const char *appName);
};
}; // namespace MAI
