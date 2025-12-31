#include "vk_descriptor.h"
#include "vk_context.h"
#include <array>
namespace MAI {

VKDescriptor::VKDescriptor(VKContext *vkContext, DescriptorSetInfo info)
    : vkContext(vkContext), info_(info) {
  createDescriptorSetLayout();
  createDescriptorPool();
  createDescriptorSets();
}

void VKDescriptor::createDescriptorSetLayout() {

  std::vector<VkDescriptorBindingFlags> bindingFlags = {
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
          VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, // binding 0
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
          VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
          VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT // binding 1
                                                              // (highest)
  };

  VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCreateInfo = {
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
      .pBindingFlags = bindingFlags.data(),
  };

  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &flagsCreateInfo,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
      .bindingCount = static_cast<uint32_t>(info_.uboLayout.size()),
      .pBindings = info_.uboLayout.data(),
  };

  if (vkCreateDescriptorSetLayout(vkContext->getDevice(), &layoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS)
    throw std::runtime_error("failed to create descriptor set layout");
}

void VKDescriptor::createDescriptorPool() {
  // std::array<VkDescriptorPoolSize, 2> poolSizes{};
  VkDescriptorPoolSize poolSizes[] = {
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES},
      {VK_DESCRIPTOR_TYPE_SAMPLER, MAX_TEXTURES},
  };

  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 2,
      .pPoolSizes = poolSizes,
  };

  if (vkCreateDescriptorPool(vkContext->getDevice(), &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS)
    throw std::runtime_error("failed to create pool descirptor");
}

void VKDescriptor::createDescriptorSets() {

  descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             descriptorSetLayout);
  uint32_t counts[] = {MAX_TEXTURES, MAX_TEXTURES};
  VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
      .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
      .pDescriptorCounts = counts,
  };

  VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = &countInfo,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
      .pSetLayouts = layouts.data(),
  };

  if (vkAllocateDescriptorSets(vkContext->getDevice(), &allocInfo,
                               descriptorSets.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate descriptor set");
}

void VKDescriptor::updateDescriptorImageWrite(VkImageView imageView,
                                              VkSampler sampler,
                                              uint32_t imageIndex) {
  VkDescriptorImageInfo imageInfo{
      .imageView = imageView,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkDescriptorImageInfo samplerInfo{
      .sampler = sampler,
  };

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = imageIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo,
    };
    descriptorWrites[1] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 1,
        .dstArrayElement = imageIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &samplerInfo,
    };
    vkUpdateDescriptorSets(vkContext->getDevice(), descriptorWrites.size(),
                           descriptorWrites.data(), 0, nullptr);
  }
}

VKDescriptor::~VKDescriptor() {
  vkDestroyDescriptorSetLayout(vkContext->getDevice(), descriptorSetLayout,
                               nullptr);
  vkDestroyDescriptorPool(vkContext->getDevice(), descriptorPool, nullptr);
}

}; // namespace MAI
