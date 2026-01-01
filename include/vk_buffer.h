#pragma once

#include "vk_cmd.h"
#include "vk_context.h"
namespace MAI {

struct BufferInfo {
  VkDeviceSize size;
  const void *data;
  VkBufferUsageFlags usage;
};

struct VKbuffer {
  VKbuffer(VKContext *vkContext, VKCmd *vkCmd, BufferInfo info);
  ~VKbuffer();

  VkBuffer getBufferModule() const { return buffer; }
  VkBufferUsageFlags getBufferUsage() const { return info_.usage; };
  VkDeviceSize getBufferSize() const { return info_.size; };

  void updateUniformBuffer(uint32_t curreImage, void *data, size_t size);

  const std::vector<VkBuffer> &getUniformBuffers() const {
    return uniformBuffers;
  }

  static void createBuffer(VKContext *vkContext, VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkBuffer &buffer,
                           VkDeviceMemory &bufferMemory,
                           bool isStorageBuffer = false);

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  static uint32_t findMemoryType(VKContext *vkContext, uint32_t typeFilter,
                                 VkMemoryPropertyFlags properties);
  uint64_t gpuAddress();

private:
  VKContext *vkContext;
  VKCmd *vkCmd;
  VkBuffer buffer;
  VkDeviceMemory bufferMemory;
  BufferInfo info_;
  // uniform buffer

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBufferMemory;
  std::vector<void *> uniformBufferMapped;

  void initBuffer();
  void createUniformBuffer();
};
}; // namespace MAI
