#include "VoVulkanBase.h"
// #include "VoVulkanCreators.h"
#include "VoVulkanUtils.h"
#include "VoWindow.h"

namespace vo::vulkan
{  //

//-----------------------------------------------

// @DANI move this to 'VoVulkanUtils' also rename it to 'VoVulkanTools'
Gpu_t getGpuDataAndScore(VkPhysicalDevice pd, VkSurfaceKHR surface)
{
  Gpu_t gpu;
  gpu.handle = pd;
  vkGetPhysicalDeviceFeatures(pd, &gpu.features);
  vkGetPhysicalDeviceProperties(pd, &gpu.properties);
  vkGetPhysicalDeviceMemoryProperties(pd, &gpu.memory);

  auto const queueIndicesOpt        = vku::queue::findIndices(pd, surface);
  bool const queueIndicesIsComplete = vku::queue::isComplete(queueIndicesOpt);
  if (queueIndicesIsComplete) { gpu.queuesIndices = vku::queue::unrollOptionals(queueIndicesOpt); }

  if (
    !queueIndicesIsComplete                                                   //
    or vku::swapchain::isEmpty(pd, surface)                                   //
    or !vku::others::checkDeviceExtensionsSupport(pd, vo::sDeviceExtensions)  //
    // or !gpu.features.geometryShader                                           //
  ) {
    return gpu;
  }

  uint32_t const isDiscreteGPU = (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
  gpu.score                    = (1000 * isDiscreteGPU) + gpu.properties.limits.maxImageDimension2D;

  return gpu;
};

//----------------------------------------------- !!

void Base::waitDevice() { vkDeviceWaitIdle(mDevice.handle); }

//-----------------------------------------------

void Base::drawFrame()
{
  static size_t currFrame = 0;

  //=====
  //=====   PRECONDITIONS

  vkWaitForFences(mDevice.handle, 1, &mSync.fences.submit[currFrame], VK_TRUE, UINT64_MAX);

  //=====
  //=====   1. GET NEXT IMAGE TO PROCESS

  // (1.1) Acquiere next image
  uint32_t   imageIndex;
  auto const acquireRet = vkAcquireNextImageKHR(
    mDevice.handle,
    mSwapChain.handle,
    UINT64_MAX,
    mSync.semaphores.present[currFrame],
    VK_NULL_HANDLE,
    &imageIndex);

  // (1.2) Validate the swapchain state
  if (acquireRet == VK_ERROR_OUT_OF_DATE_KHR) {
    swapchainReCreate();
    return;
  } else if (acquireRet != VK_SUCCESS && acquireRet != VK_SUBOPTIMAL_KHR) {
    vo__err("Failed to acquire swap chain image!");
  }

  // (1.3) Check if a previous frame is using this image (i.e. there is its fence to wait on)
  if (mSync.fences.acquire[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(mDevice.handle, 1, &mSync.fences.acquire[imageIndex], VK_TRUE, UINT64_MAX);
  }
  // (1.4) Mark the image as now being in use by this frame
  mSync.fences.acquire[imageIndex] = mSync.fences.submit[currFrame];

  //=====
  //=====   2. DRAW : Graphics Queue

  // (2.1) Sync objects
  VkPipelineStageFlags const waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore const          waitSemaphores[]   = { mSync.semaphores.present[currFrame] };
  VkSemaphore const          signalSemaphores[] = { mSync.semaphores.render[currFrame] };

  // (2.2) Submit info
  VkSubmitInfo const submitInfo {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pWaitDstStageMask    = waitStages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &mScenes[activeScene].commandBuffers[imageIndex],
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = waitSemaphores,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = signalSemaphores,
  };

  // (2.3) Reset fences right before asking for draw
  vkResetFences(mDevice.handle, 1, &mSync.fences.submit[currFrame]);
  vku__check(vkQueueSubmit(mDevice.queues.graphics, 1, &submitInfo, mSync.fences.submit[currFrame]));

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
  if (presentRet == VK_ERROR_OUT_OF_DATE_KHR || presentRet == VK_SUBOPTIMAL_KHR || vo::window::framebufferResized) {
    vo::window::framebufferResized = false;
    swapchainReCreate();
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
  //=====   SETTINGS

  if (!vku::others::checkValidationLayersSupport(vo::sValidationLayers)) {
    vo__abort("Validation layers requested, but not available!");
  }

  //=====
  //=====   CREATE INSTANCE

  auto const instanceExtensions = vku::others::getInstanceExtensions();

  VkApplicationInfo const appInfo {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = vo::window::title.c_str(),
    .pEngineName        = vo::window::title.c_str(),
    .applicationVersion = VK_API_VERSION_1_2,
    .engineVersion      = VK_API_VERSION_1_2,
    .apiVersion         = VK_API_VERSION_1_2,
  };

  VkInstanceCreateInfo const instanceCI {
    .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    // Esential extensions
    .enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size()),
    .ppEnabledExtensionNames = instanceExtensions.data(),
    // Layers
    .enabledLayerCount   = static_cast<uint32_t>(vo::sValidationLayers.size()),
    .ppEnabledLayerNames = vo::sValidationLayers.data(),
  };
  // auto const instanceCI = vku::init::instance(vo::window::title.c_str(), instanceExtensions, vo::sValidationLayers);
  vku__check(vkCreateInstance(&instanceCI, nullptr, &mInstance.handle));

  //=====
  //=====   CREATE DEBUG MESSENGER

  vku::debugmessenger::create(mInstance.handle, mInstance.debugger);

  //=====
  //=====   CREATE SURFACE  (@DANI: Check for headless Vulkan in a future)

  mInstance.surface = vo::window::createSurface(mInstance.handle);

  //=====
  //=====   PICK GPU (PHYSICAL DEVICE)

  uint32_t gpuCount = 0;
  vkEnumeratePhysicalDevices(mInstance.handle, &gpuCount, nullptr);
  std::vector<VkPhysicalDevice> gpus(gpuCount);
  vkEnumeratePhysicalDevices(mInstance.handle, &gpuCount, gpus.data());

  uint32_t maxScore = 0;
  for (const auto &pd : gpus) {
    auto const gpu = getGpuDataAndScore(pd, mInstance.surface);
    if (gpu.score > maxScore) {
      mGpu     = gpu;
      maxScore = gpu.score;
    }
  }
  if (maxScore < 1) { vo__abort("Suitable GPU not found!"); }

  //=====
  //=====   CREATE DEVICE  (GPU MANAGER / LOGICAL DEVICE)

  // . Queues' Create Infos
  // NOTE: If graphics, compute or present queues comes from the same family register it only once
  float const                          queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCIs;
  for (uint32_t queueFamily : vku::queue::getUniqueIndices(mGpu.queuesIndices)) {
    queueCIs.push_back(VkDeviceQueueCreateInfo {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount       = 1,
      .pQueuePriorities = &queuePriority,
    });
  }

  // . Device's Create Info
  VkDeviceCreateInfo const deviceCI {
    .sType            = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pEnabledFeatures = &mGpu.features,
    // Queues info
    .queueCreateInfoCount = vku__getSize(queueCIs),
    .pQueueCreateInfos    = vku__getData(queueCIs),
    // Set device extensions
    .enabledExtensionCount   = vku__getSize(vo::sDeviceExtensions),
    .ppEnabledExtensionNames = vku__getData(vo::sDeviceExtensions),
    // Set device validation layers
    .enabledLayerCount   = vku__getSize(vo::sValidationLayers),
    .ppEnabledLayerNames = vku__getData(vo::sValidationLayers),
  };

  // . Create Device !
  vku__check(vkCreateDevice(mGpu.handle, &deviceCI, nullptr, &mDevice.handle));

  // . Pick required queues
  mDevice.queues        = vku::queue::findQueues(mDevice.handle, mGpu.queuesIndices);
  mDevice.queuesIndices = mGpu.queuesIndices;

  //=====
  //=====   COMMAND POOL  :  Maybe move this out and create one per thread

  VkCommandPoolCreateInfo const commandPoolCI {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = mDevice.queuesIndices.graphics,
    .flags            = 0,  // Optional
  };
  vku__check(vkCreateCommandPool(mDevice.handle, &commandPoolCI, nullptr, &mDevice.commandPool));

  //=====
  //=====   SYNC OBJECTS  :  Maybe this is SwapChain resposibility...

  mSync.semaphores.render.resize(sInFlightMaxFrames);
  mSync.semaphores.present.resize(sInFlightMaxFrames);
  mSync.fences.submit.resize(sInFlightMaxFrames);

  static VkSemaphoreCreateInfo const semaphoreCI {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  static VkFenceCreateInfo const fenceCI {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,  // Initialize on creation to avoid 'freezes'
  };

  for (size_t i = 0; i < sInFlightMaxFrames; ++i) {
    vku__check(vkCreateSemaphore(mDevice.handle, &semaphoreCI, nullptr, &mSync.semaphores.render[i]));
    vku__check(vkCreateSemaphore(mDevice.handle, &semaphoreCI, nullptr, &mSync.semaphores.present[i]));
    vku__check(vkCreateFence(mDevice.handle, &fenceCI, nullptr, &mSync.fences.submit[i]));
  }

  //=====
  //=====   CREATE SWAPCHAIN  (DEFAULT "FBO")

  swapchainCreate();

}  // void Base::init()

//-----------------------------------------------

void Base::cleanup()
{
  //=====
  //=====   USER DEFINED

  for (auto &scene : mScenes) {
    for (auto framebuffer : scene.frameBuffers) { vkDestroyFramebuffer(mDevice.handle, framebuffer, nullptr); }

    auto &cb = scene.commandBuffers;
    if (cb.size() > 0) {
      vkFreeCommandBuffers(mDevice.handle, mDevice.commandPool, vku__getSize(cb), vku__getData(cb));
    }

    vkDestroyPipeline(mDevice.handle, scene.handle, nullptr);
    vkDestroyPipelineLayout(mDevice.handle, scene.layout, nullptr);
    vkDestroyRenderPass(mDevice.handle, scene.renderpass, nullptr);  // after: mPipelineLayout

    for (auto [name, shaderModule] : scene.shaderModules) {
      vkDestroyShaderModule(mDevice.handle, shaderModule, nullptr);
    }
  }

  //=====
  //=====   DEVICE

  swapchainCleanUp();
  vkDestroySwapchainKHR(mDevice.handle, mSwapChain.handle, nullptr);
  for (size_t i = 0; i < sInFlightMaxFrames; i++) {
    vkDestroySemaphore(mDevice.handle, mSync.semaphores.render[i], nullptr);
    vkDestroySemaphore(mDevice.handle, mSync.semaphores.present[i], nullptr);
    vkDestroyFence(mDevice.handle, mSync.fences.submit[i], nullptr);
  }
  vkDestroyCommandPool(mDevice.handle, mDevice.commandPool, nullptr);
  vkDestroyDevice(mDevice.handle, nullptr);

  //=====
  //=====   INSTANCE

  vku::debugmessenger::destroy(mInstance.handle, mInstance.debugger);
  vkDestroySurfaceKHR(mInstance.handle, mInstance.surface, nullptr);
  vkDestroyInstance(mInstance.handle, nullptr);

}  // void Base::cleanup()

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

void Base::swapchainCreate()
{
  // . Get supported settings
  mSwapChain.settings = vku::swapchain::getSettings(
    mGpu.handle,
    mInstance.surface,
    SwapShainSettings_t { true, vo::window::getFramebufferSize() });

  // . Create SwapChain
  bool const        gpDiffQueue = mDevice.queues.graphics != mDevice.queues.present;
  std::vector const gpIndices   = { mDevice.queuesIndices.graphics, mDevice.queuesIndices.present };

  VkSwapchainCreateInfoKHR const swapchainCI {
    .sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = mInstance.surface,
    .clipped = VK_TRUE,  // -> TRUE: Don't care about obscured pixels

    .presentMode    = mSwapChain.settings.presentMode,
    .preTransform   = mSwapChain.settings.preTransformFlag,    // -> i.e. Globally flips 90 degrees
    .compositeAlpha = mSwapChain.settings.compositeAlphaFlag,  // -> Blending with other windows, Opaque = None/Ignore

    .imageArrayLayers = 1,  // -> Always 1 unless you are developing a stereoscopic 3D application.
    .minImageCount    = mSwapChain.settings.minImageCount,
    .imageExtent      = mSwapChain.settings.extent2D,
    .imageFormat      = mSwapChain.settings.surfaceFormat.format,
    .imageColorSpace  = mSwapChain.settings.surfaceFormat.colorSpace,
    .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | mSwapChain.settings.extraImageUsageFlags,

    .imageSharingMode      = gpDiffQueue ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = gpDiffQueue ? vku__getSize(gpIndices) : 0,
    .pQueueFamilyIndices   = gpDiffQueue ? vku__getData(gpIndices) : nullptr,

    .oldSwapchain = mSwapChain.handle  // -> Better cleanup + ensure that we can still present already acquired images
  };
  vku__check(vkCreateSwapchainKHR(mDevice.handle, &swapchainCI, nullptr, &mSwapChain.handle));

  // . Get SwapChain Images
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(mDevice.handle, mSwapChain.handle, &imageCount, nullptr);
  mSwapChain.images.resize(imageCount);
  vkGetSwapchainImagesKHR(mDevice.handle, mSwapChain.handle, &imageCount, mSwapChain.images.data());

  mSync.fences.acquire.resize(mSwapChain.images.size(), VK_NULL_HANDLE);

  // . Get Image-Views for that Images
  mSwapChain.views.resize(mSwapChain.images.size());
  for (size_t i = 0; i < mSwapChain.images.size(); i++) {
    VkImageViewCreateInfo const imageViewCI {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = mSwapChain.images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = mSwapChain.settings.surfaceFormat.format,
      // * How to read RGBA
      .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
      // * The subresourceRange field describes what the image's purpose is and which part to be accessed.
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,  // -> MIP-MAPing the texture [??]
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };
    vku__check(vkCreateImageView(mDevice.handle, &imageViewCI, nullptr, &mSwapChain.views[i]));
  }
}

//-----------------------------------------------

void Base::swapchainCleanUp()
{
  // for (auto &scene : mScenes) {
  //   for (auto framebuffer : scene.frameBuffers) { vkDestroyFramebuffer(mDevice.handle, framebuffer, nullptr); }

  //   auto &cb = scene.commandBuffers;
  //   if (cb.size() > 0) {
  //     vkFreeCommandBuffers(mDevice.handle, mDevice.commandPool, vku__getSize(cb), vku__getData(cb));
  //   }

  //   vkDestroyPipeline(mDevice.handle, scene.handle, nullptr);
  //   vkDestroyPipelineLayout(mDevice.handle, scene.layout, nullptr);
  //   vkDestroyRenderPass(mDevice.handle, scene.renderpass, nullptr);  // after: mPipelineLayout
  // }

  for (auto imageView : mSwapChain.views) { vkDestroyImageView(mDevice.handle, imageView, nullptr); }

  // NOTE: Do not destroy here the swap-chain in order to use it as .swapchainOld parameter and have a better exchange
  // on swapchain recreations.
}

//-----------------------------------------------

void Base::swapchainReCreate()
{
  vkDeviceWaitIdle(mDevice.handle);
  swapchainCleanUp();
  swapchainCreate();
  // graphicspipelineCreateHARDCODED();
  // for (auto &scene : mScenes) {
  //   // mGraphicsPipeline.recreate();
  // }
}

//-----------------------------------------------

}  // namespace vo::vulkan
