#include "mai_vk_backend/vk_sync.h"
#include <iostream>

namespace MAI {

VKSync::VKSync(VKContext *vkContext_) : vkContext(vkContext_) {
  createSyncObjects();
}

void VKSync::createSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  drawFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    if (vkCreateSemaphore(vkContext->getDevice(), &semaphoreInfo, nullptr,
                          &imageAvailableSemaphores[i]) ||
        vkCreateSemaphore(vkContext->getDevice(), &semaphoreInfo, nullptr,
                          &renderFinishSemaphores[i]) ||
        vkCreateFence(vkContext->getDevice(), &fenceInfo, nullptr,
                      &drawFences[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to create sync objects");
}

VKSync::~VKSync() {

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(vkContext->getDevice(), imageAvailableSemaphores[i],
                       nullptr);
    vkDestroySemaphore(vkContext->getDevice(), renderFinishSemaphores[i],
                       nullptr);
    vkDestroyFence(vkContext->getDevice(), drawFences[i], nullptr);
  }
}
}; // namespace MAI
