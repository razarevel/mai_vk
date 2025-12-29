#include "vk_shader.h"
#include <fstream>
#include <iostream>

namespace MAI {
VKShader::VKShader(VKContext *vkContext, const char *filename,
                   VkShaderStageFlagBits stage)
    : vkContext(vkContext), filename(filename), stage(stage) {
  createShaderModule();
}

std::vector<char> readShaderFile(const char *filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file at path: " << filename << std::endl;
    assert(false);
  }
  const size_t fileSize = file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

void VKShader::createShaderModule() {

  std::vector<char> code = readShaderFile(filename);
  VkShaderModuleCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = static_cast<uint32_t>(code.size()),
      .pCode = reinterpret_cast<const uint32_t *>(code.data()),
  };
  if (vkCreateShaderModule(vkContext->getDevice(), &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS)
    throw std::runtime_error("failed to create shader module!");
}

VKShader::~VKShader() {
  vkDestroyShaderModule(vkContext->getDevice(), shaderModule, nullptr);
}

}; // namespace MAI
