#pragma once

#include "mai_vk_backend/vk_context.h"
namespace MAI {

struct VKDescriptor {
  VKDescriptor(VKContext *vkContext);
  ~VKDescriptor();

  VkDescriptorSetLayout createDescriptorSetLayout(
      const std::vector<VkDescriptorSetLayoutBinding> &uboLayouts);

  std::vector<VkDescriptorSet>
  createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
                       const std::vector<VkBuffer> &uniformBuffers,
                       size_t bufferSize);

  void destroyDescriptorSetLayouts(VkDescriptorSetLayout layouts);

private:
  VKContext *vkContext;
  VkDescriptorPool descriptorPool;

  void createDescriptorPool();
};
}; // namespace MAI
