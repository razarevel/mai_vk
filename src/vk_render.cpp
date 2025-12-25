#include "mai_vk_backend/vk_render.h"
#include <cassert>

namespace MAI {

VKRender::VKRender(VKContext *vkContext, VKSync *vkSyncObj,
                   VKSwapchain *vkSwapchain, VKCmd *vkCmd)
    : vkContext(vkContext), vkSync(vkSyncObj), vkSwapchain(vkSwapchain),
      vkCmd(vkCmd) {}

void VKRender::acquireSwapChainImageIndex() {
  if (vkWaitForFences(vkContext->getDevice(), 1,
                      &vkSync->getDrawFences()[frameIndex], VK_TRUE,
                      UINT64_MAX) != VK_SUCCESS)
    throw std::runtime_error("failed to wait for fence");

  vkResetFences(vkContext->getDevice(), 1,
                &vkSync->getDrawFences()[frameIndex]);

  VkResult result = vkAcquireNextImageKHR(
      vkContext->getDevice(), vkSwapchain->getSwapchain(), UINT64_MAX,
      vkSync->getImageAvailableSemaphores()[frameIndex], nullptr, &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    vkSwapchain->recreateSwapChain();
}

void transition_image_layout(
    uint32_t imageIndex, VkImageLayout oldLayout, VkImageLayout newLayout,
    VkAccessFlagBits2 srcAccessMask, VkAccessFlagBits2 dstAccessMask,
    VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask,
    VkImage swapChainImage, VkCommandBuffer commandBuffer) {

  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = srcStageMask,
      .srcAccessMask = srcAccessMask,
      .dstStageMask = dstStageMask,
      .dstAccessMask = dstAccessMask,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = swapChainImage,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  VkDependencyInfo dependencyInfo = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };

  vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

void VKRender::beginFrame() {
  acquireSwapChainImageIndex();

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = 0,
      .pInheritanceInfo = nullptr,
  };

  vkBeginCommandBuffer(vkCmd->getCommandBuffers()[frameIndex], &beginInfo);

  transition_image_layout(imageIndex, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, {},
                          VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                          vkSwapchain->getswapchainImages()[imageIndex],
                          vkCmd->getCommandBuffers()[frameIndex]);

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

  VkExtent2D extent = vkSwapchain->getSwapchainExtent();

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0,
      .width = static_cast<float>(extent.width),
      .height = static_cast<float>(extent.height),
      .minDepth = 0.0,
      .maxDepth = 1.0f,
  };

  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = extent,
  };

  VkRenderingAttachmentInfo attachmentInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = vkSwapchain->getswapchainImageViews()[imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_NONE,
      .clearValue = clearColor,
  };

  VkRenderingInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = scissor,
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
  };

  vkCmdBeginRendering(vkCmd->getCommandBuffers()[frameIndex], &renderingInfo);

  vkCmdSetViewport(vkCmd->getCommandBuffers()[frameIndex], 0, 1, &viewport);
  vkCmdSetScissor(vkCmd->getCommandBuffers()[frameIndex], 0, 1, &scissor);
}

void VKRender::endFrame() {
  vkCmdEndRendering(vkCmd->getCommandBuffers()[frameIndex]);
  transition_image_layout(imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                          VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, {},
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                          vkSwapchain->getswapchainImages()[imageIndex],
                          vkCmd->getCommandBuffers()[frameIndex]);
  vkEndCommandBuffer(vkCmd->getCommandBuffers()[frameIndex]);
}

void VKRender::submitFrame() {

  VkSemaphore waitSemaphore[] = {
      vkSync->getImageAvailableSemaphores()[frameIndex],
  };
  VkSemaphore signalSemaphore[] = {
      vkSync->getRenderFinishSemaphores()[frameIndex],
  };
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphore,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &vkCmd->getCommandBuffers()[frameIndex],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphore,
  };

  if (vkQueueSubmit(vkContext->getGraphicsQueue(), 1, &submitInfo,
                    vkSync->getDrawFences()[frameIndex]) != VK_SUCCESS)
    throw std::runtime_error("failed to submit to the queue");

  VkSwapchainKHR swapChains[] = {vkSwapchain->getSwapchain()};

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphore,
      .swapchainCount = 1,
      .pSwapchains = swapChains,
      .pImageIndices = &imageIndex,
  };

  VkResult result =
      vkQueuePresentKHR(vkContext->getPresentQueue(), &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      vkContext->frameRsized) {
    vkContext->frameRsized = false;
    vkSwapchain->recreateSwapChain();
  } else if (result != VK_SUCCESS)
    throw std::runtime_error("failed to present swap chain image");

  frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VKRender::bindPipline(VkPipelineBindPoint bindPoint, VkPipeline pipeline) {
  vkCmdBindPipeline(vkCmd->getCommandBuffers()[frameIndex], bindPoint,
                    pipeline);
}

void VKRender::cmdBindDescriptorSets(
    VkPipelineBindPoint bindPoint, VkPipelineLayout piplineLayout,
    const std::vector<VkDescriptorSet> &descriptorSets) {
  vkCmdBindDescriptorSets(vkCmd->getCommandBuffers()[frameIndex], bindPoint,
                          piplineLayout, 0, 1, &descriptorSets[frameIndex], 0,
                          nullptr);
}

void VKRender::cmdDraw(uint32_t vertexCount, uint32_t instanceCount,
                       uint32_t firstVertex, uint32_t firstInstance) {
  vkCmdDraw(vkCmd->getCommandBuffers()[frameIndex], vertexCount, instanceCount,
            firstVertex, firstInstance);
}

void VKRender::cmdBindVertexBuffers(uint32_t firstBinding,
                                    uint32_t bindingCount,
                                    const VkBuffer *pBuffers,
                                    const VkDeviceSize *offsets) {
  vkCmdBindVertexBuffers(vkCmd->getCommandBuffers()[frameIndex], firstBinding,
                         bindingCount, pBuffers, offsets);
}

void VKRender::cmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset,
                                  VkIndexType indexType) {
  vkCmdBindIndexBuffer(vkCmd->getCommandBuffers()[frameIndex], buffer, offset,
                       indexType);
}

void VKRender::cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount,
                            uint32_t firstIndex, int32_t vertexOffset,
                            uint32_t firstInstance) {
  vkCmdDrawIndexed(vkCmd->getCommandBuffers()[frameIndex], indexCount,
                   instanceCount, firstIndex, vertexOffset, firstInstance);
}

} // namespace MAI
