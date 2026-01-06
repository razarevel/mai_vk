#include "vk_pipeline.h"
#include "vk_image.h"
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

  if (info_.pushConstants.size > 0) {
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &info_.pushConstants;
  }

  if (info_.descriptorSetLayout != nullptr) {
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &info_.descriptorSetLayout;
  }

  if (vkCreatePipelineLayout(vkContext->getDevice(), &pipelineLayoutInfo,
                             nullptr, &pipelineLayout) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout");
}

void VKPipeline::setShaderModules() {
  if (info_.vert != nullptr) {
    assert(info_.vert->getShaderStage() == VK_SHADER_STAGE_VERTEX_BIT);

    stages.push_back({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = info_.vert->getShaderModule(),
        .pName = "main",
    });
  }
  if (info_.geom != nullptr) {
    assert(info_.geom->getShaderStage() == VK_SHADER_STAGE_GEOMETRY_BIT);
    stages.push_back({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
        .module = info_.geom->getShaderModule(),
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
}

void VKPipeline::createPipeline() {
  setShaderModules();
  assert(!stages.empty());

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
      VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
      VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
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

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };
  VkPipelineRasterizationStateCreateInfo raserization{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = info_.polygon,
      .cullMode = info_.cullMode,
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
  if (info_.color.blendEnable) {
    colorBlendAttachment.blendEnable = VK_TRUE;

    if (info_.color.srcColorBlned == VK_BLEND_FACTOR_SRC_ALPHA) {
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      colorBlendAttachment.dstColorBlendFactor =
          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    }

    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

    if (info_.color.dstColorBlend == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) {
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      colorBlendAttachment.dstAlphaBlendFactor =
          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    }

    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  }

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
  };

  VkFormat format = vkSwapchain->getSwapchainImageFormat();

  VkPipelineRenderingCreateInfo pipelineRenderCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &format,
      .depthAttachmentFormat = VKTexture::findDepthFormat(vkContext),
  };
  VkPipelineDepthStencilStateCreateInfo depthStencil{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_FALSE,
      .depthWriteEnable = VK_FALSE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
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
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = pipelineLayout,
      .renderPass = nullptr,
  };

  if (vkCreateGraphicsPipelines(vkContext->getDevice(), nullptr, 1, &createInfo,
                                nullptr, &pipeline) != VK_SUCCESS)
    throw std::runtime_error("failed to create graphics pipeline");

  stages.clear();
} // namespace MAI

VKPipeline::~VKPipeline() {
  vkDestroyPipelineLayout(vkContext->getDevice(), pipelineLayout, nullptr);
  vkDestroyPipeline(vkContext->getDevice(), pipeline, nullptr);
}
}; // namespace MAI
