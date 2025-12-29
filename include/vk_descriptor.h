#pragma once

#include "vk_buffer.h"
#include "vk_context.h"
#include "vk_image.h"
namespace MAI {

struct DescriptorSetInfo {
  std::vector<VkDescriptorSetLayoutBinding> uboLayout;
  std::vector<VKbuffer *> buffers;
  std::vector<VKTexture *> textures;
};

struct VKDescriptor {
  VKDescriptor(VKContext *vkContext, DescriptorSetInfo info);
  ~VKDescriptor();

  VkDescriptorSetLayout getDescriptorSetLayout() const {
    return descriptorSetLayout;
  }

  const std::vector<VkDescriptorSet> &getDescriptorSets() const {
    return descriptorSets;
  }

private:
  VKContext *vkContext;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  DescriptorSetInfo info_;

  void createDescriptorPool();
  void createDescriptorSetLayout();
  void createDescriptorSets();
};
}; // namespace MAI
