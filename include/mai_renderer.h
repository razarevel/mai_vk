#pragma once

#include "vk_buffer.h"
#include "vk_cmd.h"
#include "vk_context.h"
#include "vk_descriptor.h"
#include "vk_image.h"
#include "vk_pipeline.h"
#include "vk_render.h"
#include "vk_shader.h"
#include "vk_swapchain.h"
#include "vk_sync.h"
#include <functional>

namespace MAI {

struct TextureModule {
  int texWidth, texHeight, texComp;
  VkDeviceSize imageSize;
};

struct MAIRendererInfo {
  uint32_t width, height;
  const char *appName;
  float clearColor[4] = {0.25f, 0.25f, 0.25f, 1.0f};
  bool isFullScreen;
};

using DrawFrameFunc = std::function<void(
    uint32_t width, uint32_t height, float aspectRatio, float deltaSeconds)>;

struct MAIRenderer {
  GLFWwindow *window = nullptr;
  VKContext *vkContext;
  VKSwapchain *vkSwapchain;
  VKSync *vkSyncObj;
  VKCmd *vkCmd;
  VKRender *vkRender;
  VKTexture *depthTexture;
  MAIRendererInfo info_;
  VKPipeline *lastBindPipeline_ = nullptr;
  VKDescriptor *globalDescriptor = nullptr;

  MAIRenderer(MAIRendererInfo info);
  ~MAIRenderer();

  void run(DrawFrameFunc drawFrame);

  VKShader *createShader(const char *filename, VkShaderStageFlagBits stage);
  VKPipeline *createPipeline(PipelineInfo info);
  VKbuffer *createBuffer(BufferInfo info);
  VKDescriptor *createDescriptor(DescriptorSetInfo info);
  VKTexture *createTexture(TextureInfo info);

  void bindRenderPipeline(VKPipeline *pipeline);
  void bindVertexBuffer(uint32_t firstBinding, VKbuffer *buffer,
                        uint32_t offset = 0);
  void bindIndexBuffer(VKbuffer *buffer, VkDeviceSize offset,
                       VkIndexType indexType);
  void bindDescriptorSet(VKPipeline *pipeline,
                         const std::vector<VkDescriptorSet> &sets);

  void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1,
               uint32_t firstIndex = 0, uint32_t firstIntance = 0);

  void cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount = 1,
                    uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                    uint32_t firstInstance = 0);
  void updateBuffer(VKbuffer *buffer, void *data, size_t size);
  void updatePushConstant(uint32_t size, const void *value);

  void waitForDevice() { vkContext->waitForDevice(); }

private:
  uint32_t lastTextureCount = 0;
  GLFWwindow *initWindow();
  void createGlobalDescriptor();
};
}; // namespace MAI
