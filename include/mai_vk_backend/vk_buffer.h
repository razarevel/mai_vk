#pragma once

#include "mai_vk_backend/vk_cmd.h"
#include "mai_vk_backend/vk_context.h"
namespace MAI {

struct BufferInfo {
  const size_t size;
  const void *data;
  VkBufferUsageFlagBits usage;
};

struct VKbuffer {
  VKbuffer(VKContext *vkContext, VKCmd *vkCmd, BufferInfo info);
  ~VKbuffer();

  VkBuffer getBufferModule() const { return buffer; }
  VkBufferUsageFlagBits getBufferUsage() const { return info_.usage; };

  void updateUniformBuffer(uint32_t curreImage, void *data, size_t size);

  const std::vector<VkBuffer> &getUniformBuffers() const {
    return uniformBuffers;
  }

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  uint32_t findMemoryType(uint32_t typeFilter,
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
