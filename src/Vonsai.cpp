#include "Vonsai.h"

namespace vo
{
//-----------------------------------------------

void Vonsai::run()
{
  // ::: INIT WINDOW
  vo::window::init(1280, 720, "Vonsai");

  // ::: INIT VULKAN INSTANCE
  // . Create instance
  mInstance.create();
  vo__check(mInstance.handle);
  // . Create device
  mDevice.create(mInstance);
  vo__check(mDevice.handle);
  // . Populate shaders
  static auto const createShadersSameName =
    [&](std::string const &name, std::initializer_list<VkShaderStageFlagBits> stages) {
      for (auto const stage : stages) {
        auto const data = vku::shaders::create(mDevice.handle, name, stage);
        mDevice.shaderModules.emplace(data.path, data.module);
        mDevice.pipelineShaderStageCreateInfos.emplace_back(data.stageCreateInfo);
      }
    };
  createShadersSameName("base", { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT });
  // . SwapChain and GraphicsPipeline
  mDevice.createSwapChain();
  mDevice.createGraphicsPipeline();

  // ::: MAIN LOOP
  vo::window::loop([&]() { drawFrame(); }, [&]() { vkDeviceWaitIdle(mDevice.handle); });

  // ::: CLEAN UP
  mDevice.destroy();
  mInstance.destroy();
}

//-----------------------------------------------

//-----------------------------------------------------------------------------

void Vonsai::drawFrame()
{
  size_t currFrame = 0;
  vkWaitForFences(mDevice.handle, 1, &mDevice.inFlightFencesSubmit[currFrame], VK_TRUE, UINT64_MAX);  // MUST

  // ...

  // ::: NEXT IMAGE

  // (1.1) Acquiere next image
  uint32_t   imageIndex;
  auto const acquireRet = vkAcquireNextImageKHR(
    mDevice.handle,
    mDevice.swapchain,
    UINT64_MAX,
    mDevice.imageSemaphores[currFrame],
    VK_NULL_HANDLE,
    &imageIndex);

  // (1.2) Validate the swapchain state
  if (acquireRet == VK_ERROR_OUT_OF_DATE_KHR) {
    mDevice.reCreateSwapChain();
    return;
  } else if (acquireRet != VK_SUCCESS && acquireRet != VK_SUBOPTIMAL_KHR) {
    vo__err("Failed to acquire swap chain image!");
  }

  // (1.3) Check if a previous frame is using this image (i.e. there is its fence to wait on)
  if (mDevice.inFlightFencesAcquire[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(mDevice.handle, 1, &mDevice.inFlightFencesAcquire[imageIndex], VK_TRUE, UINT64_MAX);
  }
  // (1.4) Mark the image as now being in use by this frame
  mDevice.inFlightFencesAcquire[imageIndex] = mDevice.inFlightFencesSubmit[currFrame];

  // ...

  // ::: DRAW (Graphics Queue)

  // (2.1) Sync objects
  VkSemaphore          waitSemaphores[]   = { mDevice.imageSemaphores[currFrame] };
  VkPipelineStageFlags waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore          signalSemaphores[] = { mDevice.renderSempahores[currFrame] };
  // (2.2) Submit info
  VkSubmitInfo const submitInfo {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = waitSemaphores,
    .pWaitDstStageMask    = waitStages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &mDevice.commandBuffers[imageIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = signalSemaphores,
  };
  // (2.3) Reset fences right before asking for draw
  vkResetFences(mDevice.handle, 1, &mDevice.inFlightFencesSubmit[currFrame]);
  vku__check(vkQueueSubmit(mDevice.queueGraphics(), 1, &submitInfo, mDevice.inFlightFencesSubmit[currFrame]));

  // ...

  // ::: DUMP TO SCREEN (Present Queue)

  // (3.1) Info
  VkSwapchainKHR const   swapChains[] = { mDevice.swapchain };
  VkPresentInfoKHR const presentInfo {
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = signalSemaphores,
    .swapchainCount     = 1,
    .pSwapchains        = swapChains,
    .pImageIndices      = &imageIndex,
    .pResults           = nullptr,  // Optional
  };

  // (3.2) Ask for dump into screen
  auto const presentRet = vkQueuePresentKHR(mDevice.queuePresent(), &presentInfo);

  // (3.3) Validate swapchain state
  if (presentRet == VK_ERROR_OUT_OF_DATE_KHR || presentRet == VK_SUBOPTIMAL_KHR) {
    mDevice.reCreateSwapChain();
  } else if (presentRet != VK_SUCCESS) {
    vo__err("Failed to present swap chain image!");
  }

  // ::: Iter flight-frame
  currFrame = (currFrame + 1) % mDevice.inFlightMaxFrames;
}

//-----------------------------------------------------------------------------

}  // namespace vo
