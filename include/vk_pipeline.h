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

struct PipelineInfo {
  VKShader *vert = nullptr;
  VKShader *frag = nullptr;
  VkDescriptorSetLayout descriptorSetLayout = nullptr;
  VertextInput vertInput;
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkPolygonMode polygon = VK_POLYGON_MODE_FILL;
  VkPushConstantRange pushConstants;
};

struct VKPipeline {
  VKPipeline(VKContext *vkContext, VKSwapchain *vkSwapchain, PipelineInfo info);
  ~VKPipeline();

  VkPipeline getPipeline() const { return pipeline; }
  VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

private:
  VKContext *vkContext;
  VKSwapchain *vkSwapchain;
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  PipelineInfo info_;

  void createPipelineLayout();
  void createPipeline();
};
}; // namespace MAI
