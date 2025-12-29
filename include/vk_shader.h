#pragma once

#include "vk_context.h"
namespace MAI {
struct VKShader {
  VKShader(VKContext *vkContext, const char *filename,
           VkShaderStageFlagBits stage);
  ~VKShader();

  VkShaderModule getShaderModule() const { return shaderModule; }
  VkShaderStageFlagBits getShaderStage() const { return stage; }

private:
  const char *filename;
  VKContext *vkContext;
  VkShaderModule shaderModule = nullptr;
  VkShaderStageFlagBits stage;

  void createShaderModule();
};
}; // namespace MAI
