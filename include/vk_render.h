#pragma once

#include "vk_cmd.h"
#include "vk_context.h"
#include "vk_image.h"
#include "vk_swapchain.h"
#include "vk_sync.h"
namespace MAI {

struct DepthInfo {
  VkCompareOp compareOp;
  bool depthWriteEnable = false;
};

struct VKRender {
  VKRender(VKContext *vkContext, VKSync *vkSyncObj, VKSwapchain *vkSwapchain,
           VKCmd *vkCmd, VKTexture *texture);
  ~VKRender();

  void beginFrame(float clearValue[4]);
  void endFrame();
  void submitFrame();
  uint32_t getFrameIndex() const { return frameIndex; }

  void bindPipline(VkPipelineBindPoint bindPoint, VkPipeline pipeline);
  void
  cmdBindDescriptorSets(VkPipelineBindPoint bindPoint,
                        VkPipelineLayout piplineLayout,
                        const std::vector<VkDescriptorSet> &descriptorSets);
  void cmdDraw(uint32_t vertexCount, uint32_t instanceCount,
               uint32_t firstVertex, uint32_t firstInstance);
  void cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount,
                    uint32_t firstIndex, int32_t vertexOffset,
                    uint32_t firstInstance);

  void cmdBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                            const VkBuffer *pBuffers,
                            const VkDeviceSize *offsets);
  void cmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset,
                          VkIndexType indexType);
  void cmdPushConstants(VkPipelineLayout pipelineLayout,
                        VkShaderStageFlags shaderStage, uint32_t offset,
                        uint32_t size, const void *value);
  void cmdBindDepthState(DepthInfo info);

private:
  VKContext *vkContext;
  VKSync *vkSync;
  VKSwapchain *vkSwapchain;
  VKCmd *vkCmd;
  VKTexture *depthTexture;

  uint32_t frameIndex = 0;
  uint32_t imageIndex;
  VkFence drawFences;

  void acquireSwapChainImageIndex();
};
}; // namespace MAI
