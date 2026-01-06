#pragma once
#include "vk_context.h"
#include "vk_shader.h"
#include "vk_swapchain.h"
namespace MAI {

struct VertexAttribute {
  uint32_t binding = 0;
  uint32_t location;
  VkFormat format;
  uint32_t offset;
};

struct InputBinding {
  uint32_t binding = 0;
  uint32_t stride;
  VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
};

struct VertextInput {
  std::vector<VertexAttribute> attributes;
  InputBinding inputBinding;
};

struct ColorInfo {
  bool blendEnable = false;
  VkBlendFactor srcColorBlned;
  VkBlendFactor dstColorBlend;
};

struct PipelineInfo {
  VKShader *vert = nullptr;
  VKShader *frag = nullptr;
  VKShader *geom = nullptr;
  VkDescriptorSetLayout descriptorSetLayout = nullptr;
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkPolygonMode polygon = VK_POLYGON_MODE_FILL;
  VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
  VertextInput vertInput;
  ColorInfo color;
  VkPushConstantRange pushConstants;
};

struct VKPipeline {
  VKPipeline(VKContext *vkContext, VKSwapchain *vkSwapchain, PipelineInfo info);
  ~VKPipeline();

  VkPipeline getPipeline() const { return pipeline; }
  VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
  VkShaderStageFlags getPushConstantShaderStages() const {
    return info_.pushConstants.stageFlags;
  }

private:
  VKContext *vkContext;
  VKSwapchain *vkSwapchain;
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  PipelineInfo info_;
  std::vector<VkPipelineShaderStageCreateInfo> stages;

  void createPipelineLayout();
  void setShaderModules();
  void createPipeline();
};
}; // namespace MAI
