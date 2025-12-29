#include "vk_swapchain.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>

namespace MAI {
VKSwapchain::VKSwapchain(VKContext *vkContext_) : vkContext(vkContext_) {
  createSwapChain();
  createSwapChainImageViews();
}

SwapChainSupportDetails querrySwapChainSupport(VkPhysicalDevice device,
                                               VkSurfaceKHR surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilites);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.surfaceformats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.surfaceformats.data());
  }
  uint32_t presentCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount,
                                            nullptr);
  if (presentCount != 0) {
    details.presentModes.resize(presentCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount,
                                              details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR
chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats)
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return availableFormat;
  return availableFormats[0];
}

VkPresentModeKHR chooseSwapChainPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &presentMode : availablePresentModes)
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return presentMode;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR &capabilities,
                                 GLFWwindow *window) {
  assert(window);
  if (capabilities.minImageExtent.width != std::numeric_limits<uint32_t>::max())
    return capabilities.minImageExtent;
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  VkExtent2D extent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height),
  };

  extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width);
  extent.height = std::clamp(extent.height, capabilities.minImageExtent.height,
                             capabilities.maxImageExtent.height);
  return extent;
}

void VKSwapchain::createSwapChain() {

  uint32_t formatsCount;
  SwapChainSupportDetails swapChainDetails = querrySwapChainSupport(
      vkContext->getPhysicalDevice(), vkContext->getSurface());

  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapChainFormat(swapChainDetails.surfaceformats);
  VkPresentModeKHR prsentMode =
      chooseSwapChainPresentMode(swapChainDetails.presentModes);
  VkExtent2D extents =
      chooseSwapChainExtent(swapChainDetails.capabilites, vkContext->window);

  uint32_t imageCount = swapChainDetails.capabilites.minImageCount + 1;

  if (swapChainDetails.capabilites.maxImageCount > 0 &&
      imageCount > swapChainDetails.capabilites.maxImageCount)
    imageCount = swapChainDetails.capabilites.maxImageCount;

  VkSwapchainCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = vkContext->getSurface(),
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extents,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = swapChainDetails.capabilites.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = prsentMode,
      .clipped = true,
      .oldSwapchain = nullptr,
  };

  if (vkCreateSwapchainKHR(vkContext->getDevice(), &createInfo, nullptr,
                           &swapchain) != VK_SUCCESS)
    throw std::runtime_error("failed to create swap chain");

  vkGetSwapchainImagesKHR(vkContext->getDevice(), swapchain, &imageCount,
                          nullptr);
  swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(vkContext->getDevice(), swapchain, &imageCount,
                          swapchainImages.data());

  swapchainImageFormat = surfaceFormat.format;
  swapchainExtent = extents;
}

void VKSwapchain::createSwapChainImageViews() {

  swapchainImageViews.resize(swapchainImages.size());

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    VkImageViewCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapchainImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchainImageFormat,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    if (vkCreateImageView(vkContext->getDevice(), &createInfo, nullptr,
                          &swapchainImageViews[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to create swap image view");
  }
}

void VKSwapchain::recreateSwapChain() {

  vkDeviceWaitIdle(vkContext->getDevice());

  cleanupSwapchain();

  createSwapChain();
  createSwapChainImageViews();
}

void VKSwapchain::cleanupSwapchain() {

  for (size_t i = 0; i < swapchainImages.size(); i++)
    vkDestroyImageView(vkContext->getDevice(), swapchainImageViews[i], nullptr);

  vkDestroySwapchainKHR(vkContext->getDevice(), swapchain, nullptr);
}

VKSwapchain::~VKSwapchain() { cleanupSwapchain(); }
}; // namespace MAI
