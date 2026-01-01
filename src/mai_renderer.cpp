#include "mai_renderer.h"

namespace MAI {

MAIRenderer::MAIRenderer(MAIRendererInfo info) : info_(info) {
  window = initWindow();
  vkContext = new VKContext(info_.appName, window);
  vkSwapchain = new VKSwapchain(vkContext);
  vkSyncObj = new VKSync(vkContext);
  vkCmd = new VKCmd(vkContext);
  depthTexture = new VKTexture(vkContext, vkCmd, vkSwapchain,
                               {.format = MAI_DEPTH_TEXTURE});
  vkRender =
      new VKRender(vkContext, vkSyncObj, vkSwapchain, vkCmd, depthTexture);
  createGlobalDescriptor();
}

GLFWwindow *MAIRenderer::initWindow() {
  if (!glfwInit())
    throw std::runtime_error("failed to init glfw");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window = glfwCreateWindow(
      info_.width, info_.height, info_.appName,
      info_.isFullScreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

  return window;
}

void MAIRenderer::run(DrawFrameFunc drawFrame) {

  double timeStamp = glfwGetTime();
  float deltaSeconds = 0.0f;
  while (!glfwWindowShouldClose(window)) {
    const double newTimeStamp = glfwGetTime();
    deltaSeconds = static_cast<float>(newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;

    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;

    const float ratio = width / (float)height;

    vkRender->beginFrame(info_.clearColor);
    drawFrame((uint32_t)width, (uint32_t)height, ratio, deltaSeconds);
    vkRender->endFrame();
    vkRender->submitFrame();
    lastBindPipeline_ = nullptr;
  }

  waitForDevice();
}

VKShader *MAIRenderer::createShader(const char *filename,
                                    VkShaderStageFlagBits stage) {
  VKShader *shader = new VKShader(vkContext, filename, stage);
  return shader;
}

VKPipeline *MAIRenderer::createPipeline(PipelineInfo info) {
  info.descriptorSetLayout = globalDescriptor->getDescriptorSetLayout();
  VKPipeline *pipeline = new VKPipeline(vkContext, vkSwapchain, info);
  return pipeline;
}

VKbuffer *MAIRenderer::createBuffer(BufferInfo info) {
  VKbuffer *buffer = new VKbuffer(vkContext, vkCmd, info);

  return buffer;
}

VKDescriptor *MAIRenderer::createDescriptor(DescriptorSetInfo info) {
  VKDescriptor *descriptor = new VKDescriptor(vkContext, info);
  return descriptor;
}

VKTexture *MAIRenderer::createTexture(TextureInfo info) {
  VKTexture *texture = new VKTexture(vkContext, vkCmd, nullptr, info);

  if (info.format == MAI_TEXTURE_2D) {
    lastTextureCount++;

    assert(lastTextureCount < MAX_TEXTURES);
    texture->setTextureIndex(lastTextureCount);

    globalDescriptor->updateDescriptorImageWrite(
        texture->getTextureImageView(), texture->getTextureImageSamper(),
        lastTextureCount);
  } else if (info.format == MAI_TEXTURE_CUBE) {
    lastCubemapCount++;

    assert(lastCubemapCount < MAX_TEXTURES);
    texture->setTextureIndex(lastCubemapCount);

    globalDescriptor->updateDescriptorImageWrite(
        texture->getTextureImageView(), texture->getTextureImageSamper(),
        lastCubemapCount, true);
  }

  return texture;
}

void MAIRenderer::bindRenderPipeline(VKPipeline *pipeline) {
  assert(pipeline->getPipeline());
  if (lastBindPipeline_ != pipeline) {
    lastBindPipeline_ = pipeline;
    vkRender->bindPipline(VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline->getPipeline());
    vkRender->cmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline->getPipelineLayout(),
                                    globalDescriptor->getDescriptorSets());
  }
}

void MAIRenderer::bindVertexBuffer(uint32_t firstBinding, VKbuffer *buffer,
                                   uint32_t offset) {
  assert(lastBindPipeline_);
  assert(buffer->getBufferModule());
  VkBuffer vertexBuffer[] = {buffer->getBufferModule()};
  VkDeviceSize offsets[] = {offset};
  vkRender->cmdBindVertexBuffers(firstBinding, 1, vertexBuffer, offsets);
}

void MAIRenderer::bindIndexBuffer(VKbuffer *buffer, VkDeviceSize offset,
                                  VkIndexType indexType) {
  assert(lastBindPipeline_);
  assert(buffer);
  assert(buffer->getBufferUsage() & VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  vkRender->cmdBindIndexBuffer(buffer->getBufferModule(), offset, indexType);
}

void MAIRenderer::bindDescriptorSet(VKPipeline *pipeline,
                                    const std::vector<VkDescriptorSet> &sets) {
  assert(lastBindPipeline_);
  vkRender->cmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  pipeline->getPipelineLayout(), sets);
}

void MAIRenderer::cmdDraw(uint32_t vertexCount, uint32_t instanceCount,
                          uint32_t firstIndex, uint32_t firstIntance) {
  assert(lastBindPipeline_);
  vkRender->cmdDraw(vertexCount, instanceCount, firstIndex, firstIntance);
}

void MAIRenderer::cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount,
                               uint32_t firstIndex, int32_t vertexOffset,
                               uint32_t firstInstance) {
  assert(lastBindPipeline_);
  vkRender->cmdDrawIndex(indexCount, instanceCount, firstIndex, vertexOffset,
                         firstInstance);
}

void MAIRenderer::updatePushConstant(uint32_t size, const void *value) {
  assert(lastBindPipeline_);
  vkRender->cmdPushConstants(lastBindPipeline_->getPipelineLayout(),
                             lastBindPipeline_->getPushConstantShaderStages(),
                             0, size, value);
}

void MAIRenderer::updateBuffer(VKbuffer *buffer, void *data, size_t size) {
  assert(lastBindPipeline_);
  assert(buffer->getBufferUsage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  buffer->updateUniformBuffer(vkRender->getFrameIndex(), data, size);
}

void MAIRenderer::BindDepthState(DepthInfo info) {
  assert(lastBindPipeline_);
  vkRender->cmdBindDepthState(info);
}

void MAIRenderer::createGlobalDescriptor() {
  DescriptorSetInfo info = {
      .uboLayout =
          {
              // 2d textures
              {
                  .binding = 0,
                  .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                  .descriptorCount = MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT,
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              },
              {
                  .binding = 1,
                  .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                  .descriptorCount = MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT,
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              },
              // cubemap
              {
                  .binding = 2,
                  .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                  .descriptorCount = MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT,
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              },
          },
  };
  globalDescriptor = new VKDescriptor(vkContext, info);
}

MAIRenderer::~MAIRenderer() {
  vkContext->waitForDevice();
  delete globalDescriptor;
  delete vkRender;
  delete vkCmd;
  delete vkSyncObj;
  delete vkSwapchain;
  delete vkContext;
  glfwDestroyWindow(window);
  glfwTerminate();
}

}; // namespace MAI
