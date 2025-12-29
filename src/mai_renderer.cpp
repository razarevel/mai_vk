#include "mai_renderer.h"

namespace MAI {
MAIRenderer::MAIRenderer(uint32_t width, uint32_t height, const char *appName) {
  window = initWindow(width, height, appName);
  vkContext = new VKContext(appName, window);
  vkSwapchain = new VKSwapchain(vkContext);
  vkSyncObj = new VKSync(vkContext);
  vkCmd = new VKCmd(vkContext);
  depthTexture = new VKTexture(vkContext, vkCmd, vkSwapchain,
                               {.format = MAI_DEPTH_TEXTURE});
  vkRender =
      new VKRender(vkContext, vkSyncObj, vkSwapchain, vkCmd, depthTexture);
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

VKDescriptor *MAIRenderer::createDescriptor(DescriptorSetInfo info) {
  VKDescriptor *descriptor = new VKDescriptor(vkContext, info);
  return descriptor;
}

VKTexture *MAIRenderer::createTexture(const char *filename, TextureInfo info) {
  VKTexture *texture = new VKTexture(vkContext, vkCmd, nullptr, info);
  return texture;
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

void MAIRenderer::updatePushConstant(VKPipeline *pipeline, uint32_t size,
                                     const void *value) {
  vkRender->cmdPushConstants(pipeline->getPipelineLayout(),
                             VK_SHADER_STAGE_VERTEX_BIT, 0, size, value);
}

void MAIRenderer::updateBuffer(VKbuffer *buffer, void *data, size_t size) {
  assert(buffer->getBufferUsage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  buffer->updateUniformBuffer(vkRender->getFrameIndex(), data, size);
}

MAIRenderer::~MAIRenderer() {
  vkContext->waitForDevice();
  delete vkRender;
  delete vkCmd;
  delete vkSyncObj;
  delete vkSwapchain;
  delete vkContext;
  glfwDestroyWindow(window);
  glfwTerminate();
}

}; // namespace MAI
