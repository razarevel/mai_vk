#pragma once

#include "vk_buffer.h"
#include "vk_cmd.h"
#include "vk_context.h"
#include "vk_swapchain.h"
#include <cstdint>
namespace MAI {

enum TextureFormat : uint8_t {
  MAI_TEXTURE_2D,
  MAI_DEPTH_TEXTURE,
};

struct TextureInfo {
  uint32_t width;
  uint32_t height;
  const void *data;
  TextureFormat format = MAI_TEXTURE_2D;
};

struct VKTexture {
  VKTexture(VKContext *vkContext, VKCmd *vkCmdVKbuffer,
            VKSwapchain *vkSwapChain, TextureInfo info);
  ~VKTexture();

  void createImage(uint32_t width, uint32_t height, VkImageType type,
                   VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                   VkImage &image, VkDeviceMemory &imageMemory);

  VkImage getTextureImage() const { return texture; }
  VkImageView getTextureImageView() const { return textureView; }
  VkSampler getTextureImageSamper() const { return textureSampler; }
  TextureFormat getTextureFormat() const { return info_.format; }
  VkFormat getDepthFormat() const { return depthFormat; }

  static VkFormat findDepthFormat(VKContext *vkContext);

private:
  TextureInfo info_;
  VKContext *vkContext;
  VKSwapchain *vkSwapChain;
  VKCmd *vkCmd;
  VkImage texture;
  VkImageView textureView;
  VkSampler textureSampler = VK_NULL_HANDLE;
  VkDeviceMemory textureMemory;
  VkFormat depthFormat;

  void createTextureImage();
  void createTextureImageView(VkFormat format, VkImageViewType viewType,
                              VkImageAspectFlags aspect);
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
