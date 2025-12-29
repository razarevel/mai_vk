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

  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(info_.uboLayout.size()),
      .pBindings = info_.uboLayout.data(),
  };

  if (vkCreateDescriptorSetLayout(vkContext->getDevice(), &layoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS)
    throw std::runtime_error("failed to create descriptor set layout");
}

void VKDescriptor::createDescriptorPool() {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};

  poolSizes[0] = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = static_cast<uint32_t>(info_.buffers.size()) *
                         static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
  };

  poolSizes[1] = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = static_cast<uint32_t>(info_.textures.size()) *
                         static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
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

void VKDescriptor::createDescriptorSets() {

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

    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(info_.buffers.size());

    for (size_t j = 0; j < info_.buffers.size(); j++) {
      assert(info_.buffers[j]->getBufferUsage() &
             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      bufferInfos.push_back({
          .buffer = info_.buffers[j]->getUniformBuffers()[i],
          .offset = j,
          .range = info_.buffers[j]->getBufferSize(),
      });
    }

    std::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.reserve(info_.textures.size());

    for (size_t j = 0; j < info_.textures.size(); j++)
      imageInfos.push_back({
          .sampler = info_.textures[j]->getTextureImageSamper(),
          .imageView = info_.textures[j]->getTextureImageView(),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      });

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = bufferInfos.data(),
    };

    descriptorWrites[1] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = static_cast<uint32_t>(info_.buffers.size()),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = imageInfos.data(),
    };

    vkUpdateDescriptorSets(vkContext->getDevice(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

VKDescriptor::~VKDescriptor() {
  vkDestroyDescriptorSetLayout(vkContext->getDevice(), descriptorSetLayout,
                               nullptr);
  vkDestroyDescriptorPool(vkContext->getDevice(), descriptorPool, nullptr);
}

}; // namespace MAI
