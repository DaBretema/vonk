#include "Vonk.h"
#include "VonkResources.h"
#include "VonkTools.h"
#include "VonkWindow.h"

namespace vonk
{  //

//
// // MESHEs
//

//-----------------------------------------------

Mesh_t const &Vonk::createMesh(std::vector<Vertex_t> const &verticesData)
{
  // @DANI : Change this for a "trace and fill the gaps"-pattern, basically a stack of removed IDs;
  static uint32_t meshCountID = 0;
  uint32_t        meshID      = 0;

  if (!mRemovedMeshes.empty()) {
    meshID = mRemovedMeshes.top();
    mRemovedMeshes.pop();
  } else {
    meshID = meshCountID++;
  }

  mMeshes[meshID] = vonk::createMesh(mDevice, verticesData);
  return mMeshes[meshID];
}

//-----------------------------------------------

void Vonk::drawMesh(VkCommandBuffer cmd, Mesh_t const &mesh) { vonk::drawMesh(cmd, mesh); }

//-----------------------------------------------

//
// // SHADERs
//

//-----------------------------------------------

DrawShader_t const &
  Vonk::createDrawShader(std::string const &keyName, std::string const &vertexName, std::string const &fragmentName)
{
  mDrawShaders[keyName] = {
    .vert = vonk::createShader(mDevice, vertexName, VK_SHADER_STAGE_VERTEX_BIT),
    .frag = vonk::createShader(mDevice, fragmentName, VK_SHADER_STAGE_FRAGMENT_BIT),
  };
  return mDrawShaders[keyName];
}

//-----------------------------------------------

DrawShader_t const &Vonk::getDrawShader(std::string const &keyName)
{
  AbortIfMsg(mDrawShaders.count(keyName) < 1, "Draw Shader Not Found!");
  return mDrawShaders[keyName];
}

//-----------------------------------------------

Shader_t const &Vonk::createComputeShader(std::string const &name)
{
  mComputeShaders[name] = vonk::createShader(mDevice, name, VK_SHADER_STAGE_COMPUTE_BIT);
  return mComputeShaders[name];
}

//-----------------------------------------------

Shader_t const &Vonk::getComputeShader(std::string const &name)
{
  AbortIfMsg(mComputeShaders.count(name) < 1, "Compute Shader Not Found!");
  return mComputeShaders[name];
}

//-----------------------------------------------

//
// // PIPELINEs
//

//-----------------------------------------------

void Vonk::addPipeline(DrawPipelineData_t const &ci)
{
  mPipelinesCI.push_back(ci);
  mPipelines.push_back(vonk::createPipeline(
    {},
    mPipelinesCI.back(),
    mSwapChain,
    mDevice.handle,
    mDevice.commandPool,
    mSwapChain.defaultRenderPass,
    mSwapChain.defaultFrameBuffers));
};

//-----------------------------------------------

//
// // FRAMEs
//

//-----------------------------------------------

void Vonk::waitDevice() { vkDeviceWaitIdle(mDevice.handle); }

//-----------------------------------------------

void Vonk::drawFrame()
{
  if (mPipelines.empty()) return;

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
    LogError("Failed to acquire swap chain image!");
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
  VkCheck(vkQueueSubmit(mDevice.graphicsQ, 1, &submitInfo, mSwapChain.fences.submit[currFrame]));

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
  auto const presentRet = vkQueuePresentKHR(mDevice.presentQ, &presentInfo);
  // (3.3) Validate swapchain state
  if (presentRet == VK_ERROR_OUT_OF_DATE_KHR || presentRet == VK_SUBOPTIMAL_KHR || vonk::window::framebufferResized) {
    vonk::window::framebufferResized = false;  // move this variable to vonk::Vonk
    recreateSwapChain();
  } else if (presentRet != VK_SUCCESS) {
    LogError("Failed to present swap chain image!");
  }

  //=====
  //=====   EXTRA TASKS
  currFrame = (currFrame + 1) % sInFlightMaxFrames;
}

//-----------------------------------------------

//
// // SWAPCHAIN
//

//-----------------------------------------------

void Vonk::destroySwapChainDependencies()
{
  for (auto &pipeline : mPipelines) {
    // . Command Buffers
    auto &cb = pipeline.commandBuffers;
    if (cb.size() > 0) { vkFreeCommandBuffers(mDevice.handle, mDevice.commandPool, GetSizeU32(cb), GetData(cb)); }

    // // . Frame Buffers
    // for (size_t i = 0; i < pipeline.frameBuffers.size(); ++i) {
    //   if (pipeline.frameBuffers[i] != mSwapChain.defaultFrameBuffers[i]) {
    //     vkDestroyFramebuffer(mDevice.handle, pipeline.frameBuffers[i], nullptr);
    //   }
    // }
  }
}

//-----------------------------------------------

void Vonk::recreateSwapChain()
{
  vkDeviceWaitIdle(mDevice.handle);

  destroySwapChainDependencies();
  mSwapChain = vonk::createSwapChain(mDevice, mSwapChain);

  for (size_t i = 0; i < mPipelines.size(); ++i) {
    // vonk::createPipeline(mDevice, mPipelines[i], currSwapChainExtent2D, [renderpass], [framebuffer]);
    vonk::createPipeline(
      mPipelines[i],
      mPipelinesCI[i],
      mSwapChain,
      mDevice.handle,
      mDevice.commandPool,
      mSwapChain.defaultRenderPass,
      mSwapChain.defaultFrameBuffers);
  }
}

//-----------------------------------------------

//
// // FLOW
//

//-----------------------------------------------

void Vonk::init()
{
  // . Validation layers support
  AbortIfMsg(!vonk::checkValidationLayersSupport(mInstance.layers), "Required Layers Not Found!");
  // . Create Instance : VkInstance, VkDebugMessenger, VkSurfaceKHR
  mInstance = vonk::createInstance(vonk::window::title.c_str(), VK_API_VERSION_1_2);
  // . Pick Gpu (aka: physical device)
  mGpu = vonk::pickGpu(mInstance, true, true, false, false);
  // . Create Device (aka: gpu-manager / logical-device)
  mDevice = vonk::createDevice(mInstance, mGpu);
  // . Create SwapChain
  mSwapChain = vonk::createSwapChain(mDevice, mSwapChain);
}

//-----------------------------------------------

void Vonk::cleanup()
{
  // . Pipelines

  for (auto &pipeline : mPipelines) { vonk::destroyPipeline(mSwapChain, pipeline); }

  // . Meshes

  for (auto &[k, m] : mMeshes) {
    mRemovedMeshes.push(k);
    vonk::destroyMesh(mDevice, m);
  }

  // . Shaders

  for (auto const &[k, ds] : mDrawShaders) { vonk::destroyDrawShader(mDevice, ds); }
  for (auto const &[k, cs] : mComputeShaders) { vkDestroyShaderModule(mDevice.handle, cs.module, nullptr); }

  // . Context ¿?

  vonk::destroySwapChain(mSwapChain, false);
  vonk::destroyDevice(mDevice);
  vonk::destroyInstance(mInstance);
}

//-----------------------------------------------

}  // namespace vonk
