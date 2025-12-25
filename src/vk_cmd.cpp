#include "mai_vk_backend/vk_cmd.h"
#include <iostream>

namespace MAI {
VKCmd::VKCmd(VKContext *vkContext) : vkContext(vkContext) {
  createCommandPool();
  createCommandBuffers();
}

void VKCmd::createCommandPool() {
  QueueFamilyIndices indices = vkContext->getFamilyIndices();
  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = indices.graphcisFamily.value(),
  };

  if (vkCreateCommandPool(vkContext->getDevice(), &poolInfo, nullptr,
                          &commandPool) != VK_SUCCESS)
    throw std::runtime_error("failed to create command pool!");
}

void VKCmd::createCommandBuffers() {
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
  };
  if (vkAllocateCommandBuffers(vkContext->getDevice(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate command buffer!");
}

VkCommandBuffer VKCmd::beginSingleCommandBuffer() {
  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(vkContext->getDevice(), &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void VKCmd::endSingleCommandBuffer(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
  };

  vkQueueSubmit(vkContext->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(vkContext->getGraphicsQueue());
  vkFreeCommandBuffers(vkContext->getDevice(), commandPool, 1, &commandBuffer);
}

VKCmd::~VKCmd() {
  vkDestroyCommandPool(vkContext->getDevice(), commandPool, nullptr);
}
}; // namespace MAI
