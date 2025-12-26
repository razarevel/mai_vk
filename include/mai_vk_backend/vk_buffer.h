#pragma once

#include "mai_vk_backend/vk_cmd.h"
#include "mai_vk_backend/vk_context.h"
namespace MAI {

struct BufferInfo {
  VkDeviceSize size;
  const void *data;
  VkBufferUsageFlagBits usage;
};

struct VKbuffer {
  VKbuffer(VKContext *vkContext, VKCmd *vkCmd, BufferInfo info);
  ~VKbuffer();

  VkBuffer getBufferModule() const { return buffer; }
  VkBufferUsageFlagBits getBufferUsage() const { return info_.usage; };
  VkDeviceSize getBufferSize() const { return info_.size; };

  void updateUniformBuffer(uint32_t curreImage, void *data, size_t size);

  const std::vector<VkBuffer> &getUniformBuffers() const {
    return uniformBuffers;
  }

  static void createBuffer(VKContext *vkContext, VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkBuffer &buffer,
                           VkDeviceMemory &bufferMemory);

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  static uint32_t findMemoryType(VKContext *vkContext, uint32_t typeFilter,
                                 VkMemoryPropertyFlags properties);

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
