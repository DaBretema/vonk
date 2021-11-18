#include "VonkResources.h"

namespace vonk
{  //

//-----------------------------------------------

//
// // === DEBUGGER
//

//-----------------------------------------------

VKAPI_ATTR VkBool32 VKAPI_CALL sDebugMessengerCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  MBU void *                                  pUserData)
{
  static std::unordered_map<int32_t, bool> CACHE {};
  auto const                               ID = pCallbackData->messageIdNumber;

  if (!CACHE[ID] && messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    fmt::print(
      "\n ❗️ [VALIDATION LAYERS] - {} at {}{}"
      "\n--------------------------------------------------------------------"
      "------------\n{}\n",
      vonk::ToStr_DebugSeverity.at(messageSeverity),
      vonk::ToStr_DebugType.at(messageType),
      pUserData ? fmt::format("- {}", pUserData) : std::string { "" },
      pCallbackData->pMessage);

    CACHE[ID] = true;
  }

  return VK_FALSE;
}

//-----------------------------------------------

static inline VkDebugUtilsMessengerCreateInfoEXT sDebugMessengerCI {
  .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,

  .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT    //
                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT  //
                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

  .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT       //
                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  //
                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

  .pfnUserCallback = sDebugMessengerCallback,

  .pUserData = nullptr,  // Optional
  .pNext     = nullptr,  // Mandatory
  .flags     = 0,        // Mandatory
};

//-----------------------------------------------

//
// // === INSTANCEs
//

//-----------------------------------------------

