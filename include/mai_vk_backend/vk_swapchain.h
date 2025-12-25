#pragma once

#include "mai_vk_backend/vk_context.h"
#include <vector>

namespace MAI {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilites;
  std::vector<VkSurfaceFormatKHR> surfaceformats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct VKSwapchain {

  VKSwapchain(VKContext *vkContext_);
  ~VKSwapchain();

  VkSwapchainKHR getSwapchain() const { return swapchain; }
  VkFormat getSwapchainImageFormat() const { return swapchainImageFormat; }
  VkExtent2D getSwapchainExtent() const { return swapchainExtent; }
  const std::vector<VkImage> &getswapchainImages() const {
    return swapchainImages;
  }
  const std::vector<VkImageView> &getswapchainImageViews() const {
    return swapchainImageViews;
  }

  void recreateSwapChain();

private:
  VKContext *vkContext;
  VkSwapchainKHR swapchain;
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;

  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  void createSwapChain();

  void createSwapChainImageViews();
  void cleanupSwapchain();
};
}; // namespace MAI
