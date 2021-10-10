#include "VoVulkan.h"

#include "Macros.h"
#include "VoWindow.h"

namespace vo::vulkan
{  //

//-----------------------------------------------

std::vector<char const *> getInstanceExtensions()
{
  auto exts = vo::window::getRequiredInstanceExtensions();

  if (vo::sHasValidationLayers) { exts.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

  if (vo::sHasInstanceExtensions) {
    for (auto &&ext : vo::sInstanceExtensions) { exts.emplace_back(ext); }
  }

  return exts;
}

//-----------------------------------------------

bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, std::vector<char const *> const &exts)
{
  if (exts.empty()) return true;

  uint32_t count;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> available(count);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, available.data());

  std::set<std::string> required(exts.begin(), exts.end());
  for (const auto &item : available) { required.erase(item.extensionName); }
  return required.empty();
}

//-----------------------------------------------

bool checkValidationLayersSupport(std::vector<char const *> const &layers)
{
  if (layers.empty()) return true;

  uint32_t count;
  vkEnumerateInstanceLayerProperties(&count, nullptr);
  std::vector<VkLayerProperties> available(count);
  vkEnumerateInstanceLayerProperties(&count, available.data());

  std::set<std::string> required(layers.begin(), layers.end());
  for (const auto &item : available) { required.erase(item.layerName); }
  return required.empty();
}

//-----------------------------------------------

