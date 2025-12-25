#include "mai_vk_backend/vk_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace MAI {
VKTexture::VKTexture(VKContext *vkContext, VKCmd *vkCmd, VKbuffer *vkBuffer,
                     const char *filename)
    : vkContext(vkContext), vkCmd(vkCmd), vkBuffer(vkBuffer),
      filename(filename) {
  createTextureImage();
  createTextureImageView();
  createTextureSampler();
}

void VKTexture::createTextureImage() {
  int w, h, comp;
  stbi_uc *pixels = stbi_load(filename, &w, &h, &comp, STBI_rgb_alpha);
  VkDeviceSize imageSize = w * h * 4;
  if (!pixels)
    throw std::runtime_error("failed to load texture image!");

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  vkBuffer->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(vkContext->getDevice(), stagingBufferMemory, 0, imageSize, 0,
              &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(vkContext->getDevice(), stagingBufferMemory);

  stbi_image_free(pixels);

  createImage(w, h, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture, textureMemory);

  transitionImageLayout(texture, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(stagingBuffer, texture, static_cast<uint32_t>(w),
                    static_cast<uint32_t>(h));
  transitionImageLayout(texture, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(vkContext->getDevice(), stagingBuffer, nullptr);
  vkFreeMemory(vkContext->getDevice(), stagingBufferMemory, nullptr);
}

void VKTexture::createTextureImageView() {
  VkImageViewCreateInfo viewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .image = texture,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  if (vkCreateImageView(vkContext->getDevice(), &viewInfo, nullptr,
                        &textureView) != VK_SUCCESS)
    throw std::runtime_error("failed to create image view");
}

void VKTexture::createTextureSampler() {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(vkContext->getPhysicalDevice(), &properties);

  VkSamplerCreateInfo samplerInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = 0.0f,
  };

  if (vkCreateSampler(vkContext->getDevice(), &samplerInfo, nullptr,
                      &textureSampler) != VK_SUCCESS)
    throw std::runtime_error("failed to creat texture sampler");
}

void VKTexture::createImage(uint32_t width, uint32_t height, VkFormat format,
                            VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage &image,
                            VkDeviceMemory &imageMemory) {

  VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent =
          {
              .width = width,
              .height = width,
              .depth = 1,
          },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  if (vkCreateImage(vkContext->getDevice(), &imageInfo, nullptr, &image) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create texture image");

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(vkContext->getDevice(), image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = vkBuffer->findMemoryType(
          memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };

  if (vkAllocateMemory(vkContext->getDevice(), &allocInfo, nullptr,
                       &imageMemory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate image memory");

  vkBindImageMemory(vkContext->getDevice(), image, imageMemory, 0);
}

void VKTexture::transitionImageLayout(VkImage image, VkFormat format,
                                      VkImageLayout oldLayout,
                                      VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer = vkCmd->beginSingleCommandBuffer();
  VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_COMPUTE_BIT,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  VkPipelineStageFlags sourcesStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourcesStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourcesStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else
    throw std::invalid_argument("unsupported layout transition!");

  vkCmdPipelineBarrier(commandBuffer, sourcesStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  vkCmd->endSingleCommandBuffer(commandBuffer);
}

void VKTexture::copyBufferToImage(VkBuffer buffer, VkImage image,
                                  uint32_t width, uint32_t height) {

  VkCommandBuffer commandBuffer = vkCmd->beginSingleCommandBuffer();

  VkBufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,

      .imageSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1},
  };

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  vkCmd->endSingleCommandBuffer(commandBuffer);
}

VKTexture::~VKTexture() {

  vkDestroySampler(vkContext->getDevice(), textureSampler, nullptr);
  vkDestroyImageView(vkContext->getDevice(), textureView, nullptr);

  vkDestroyImage(vkContext->getDevice(), texture, nullptr);
  vkFreeMemory(vkContext->getDevice(), textureMemory, nullptr);
}

}; // namespace MAI
