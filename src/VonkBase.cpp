#include "VonkBase.h"
#include "VonkCreate.h"
#include "VonkTools.h"
#include "VonkWindow.h"

namespace vonk
{  //

//----------------------------------------------- !!

void Base::addPipeline(PipelineData_t const &ci)
{
  mPipelinesCI.push_back(ci);
  mPipelines.push_back(vonk::create::pipeline(
    mPipelinesCI.back(),
    mSwapChain,
    mDevice.handle,
    mDevice.commandPool,
    mSwapChain.defaultRenderPass,
    mSwapChain.defaultFrameBuffers));
};

//-----------------------------------------------

void Base::waitDevice() { vkDeviceWaitIdle(mDevice.handle); }

//-----------------------------------------------

void Base::drawFrame()
{
  static size_t currFrame = 0;

  //=====
  //=====   PRECONDITIONS

  vkWaitForFences(mDevice.handle, 1, &mSwapChain.fences.submit[currFrame], VK_TRUE, UINT64_MAX);

  //=====
  //=====   1. GET NEXT IMAGE TO PROCESS

  // (1.1) Acquiere next image
  uint32_t   imageIndex;
  auto const acquireRet = vkAcquireNextImageKHR(
    mDevice.handle,
    mSwapChain.handle,
    UINT64_MAX,
    mSwapChain.semaphores.present[currFrame],
    VK_NULL_HANDLE,
    &imageIndex);

  // (1.2) Validate the swapchain state
  if (acquireRet == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  } else if (acquireRet != VK_SUCCESS && acquireRet != VK_SUBOPTIMAL_KHR) {
    vo__err("Failed to acquire swap chain image!");
  }

  // (1.3) Check if a previous frame is using this image (i.e. there is its fence to wait on)
  if (mSwapChain.fences.acquire[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(mDevice.handle, 1, &mSwapChain.fences.acquire[imageIndex], VK_TRUE, UINT64_MAX);
  }
  // (1.4) Mark the image as now being in use by this frame
  mSwapChain.fences.acquire[imageIndex] = mSwapChain.fences.submit[currFrame];

  //=====
  //=====   2. DRAW : Graphics Queue

  // (2.1) Sync objects
  VkPipelineStageFlags const waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore const          waitSemaphores[]   = { mSwapChain.semaphores.present[currFrame] };
  VkSemaphore const          signalSemaphores[] = { mSwapChain.semaphores.render[currFrame] };

  // (2.2) Submit info
  VkSubmitInfo const submitInfo {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pWaitDstStageMask    = waitStages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &mPipelines[mActivePipeline].commandBuffers[imageIndex],
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = waitSemaphores,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = signalSemaphores,
  };

  // (2.3) Reset fences right before asking for draw
  vkResetFences(mDevice.handle, 1, &mSwapChain.fences.submit[currFrame]);
  vonk__check(vkQueueSubmit(mDevice.queues.graphics, 1, &submitInfo, mSwapChain.fences.submit[currFrame]));

  //=====
  //=====   3. DUMP TO SCREEN : Present Queue

  // (3.1) Info
  VkSwapchainKHR const   swapChains[] = { mSwapChain.handle };
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
  auto const presentRet = vkQueuePresentKHR(mDevice.queues.present, &presentInfo);

  // (3.3) Validate swapchain state
  if (presentRet == VK_ERROR_OUT_OF_DATE_KHR || presentRet == VK_SUBOPTIMAL_KHR || vonk::window::framebufferResized) {
    vonk::window::framebufferResized = false;  // move this variable to vonk::Base
    recreateSwapChain();
  } else if (presentRet != VK_SUCCESS) {
    vo__err("Failed to present swap chain image!");
  }

  //=====
  //=====   EXTRA TASKS

  currFrame = (currFrame + 1) % sInFlightMaxFrames;
}

//-----------------------------------------------

void Base::init()
{
  //=====
  //=====   SETTINGS ¿?¿?¿?¿?¿?¿

  if (!vonk::checkValidationLayersSupport(mInstance.layers)) {
    vo__abort("Validation layers requested, but not available!");
  }

  // . Create Instance : VkInstance, VkDebugMessenger, VkSurfaceKHR
  mInstance = vonk::createInstance(vonk::window::title.c_str(), VK_API_VERSION_1_2);
  // . Pick Gpu (aka: physical device)
  mGpu = vonk::pickGpu(mInstance, mDevice.exts, true, true, false, false);
  // . Create Device (aka: gpu-manager / logical-device)
  mDevice = vonk::createDevice(mInstance, mGpu);
  // . Create SwapChain
  mSwapChain = vonk::createSwapChain(mInstance, mGpu, mDevice, mSwapChain);
}

//-----------------------------------------------

void Base::cleanup()
{
  //=====
  //=====   USER DEFINED

  for (auto &pipeline : mPipelines) {
    vkDestroyPipeline(mDevice.handle, pipeline.handle, nullptr);
    vkDestroyPipelineLayout(mDevice.handle, pipeline.layout, nullptr);
    if (pipeline.renderpass != mSwapChain.defaultRenderPass)
      vkDestroyRenderPass(mDevice.handle, pipeline.renderpass, nullptr);

    for (auto [name, shaderModule] : pipeline.shaderModules) {
      vkDestroyShaderModule(mDevice.handle, shaderModule, nullptr);
    }
    // vonk::destroyPipeline(pipeline, mDefaultRenderPass);
  }

  vonk::destroySwapChain(mDevice, mSwapChain, false);
  vonk::destroyDevice(mDevice);
  vonk::destroyInstance(mInstance);
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

//-----------------------------------------------

void Base::destroySwapChainDependencies()
{
  for (auto &pipeline : mPipelines) {
    // . Command Buffers
    auto &cb = pipeline.commandBuffers;
    if (cb.size() > 0) {
      vkFreeCommandBuffers(mDevice.handle, mDevice.commandPool, vonk__getSize(cb), vonk__getData(cb));
    }

    // . Frame Buffers
    for (size_t i = 0; i < pipeline.frameBuffers.size(); ++i) {
      if (pipeline.frameBuffers[i] != mSwapChain.defaultFrameBuffers[i]) {
        vkDestroyFramebuffer(mDevice.handle, pipeline.frameBuffers[i], nullptr);
      }
    }
  }
}

//-----------------------------------------------

void Base::recreateSwapChain()
{
  vkDeviceWaitIdle(mDevice.handle);

  destroySwapChainDependencies();
  mSwapChain = vonk::createSwapChain(mInstance, mGpu, mDevice, mSwapChain);

  for (size_t i = 0; i < mPipelines.size(); ++i) {
    // vonk::createPipeline(mDevice, mSwapChain, mPipelines[i]);
    // auto &ci = mPipelinesCI[i];
    vonk::create::pipelineRecreation(
      mPipelines[i],
      true,
      mPipelinesCI[i],
      mSwapChain,
      mDevice.handle,
      mDevice.commandPool,
      mSwapChain.defaultRenderPass,
      mSwapChain.defaultFrameBuffers);
  }
}

//-----------------------------------------------

}  // namespace vonk