void createInstance(Instance &instance)
{
  // . Create instance
  if (!vku::checkValidationLayersSupport(vo::sValidationLayers)) {
    vo__abort("Validation layers requested, but not available!");
  }

  auto const engineName = std::string(vo::window::title + " Engine");

  VkApplicationInfo const appInfo {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = vo::window::title.c_str(),
    .applicationVersion = VK_API_VERSION_1_2,
    .pEngineName        = engineName.c_str(),
    .engineVersion      = VK_API_VERSION_1_2,
    .apiVersion         = VK_API_VERSION_1_2,
  };

  auto const instanceExtensions = vku::getInstanceExtensions();

  VkInstanceCreateInfo const createInfo {
    .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    // Esential extensions
    .enabledExtensionCount   = vku__castsize(instanceExtensions.size()),
    .ppEnabledExtensionNames = instanceExtensions.data(),
    // Layers
    .enabledLayerCount   = static_cast<uint32_t>(vo::sValidationLayers.size()),
    .ppEnabledLayerNames = vo::sValidationLayers.data(),
  };

  vku__check(vkCreateInstance(&createInfo, nullptr, &instance.handle));

  // ...

  // . Create debug messenger
  vku::debugmessenger::create(instance.handle, instance.debugMessenger);

  // ...

  // . Create surface
  instance.surface = vo::window::createSurface(instance.handle);

  // ...

  // . Pick physical device

  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(instance.handle, &physicalDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(instance.handle, &physicalDeviceCount, physicalDevices.data());

  uint32_t maxScore = 0;
  for (const auto &physicalDevice : physicalDevices) {  //

    VkPhysicalDeviceFeatures   features;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    auto const queueIndices = vku::queuefamily::findIndices(physicalDevice, instance.surface);

    const int highValue = (                                            //
      (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)  //
    );
    const int midValue  = (                   //
      properties.limits.maxImageDimension2D  //
    );
    const int nullValue = { !vku::queuefamily::isComplete(queueIndices)                                   //
                            or vku::swapchain::isEmpty(physicalDevice, instance.surface)                  //
                            or !vku::checkDeviceExtensionsSupport(physicalDevice, vo::sDeviceExtensions)  //
                            or !features.geometryShader };

    uint32_t const finalScore = nullValue * ((1000 * highValue) + midValue);
    if (finalScore > maxScore) {
      instance.physicalDevice           = physicalDevice;
      instance.physicalDeviceFeatures   = features;
      instance.physicalDeviceProperties = properties;
      instance.queuesIndex              = vku::queuefamily::unrollOptionals(queueIndices);
      maxScore                          = finalScore;
    }
  }
  if (maxScore < 1) { vo__abort("Suitable GPU not found!"); }

  // ...
}

void destroyInstance(Instance &instance)
{
  vku::debugmessenger::destroy(instance.handle, instance.debugMessenger);
  vkDestroySurfaceKHR(instance.handle, instance.surface, nullptr);
  vkDestroyInstance(instance.handle, nullptr);
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

void createSwapChain(Device &device, Instance const &instance)
{
  // . Get old settings
  auto const prevSwapChain = device.swapChain;

  // ...

  // . Get current settings
  device.swapChainSettings = vku::swapchain::getSettings(
    instance.physicalDevice,
    instance.surface,
    vku::swapchain::Settings { vo::window::getFramebufferSize() });

  auto const &s          = device.swapChainSettings;
  device.maxFlightFrames = s.minImageCount;

  // ...

  // . Create swapchain
  bool const        gpSameQueue = device.queueGraphics() == device.queuePresent();
  std::vector const gpIndices   = { device.queueGraphicsIndex(), device.queuePresentIndex() };

  VkSwapchainCreateInfoKHR const swapchainCreateInfo {
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface               = instance.surface,
    .minImageCount         = s.minImageCount,
    .imageFormat           = s.surfaceFormat.format,
    .imageColorSpace       = s.surfaceFormat.colorSpace,
    .imageExtent           = s.extent2D,
    .presentMode           = s.presentMode,
    .imageArrayLayers      = 1,  // .. Always 1 unless you are developing a stereoscopic 3D application.
    .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // On swap-chain-present don't need more... (???)
    .imageSharingMode      = gpSameQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
    .queueFamilyIndexCount = gpSameQueue ? 0 : vku__castsize(gpIndices.size()),
    .pQueueFamilyIndices   = gpSameQueue ? nullptr : gpIndices.data(),
    .preTransform          = s.capabilities.currentTransform,    // i.e. globally flips 90degrees
    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,  // blending with other windows
    .clipped               = VK_TRUE,       // VK_TRUE : we don't care about the color of pixels that are obscured
    .oldSwapchain          = prevSwapChain  // Better cleanups
  };

  vku__check(vkCreateSwapchainKHR(device.handle, &swapchainCreateInfo, nullptr, &device.swapChain));

  // ...

  // . Capture swapchain 'internal' images
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(device.handle, device.swapChain, &imageCount, nullptr);
  device.swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device.handle, device.swapChain, &imageCount, device.swapChainImages.data());

  // ...

  // . Create a image-view per 'internal' image
  device.swapChainImageViews.resize(device.swapChainImages.size());

  for (size_t i = 0; i < device.swapChainImages.size(); i++) {
    VkImageViewCreateInfo imageViewCreateInfo {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = device.swapChainImages[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = device.swapChainSettings.surfaceFormat.format,
      // ... How to read RGBA
      .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
      // ... The subresourceRange field describes what the image's purpose is and which part of the image should be
      // accessed. **For now** set it as color targets without any mipmapping levels or multiple layers
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,  // MIP-MAPing the texture [TODO]
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };

    vku__check(vkCreateImageView(device.handle, &imageViewCreateInfo, nullptr, &device.swapChainImageViews[i]));
  }

  // ...

}  // void createSwapChain(Device &device, Instance const &instance, Device const &oldDevice)

//-----------------------------------------------

void createDevice(Device &device, Instance const &instance)
{
  // void createLogicalDevice();
  //...........................................................................

  // . Copy from INSTANCE

  device.queuesIndex = instance.queuesIndex;

  // . [Create-Info] Queues

  float const                          queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  for (uint32_t queueFamily : vku::queuefamily::getUniqueIndices(device.queuesIndex)) {
    queueCreateInfos.push_back(VkDeviceQueueCreateInfo {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount       = 1,
      .pQueuePriorities = &queuePriority,
    });
  }

  // . [Create-Info] Logical Device

  VkDeviceCreateInfo const deviceCreateInfo {
    .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pQueueCreateInfos    = queueCreateInfos.data(),
    .queueCreateInfoCount = vku__castsize(queueCreateInfos.size()),
    .pEnabledFeatures     = &instance.physicalDeviceFeatures,
    // .. Set device extensions
    .enabledExtensionCount   = vku__castsize(vo::sDeviceExtensions.size()),
    .ppEnabledExtensionNames = vo::sDeviceExtensions.data(),
    // .. Set device validation layers
    .enabledLayerCount   = vku__castsize(vo::sValidationLayers.size()),
    .ppEnabledLayerNames = vo::sValidationLayers.data(),
  };

  // . Creation and pick real queues from it
  vku__check(vkCreateDevice(instance.physicalDevice, &deviceCreateInfo, nullptr, &device.handle));
  device.queues = vku::queuefamily::findQueues(device.handle, device.queuesIndex);

  // ...

  // void createImageViews();
  //...........................................................................

  // ...

  // void createRenderPass();
  //...........................................................................

  // ...

  // void createShaders();
  //...........................................................................

  // ...

  // void createGraphicsPipeline();
  //...........................................................................

  // ...

  // void createFramebuffers();
  //...........................................................................

  // ...

  // void createCommandPool();
  //...........................................................................

  // ...

  // void createCommandBuffers();
  //...........................................................................

  // ...

  // void createSyncObjects();
  //...........................................................................

  // ...
}

//-----------------------------------------------

void destroyDevice(Device &device)
{
  //   for (auto framebuffer : mSwapChainFramebuffers) {
  //     vkDestroyFramebuffer(device.handle, framebuffer, nullptr);
  //   }

  //   vkFreeCommandBuffers(
  //     device.handle,
  //     mCommandPool,
  //     VW_SIZE_CAST(mCommandBuffers.size()),
  //     mCommandBuffers.data());

  //   vkDestroyPipeline(device.handle, mGraphicsPipeline, nullptr);
  //   vkDestroyPipelineLayout(device.handle, mPipelineLayout, nullptr);
  //   vkDestroyRenderPass(device.handle, mRenderPass, nullptr);  // after: mPipelineLayout

  for (auto imageView : device.swapChainImageViews) { vkDestroyImageView(device.handle, imageView, nullptr); }
  vkDestroySwapchainKHR(device.handle, device.swapChain, nullptr);
  vkDestroyDevice(device.handle, nullptr);
}

//-----------------------------------------------
//-----------------------------------------------

}  // namespace vo::vulkan
