#include "mai_vk_backend/vk_pipeline.h"
#include <cassert>

namespace MAI {
VKPipeline::VKPipeline(VKContext *vkContext, VKSwapchain *vkSwapchain,
                       PipelineInfo info)
    : vkContext(vkContext), info_(info), vkSwapchain(vkSwapchain) {
  createPipelineLayout();
  createPipeline();
}

void VKPipeline::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 0,
  };

  if (info_.descriptorSetLayout != nullptr) {
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &info_.descriptorSetLayout;
  }

  if (vkCreatePipelineLayout(vkContext->getDevice(), &pipelineLayoutInfo,
                             nullptr, &pipelineLayout) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout");
}

void VKPipeline::createPipeline() {
  std::vector<VkPipelineShaderStageCreateInfo> stages;
  if (info_.vert != nullptr) {
    assert(info_.vert->getShaderStage() == VK_SHADER_STAGE_VERTEX_BIT);

    stages.push_back({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = info_.vert->getShaderModule(),
        .pName = "main",
    });
  }
  if (info_.frag != nullptr) {
    assert(info_.frag->getShaderStage() == VK_SHADER_STAGE_FRAGMENT_BIT);
    stages.push_back({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = info_.frag->getShaderModule(),
        .pName = "main",
    });
  }

  VkPipelineVertexInputStateCreateInfo vertInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
  };
  std::vector<VkVertexInputAttributeDescription> attributes;
  VkVertexInputBindingDescription bindingDescriptor;
  if (!info_.vertInput.attributes.empty()) {
    attributes.reserve(info_.vertInput.attributes.size());
    for (VertexAttribute &input : info_.vertInput.attributes)
      attributes.push_back({
          .location = input.location,
          .binding = input.binding,
          .format = input.format,
          .offset = input.offset,
      });
    bindingDescriptor = {
        .binding = info_.vertInput.inputBinding.binding,
        .stride = info_.vertInput.inputBinding.stride,
        .inputRate = info_.vertInput.inputBinding.inputRate,
    };

    vertInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributes.size());
    vertInputInfo.pVertexAttributeDescriptions = attributes.data();
    vertInputInfo.vertexBindingDescriptionCount = 1;
    vertInputInfo.pVertexBindingDescriptions = &bindingDescriptor;
  }

  std::vector dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = info_.topology,
      .primitiveRestartEnable = VK_FALSE,
  };

  VkExtent2D extent = vkSwapchain->getSwapchainExtent();

  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(extent.width),
      .height = static_cast<float>(extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = extent,
  };

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };
  VkPipelineRasterizationStateCreateInfo raserization{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = info_.polygon,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };
  VkPipelineMultisampleStateCreateInfo multiSampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
  };

  VkFormat format = vkSwapchain->getSwapchainImageFormat();

  VkPipelineRenderingCreateInfo pipelineRenderCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &format,
  };

  VkGraphicsPipelineCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &pipelineRenderCreateInfo,
      .stageCount = static_cast<uint32_t>(stages.size()),
      .pStages = stages.data(),
      .pVertexInputState = &vertInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &raserization,
      .pMultisampleState = &multiSampling,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = pipelineLayout,
      .renderPass = nullptr,
  };

  if (vkCreateGraphicsPipelines(vkContext->getDevice(), nullptr, 1, &createInfo,
                                nullptr, &pipeline) != VK_SUCCESS)
    throw std::runtime_error("failed to create graphics pipeline");
}

VKPipeline::~VKPipeline() {
  vkDestroyPipelineLayout(vkContext->getDevice(), pipelineLayout, nullptr);
  vkDestroyPipeline(vkContext->getDevice(), pipeline, nullptr);
}
}; // namespace MAI
