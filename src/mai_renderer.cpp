#include "mai_renderer.h"
#include <iostream>

namespace MAI {
MAIRenderer::MAIRenderer(uint32_t width, uint32_t height, const char *appName) {
  window = initWindow(width, height, appName);
  vkContext = new VKContext(appName, window);
  vkSwapchain = new VKSwapchain(vkContext);
  vkSyncObj = new VKSync(vkContext);
  vkCmd = new VKCmd(vkContext);
  vkDescriptor = new VKDescriptor(vkContext);
  vkRender = new VKRender(vkContext, vkSyncObj, vkSwapchain, vkCmd);
}

GLFWwindow *MAIRenderer::initWindow(uint32_t width, uint32_t height,
                                    const char *appName) {
  if (!glfwInit())
    throw std::runtime_error("failed to init glfw");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window =
      glfwCreateWindow(width, height, appName, nullptr, nullptr);

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

    vkRender->beginFrame();
    drawFrame((uint32_t)width, (uint32_t)height, ratio, deltaSeconds);
    vkRender->endFrame();
    vkRender->submitFrame();
  }
}

VKShader *MAIRenderer::createShader(const char *filename,
                                    VkShaderStageFlagBits stage) {
  VKShader *shader = new VKShader(vkContext, filename, stage);
  return shader;
}

VKPipeline *MAIRenderer::createPipeline(PipelineInfo info) {
  VKPipeline *pipeline = new VKPipeline(vkContext, vkSwapchain, info);

  return pipeline;
}

VKbuffer *MAIRenderer::createBuffer(BufferInfo info) {
  VKbuffer *buffer = new VKbuffer(vkContext, vkCmd, info);

  return buffer;
}

VkDescriptorSetLayout MAIRenderer::createDescriptorSetLayout(
    std::vector<VkDescriptorSetLayoutBinding> uboLayouts) {
  VkDescriptorSetLayout descriptorSetLayout =
      vkDescriptor->createDescriptorSetLayout(uboLayouts);
  return descriptorSetLayout;
}

std::vector<VkDescriptorSet>
MAIRenderer::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
                                  const std::vector<VkBuffer> &uniformBuffers,
                                  size_t bufferSize) {
  std::vector<VkDescriptorSet> descriptorSet =
      vkDescriptor->createDescriptorSets(descriptorSetLayout, uniformBuffers,
                                         bufferSize);
  return descriptorSet;
}

void MAIRenderer::bindRenderPipeline(VKPipeline *pipeline) {
  vkRender->bindPipline(VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->getPipeline());
}

void MAIRenderer::bindVertexBuffer(uint32_t firstBinding, VKbuffer *buffer,
                                   uint32_t offset) {
  VkBuffer vertexBuffer[] = {buffer->getBufferModule()};
  VkDeviceSize offsets[] = {0};
  vkRender->cmdBindVertexBuffers(firstBinding, 1, vertexBuffer, offsets);
}

void MAIRenderer::bindIndexBuffer(VKbuffer *buffer, VkDeviceSize offset,
                                  VkIndexType indexType) {
  assert(buffer->getBufferUsage() & VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  vkRender->cmdBindIndexBuffer(buffer->getBufferModule(), offset, indexType);
}

void MAIRenderer::bindDescriptorSet(VKPipeline *pipeline,
                                    const std::vector<VkDescriptorSet> &sets) {
  vkRender->cmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  pipeline->getPipelineLayout(), sets);
}

void MAIRenderer::cmdDraw(uint32_t vertexCount, uint32_t instanceCount,
                          uint32_t firstIndex, uint32_t firstIntance) {
  vkRender->cmdDraw(vertexCount, instanceCount, firstIndex, firstIntance);
}

void MAIRenderer::cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount,
                               uint32_t firstIndex, int32_t vertexOffset,
                               uint32_t firstInstance) {
  vkRender->cmdDrawIndex(indexCount, instanceCount, firstIndex, vertexOffset,
                         firstInstance);
}

void MAIRenderer::updateBuffer(VKbuffer *buffer, void *data, size_t size) {
  assert(buffer->getBufferUsage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  buffer->updateUniformBuffer(vkRender->getFrameIndex(), data, size);
}

void MAIRenderer::destroyDescriptorSetLayout(VkDescriptorSetLayout layout) {
  vkDescriptor->destroyDescriptorSetLayouts(layout);
}

TextureModule MAIRenderer::createTexture(const char *filename) {
  int w, h, comp;
  stbi_uc *pixel =
      stbi_load(RESOURCES_PATH "statue-1275469_1280.jpg", &w, &h, &comp, 4);
}

MAIRenderer::~MAIRenderer() {
  vkContext->waitForDevice();
  delete vkRender;
  delete vkDescriptor;
  delete vkCmd;
  delete vkSyncObj;
  delete vkSwapchain;
  delete vkContext;
  glfwDestroyWindow(window);
  glfwTerminate();
}

}; // namespace MAI
