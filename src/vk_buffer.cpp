#include "mai_vk_backend/vk_buffer.h"

namespace MAI {

VKbuffer::VKbuffer(VKContext *vkContext, VKCmd *vkCmd, BufferInfo info)
    : vkContext(vkContext), vkCmd(vkCmd), info_(info) {
  if (info_.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    createUniformBuffer();
  else
    initBuffer();
}

void VKbuffer::initBuffer() {
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  createBuffer(vkContext, info_.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(vkContext->getDevice(), stagingBufferMemory, 0, info_.size, 0,
              &data);

  memcpy(data, info_.data, (size_t)info_.size);
  vkUnmapMemory(vkContext->getDevice(), stagingBufferMemory);

  createBuffer(vkContext, info_.size,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT | info_.usage,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
  copyBuffer(stagingBuffer, buffer, info_.size);

  vkDestroyBuffer(vkContext->getDevice(), stagingBuffer, nullptr);
  vkFreeMemory(vkContext->getDevice(), stagingBufferMemory, nullptr);
}

void VKbuffer::createUniformBuffer() {
  uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBufferMapped.resize(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(vkContext, info_.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniformBuffers[i], uniformBufferMemory[i]);
    vkMapMemory(vkContext->getDevice(), uniformBufferMemory[i], 0, info_.size,
                0, &uniformBufferMapped[i]);
  }
}

void VKbuffer::createBuffer(VKContext *vkContext, VkDeviceSize size,
                            VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkBuffer &buffer,
                            VkDeviceMemory &bufferMemory) {

  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  if (vkCreateBuffer(vkContext->getDevice(), &bufferInfo, nullptr, &buffer) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create buffer module");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(vkContext->getDevice(), buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(vkContext, memRequirements.memoryTypeBits, properties),
  };

  if (vkAllocateMemory(vkContext->getDevice(), &allocInfo, nullptr,
                       &bufferMemory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate buffer memory");

  vkBindBufferMemory(vkContext->getDevice(), buffer, bufferMemory, 0);
}

void VKbuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                          VkDeviceSize size) {
  VkCommandBuffer commandBuffer = vkCmd->beginSingleCommandBuffer();

  VkBufferCopy copyRegion{
      .size = size,
  };
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkCmd->endSingleCommandBuffer(commandBuffer);
}

uint32_t VKbuffer::findMemoryType(VKContext *vkContext, uint32_t typeFilter,
                                  VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(vkContext->getPhysicalDevice(),
                                      &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;

  throw std::runtime_error("failed to find suitable memory type!");
}

void VKbuffer::updateUniformBuffer(uint32_t curreImage, void *data,
                                   size_t size) {
  memcpy(uniformBufferMapped[curreImage], data, size);
}

VKbuffer::~VKbuffer() {

  if (info_.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyBuffer(vkContext->getDevice(), uniformBuffers[i], nullptr);
      vkFreeMemory(vkContext->getDevice(), uniformBufferMemory[i], nullptr);
    }

  else {
    vkDestroyBuffer(vkContext->getDevice(), buffer, nullptr);
    vkFreeMemory(vkContext->getDevice(), bufferMemory, nullptr);
  }
}

}; // namespace MAI
