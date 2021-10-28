#include "VonkBase.h"
#include "VonkCreate.h"
#include "VonkTools.h"
#include "VonkWindow.h"

namespace vonk
{  //

//-----------------------------------------------

// @DANI move this to 'VonkUtils' also rename it to 'VonkTools'
Gpu_t getGpuDataAndScore(VkPhysicalDevice pd, VkSurfaceKHR surface)
{
  Gpu_t gpu;
  gpu.handle = pd;
  vkGetPhysicalDeviceFeatures(pd, &gpu.features);
  vkGetPhysicalDeviceProperties(pd, &gpu.properties);
  vkGetPhysicalDeviceMemoryProperties(pd, &gpu.memory);

  auto const queueIndicesOpt        = vonk::queue::findIndices(pd, surface);
  bool const queueIndicesIsComplete = vonk::queue::isComplete(queueIndicesOpt);
  if (queueIndicesIsComplete) { gpu.queuesIndices = vonk::queue::unrollOptionals(queueIndicesOpt); }

  if (
    !queueIndicesIsComplete                                                    //
    or vonk::swapchain::isEmpty(pd, surface)                                   //
    or !vonk::others::checkDeviceExtensionsSupport(pd, vo::sDeviceExtensions)  //
    // or !gpu.features.geometryShader                                           //
  ) {
    return gpu;
  }

  uint32_t const isDiscreteGPU = (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
  gpu.score                    = (1000 * isDiscreteGPU) + gpu.properties.limits.maxImageDimension2D;

  return gpu;
};

//----------------------------------------------- !!

void Base::addPipeline(PipelineCreateInfo_t const &ci)
{
  mPipelinesCI.push_back(ci);
  mPipelines.push_back(vonk::create::pipeline(mPipelinesCI.back(), mSwapChain, mDevice.handle, mDevice.commandPool));
};

//-----------------------------------------------

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
    .pCommandBuffers      = &mPipelines[mActivePipeline].commandBuffers[imageIndex],
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = waitSemaphores,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = signalSemaphores,
  };

  // (2.3) Reset fences right before asking for draw
  vkResetFences(mDevice.handle, 1, &mSync.fences.submit[currFrame]);
  vonk__check(vkQueueSubmit(mDevice.queues.graphics, 1, &submitInfo, mSync.fences.submit[currFrame]));

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

  if (!vonk::others::checkValidationLayersSupport(vo::sValidationLayers)) {
    vo__abort("Validation layers requested, but not available!");
  }

  //=====
  //=====   CREATE INSTANCE
  mInstance.handle = vonk::create::instance(
    vonk::window::title.c_str(),
    vonk::others::getInstanceExtensions(),
    vo::sValidationLayers,
    VK_API_VERSION_1_2);

  //=====
  //=====   CREATE DEBUG MESSENGER

  vonk::debugmessenger::create(mInstance.handle, mInstance.debugger);

  //=====
  //=====   CREATE SURFACE  (@DANI: Check for headless Vulkan in a future)

  mInstance.surface = vonk::window::createSurface(mInstance.handle);

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
  //=====   CAPTURE DATA : Related with GPU and SURFACE

  mSwapChain.settings = vonk::swapchain::getSettings(
    mGpu.handle,
    mInstance.surface,
    SwapShainSettings_t { true, vonk::window::getFramebufferSize() });

  //=====
  //=====   CREATE DEVICE  (GPU MANAGER / LOGICAL DEVICE)

  // . Queues' Create Infos
  // NOTE: If graphics, compute or present queues comes from the same family register it only once
  float const                          queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCIs;
  for (uint32_t queueFamily : vonk::queue::getUniqueIndices(mGpu.queuesIndices)) {
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
    .queueCreateInfoCount = vonk__getSize(queueCIs),
    .pQueueCreateInfos    = vonk__getData(queueCIs),
    // Set device extensions
    .enabledExtensionCount   = vonk__getSize(vo::sDeviceExtensions),
    .ppEnabledExtensionNames = vonk__getData(vo::sDeviceExtensions),
    // Set device validation layers
    .enabledLayerCount   = vonk__getSize(vo::sValidationLayers),
    .ppEnabledLayerNames = vonk__getData(vo::sValidationLayers),
  };
  // . Create Device !
  vonk__check(vkCreateDevice(mGpu.handle, &deviceCI, nullptr, &mDevice.handle));

