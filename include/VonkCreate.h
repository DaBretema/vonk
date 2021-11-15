#pragma once
#include "VonkTypes.h"
#include "VonkToStr.h"
#include "_vulkan.h"

#include "Macros.h"
#include "Utils.h"
#include <unordered_map>
#include <vector>

#include <algorithm>

namespace vonk
{  //

//-----------------------------------------------

//
// // === DEBUGGER : @DANI : Move to .cpp
//

//-----------------------------------------------

static inline VKAPI_ATTR VkBool32 VKAPI_CALL sDebugMessengerCallback(
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
      "\n--------------------------------------------------------------------------------\n{}\n",
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
// // === INSTANCE
//

//-----------------------------------------------

inline Instance_t createInstance(const char *title = "VONK", uint32_t apiVersion = VK_API_VERSION_1_2)
{
  Instance_t instance;

  // . Info
  // .. Of: Extensions

  if (!instance.layers.empty()) { instance.exts.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

  if (auto const windowExts = vonk::window::getInstanceExts(); !windowExts.empty()) {
    // std::merge(instance.exts.begin(), instance.exts.end(), windowExts.begin(), windowExts.end(),
    // instance.exts.end());
    instance.exts.insert(instance.exts.end(), windowExts.begin(), windowExts.end());
    // for (auto &&ext : windowExts) { instance.exts.emplace_back(ext); }
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
    .enabledExtensionCount   = vonk__getSize(instance.exts),
    .ppEnabledExtensionNames = vonk__getData(instance.exts),
    // Layers
    .enabledLayerCount   = vonk__getSize(instance.layers),
    .ppEnabledLayerNames = vonk__getData(instance.layers),
  };

  //. Create
  //  .. Instance
  vonk__check(vkCreateInstance(&instanceCI, nullptr, &instance.handle));
  //  .. Surface
  instance.surface = vonk::window::createSurface(instance.handle);
  //  .. Debugger
  if (!instance.layers.empty()) {
    vonk__instanceFn(instance.handle, vkCreateDebugUtilsMessengerEXT, &sDebugMessengerCI, nullptr, &instance.debugger);
  }

  return instance;
}

//-----------------------------------------------

inline void destroyInstance(Instance_t &instance)
{
  if (!instance.layers.empty()) {
    vonk__instanceFn(instance.handle, vkDestroyDebugUtilsMessengerEXT, instance.debugger, nullptr);
  }
  vkDestroySurfaceKHR(instance.handle, instance.surface, nullptr);
  vkDestroyInstance(instance.handle, nullptr);

  instance = Instance_t {};
}
//-----------------------------------------------

//
// // === GPU (PHYSICAL DEVICE)
//

//-----------------------------------------------

inline Gpu_t pickGpu(
  Instance_t &                     instance,
  std::vector<const char *> const &deviceExts,
  bool                             enableGraphics,
  bool                             enablePresent,
  bool                             enableCompute,
  bool                             enableTransfer)
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

    // . Queues Indices
    // .. Get families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu.handle, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu.handle, &queueFamilyCount, queueFamilies.data());
    // .. Get indices of them
    bool hasGraphics = false, hasPresent = false, hasCompute = false, hasTransfer = false;
    for (uint32_t i = 0u; i < queueFamilies.size(); ++i) {
      auto const flags = queueFamilies.at(i).queueFlags;

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(gpu.handle, i, instance.surface, &presentSupport);

      if (!hasGraphics && enableGraphics && (flags & VK_QUEUE_GRAPHICS_BIT)) {
        hasGraphics          = true;
        gpu.indices.graphics = i;
      }
      if (!hasPresent && enablePresent && presentSupport) {
        auto const &G       = gpu.indices.graphics;
        hasPresent          = (!G.has_value() || (G.has_value() && G.value() != i));  // Different from graphics
        gpu.indices.present = i;
      }
      if (!hasCompute && enableCompute && (flags & VK_QUEUE_COMPUTE_BIT)) {
        hasCompute          = true;
        gpu.indices.compute = i;
      }
      if (!hasTransfer && enableTransfer && (flags & VK_QUEUE_TRANSFER_BIT)) {
        hasTransfer          = true;
        gpu.indices.transfer = i;
      }

      if (hasGraphics && hasPresent && hasCompute && hasTransfer) break;
    }

    bool queueIndicesIsComplete = true;
    if (enableGraphics) queueIndicesIsComplete &= gpu.indices.graphics.has_value();
    if (enablePresent) queueIndicesIsComplete &= gpu.indices.present.has_value();
    if (enableCompute) queueIndicesIsComplete &= gpu.indices.compute.has_value();
    if (enableTransfer) queueIndicesIsComplete &= gpu.indices.transfer.has_value();

    // . Validate the gpu
    if (
      !queueIndicesIsComplete                                         //
      or vonk::swapchain::isEmpty(gpu.handle, instance.surface)       //
      or !vonk::checkDeviceExtensionsSupport(gpu.handle, deviceExts)  //
    ) {
      return gpu;
    }

    // . Get score from a valid gpu
    uint32_t const isDiscreteGPU = (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    uint32_t       score         = (1000 * isDiscreteGPU) + gpu.properties.limits.maxImageDimension2D;

    if (score > maxScore) {
      outGpu   = gpu;
      maxScore = score;
    }
  }

  vo__infof(
    "QUEUE INDICES -> g:{} p:{} c:{} t:{}",
    outGpu.indices.graphics.has_value() ? fmt::to_string(outGpu.indices.graphics.value()) : "<>",
    outGpu.indices.present.has_value() ? fmt::to_string(outGpu.indices.present.value()) : "<>",
    outGpu.indices.compute.has_value() ? fmt::to_string(outGpu.indices.compute.value()) : "<>",
    outGpu.indices.transfer.has_value() ? fmt::to_string(outGpu.indices.transfer.value()) : "<>");

  // . Return if valid gpu has been found
  if (maxScore < 1) { vo__abort("Suitable GPU not found!"); }
  return outGpu;
}

//-----------------------------------------------

//
// // === DEVICE
//

//-----------------------------------------------

inline Device_t createDevice(Instance_t const &instance, Gpu_t const &gpu)
{
  Device_t device;

  // . Queues' Create Infos
  float const                          queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCIs;

  // NOTE: If graphics, compute or present queues comes from the same family register it only once

  std::set<uint32_t> uniqueIndices {};
  if (gpu.indices.graphics.has_value()) uniqueIndices.emplace(gpu.indices.graphics.value());
  if (gpu.indices.compute.has_value()) uniqueIndices.emplace(gpu.indices.compute.value());
  if (gpu.indices.transfer.has_value()) uniqueIndices.emplace(gpu.indices.transfer.value());
  if (gpu.indices.present.has_value()) uniqueIndices.emplace(gpu.indices.present.value());

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
    .queueCreateInfoCount    = vonk__getSize(queueCIs),
    .pQueueCreateInfos       = vonk__getData(queueCIs),
    .enabledExtensionCount   = vonk__getSize(device.exts),
    .ppEnabledExtensionNames = vonk__getData(device.exts),
    .enabledLayerCount       = vonk__getSize(instance.layers),
    .ppEnabledLayerNames     = vonk__getData(instance.layers),
  };

  // . Create Device !
  vonk__check(vkCreateDevice(gpu.handle, &deviceCI, nullptr, &device.handle));

  // . Pick required queues
  if (gpu.indices.graphics.has_value())
    vkGetDeviceQueue(device.handle, gpu.indices.graphics.value(), 0, &device.queues.graphics);
  if (gpu.indices.compute.has_value())
    vkGetDeviceQueue(device.handle, gpu.indices.compute.value(), 0, &device.queues.compute);
  if (gpu.indices.transfer.has_value())
    vkGetDeviceQueue(device.handle, gpu.indices.transfer.value(), 0, &device.queues.transfer);
  if (gpu.indices.present.has_value())
    vkGetDeviceQueue(device.handle, gpu.indices.present.value(), 0, &device.queues.present);
  device.indices = gpu.indices;

  // . Command Pool :  Maybe move this out and create one per thread
  VkCommandPoolCreateInfo const commandPoolCI {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = gpu.indices.graphics.value(),
  };
  vonk__check(vkCreateCommandPool(device.handle, &commandPoolCI, nullptr, &device.commandPool));

  return device;
}

//-----------------------------------------------

inline void destroyDevice(Device_t &device)
{
  vkDestroyCommandPool(device.handle, device.commandPool, nullptr);
  vkDestroyDevice(device.handle, nullptr);
  device = Device_t {};
}

//-----------------------------------------------

//
// // === RENDER PASS
//

//-----------------------------------------------

inline VkRenderPass createRenderPass(VkDevice device, RenderPassData_t const &rpd)
{
  VkRenderPassCreateInfo const renderpassCI {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = vonk__getSize(rpd.attachments),
    .pAttachments    = vonk__getData(rpd.attachments),
    .subpassCount    = vonk__getSize(rpd.subpassDescs),
    .pSubpasses      = vonk__getData(rpd.subpassDescs),
    .dependencyCount = vonk__getSize(rpd.subpassDeps),
    .pDependencies   = vonk__getData(rpd.subpassDeps),
  };

  VkRenderPass renderpass;
  vonk__check(vkCreateRenderPass(device, &renderpassCI, nullptr, &renderpass));
  return renderpass;
}

//-----------------------------------------------

inline VkRenderPass createDefaultRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat)
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
// // === TEXTURE
//

