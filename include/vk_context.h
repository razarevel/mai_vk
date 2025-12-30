#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

namespace MAI {

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct QueueFamilyIndices {
  std::optional<uint32_t> graphcisFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() const {
    return graphcisFamily.has_value() && presentFamily.has_value();
  }
};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
};

struct VKContext {

  GLFWwindow *window;
  bool enableValidationLayers = true;
  bool frameRsized = false;
  VKContext(const char *appName, GLFWwindow *window);
  ~VKContext();

  VkInstance getInstance() const { return instance; }
  VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
  VkDevice getDevice() const { return device; }
  VkSurfaceKHR getSurface() const { return surface; }
  VkQueue getGraphicsQueue() const { return graphicsQueue; }
  VkQueue getPresentQueue() const { return presentQueue; }
  QueueFamilyIndices getFamilyIndices() const { return indices; }

  void waitForDevice() {
    if (vkDeviceWaitIdle(device) != VK_SUCCESS) {
      std::cerr << "failed to wait for device" << std::endl;
      assert(false);
    }
  }

private:
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSurfaceKHR surface;
  QueueFamilyIndices indices;

  VkDebugUtilsMessengerEXT debugMessenger;

  void windowCallbacks();
  void createInstance(const char *appName);
  void setupDebugMessenger();
  void createSurfaceKHR();
  void pickPhysicalDevice();
  void createLogicalDevice();
};

}; // namespace MAI