  // . Pick required queues
  mDevice.queues        = vonk::queue::findQueues(mDevice.handle, mGpu.queuesIndices);
  mDevice.queuesIndices = mGpu.queuesIndices;

  //=====
  //=====   COMMAND POOL  :  Maybe move this out and create one per thread

  VkCommandPoolCreateInfo const commandPoolCI {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = mDevice.queuesIndices.graphics,
    .flags            = 0,  // Optional
  };
  vonk__check(vkCreateCommandPool(mDevice.handle, &commandPoolCI, nullptr, &mDevice.commandPool));

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
    vonk__check(vkCreateSemaphore(mDevice.handle, &semaphoreCI, nullptr, &mSync.semaphores.render[i]));
    vonk__check(vkCreateSemaphore(mDevice.handle, &semaphoreCI, nullptr, &mSync.semaphores.present[i]));
    vonk__check(vkCreateFence(mDevice.handle, &fenceCI, nullptr, &mSync.fences.submit[i]));
  }

  //=====
  //=====   DEFAULT RENDERPASS

  RenderPassData_t rpd;
  rpd.attachments = {
    {
      .format         = mSwapChain.settings.colorFormat,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    },
    {
      .format         = mSwapChain.settings.depthFormat,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    },
  };
  rpd.attachmentRefs = {
    {
      // color
      .attachment = 0,
      .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    },
    {
      // depth
      .attachment = 1,
      .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    },
  };
  rpd.subpassDescs = {
    {
      .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount    = 1,
      .pColorAttachments       = &rpd.attachmentRefs[0],
      .pDepthStencilAttachment = &rpd.attachmentRefs[1],
      .inputAttachmentCount    = 0,
      .pInputAttachments       = nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments    = nullptr,
      .pResolveAttachments     = nullptr,
    },
  };
  rpd.subpassDeps = {
    // 0
    {
      .srcSubpass      = VK_SUBPASS_EXTERNAL,
      .dstSubpass      = 0,
      .srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
      .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,

    },
    // 1
    {
      .srcSubpass      = 0,
      .dstSubpass      = VK_SUBPASS_EXTERNAL,
      .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    },
  };
  mDefaultRenderPass = vonk::create::renderpass(mDevice.handle, rpd);

  //=====
  //=====   CREATE SWAPCHAIN  (DEFAULT "FBO")

  swapchainCreate();

}  // namespace vonk

//-----------------------------------------------

void Base::cleanup()
{
  //=====
  //=====   USER DEFINED

  for (auto &pipeline : mPipelines) {
    vkDestroyPipeline(mDevice.handle, pipeline.handle, nullptr);
    vkDestroyPipelineLayout(mDevice.handle, pipeline.layout, nullptr);
    vkDestroyRenderPass(mDevice.handle, pipeline.renderpass, nullptr);  // after: mPipelineLayout

    for (auto [name, shaderModule] : pipeline.shaderModules) {
      vkDestroyShaderModule(mDevice.handle, shaderModule, nullptr);
    }
  }

  //=====
  //=====   DEFAULTS

  vkDestroyRenderPass(mDevice.handle, mDefaultRenderPass, nullptr);

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

  vonk::debugmessenger::destroy(mInstance.handle, mInstance.debugger);
  vkDestroySurfaceKHR(mInstance.handle, mInstance.surface, nullptr);
  vkDestroyInstance(mInstance.handle, nullptr);

}  // void Base::cleanup()

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

