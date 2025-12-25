#pragma once

#include "mai_vk_backend/vk_buffer.h"
#include "mai_vk_backend/vk_cmd.h"
#include "mai_vk_backend/vk_context.h"
namespace MAI {

struct VKTexture {
  VKTexture(VKContext *vkContext, VKCmd *vkCmd, VKbuffer *vkBuffer,
            const char *filename);
  ~VKTexture();

  void createImage(uint32_t width, uint32_t height, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);

private:
  const char *filename;
  VKContext *vkContext;
  VKCmd *vkCmd;
  VKbuffer *vkBuffer;
  VkImage texture;
  VkImageView textureView;
  VkSampler textureSampler;
  VkDeviceMemory textureMemory;

  void createTextureImage();
  void createTextureImageView();
  void createTextureSampler();

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height);
};
}; // namespace MAI
