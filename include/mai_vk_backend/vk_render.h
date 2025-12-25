#pragma once

#include "mai_vk_backend/vk_cmd.h"
#include "mai_vk_backend/vk_context.h"
#include "mai_vk_backend/vk_swapchain.h"
#include "mai_vk_backend/vk_sync.h"
namespace MAI {
struct VKRender {
  VKRender(VKContext *vkContext, VKSync *vkSyncObj, VKSwapchain *vkSwapchain,
           VKCmd *vkCmd);

  void beginFrame();
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

private:
  VKContext *vkContext;
  VKSync *vkSync;
  VKSwapchain *vkSwapchain;
  VKCmd *vkCmd;

  uint32_t frameIndex = 0;
  uint32_t imageIndex;
  VkFence drawFences;

  void acquireSwapChainImageIndex();
};
}; // namespace MAI
