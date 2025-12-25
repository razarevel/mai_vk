#include "mai_vk_backend/vk_context.h"
#include <iostream>
#include <set>
#include <stdexcept>

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  auto app =
      reinterpret_cast<MAI::VKContext *>(glfwGetWindowUserPointer(window));
  app->frameRsized = true;
}

namespace MAI {

VKContext::VKContext(const char *appName, GLFWwindow *window) : window(window) {

  windowCallbacks();
  createInstance(appName);
  setupDebugMessenger();
  createSurfaceKHR();
  pickPhysicalDevice();
  createLogicalDevice();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT type,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallback, void *) {
  std::cerr << "validation layer: " << pCallback->pMessage << std::endl;
  return VK_FALSE;
}

void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debugCallback,
  };
}

bool checkValiadationLayers() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;
    for (const auto &layerProperties : availableLayers)
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    if (!layerFound)
      return false;
  }

  return true;
}

std::vector<const char *>
getRequriedExtensiosn(bool enableValidationLayers = false) {

  uint32_t glfwExtensionsCount = 0;
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionsCount);
  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
  QueueFamilyIndices indices;
  uint32_t queueFamiliesCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount,
                                           nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilyProperties(
      queueFamiliesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount,
                                           queueFamilyProperties.data());
  for (uint32_t i = 0; i < queueFamiliesCount; i++) {
    if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphcisFamily = i;

    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);

    if (supported)
      indices.presentFamily = i;

    if (indices.isComplete())
      break;
  }

  return indices;
}

void VKContext::windowCallbacks() {
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetKeyCallback(
      window, [](GLFWwindow *window, int keys, int, int action, int mod) {
        if (keys == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
          glfwSetWindowShouldClose(window, GLFW_TRUE);
      });
}

void VKContext::createInstance(const char *appName) {
  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = appName,
      .applicationVersion = VK_API_VERSION_1_0,
      .pEngineName = appName,
      .engineVersion = VK_API_VERSION_1_0,
      .apiVersion = VK_API_VERSION_1_3,
  };
  if (enableValidationLayers && !checkValiadationLayers())
    throw std::runtime_error("validation layer requestion but not available!");

  auto extensions = getRequriedExtensiosn(enableValidationLayers);
  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

  VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessenger(debugInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    throw std::runtime_error("failed to create instance");
}

VkResult
CreateDebugUtilsMessengerEXT(VkInstance instance,
                             VkDebugUtilsMessengerCreateInfoEXT *createInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
    return func(instance, createInfo, nullptr, pDebugMessenger);
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance, messenger, nullptr);
}

void VKContext::setupDebugMessenger() {

  if (!enableValidationLayers)
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  populateDebugMessenger(createInfo);
  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                   &debugMessenger) != VK_SUCCESS)
    throw std::runtime_error("failed to debug Utils Messenger");
}

void VKContext::createSurfaceKHR() {
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create window surface");
}

bool isDeviceSuitable(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       extensions.data());

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

  bool isSuitable = properties.apiVersion >= VK_API_VERSION_1_3;

  std::set<const char *> requiredExtensions(deviceExtensions.begin(),
                                            deviceExtensions.end());

  for (uint32_t i = 0; i < extensionCount; i++)
    for (const char *extension : deviceExtensions)
      if (strcmp(extension, extensions[i].extensionName) == 0)
        requiredExtensions.erase(extension);

  isSuitable = requiredExtensions.empty();

  return isSuitable;
}

void VKContext::pickPhysicalDevice() {

  uint32_t deviceCount;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0)
    throw std::runtime_error("faield to find a GPU with vulkan support");

  std::vector<VkPhysicalDevice> devices;
  devices.resize(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (uint32_t i = 0; i < deviceCount; i++)
    if (isDeviceSuitable(devices[i])) {
      physicalDevice = devices[i];
      break;
    }

  if (physicalDevice == VK_NULL_HANDLE)
    throw std::runtime_error("failed to find suitable GPU!");
}

void VKContext::createLogicalDevice() {

  indices = findQueueFamilies(physicalDevice, surface);

  std::set<uint32_t> uniqueQueueFamilies = {indices.graphcisFamily.value(),
                                            indices.presentFamily.value()};

  std::vector<VkDeviceQueueCreateInfo> deviceQueueInfos;
  float queuePriority = 0.5f;

  for (const auto &uniqueQueue : uniqueQueueFamilies)
    deviceQueueInfos.push_back({
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = uniqueQueue,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    });

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicStateFeatues = {
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .extendedDynamicState = true,
  };

  VkPhysicalDeviceVulkan13Features vulkan13Features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &dynamicStateFeatues,
      .synchronization2 = true,
      .dynamicRendering = true,
  };

  VkPhysicalDeviceFeatures2 deviceFeatures2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &vulkan13Features,
      .features = deviceFeatures,
  };

  VkDeviceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &deviceFeatures2,
      .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueInfos.size()),
      .pQueueCreateInfos = deviceQueueInfos.data(),
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data(),
  };

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create logical device");

  vkGetDeviceQueue(device, indices.graphcisFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

VKContext::~VKContext() {

  vkDestroyDevice(device, nullptr);

  vkDestroySurfaceKHR(instance, surface, nullptr);
  DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  vkDestroyInstance(instance, nullptr);
}

}; // namespace MAI
