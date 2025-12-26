#pragma once

#include "mai_vk_backend/vk_buffer.h"
#include "mai_vk_backend/vk_cmd.h"
#include "mai_vk_backend/vk_context.h"
#include "mai_vk_backend/vk_swapchain.h"
namespace MAI {

enum TextureFormat : uint8_t {
  MAI_TEXTURE_2D,
  MAI_DEPTH_TEXTURE,
};

struct VKTexture {
  VKTexture(VKContext *vkContext, VKCmd *vkCmdVKbuffer,
            VKSwapchain *vkSwapChain, const char *filename,
            TextureFormat format);
  ~VKTexture();

  void createImage(uint32_t width, uint32_t height, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);

  VkImage getTextureImage() const { return texture; }
  VkImageView getTextureImageView() const { return textureView; }
  VkSampler getTextureImageSamper() const { return textureSampler; }
  TextureFormat getTextureFormat() const { return textureFormat; }
  VkFormat getDepthFormat() const { return depthFormat; }

  static VkFormat findDepthFormat(VKContext *vkContext);

private:
  const char *filename;
  TextureFormat textureFormat;
  VKContext *vkContext;
  VKSwapchain *vkSwapChain;
  VKCmd *vkCmd;
  VkImage texture;
  VkImageView textureView;
  VkSampler textureSampler = VK_NULL_HANDLE;
  VkDeviceMemory textureMemory;
  VkFormat depthFormat;

  void createTextureImage();
  void createTextureImageView(VkFormat format, VkImageAspectFlags aspect);
  void createTextureSampler();

  void createDepthResources();

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height);
  static VkFormat findSupportedFormat(VKContext *vkContext,
                                      const std::vector<VkFormat> &candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features);
};
}; // namespace MAI
