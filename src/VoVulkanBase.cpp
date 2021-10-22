#include "VoVulkanBase.h"
#include "VoVulkanInit.h"
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
    .pCommandBuffers      = &mDrawPipeline.commandBuffers[imageIndex],
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
    .queueCreateInfoCount = vku__castsize(queueCIs.size()),
    .pQueueCreateInfos    = queueCIs.data(),
    // Set device extensions
    .enabledExtensionCount   = vku__castsize(vo::sDeviceExtensions.size()),
    .ppEnabledExtensionNames = vo::sDeviceExtensions.data(),
    // Set device validation layers
    .enabledLayerCount   = vku__castsize(vo::sValidationLayers.size()),
    .ppEnabledLayerNames = vo::sValidationLayers.data(),
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
  graphicspipelineCreateHARDCODED();

}  // void Base::init()

//-----------------------------------------------

void Base::cleanup()
{
  //=====
  //=====   USER DEFINED

  for (auto [name, shaderModule] : mDrawPipeline.shaderModules) {
    vkDestroyShaderModule(mDevice.handle, shaderModule, nullptr);
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

void Base::swapchainCreate()
{
  // . Get supported settings
  mSwapChain.settings = vku::swapchain::getSettings(
    mGpu.handle,
    mInstance.surface,
    SwapShainSettings_t { true, vo::window::getFramebufferSize() });

  // . Create SwapChain
  bool const        gpSameQueue = mDevice.queues.graphics == mDevice.queues.present;
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

    .imageSharingMode      = gpSameQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
    .queueFamilyIndexCount = gpSameQueue ? 0 : vku__castsize(gpIndices.size()),
    .pQueueFamilyIndices   = gpSameQueue ? nullptr : gpIndices.data(),

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
  for (auto framebuffer : mDrawPipeline.frameBuffers) { vkDestroyFramebuffer(mDevice.handle, framebuffer, nullptr); }

  auto &cb = mDrawPipeline.commandBuffers;
  if (cb.size() > 0) { vkFreeCommandBuffers(mDevice.handle, mDevice.commandPool, vku__castsize(cb.size()), cb.data()); }

  vkDestroyPipeline(mDevice.handle, mDrawPipeline.handle, nullptr);
  vkDestroyPipelineLayout(mDevice.handle, mDrawPipeline.layout, nullptr);
  vkDestroyRenderPass(mDevice.handle, mDrawPipeline.renderpass, nullptr);  // after: mPipelineLayout

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
  graphicspipelineCreateHARDCODED();
  // mGraphicsPipeline.recreate();
}

//-----------------------------------------------

// @DANI this is temporal and should be replaced by user code to define multiple scenes...
void Base::graphicspipelineCreateHARDCODED()
{
  //=====
  //=====   CREATE SUB-PASS

  // . Attachment References
  VkAttachmentReference const subpassColorAttRef {
    .attachment = 0,
    .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  // . Description
  VkSubpassDescription const subpassDesc {
    // Every subpass references one or more of the attachments using VkAttachmentReference.
    // This allows attachment resuse between render-subpasses.
    .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,  // also could be: Compute or Raytracing
    .colorAttachmentCount = 1,
    .pColorAttachments    = &subpassColorAttRef,
  };

  // . Subpass dependencies (required for user-defined subpasses and 'implicit' ones)
  VkSubpassDependency subpassDeps {
    .srcSubpass    = VK_SUBPASS_EXTERNAL,
    .dstSubpass    = 0,
    .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  //=====
  //=====   CREATE RENDER-PASS

  // . Attachment
  VkAttachmentDescription const renderpassColorAtt {
    .format  = mSwapChain.settings.surfaceFormat.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,  // VK_SAMPLE_COUNT_1_BIT for No-Multisampling
    // * Ops
    // VkAttachmentDescription.loadOp
    //  - VK_ATTACHMENT_LOAD_OP_LOAD      : Preserve the existing contents of the attachment.
    //  - VK_ATTACHMENT_LOAD_OP_CLEAR     : Clear the values to a constant (clear color) at the start.
    //  - VK_ATTACHMENT_LOAD_OP_DONT_CARE : Existing contents are undefined; we don't care about them.
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    // VkAttachmentDescription.storeOp
    //  - VK_ATTACHMENT_STORE_OP_STORE     : Rendered contents will be stored in memory and can be read later.
    //  - VK_ATTACHMENT_STORE_OP_DONT_CARE : Contents of the framebuffer will be undefined after the rendering op.
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    // * Stencil
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    // * Layouts
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  // . RenderPass create info
  VkRenderPassCreateInfo const renderpassCI {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments    = &renderpassColorAtt,
    .subpassCount    = 1,
    .pSubpasses      = &subpassDesc,
    .dependencyCount = 1,
    .pDependencies   = &subpassDeps,
  };

  // . Creation
  vku__check(vkCreateRenderPass(mDevice.handle, &renderpassCI, nullptr, &mDrawPipeline.renderpass));

  //=====
  //=====   CREATE IMAGE-VIEWS' FRAMEBUFFERS FOR CURRENT THIS RENDERPASS

  mDrawPipeline.frameBuffers.resize(mSwapChain.views.size());

  for (size_t i = 0; i < mSwapChain.views.size(); ++i) {
    VkImageView const             attachments[] = { mSwapChain.views[i] };
    VkFramebufferCreateInfo const framebufferCI {
      .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass      = mDrawPipeline.renderpass,
      .attachmentCount = 1,  // Modify this for MRT ??
      .pAttachments    = attachments,
      .width           = mSwapChain.settings.extent2D.width,
      .height          = mSwapChain.settings.extent2D.height,
      .layers          = 1,
    };
    vku__check(vkCreateFramebuffer(mDevice.handle, &framebufferCI, nullptr, &mDrawPipeline.frameBuffers[i]));
  }

  //=====
  //=====   FIXED FUNCTIONS

  // . Viewport
  // ..  Base
  VkViewport const viewport {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = (float)mSwapChain.settings.extent2D.width,
    .height   = (float)mSwapChain.settings.extent2D.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  // ..  Scissor
  VkRect2D const scissor {
    .offset = { 0, 0 },
    .extent = mSwapChain.settings.extent2D,
  };
  // ..  CreateInfo
  VkPipelineViewportStateCreateInfo const viewportStateCI {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports    = &viewport,
    .scissorCount  = 1,
    .pScissors     = &scissor,
  };

  // . Rasterization
  VkPipelineRasterizationStateCreateInfo const rasterizationStateCI {
    .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,  // or _LINE / _POINT as always
    .cullMode    = VK_CULL_MODE_BACK_BIT,
    .frontFace   = VK_FRONT_FACE_CLOCKWISE,  // or VK_FRONT_FACE_COUNTER_CLOCKWISE
    .lineWidth   = 1.0f,
    // Disable any output
    .rasterizerDiscardEnable = VK_FALSE,
    // Usefull for Shadow-Maps
    .depthClampEnable        = VK_FALSE,
    .depthBiasEnable         = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,  // Optional
    .depthBiasClamp          = 0.0f,  // Optional
    .depthBiasSlopeFactor    = 0.0f,  // Optional
  };

  // . Multisampling : Currently OFF
  VkPipelineMultisampleStateCreateInfo const multisamplingCI {
    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable   = VK_FALSE,
    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
    .minSampleShading      = 1.0f,      // Optional
    .pSampleMask           = nullptr,   // Optional
    .alphaToCoverageEnable = VK_FALSE,  // Optional
    .alphaToOneEnable      = VK_FALSE,  // Optional
  };

  // . Depth / Stencil
  VkPipelineDepthStencilStateCreateInfo const depthstencilCI {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .stencilTestEnable = VK_FALSE,
    .depthWriteEnable  = VK_TRUE,
    .depthTestEnable   = VK_TRUE,
    .depthCompareOp    = VkCompareOp::VK_COMPARE_OP_LESS,
  };

  // . Blending
  // ..  Define blending logic
  VkPipelineColorBlendAttachmentState const colorblendStateAtt {
    .colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    // * Color pass through
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional
    .colorBlendOp        = VK_BLEND_OP_ADD,       // Optional
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional
    .alphaBlendOp        = VK_BLEND_OP_ADD,       // Optional
    // * Classic alpha blending
    // .blendEnable         = VK_TRUE,
    // .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    // .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    // .colorBlendOp        = VK_BLEND_OP_ADD,
    // .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    // .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    // .alphaBlendOp        = VK_BLEND_OP_ADD,
  };
  // ..  Create Info
  VkPipelineColorBlendStateCreateInfo const colorblendStateCI {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable     = VK_FALSE,
    .logicOp           = VK_LOGIC_OP_COPY,  // Optional
    .attachmentCount   = 1,
    .pAttachments      = &colorblendStateAtt,
    .blendConstants[0] = 0.0f,  // Optional
    .blendConstants[1] = 0.0f,  // Optional
    .blendConstants[2] = 0.0f,  // Optional
    .blendConstants[3] = 0.0f,  // Optional
  };

  //=====
  //=====   GRAPHICS PIPELINE

  // . Dynamic State
  // ..  Things where apply a Dynamic Behaviour
  VkDynamicState const dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,   //
    VK_DYNAMIC_STATE_LINE_WIDTH  //
  };
  // ..  Create Info
  MBU VkPipelineDynamicStateCreateInfo const dynamicStateCI {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates    = dynamicStates,
  };

  // . (?) Descriptors
  // ..  Pipeline Layout : This will grow up when using -Descriptors-
  VkPipelineLayoutCreateInfo const pipelineLayoutCI {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount         = 0,        // Optional
    .pSetLayouts            = nullptr,  // Optional
    .pushConstantRangeCount = 0,        // Optional
    .pPushConstantRanges    = nullptr,  // Optional
  };
  vku__check(vkCreatePipelineLayout(mDevice.handle, &pipelineLayoutCI, nullptr, &mDrawPipeline.layout));
  // ..  Input-State Vertex : Probably tends to grow with the 'pipelineLayout'
  VkPipelineVertexInputStateCreateInfo const inputstateVertexCI {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount   = 0,
    .pVertexBindingDescriptions      = nullptr,  // Optional
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions    = nullptr,  // Optional
  };
  // ..  Input-State Assembly : Probably could be 'static'
  VkPipelineInputAssemblyStateCreateInfo const inputstateAssemblyCI {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  // . Pipeline

  // ..  Populate shaders
  static auto const createShadersSameName =
    [&](std::string const &name, std::initializer_list<VkShaderStageFlagBits> stages) {
      for (auto const stage : stages) {
        auto const data = vku::shaders::create(mDevice.handle, name, stage);
        mDrawPipeline.shaderModules.emplace(data.path, data.module);
        mDrawPipeline.stagesCI.emplace_back(data.stageCreateInfo);
      }
    };
  MBU static int const do_once = [&]() {
    createShadersSameName("base", { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT });
    return 1;
  }();

  // ..  Create Info
  VkGraphicsPipelineCreateInfo const graphicsPipelineCI {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount          = vku__castsize(mDrawPipeline.stagesCI.size()),
    .pStages             = mDrawPipeline.stagesCI.data(),
    .pVertexInputState   = &inputstateVertexCI,
    .pInputAssemblyState = &inputstateAssemblyCI,
    .pViewportState      = &viewportStateCI,
    .pRasterizationState = &rasterizationStateCI,
    .pMultisampleState   = &multisamplingCI,
    .pDepthStencilState  = &depthstencilCI,  // Optional
    .pColorBlendState    = &colorblendStateCI,
    .pDynamicState       = &dynamicStateCI,  // Optional
    .layout              = mDrawPipeline.layout,
    .renderPass          = mDrawPipeline.renderpass,
    .subpass             = 0,               // index of subpass (or first subpass, not sure yet...)
    .basePipelineHandle  = VK_NULL_HANDLE,  // Optional
    .basePipelineIndex   = -1,              // Optional
  };
  // ..  Creation
  vku__check(
    vkCreateGraphicsPipelines(mDevice.handle, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &mDrawPipeline.handle));

  //=====
  //=====   COMMAND BUFFERS
  auto &cbs = mDrawPipeline.commandBuffers;

  // . Allocation
  cbs.resize(mDrawPipeline.frameBuffers.size());

  VkCommandBufferAllocateInfo const allocInfo {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool        = mDevice.commandPool,
    .commandBufferCount = vku__castsize(cbs.size()),
    // NOTE: The VkCommandBufferAllocateInfo.level parameter specifies:
    // - VK_COMMAND_BUFFER_LEVEL_PRIMARY:    [V] Submitted to a queue for execution.   [X] Called from others.
    // - VK_COMMAND_BUFFER_LEVEL_SECONDARY:  [V] Called from primary command buffers.  [X] Submitted directly.
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  };

  vku__check(vkAllocateCommandBuffers(mDevice.handle, &allocInfo, cbs.data()));

  // . Recording : Define needed structs

  VkClearValue const clearColor { { { 0.0175f, 0.0f, 0.0175f, 1.0f } } };  // @DANI : Put this in global access

  VkCommandBufferBeginInfo const commandBufferBI {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,  // @DANI : Review
    .pInheritanceInfo = nullptr,                                       // Optional
  };

  VkRenderPassBeginInfo /* const */ renderpassBI {
    .sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = mDrawPipeline.renderpass,
    // .framebuffer       = this->swapchainFramebuffers[i],
    .renderArea.offset = { 0, 0 },
    .renderArea.extent = mSwapChain.settings.extent2D,
    .clearValueCount   = 1,
    .pClearValues      = &clearColor,
  };

  // . Recording

  for (size_t i = 0; i < cbs.size(); ++i) {
    renderpassBI.framebuffer = mDrawPipeline.frameBuffers[i];
    vku__check(vkBeginCommandBuffer(cbs[i], &commandBufferBI));
    vkCmdBeginRenderPass(cbs[i], &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cbs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mDrawPipeline.handle);
    vkCmdSetViewport(cbs[i], 0, 1, &viewport);  // -> If Dynamic Viewport
    vkCmdDraw(cbs[i], 6, 1, 0, 0);
    vkCmdEndRenderPass(cbs[i]);
    vku__check(vkEndCommandBuffer(cbs[i]));
  }

  //=====

}  // void Base::createGraphicsPipeline()

}  // namespace vo::vulkan