//-----------------------------------------------

inline Texture_t createTexture(
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
  vonk__check(vkCreateImage(device, &imageCI, nullptr, &tex.image));

  // . Memory
  VkMemoryRequirements memReqs {};
  vkGetImageMemoryRequirements(device, tex.image, &memReqs);
  VkMemoryAllocateInfo const memAllloc {
    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize  = memReqs.size,
    .memoryTypeIndex = vonk::getMemoryType(memProps, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };
  vonk__check(vkAllocateMemory(device, &memAllloc, nullptr, &tex.memory));
  vonk__check(vkBindImageMemory(device, tex.image, tex.memory, 0));

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
  vonk__check(vkCreateImageView(device, &imageViewCI, nullptr, &tex.view));

  return tex;
}

//-----------------------------------------------

inline void destroyTexture(VkDevice device, Texture_t const &tex)
{
  vkDestroyImageView(device, tex.view, nullptr);
  vkFreeMemory(device, tex.memory, nullptr);
  vkDestroyImage(device, tex.image, nullptr);
}

//-----------------------------------------------

inline bool isEmptyTexture(Texture_t const &tex) { return !tex.view or !tex.memory or !tex.image; }

//-----------------------------------------------

//
// // === SEMAPHORES
//

//-----------------------------------------------

inline VkSemaphore createSemaphore(VkDevice device)
{
  static VkSemaphoreCreateInfo const semaphoreCI {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  VkSemaphore semaphore;
  vonk__check(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
  return semaphore;
}

//-----------------------------------------------

//-----------------------------------------------

//
// // === FENCES
//

//-----------------------------------------------

inline VkFence createFence(VkDevice device)
{
  static VkFenceCreateInfo const fenceCI {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,  // Initialize on creation to avoid 'freezes'
  };

  VkFence fence;
  vonk__check(vkCreateFence(device, &fenceCI, nullptr, &fence));
  return fence;
}

//-----------------------------------------------

//-----------------------------------------------

//
// // === SWAP CHAIN
//

//-----------------------------------------------

inline void destroySwapChain(Device_t const &device, SwapChain_t const &swapchain, bool justForRecreation = false)
{
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
}

//-----------------------------------------------

inline SwapChain_t
  createSwapChain(Instance_t const &instance, Gpu_t const &gpu, Device_t const &device, SwapChain_t oldSwapChain)
{
  VkSwapchainKHR oldSwapChainHandle = oldSwapChain.handle;
  SwapChain_t    swapchain          = std::move(oldSwapChain);

  bool const        gpDiffQueue = device.queues.graphics != device.queues.present;
  std::vector const gpIndices   = { device.indices.graphics.value(), device.indices.present.value() };

  // . Get supported settings
  swapchain.settings = vonk::swapchain::getSettings(
    gpu.handle,
    instance.surface,
    SwapShainSettings_t { true, vonk::window::getFramebufferSize() });
  auto const &st = swapchain.settings;

  // . Create SwapChain
  VkSwapchainCreateInfoKHR const swapchainCI {
    .sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = instance.surface,
    .clipped = VK_TRUE,  // -> TRUE: Don't care about obscured pixels

    .presentMode    = st.presentMode,
    .preTransform   = st.preTransformFlag,    // -> i.e. Globally flips 90 degrees
    .compositeAlpha = st.compositeAlphaFlag,  // -> Blending with other windows, Opaque = None/Ignore

    .imageArrayLayers = 1,  // -> Always 1 unless you are developing a stereoscopic 3D application.
    .minImageCount    = st.minImageCount,
    .imageExtent      = st.extent2D,
    .imageFormat      = st.colorFormat,
    .imageColorSpace  = st.colorSpace,
    .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | st.extraImageUsageFlags,

    .imageSharingMode      = gpDiffQueue ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = gpDiffQueue ? vonk__getSize(gpIndices) : 0,
    .pQueueFamilyIndices   = gpDiffQueue ? vonk__getData(gpIndices) : nullptr,

    .oldSwapchain = swapchain.handle  // -> Ensure that we can still present already acquired images
  };
  vonk__check(vkCreateSwapchainKHR(device.handle, &swapchainCI, nullptr, &swapchain.handle));

  // . Get SwapChain Images
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &imageCount, nullptr);
  swapchain.images.resize(imageCount);
  vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &imageCount, swapchain.images.data());

  // . If an existing swap chain is re-created, destroy the old image-views
  //    [and swap-chain : This also cleans up all the presentable images ¿?¿?¿?]
  if (oldSwapChainHandle != VK_NULL_HANDLE) { destroySwapChain(device, swapchain, true); }

  // . Get Image-Views for that Images
  swapchain.views.resize(swapchain.images.size());
  for (size_t i = 0; i < swapchain.images.size(); i++) {
    VkImageViewCreateInfo const imageViewCI {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = swapchain.images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = st.colorFormat,
      // How to read RGBA
      .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
      // The subresourceRange field describes what the image's purpose is and which part to be accessed.
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,  // -> MIP-MAPing the texture [??]
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };
    vonk__check(vkCreateImageView(device.handle, &imageViewCI, nullptr, &swapchain.views[i]));
  }

  // . Setup default render-pass if needed
  if (!swapchain.defaultRenderPass) {
    swapchain.defaultRenderPass = createDefaultRenderPass(device.handle, st.colorFormat, st.depthFormat);
  }

  // . Setup default framebuffers' depth-stencil if needed
  swapchain.defaultDepthTexture = vonk::createTexture(
    device.handle,
    gpu.memory,
    st.extent2D,
    st.depthFormat,
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
    .width           = st.extent2D.width,
    .height          = st.extent2D.height,
    .layers          = 1,
  };
  swapchain.defaultFrameBuffers.resize(st.minImageCount);
  for (uint32_t i = 0; i < swapchain.defaultFrameBuffers.size(); ++i) {
    attachments[0] = swapchain.views[i];  // Color : 'Links' with the image-views of the swap-chain
    vonk__check(vkCreateFramebuffer(device.handle, &frameBufferCI, nullptr, &swapchain.defaultFrameBuffers[i]));
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

  return swapchain;
}

//-----------------------------------------------

}  // namespace vonk

namespace vonk::create
{  //

//-----------------------------------------------

//-----------------------------------------------

//-----------------------------------------------

//-----------------------------------------------

inline void pipelineRecreation(
  Pipeline_t &               pipeline,
  bool                       onlyRecreateImageViewDependencies,
  PipelineData_t             ci,
  SwapChain_t const &        swapchain,
  VkDevice                   device,
  VkCommandPool              commandPool,
  VkRenderPass               renderpass,
  std::vector<VkFramebuffer> frameBuffers)
{
  /*

  // Separated logic
    auto const createPipeline = [&]() {

    };
    auto const createCommands = [&]() {

    };

  // Behaviour
    if(!onlyRecreateImageViewDependencies) {
      createPipeline();
    }
    createCommands();

  */

  if (!onlyRecreateImageViewDependencies) {
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

    // . FIXED FUNCS - Blending   @DANI NOTE : num of BlendTypes == num of renderpass' attachments.
    std::vector<VkPipelineColorBlendAttachmentState> const blendingPerAttachment = { { BlendType::None } };
    VkPipelineColorBlendStateCreateInfo const              blendingCI            = {
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable   = VK_FALSE,
      .logicOp         = VK_LOGIC_OP_COPY,  // Optional
      .attachmentCount = vonk__getSize(blendingPerAttachment),
      .pAttachments    = vonk__getData(blendingPerAttachment),
    };

    // . Render Pass
    pipeline.renderpass = renderpass;  // renderpass(device, ci.renderPassData);

    // . Shaders
    for (auto const &sd : ci.shadersData) {
      auto const data = vonk::shaders::create(device, sd.first, sd.second);
      pipeline.shaderModules.emplace(data.path, data.module);
      pipeline.stagesCI.emplace_back(data.stageCreateInfo);
    }

    // . Pipeline Layout
    vonk__check(vkCreatePipelineLayout(device, &ci.pipelineLayoutData.pipelineLayoutCI, nullptr, &pipeline.layout));

    // . Pipeline   @DANI NOTE : Research about PipelineCache object.
    std::vector<VkDynamicState> const dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,  //
      VK_DYNAMIC_STATE_SCISSOR,   //
    };
    VkPipelineDynamicStateCreateInfo const dynamicStateCI {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = vonk__getSize(dynamicStates),
      .pDynamicStates    = vonk__getData(dynamicStates),
    };

    VkPipelineViewportStateCreateInfo const viewportStateCI {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = vonk__getSize(ci.viewports),
      .pViewports    = vonk__getData(ci.viewports),
      .scissorCount  = vonk__getSize(ci.scissors),
      .pScissors     = vonk__getData(ci.scissors),
    };

    VkGraphicsPipelineCreateInfo const graphicsPipelineCI {
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount          = vonk__getSize(pipeline.stagesCI),
      .pStages             = vonk__getData(pipeline.stagesCI),
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
    vonk__check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &pipeline.handle));
  }

  // . Set Viewports and Scissors.
  // NOTE_1: If the Viewport size is negative, read it as percentage of current
  // swapchain-size NOTE_2: If the Scissor size is UINT32_MAX, set the current
  // swapchain-size
  float const             swapH = swapchain.settings.extent2D.height;
  float const             swapW = swapchain.settings.extent2D.width;
  std::vector<VkViewport> viewports;
  for (auto &viewport : ci.viewports) {
    auto &v = viewports.emplace_back(viewport);
    if (v.x < 0) { v.x = swapW * (-viewport.x * 0.005f); }
    if (v.y < 0) { v.y = swapH * (-viewport.y * 0.005f); }
    if (v.width < 0) { v.width = swapW * (-viewport.width * 0.01f); }
    if (v.height < 0) { v.height = swapH * (-viewport.height * 0.01f); }
  }
  std::vector<VkRect2D> scissors;
  for (auto &scissor : ci.scissors) {
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
  //     vonk__check(vkCreateFramebuffer(device, &framebufferCI, nullptr,
  //     &pipeline.frameBuffers[i]));
  //   }
  // }

  // . Commad Buffers Allocation
  pipeline.commandBuffers.resize(frameBuffers.size() /* pipeline.frameBuffers.size() */);
  VkCommandBufferAllocateInfo const commandBufferAllocInfo {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool        = commandPool,
    .commandBufferCount = vonk__getSize(pipeline.commandBuffers),
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  };
  vonk__check(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, vonk__getData(pipeline.commandBuffers)));

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
      .sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = pipeline.renderpass,  // with multiple renderpasses:
      // commandBuffesData.renderPassIdx
      .clearValueCount = vonk__getSize(clearValues),
      .pClearValues    = vonk__getData(clearValues),
      // ?? Use this both for blitting.
      .renderArea.offset = { 0, 0 },
      .renderArea.extent = swapchain.settings.extent2D,
    };

    for (size_t i = 0; i < pipeline.commandBuffers.size(); ++i) {
      auto const commandBuffer = pipeline.commandBuffers[i];
      renderpassBI.framebuffer = frameBuffers.at(i);  // pipeline.frameBuffers[i];
      vonk__check(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));
      vkCmdBeginRenderPass(commandBuffer, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
      vkCmdSetViewport(commandBuffer, 0, vonk__getSize(viewports), vonk__getData(viewports));  // Dynamic Viewport
      vkCmdSetScissor(commandBuffer, 0, vonk__getSize(scissors), vonk__getData(scissors));     // Dynamic Scissors
      if (commandBuffesData.commands) { commandBuffesData.commands(commandBuffer); }
      vkCmdEndRenderPass(commandBuffer);
      vonk__check(vkEndCommandBuffer(commandBuffer));
    }
  }
}

//-----------------------------------------------

inline Pipeline_t pipeline(
  PipelineData_t             ci,
  SwapChain_t const &        swapchain,
  VkDevice                   device,
  VkCommandPool              commandPool,
  VkRenderPass               renderpass,
  std::vector<VkFramebuffer> frameBuffers)
{
  Pipeline_t pipeline;
  pipelineRecreation(pipeline, false, ci, swapchain, device, commandPool, renderpass, frameBuffers);

  return pipeline;
}

//-----------------------------------------------

}  // namespace vonk::create
