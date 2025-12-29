#pragma once

#include "vk_context.h"

namespace MAI {
struct VKSync {

  std::vector<VkSemaphore> getImageAvailableSemaphores() const {
    return imageAvailableSemaphores;
  }
  std::vector<VkSemaphore> getRenderFinishSemaphores() const {
    return renderFinishSemaphores;
  }
  std::vector<VkFence> getDrawFences() const { return drawFences; }

  VKSync(VKContext *vkContext_);
  ~VKSync();

private:
  VKContext *vkContext;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishSemaphores;
  std::vector<VkFence> drawFences;

  void createSyncObjects();
};
}; // namespace MAI