void Base::swapchainCreate()
{
  // // . Get supported settings
  // mSwapChain.settings = vonk::swapchain::getSettings(
  //   mGpu.handle,
  //   mInstance.surface,
  //   SwapShainSettings_t { true, vonk::window::getFramebufferSize() });

  // . Create SwapChain
  static bool const        gpDiffQueue = mDevice.queues.graphics != mDevice.queues.present;
  static std::vector const gpIndices   = { mDevice.queuesIndices.graphics, mDevice.queuesIndices.present };

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
    .imageFormat      = mSwapChain.settings.colorFormat,
    .imageColorSpace  = mSwapChain.settings.colorSpace,
    .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | mSwapChain.settings.extraImageUsageFlags,

    .imageSharingMode      = gpDiffQueue ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = gpDiffQueue ? vonk__getSize(gpIndices) : 0,
    .pQueueFamilyIndices   = gpDiffQueue ? vonk__getData(gpIndices) : nullptr,

    .oldSwapchain = mSwapChain.handle  // -> Better cleanup + ensure that we can still present already acquired images
  };
  vonk__check(vkCreateSwapchainKHR(mDevice.handle, &swapchainCI, nullptr, &mSwapChain.handle));

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
      .format   = mSwapChain.settings.colorFormat,
      // * How to read RGBA
      .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
      // * The subresourceRange field describes what the image's purpose is and which part to be accessed.
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,  // -> MIP-MAPing the texture [??]
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };
    vonk__check(vkCreateImageView(mDevice.handle, &imageViewCI, nullptr, &mSwapChain.views[i]));
  }

  // // . Setup default framebuffers
  // VkImageView attachments[1];

  // // Depth/Stencil attachment is the same for all frame buffers
  // attachments[1] = depthStencil.view;

  // VkFramebufferCreateInfo frameBufferCreateInfo = {};
  // frameBufferCreateInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  // frameBufferCreateInfo.pNext                   = NULL;
  // frameBufferCreateInfo.renderPass              = renderPass;
  // frameBufferCreateInfo.attachmentCount         = 2;
  // frameBufferCreateInfo.pAttachments            = attachments;
  // frameBufferCreateInfo.width                   = width;
  // frameBufferCreateInfo.height                  = height;
  // frameBufferCreateInfo.layers                  = 1;

  // // Create frame buffers for every swap chain image
  // frameBuffers.resize(swapChain.imageCount);
  // for (uint32_t i = 0; i < frameBuffers.size(); i++) {
  //   attachments[0] = swapChain.buffers[i].view;
  //   VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
  // }
}

//-----------------------------------------------

void Base::swapchainCleanUp()
{
  // for (auto &scene : mScenes) {
  //   for (auto framebuffer : scene.frameBuffers) { vkDestroyFramebuffer(mDevice.handle, framebuffer, nullptr); }

  //   auto &cb = scene.commandBuffers;
  //   if (cb.size() > 0) {
  //     vkFreeCommandBuffers(mDevice.handle, mDevice.commandPool, vonk__getSize(cb), vonk__getData(cb));
  //   }

  //   vkDestroyPipeline(mDevice.handle, scene.handle, nullptr);
  //   vkDestroyPipelineLayout(mDevice.handle, scene.layout, nullptr);
  //   vkDestroyRenderPass(mDevice.handle, scene.renderpass, nullptr);  // after: mPipelineLayout
  // }

  for (auto &pipeline : mPipelines) {
    auto &cb = pipeline.commandBuffers;
    if (cb.size() > 0) {
      vkFreeCommandBuffers(mDevice.handle, mDevice.commandPool, vonk__getSize(cb), vonk__getData(cb));
    }

    for (auto framebuffer : pipeline.frameBuffers) { vkDestroyFramebuffer(mDevice.handle, framebuffer, nullptr); }
  }

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
  for (size_t i = 0; i < mPipelines.size(); ++i) {
    // auto &ci = mPipelinesCI[i];
    vonk::create::pipelineRecreation(
      mPipelines[i],
      true,
      mPipelinesCI[i],
      mSwapChain,
      mDevice.handle,
      mDevice.commandPool);
  }
}

//-----------------------------------------------

}  // namespace vonk