Instance_t createInstance(const char *title, uint32_t apiVersion)
{
  Instance_t instance;

  // . Info
  // .. Of: Extensions

  if (!instance.layers.empty()) { instance.exts.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

  if (auto const windowExts = vonk::window::getInstanceExts(); !windowExts.empty()) {
    instance.exts.insert(instance.exts.end(), windowExts.begin(), windowExts.end());
  }

  // .. Of: App
  VkApplicationInfo const appInfo {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = title,
    .pEngineName        = title,
    .applicationVersion = apiVersion,
    .engineVersion      = apiVersion,
    .apiVersion         = apiVersion,
  };
  // .. Of: Instance
  VkInstanceCreateInfo const instanceCI {
    .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    // Extensions
    .enabledExtensionCount   = GetSizeU32(instance.exts),
    .ppEnabledExtensionNames = GetData(instance.exts),
    // Layers
    .enabledLayerCount   = GetSizeU32(instance.layers),
    .ppEnabledLayerNames = GetData(instance.layers),
  };

  //. Create
  //  .. Instance
  VkCheck(vkCreateInstance(&instanceCI, nullptr, &instance.handle));
  //  .. Surface
  instance.surface = vonk::window::createSurface(instance.handle);
  //  .. Debugger
  if (!instance.layers.empty()) {
    VkInstanceFn(instance.handle, vkCreateDebugUtilsMessengerEXT, &sDebugMessengerCI, nullptr, &instance.debugger);
  }

  return instance;
}

//-----------------------------------------------

void destroyInstance(Instance_t &instance)
{
  if (!instance.layers.empty()) {
    VkInstanceFn(instance.handle, vkDestroyDebugUtilsMessengerEXT, instance.debugger, nullptr);
  }
  vkDestroySurfaceKHR(instance.handle, instance.surface, nullptr);
  vkDestroyInstance(instance.handle, nullptr);

  instance = Instance_t {};
}
//-----------------------------------------------

//
// // === GPUs (PHYSICAL DEVICEs)
//

//-----------------------------------------------

Gpu_t pickGpu(Instance_t &instance, bool enableGraphics, bool enablePresent, bool enableCompute, bool enableTransfer)
{
  // . Iteration variables

  uint32_t gpuCount = 0;
  vkEnumeratePhysicalDevices(instance.handle, &gpuCount, nullptr);
  std::vector<VkPhysicalDevice> gpus(gpuCount);
  vkEnumeratePhysicalDevices(instance.handle, &gpuCount, gpus.data());

  Gpu_t    outGpu;
  uint32_t maxScore = 0;

  // . Evaluate all the gpus

  for (const auto &gpuHandle : gpus) {
    Gpu_t gpu;

    // . Get physical-device info
    gpu.handle = gpuHandle;
    vkGetPhysicalDeviceFeatures(gpu.handle, &gpu.features);
    vkGetPhysicalDeviceProperties(gpu.handle, &gpu.properties);
    vkGetPhysicalDeviceMemoryProperties(gpu.handle, &gpu.memory);

    gpu.surfSupp = vonk::getSurfaceSupport(gpu.handle, instance.surface);

    // . Queues Indices

    // ... Get families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu.handle, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu.handle, &queueFamilyCount, queueFamilies.data());

    // ... Get indices of them : Only evaluate enabled ones

    bool hasGraphics = !enableGraphics;
    bool hasPresent  = !enablePresent;
    bool hasCompute  = !enableCompute;
    bool hasTransfer = !enableTransfer;

    for (uint32_t i = 0u; i < queueFamilies.size(); ++i) {
      auto const flags = queueFamilies.at(i).queueFlags;

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(gpu.handle, i, instance.surface, &presentSupport);

      if (!hasGraphics and enableGraphics and (flags & VK_QUEUE_GRAPHICS_BIT)) {
        hasGraphics     = true;
        gpu.graphicsIdx = i;
      }
      if (!hasPresent and enablePresent and presentSupport) {
        auto const &G  = gpu.graphicsIdx;
        hasPresent     = (!G.has_value() || (G.has_value() and G.value() != i));  // Different from graphics
        gpu.presentIdx = i;
      }
      if (!hasCompute and enableCompute and (flags & VK_QUEUE_COMPUTE_BIT)) {
        hasCompute     = true;
        gpu.computeIdx = i;
      }
      if (!hasTransfer and enableTransfer and (flags & VK_QUEUE_TRANSFER_BIT)) {
        hasTransfer     = true;
        gpu.transferIdx = i;
      }
      if (hasGraphics and hasPresent and hasCompute and hasTransfer) break;
    }

    // . Validate the gpu
    if (
      !(hasGraphics and gpu.presentIdx.has_value() and hasCompute and hasTransfer)  //
      or (gpu.surfSupp.presentModes.empty() or gpu.surfSupp.formats.empty())
      or !vonk::checkGpuExtensionsSupport(gpu)  //
    ) {
      return gpu;
    }

    // . Get score from a valid gpu
    uint32_t const isDiscreteGPU = (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    uint32_t const score         = (1000 * isDiscreteGPU) + gpu.properties.limits.maxImageDimension2D;

    if (score > maxScore) {
      outGpu   = gpu;
      maxScore = score;
    }
  }

  LogInfof(
    "QUEUE INDICES -> g:{} p:{} c:{} t:{}",
    outGpu.graphicsIdx.has_value() ? fmt::to_string(outGpu.graphicsIdx.value()) : "x",
    outGpu.presentIdx.has_value() ? fmt::to_string(outGpu.presentIdx.value()) : "x",
    outGpu.computeIdx.has_value() ? fmt::to_string(outGpu.computeIdx.value()) : "x",
    outGpu.transferIdx.has_value() ? fmt::to_string(outGpu.transferIdx.value()) : "x");

  // . Return if valid gpu has been found
  AbortIfMsg((maxScore < 1), "Suitable GPU not found!");

  outGpu.pInstance = &instance;
  return outGpu;
}

//-----------------------------------------------

//
// // === DEVICEs
//

//-----------------------------------------------

Device_t createDevice(Instance_t const &instance, Gpu_t const &gpu)
{
  Device_t device;

  // . Queues' Create Infos
  // NOTE: If graphics, compute or present queues comes from the same family
  // register it only once
  float const                          queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCIs;
  queueCIs.reserve(4);
  std::set<uint32_t> uniqueIndices {};
  if (gpu.graphicsIdx.has_value()) uniqueIndices.emplace(gpu.graphicsIdx.value());
  if (gpu.computeIdx.has_value()) uniqueIndices.emplace(gpu.computeIdx.value());
  if (gpu.transferIdx.has_value()) uniqueIndices.emplace(gpu.transferIdx.value());
  if (gpu.presentIdx.has_value()) uniqueIndices.emplace(gpu.presentIdx.value());
  for (uint32_t queueFamily : uniqueIndices) {
    queueCIs.push_back(VkDeviceQueueCreateInfo {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount       = 1,
      .pQueuePriorities = &queuePriority,
    });
  }

  // . Device's Create Info
  VkDeviceCreateInfo const deviceCI {
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pEnabledFeatures        = &gpu.features,
    .queueCreateInfoCount    = GetSizeU32(queueCIs),
    .pQueueCreateInfos       = GetData(queueCIs),
    .enabledExtensionCount   = GetSizeU32(gpu.exts),
    .ppEnabledExtensionNames = GetData(gpu.exts),
    .enabledLayerCount       = GetSizeU32(instance.layers),
    .ppEnabledLayerNames     = GetData(instance.layers),
  };

  // . Create Device !
  VkCheck(vkCreateDevice(gpu.handle, &deviceCI, nullptr, &device.handle));

  // . Pick required queues
  if (gpu.graphicsIdx.has_value()) vkGetDeviceQueue(device.handle, gpu.graphicsIdx.value(), 0, &device.graphicsQ);
  if (gpu.computeIdx.has_value()) vkGetDeviceQueue(device.handle, gpu.computeIdx.value(), 0, &device.computeQ);
  if (gpu.transferIdx.has_value()) vkGetDeviceQueue(device.handle, gpu.transferIdx.value(), 0, &device.transferQ);
  if (gpu.presentIdx.has_value()) vkGetDeviceQueue(device.handle, gpu.presentIdx.value(), 0, &device.presentQ);

  // . Command Pool :  Maybe move this out and create one per thread ¿?
  VkCommandPoolCreateInfo const commandPoolCI {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = gpu.graphicsIdx.value(),
  };
  VkCheck(vkCreateCommandPool(device.handle, &commandPoolCI, nullptr, &device.commandPool));

  device.pGpu = &gpu;
  return device;
}

//-----------------------------------------------

void destroyDevice(Device_t &device)
{
  vkDestroyCommandPool(device.handle, device.commandPool, nullptr);
  vkDestroyDevice(device.handle, nullptr);
  device = Device_t {};
}

//-----------------------------------------------

//
// // === RENDER PASSes
//

//-----------------------------------------------

VkRenderPass createRenderPass(VkDevice device, RenderPassData_t const &rpd)
{
  VkRenderPassCreateInfo const renderpassCI {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = GetSizeU32(rpd.attachments),
    .pAttachments    = GetData(rpd.attachments),
    .subpassCount    = GetSizeU32(rpd.subpassDescs),
    .pSubpasses      = GetData(rpd.subpassDescs),
    .dependencyCount = GetSizeU32(rpd.subpassDeps),
    .pDependencies   = GetData(rpd.subpassDeps),
  };

  VkRenderPass renderpass;
  VkCheck(vkCreateRenderPass(device, &renderpassCI, nullptr, &renderpass));
  return renderpass;
}

//-----------------------------------------------

VkRenderPass createDefaultRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat)
{
  RenderPassData_t rpd;
  rpd.attachments = {
    {
      .format         = colorFormat,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    },
    {
      .format         = depthFormat,
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
  // @DANI : research more about the implications of this
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

  return vonk::createRenderPass(device, rpd);
}

//-----------------------------------------------

//
// // === TEXTUREs
//

//-----------------------------------------------

Texture_t createTexture(
  VkDevice                                device,
  VkPhysicalDeviceMemoryProperties const &memProps,
  VkExtent2D const &                      extent2D,
  VkFormat const &                        format,
  VkSampleCountFlagBits const &           samples,
  VkImageUsageFlags const &               usage,
  VkImageAspectFlagBits const &           aspectMaskBits)
{
  Texture_t tex;

  // . Image
  VkImageCreateInfo const imageCI {
    .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType   = VK_IMAGE_TYPE_2D,
    .format      = format,
    .extent      = { extent2D.width, extent2D.height, 1 },
    .mipLevels   = 1,
    .arrayLayers = 1,
    .samples     = samples,
    .tiling      = VK_IMAGE_TILING_OPTIMAL,
    .usage       = usage,
  };
  VkCheck(vkCreateImage(device, &imageCI, nullptr, &tex.image));

  // . Memory
  VkMemoryRequirements memReqs {};
  vkGetImageMemoryRequirements(device, tex.image, &memReqs);
  VkMemoryAllocateInfo const memAllloc {
    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize  = memReqs.size,
    .memoryTypeIndex = vonk::getMemoryType(memProps, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };
  VkCheck(vkAllocateMemory(device, &memAllloc, nullptr, &tex.memory));
  VkCheck(vkBindImageMemory(device, tex.image, tex.memory, 0));

  // . View : add stencil bit if is depth texture and the format allows
  bool const needStencilBit = (VK_IMAGE_ASPECT_DEPTH_BIT & aspectMaskBits) && format >= VK_FORMAT_D16_UNORM_S8_UINT;
  auto const stencilBit     = (needStencilBit) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0u;
  VkImageViewCreateInfo const imageViewCI {
    .sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .viewType                        = VK_IMAGE_VIEW_TYPE_2D,
    .image                           = tex.image,
    .format                          = format,
    .subresourceRange.baseMipLevel   = 0,
    .subresourceRange.levelCount     = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount     = 1,
    .subresourceRange.aspectMask     = aspectMaskBits | stencilBit,
  };
  VkCheck(vkCreateImageView(device, &imageViewCI, nullptr, &tex.view));

  return tex;
}

//-----------------------------------------------

void destroyTexture(VkDevice device, Texture_t const &tex)
{
  vkDestroyImageView(device, tex.view, nullptr);
  vkFreeMemory(device, tex.memory, nullptr);
  vkDestroyImage(device, tex.image, nullptr);
}

//-----------------------------------------------

bool isEmptyTexture(Texture_t const &tex) { return !tex.view or !tex.memory or !tex.image; }

//-----------------------------------------------

//
// // === SEMAPHOREs
//

//-----------------------------------------------

VkSemaphore createSemaphore(VkDevice device)
{
  static VkSemaphoreCreateInfo const semaphoreCI {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  VkSemaphore semaphore;
  VkCheck(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
  return semaphore;
}

//-----------------------------------------------

//-----------------------------------------------

//
// // === FENCEs
//

//-----------------------------------------------

VkFence createFence(VkDevice device)
{
  static VkFenceCreateInfo const fenceCI {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,  // Initialize on creation to avoid wait on first frame
  };

  VkFence fence;
  VkCheck(vkCreateFence(device, &fenceCI, nullptr, &fence));
  return fence;
}

//-----------------------------------------------

//-----------------------------------------------

//
// // === SWAP CHAIN
//

//-----------------------------------------------

void destroySwapChain(SwapChain_t &swapchain, bool justForRecreation)
{
  Device_t const &device = *swapchain.pDevice;

  // . Defaults : DepthTexture, FrameBuffers, ImageViews
  vonk::destroyTexture(device.handle, swapchain.defaultDepthTexture);
  for (auto framebuffer : swapchain.defaultFrameBuffers) { vkDestroyFramebuffer(device.handle, framebuffer, nullptr); }
  for (auto imageView : swapchain.views) { vkDestroyImageView(device.handle, imageView, nullptr); }

  // . Just For Recreation 'Barrier'
  if (justForRecreation) return;

  // . Default renderpass
  vkDestroyRenderPass(device.handle, swapchain.defaultRenderPass, nullptr);

  // . SwapChain
  vkDestroySwapchainKHR(device.handle, swapchain.handle, nullptr);

  // . Sync objects
  for (size_t i = 0; i < swapchain.sInFlightMaxFrames; i++) {
    vkDestroySemaphore(device.handle, swapchain.semaphores.render[i], nullptr);
    vkDestroySemaphore(device.handle, swapchain.semaphores.present[i], nullptr);
    vkDestroyFence(device.handle, swapchain.fences.submit[i], nullptr);
  }

  swapchain = SwapChain_t {};
}

//-----------------------------------------------

SwapChain_t createSwapChain(Device_t const &device, SwapChain_t oldSwapChain)
{
  auto const &gpu      = *device.pGpu;
  auto const &instance = *gpu.pInstance;

  VkSwapchainKHR oldSwapChainHandle = oldSwapChain.handle;
  SwapChain_t    swapchain          = std::move(oldSwapChain);

  bool const        gpDiffQueue = device.graphicsQ != device.presentQ;
  std::vector const gpIndices   = { device.pGpu->graphicsIdx.value(), device.pGpu->presentIdx.value() };

  // . Get supported settings
  auto const &SS  = gpu.surfSupp;
  swapchain.vsync = true;  // [***] to get from some user settings
  // ... Min number of images
  swapchain.minImageCount = SS.caps.minImageCount + 1;
  if ((SS.caps.maxImageCount > 0) and (swapchain.minImageCount > SS.caps.maxImageCount)) {
    swapchain.minImageCount = SS.caps.maxImageCount;
  }
  // ... Extent
  if (SS.caps.currentExtent.width != UINT32_MAX) {
    swapchain.extent2D = SS.caps.currentExtent;
  } else {
    auto const  windowSize = vonk::window::getFramebufferSize();
    uint32_t    w          = windowSize.width;
    uint32_t    h          = windowSize.height;
    auto const &min        = SS.caps.minImageExtent;
    auto const &max        = SS.caps.maxImageExtent;
    swapchain.extent2D     = VkExtent2D { std::clamp(w, min.width, max.width), std::clamp(h, min.height, max.height) };
  }
  // ... Transformation
  if (SS.caps.supportedTransforms & swapchain.preTransformFlag) {
    swapchain.preTransformFlag = swapchain.preTransformFlag;  // [***] to get from some user settings
  } else {
    swapchain.preTransformFlag = SS.caps.currentTransform;
  }
  // ... Alpha Composite
  if (SS.caps.supportedCompositeAlpha & swapchain.compositeAlphaFlag) {
    swapchain.compositeAlphaFlag = swapchain.compositeAlphaFlag;  // [***] to get from some user settings
  } else {
    for (auto &compositeAlphaFlag : {
           VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
           VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
           VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
           VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
         }) {
      if (SS.caps.supportedCompositeAlpha & compositeAlphaFlag) {
        swapchain.compositeAlphaFlag = compositeAlphaFlag;
        break;
      }
    }
  }
  // ... Image Usage Flags
  if (SS.caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    swapchain.extraImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if (SS.caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    swapchain.extraImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  // ... Present Modes
  // * VK_PRESENT_MODE_FIFO_KHR    : Guaranteed to be available : vsync + double-buffer
  // * VK_PRESENT_MODE_MAILBOX_KHR :
  //    Render as fast as possible while still avoiding tearing : *almost* vsync + triple-buffer
  //    NOTE: Do not use it on mobiles due to power-consumption !!
  swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;  // fallback
  if (!swapchain.vsync) {                            // [***] to get from some user settings
    if (std::find(SS.presentModes.begin(), SS.presentModes.end(), swapchain.presentMode) != SS.presentModes.end()) {
      swapchain.presentMode = swapchain.presentMode;  // [***] to get from some user settings
    }
  }
  // ... Image Formats
  bool isValidSurfaceFormat = false;
  for (const auto &available : SS.formats) {
    if (available.format == swapchain.colorFormat && available.colorSpace == swapchain.colorSpace) {
      isValidSurfaceFormat = true;
      break;
    }
  }
  if (isValidSurfaceFormat) {
    swapchain.colorFormat = swapchain.colorFormat;  // [***] to get from some user settings
    swapchain.colorSpace  = swapchain.colorSpace;   // [***] to get from some user settings
  } else {
    swapchain.colorFormat = SS.formats[0].format;
    swapchain.colorSpace  = SS.formats[0].colorSpace;
  }
  swapchain.depthFormat = SS.depthFormat;

  // . Create SwapChain

  VkSwapchainCreateInfoKHR const swapchainCI {
    .sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = instance.surface,
    .clipped = VK_TRUE,  // -> TRUE: Don't care about obscured pixels

    .presentMode    = swapchain.presentMode,
    .preTransform   = swapchain.preTransformFlag,    // -> i.e. Globally flips 90 degrees
    .compositeAlpha = swapchain.compositeAlphaFlag,  // -> Blending with other
                                                     // windows, Opaque = None/Ignore

    .imageArrayLayers = 1,  // -> Always 1 unless you are developing a
                            // stereoscopic 3D application.
    .minImageCount   = swapchain.minImageCount,
    .imageExtent     = swapchain.extent2D,
    .imageFormat     = swapchain.colorFormat,
    .imageColorSpace = swapchain.colorSpace,
    .imageUsage      = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | swapchain.extraImageUsageFlags,

    .imageSharingMode      = gpDiffQueue ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = gpDiffQueue ? GetSizeU32(gpIndices) : 0,
    .pQueueFamilyIndices   = gpDiffQueue ? GetData(gpIndices) : nullptr,

    .oldSwapchain = swapchain.handle  // -> Ensure that we can still present
                                      // already acquired images
  };
  VkCheck(vkCreateSwapchainKHR(device.handle, &swapchainCI, nullptr, &swapchain.handle));

  // . Get SwapChain Images
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &imageCount, nullptr);
  swapchain.images.resize(imageCount);
  vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &imageCount, swapchain.images.data());

  // . If an existing swap chain is re-created, destroy the old image-views
  //    [and swap-chain : This also cleans up all the presentable images ¿?¿?¿?]
  if (oldSwapChainHandle != VK_NULL_HANDLE) { destroySwapChain(swapchain, true); }

  // . Get Image-Views for that Images
  swapchain.views.resize(swapchain.images.size());
  for (size_t i = 0; i < swapchain.images.size(); i++) {
    VkImageViewCreateInfo const imageViewCI {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = swapchain.images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = swapchain.colorFormat,
      // How to read RGBA
      .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
      // The subresourceRange field describes what the image's purpose is and
      // which part to be accessed.
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,  // -> MIP-MAPing the texture [??]
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };
    VkCheck(vkCreateImageView(device.handle, &imageViewCI, nullptr, &swapchain.views[i]));
  }

  // . Setup default render-pass if needed
  if (!swapchain.defaultRenderPass) {
    swapchain.defaultRenderPass = createDefaultRenderPass(device.handle, swapchain.colorFormat, swapchain.depthFormat);
  }

  // . Setup default framebuffers' depth-stencil if needed
  swapchain.defaultDepthTexture = vonk::createTexture(
    device.handle,
    gpu.memory,
    swapchain.extent2D,
    swapchain.depthFormat,
    VK_SAMPLE_COUNT_1_BIT,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT);

  // . Setup default framebuffers
  VkImageView attachments[2];
  attachments[1]                              = swapchain.defaultDepthTexture.view;  // Depth
  VkFramebufferCreateInfo const frameBufferCI = {
    .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext           = NULL,
    .renderPass      = swapchain.defaultRenderPass,
    .attachmentCount = 2,
    .pAttachments    = attachments,
    .width           = swapchain.extent2D.width,
    .height          = swapchain.extent2D.height,
    .layers          = 1,
  };
  swapchain.defaultFrameBuffers.resize(swapchain.minImageCount);
  for (uint32_t i = 0; i < swapchain.defaultFrameBuffers.size(); ++i) {
    attachments[0] = swapchain.views[i];  // Color : 'Links' with the
                                          // image-views of the swap-chain
    VkCheck(vkCreateFramebuffer(device.handle, &frameBufferCI, nullptr, &swapchain.defaultFrameBuffers[i]));
  }

  // . Setup sync objects
  swapchain.fences.acquire.resize(swapchain.images.size(), VK_NULL_HANDLE);

  if (!oldSwapChainHandle) {
    swapchain.semaphores.render.resize(swapchain.sInFlightMaxFrames);
    swapchain.semaphores.present.resize(swapchain.sInFlightMaxFrames);
    swapchain.fences.submit.resize(swapchain.sInFlightMaxFrames);

    for (size_t i = 0; i < swapchain.sInFlightMaxFrames; ++i) {
      swapchain.semaphores.render[i]  = createSemaphore(device.handle);
      swapchain.semaphores.present[i] = createSemaphore(device.handle);
      swapchain.fences.submit[i]      = createFence(device.handle);
    }
  }

  swapchain.pDevice = &device;
  return swapchain;
}

//-----------------------------------------------

//
// // === SHADERs
//

//-----------------------------------------------

#define VONK_SHADER_CACHE 0

Shader_t createShader(VkDevice device, std::string const &name, VkShaderStageFlagBits stage)
{
  static std::unordered_map<VkShaderStageFlagBits, std::string> sStageToExtension {
    { VK_SHADER_STAGE_VERTEX_BIT, "vert" },
    { VK_SHADER_STAGE_FRAGMENT_BIT, "frag" },
    { VK_SHADER_STAGE_COMPUTE_BIT, "comp" },
    { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tesc" },
    { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tese" },
    { VK_SHADER_STAGE_GEOMETRY_BIT, "geom" },
  };
  static auto const sShadersPath = std::string("./assets/shaders/");  // get this path from #define

  std::string const path = sShadersPath + name + "." + sStageToExtension[stage] + ".spv";

  // . Evaluate cache first for early return if found
#if VONK_SHADER_CACHE
  //  NOTE: Temporary disabled due to requirement of ref_counting system to avoid destoy
  //  the shadermodule if other item is using it from the cache.
  static std::unordered_map<std::string, Shader_t> cache = {};
  if (cache.count(path) > 0) {
    LogInfo("CACHED!!!");
    return cache.at(path);
  }
#endif

  // . Otherwise create the shader for first time
  std::vector<char> const code = vo::files::read(path);
  if (code.empty()) { LogErrorf("Failed to open shader '{}'!", path); }

  VkShaderModuleCreateInfo const shadermoduleCI {
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = GetSizeU32(code),
    .pCode    = GetDataAs(const uint32_t *, code),
  };
  VkShaderModule shadermodule;
  VkCheck(vkCreateShaderModule(device, &shadermoduleCI, nullptr, &shadermodule));

  // . This is the thing that pipelines need
  VkPipelineShaderStageCreateInfo const shaderStageCI {
    .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage  = stage,
    .module = shadermodule,
    .pName  = "main",  // MUST : Entrypoint function name
  };

// . Finally store this on the cache-map and return
#if VONK_SHADER_CACHE
  cache[path] = { path, shadermodule, shaderStageCI };
  return cache.at(path);
#else
  return { path, shadermodule, shaderStageCI };
#endif
}

//-----------------------------------------------

DrawShader_t createDrawShader(
  Device_t const &   device,
  std::string const &vert,
  std::string const &frag,
  std::string const &tesc,
  std::string const &tese,
  std::string const &geom)
{
  DrawShader_t ds;

  Assert(!vert.empty() and !frag.empty());
  ds.vert = vonk::createShader(device.handle, vert, VK_SHADER_STAGE_VERTEX_BIT);
  ds.frag = vonk::createShader(device.handle, frag, VK_SHADER_STAGE_FRAGMENT_BIT);

  if (!tesc.empty() or !tesc.empty()) {
    Assert(!tesc.empty() and !tesc.empty() and device.pGpu->features.tessellationShader);
    ds.tesc = vonk::createShader(device.handle, tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    ds.tese = vonk::createShader(device.handle, tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
  }

  if (!geom.empty()) {
    Assert(device.pGpu->features.geometryShader);
    ds.geom = vonk::createShader(device.handle, geom, VK_SHADER_STAGE_GEOMETRY_BIT);
  }

  return ds;
}

//-----------------------------------------------

void destroyDrawShader(VkDevice device, DrawShader_t const &ds)
{
  vkDestroyShaderModule(device, ds.vert.module, nullptr);
  vkDestroyShaderModule(device, ds.frag.module, nullptr);
  vkDestroyShaderModule(device, ds.tesc.module, nullptr);
  vkDestroyShaderModule(device, ds.tese.module, nullptr);
  vkDestroyShaderModule(device, ds.geom.module, nullptr);
}

//-----------------------------------------------

// void destroyShader(Shader_t const &shader) {
//   vkDestroyShaderModule(shader.pDevice->handle, shader.module, nullptr);
// }

//-----------------------------------------------

//
// // === PIPELINEs
//

//-----------------------------------------------

DrawPipeline_t createPipeline(
  DrawPipeline_t const &            oldPipeline,
  DrawPipelineData_t const &        ci,
  SwapChain_t const &               swapchain,
  VkDevice                          device,
  VkCommandPool                     commandPool,
  VkRenderPass                      renderpass,
  std::vector<VkFramebuffer> const &frameBuffers)
{
  auto const     oldPipelineHandle = oldPipeline.handle;
  DrawPipeline_t pipeline          = std::move(oldPipeline);

  // Pipeline !

  auto const createPipeline_pipeline = [&]() {
    // . FIXED FUNCS - Rasterization
    VkPipelineRasterizationStateCreateInfo const rasterizationStateCI = {
      .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = ci.ffPolygonMode,
      .cullMode    = ci.ffCullMode,
      .frontFace   = ci.ffTriangleDirection,
      .lineWidth   = 1.0f,
    };

    // . FIXED FUNCS - Multisampling : Default OFF == 1_BIT
    VkPipelineMultisampleStateCreateInfo const multisamplingCI = {
      .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = ci.ffSamples,
      .sampleShadingEnable  = VK_FALSE,
      .minSampleShading     = 1.0f,
    };

    // . FIXED FUNCS - Depth (default ON) / Stencil (default OFF)
    VkPipelineDepthStencilStateCreateInfo const depthstencilCI = {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthWriteEnable  = (ci.ffDepthOp != VkCompareOp::VK_COMPARE_OP_MAX_ENUM),
      .depthTestEnable   = (ci.ffDepthOp != VkCompareOp::VK_COMPARE_OP_MAX_ENUM),
      .depthCompareOp    = ci.ffDepthOp,
      .stencilTestEnable = VK_FALSE,
    };

    // . FIXED FUNCS - Blending   @DANI NOTE : num of BlendTypes == num of
    // renderpass' attachments.
    std::vector<VkPipelineColorBlendAttachmentState> const blendingPerAttachment = { { BlendType::None } };
    VkPipelineColorBlendStateCreateInfo const              blendingCI            = {
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable   = VK_FALSE,
      .logicOp         = VK_LOGIC_OP_COPY,  // Optional
      .attachmentCount = GetSizeU32(blendingPerAttachment),
      .pAttachments    = GetData(blendingPerAttachment),
    };

    // . Render Pass
    pipeline.renderpass = renderpass;  // renderpass(device, ci.renderPassData);

    // . Shaders : @DANI : Improve : Write ONLY present shaders, i.e. avoid register tess if it's no present.
    pipeline.stagesCI.reserve(8);
    pipeline.stagesCI.emplace_back(ci.pDrawShader->vert.stageCI);
    pipeline.stagesCI.emplace_back(ci.pDrawShader->frag.stageCI);

    // . Pipeline Layout
    VkCheck(vkCreatePipelineLayout(device, &ci.pipelineLayoutData.pipelineLayoutCI, nullptr, &pipeline.layout));

    // . Pipeline   @DANI NOTE : Research about PipelineCache object.
    std::vector<VkDynamicState> const dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,  //
      VK_DYNAMIC_STATE_SCISSOR,   //
    };
    VkPipelineDynamicStateCreateInfo const dynamicStateCI {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = GetSizeU32(dynamicStates),
      .pDynamicStates    = GetData(dynamicStates),
    };

    VkPipelineViewportStateCreateInfo const viewportStateCI {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = GetSizeU32(ci.viewports),
      .pViewports    = GetData(ci.viewports),
      .scissorCount  = GetSizeU32(ci.scissors),
      .pScissors     = GetData(ci.scissors),
    };

    VkGraphicsPipelineCreateInfo const graphicsPipelineCI {
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount          = GetSizeU32(pipeline.stagesCI),
      .pStages             = GetData(pipeline.stagesCI),
      .pVertexInputState   = &ci.pipelineLayoutData.inputstateVertexCI,
      .pInputAssemblyState = &ci.pipelineLayoutData.inputstateAssemblyCI,
      .pViewportState      = &viewportStateCI,
      .pRasterizationState = &rasterizationStateCI,
      .pMultisampleState   = &multisamplingCI,
      .pDepthStencilState  = &depthstencilCI,
      .pColorBlendState    = &blendingCI,
      .pDynamicState       = &dynamicStateCI,  // Optional
      .layout              = pipeline.layout,
      .renderPass          = pipeline.renderpass,
      .subpass             = 0,               // index of subpass (or first subpass, not sure yet...)
      .basePipelineHandle  = VK_NULL_HANDLE,  // Optional
      .basePipelineIndex   = -1,              // Optional
    };
    VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &pipeline.handle));
  };

  // Commands !

  auto const createPipeline_commands = [&]() {
    // . Set Viewports and Scissors.
    // NOTE_1: If the Viewport size is negative, read it as percentage of current swapchain-size
    // NOTE_2: If the Scissor size is UINT32_MAX, set the current swapchain-size
    float const             swapH = swapchain.extent2D.height;
    float const             swapW = swapchain.extent2D.width;
    std::vector<VkViewport> viewports;
    viewports.reserve(ci.viewports.size());
    for (auto const &viewport : ci.viewports) {
      auto &v = viewports.emplace_back(viewport);
      if (v.x < 0) { v.x = swapW * (-viewport.x * 0.005f); }
      if (v.y < 0) { v.y = swapH * (-viewport.y * 0.005f); }
      if (v.width < 0) { v.width = swapW * (-viewport.width * 0.01f); }
      if (v.height < 0) { v.height = swapH * (-viewport.height * 0.01f); }
    }
    std::vector<VkRect2D> scissors;
    scissors.reserve(ci.scissors.size());
    for (auto const &scissor : ci.scissors) {
      auto &s = scissors.emplace_back(scissor);
      if (s.extent.height == UINT32_MAX) { s.extent.height = swapH; }
      if (s.extent.width == UINT32_MAX) { s.extent.width = swapW; }
    }

    // // . Set framebuffers
    // if (useAsOutput) {
    //   pipeline.frameBuffers.resize(swapchain.views.size());
    //   for (size_t i = 0; i < swapchain.views.size(); ++i) {
    //     VkImageView const             attachments[] = { swapchain.views[i] };
    //     VkFramebufferCreateInfo const framebufferCI {
    //       .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    //       .renderPass      = pipeline.renderpass,
    //       .attachmentCount = 1,  // Modify this for MRT ??
    //       .pAttachments    = attachments,
    //       .width           = swapchain.settings.extent2D.width,
    //       .height          = swapchain.settings.extent2D.height,
    //       .layers          = 1,
    //     };
    //     VkCheck(vkCreateFramebuffer(device, &framebufferCI, nullptr,
    //     &pipeline.frameBuffers[i]));
    //   }
    // }

    // . Commad Buffers Allocation
    pipeline.commandBuffers.resize(frameBuffers.size() /* pipeline.frameBuffers.size() */);
    VkCommandBufferAllocateInfo const commandBufferAllocInfo {
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = commandPool,
      .commandBufferCount = GetSizeU32(pipeline.commandBuffers),
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };
    VkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, GetData(pipeline.commandBuffers)));

    // . Commad Buffers Recording
    for (auto const &commandBuffesData : ci.commandBuffersData) {
      VkCommandBufferBeginInfo const commandBufferBI {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,  // @DANI : Review
        .pInheritanceInfo = nullptr,                                       // Optional
      };

      std::vector<VkClearValue> const clearValues { { .color = commandBuffesData.clearColor },
                                                    { .depthStencil = commandBuffesData.clearDephtStencil } };

      VkRenderPassBeginInfo renderpassBI {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass      = pipeline.renderpass,  // with multiple renderpasses use commandBuffesData.renderPassIdx
        .clearValueCount = GetSizeU32(clearValues),
        .pClearValues    = GetData(clearValues),
        // ?? Use this both for blitting.
        .renderArea.offset = { 0, 0 },
        .renderArea.extent = swapchain.extent2D,
      };

      for (size_t i = 0; i < pipeline.commandBuffers.size(); ++i) {
        auto const commandBuffer = pipeline.commandBuffers[i];
        renderpassBI.framebuffer = frameBuffers.at(i);  // pipeline.frameBuffers[i];
        VkCheck(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));
        vkCmdBeginRenderPass(commandBuffer, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
        vkCmdSetViewport(commandBuffer, 0, GetSizeU32(viewports),
                         GetData(viewports));  // Dynamic Viewport
        vkCmdSetScissor(commandBuffer, 0, GetSizeU32(scissors),
                        GetData(scissors));  // Dynamic Scissors
        if (commandBuffesData.commands) { commandBuffesData.commands(commandBuffer); }
        vkCmdEndRenderPass(commandBuffer);
        VkCheck(vkEndCommandBuffer(commandBuffer));
      }
    }
  };

  if (oldPipelineHandle == VK_NULL_HANDLE) { createPipeline_pipeline(); }
  createPipeline_commands();

  return pipeline;
}

//-----------------------------------------------

void destroyPipeline(SwapChain_t const &swapchain, DrawPipeline_t const &pipeline)
{
  auto const &device = swapchain.pDevice->handle;

  vkDestroyPipeline(device, pipeline.handle, nullptr);
  vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
  if (pipeline.renderpass != swapchain.defaultRenderPass) vkDestroyRenderPass(device, pipeline.renderpass, nullptr);
}

//-----------------------------------------------

}  // namespace vonk
