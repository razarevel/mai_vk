#pragma once

#include "vk_context.h"

namespace MAI {
struct VKCmd {
  VKCmd(VKContext *vkContext);
  ~VKCmd();

  VkCommandPool getCommandPool() const { return commandPool; }
  const std::vector<VkCommandBuffer> &getCommandBuffers() const {
    return commandBuffers;
  }

  VkCommandBuffer beginSingleCommandBuffer();
  void endSingleCommandBuffer(VkCommandBuffer commandBuffer);

private:
  VKContext *vkContext;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  void createCommandPool();
  void createCommandBuffers();
};
}; // namespace MAI
