#include "mai_vk_backend/vk_descriptor.h"
#include "mai_vk_backend/vk_context.h"

namespace MAI {

VKDescriptor::VKDescriptor(VKContext *vkContext) : vkContext(vkContext) {
  createDescriptorPool();
}

VkDescriptorSetLayout VKDescriptor::createDescriptorSetLayout(
    const std::vector<VkDescriptorSetLayoutBinding> &uboLayouts) {
  VkDescriptorSetLayout descriptorSetLayout;

  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(uboLayouts.size()),
      .pBindings = uboLayouts.data(),
  };

  if (vkCreateDescriptorSetLayout(vkContext->getDevice(), &layoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS)
    throw std::runtime_error("failed to create descriptor set layout");
  return descriptorSetLayout;
}

void VKDescriptor::createDescriptorPool() {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0] = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
  };
  poolSizes[1] = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
  };

  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };
  if (vkCreateDescriptorPool(vkContext->getDevice(), &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS)
    throw std::runtime_error("failed to create pool descirptor");
}

std::vector<VkDescriptorSet>
VKDescriptor::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
                                   const std::vector<VkBuffer> &uniformBuffers,
                                   size_t bufferSize) {

  std::vector<VkDescriptorSet> descriptorSets;
  descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
      .pSetLayouts = layouts.data(),
  };

  if (vkAllocateDescriptorSets(vkContext->getDevice(), &allocInfo,
                               descriptorSets.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate descriptor set");

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

    VkDescriptorBufferInfo bufferInfo{
        .buffer = uniformBuffers[i],
        .offset = 0,
        .range = bufferSize,
    };

    // VkDescriptorImageInfo imageInfo{
    // 		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    // };

    VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
    };

    vkUpdateDescriptorSets(vkContext->getDevice(), 1, &descriptorWrite, 0,
                           nullptr);
  }
  return descriptorSets;
}

void VKDescriptor::destroyDescriptorSetLayouts(VkDescriptorSetLayout layout) {
  vkDestroyDescriptorSetLayout(vkContext->getDevice(), layout, nullptr);
}

VKDescriptor::~VKDescriptor() {
  vkDestroyDescriptorPool(vkContext->getDevice(), descriptorPool, nullptr);
}

}; // namespace MAI
