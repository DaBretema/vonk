#include "VoVulkanDevice.h"

#include "Macros.h"
#include "VoWindow.h"

namespace vo::vulkan
{  //

//-----------------------------------------------

void Device::createSwapChain()
{
  vo__assert(mInstance);

  // . Get old settings
  this->swapchainOld = this->swapchain;

  // ...

  // . Get current settings
  this->swapchainSettings = vku::swapchain::getSettings(
    mInstance->physicalDevice,
    mInstance->surface,
    vku::swapchain::Settings { vo::window::getFramebufferSize() });

  auto const &s           = this->swapchainSettings;
  this->inFlightMaxFrames = s.minImageCount;

  // ...

  // . Create swapchain
  bool const        gpSameQueue = this->queueGraphics() == this->queuePresent();
  std::vector const gpIndices   = { this->queueGraphicsIndex(), this->queuePresentIndex() };

  VkSwapchainCreateInfoKHR const swapchainCreateInfo {
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface               = mInstance->surface,
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
    .clipped               = VK_TRUE,            // VK_TRUE : we don't care about the color of pixels that are obscured
    .oldSwapchain          = this->swapchainOld  // Better cleanups
  };

  vku__check(vkCreateSwapchainKHR(this->handle, &swapchainCreateInfo, nullptr, &this->swapchain));

  // ...

  // . Capture swapchain 'internal' images
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(this->handle, this->swapchain, &imageCount, nullptr);
  this->swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(this->handle, this->swapchain, &imageCount, this->swapchainImages.data());

  // ...

  // . Create a image-view per 'internal' image
  this->swapchainImageViews.resize(this->swapchainImages.size());

  for (size_t i = 0; i < this->swapchainImages.size(); i++) {
    VkImageViewCreateInfo imageViewCreateInfo {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = this->swapchainImages[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = this->swapchainSettings.surfaceFormat.format,
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

    vku__check(vkCreateImageView(this->handle, &imageViewCreateInfo, nullptr, &this->swapchainImageViews[i]));
  }

  // ...

  // . Create FrameBuffers

  // this->swapchainFramebuffers.resize(this->swapchainImageViews.size());
  // for (size_t i = 0; i < this->swapchainImageViews.size(); ++i) {
  //   VkImageView             attachments[] = { this->swapchainImageViews[i] };
  //   VkFramebufferCreateInfo framebufferCreateInfo {
  //     .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
  //     .renderPass      = mRenderPass,
  //     .attachmentCount = 1,  // Modify this for MRT ??
  //     .pAttachments    = attachments,
  //     .width           = this->swapchainSettings.extent2D.width,
  //     .height          = this->swapchainSettings.extent2D.height,
  //     .layers          = 1,
  //   };
  //   vku__check(vkCreateFramebuffer(this->handle, &framebufferCreateInfo, nullptr, &this->swapchainFramebuffers[i]));
  // }

  // ...

  // . Create CommandPool

  VkCommandPoolCreateInfo commandpoolCreateInfo {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = this->queueGraphicsIndex(),
    .flags            = 0,  // Optional
  };
  vku__check(vkCreateCommandPool(this->handle, &commandpoolCreateInfo, nullptr, &this->commandpool));

  // ...

}  // void createSwapChain(Device &device, Instance const &instance, Device const &oldDevice)

//-----------------------------------------------

// .

//-----------------------------------------------

void Device::create(Instance const &instance)
{
  // void createLogicalDevice();
  //...........................................................................

  // . Copy from INSTANCE

  mInstance         = &instance;
  this->queuesIndex = mInstance->queuesIndex;

  // . CreateInfo Queues

  float const                          queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  for (uint32_t queueFamily : vku::queuefamily::getUniqueIndices(this->queuesIndex)) {
    queueCreateInfos.push_back(VkDeviceQueueCreateInfo {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount       = 1,
      .pQueuePriorities = &queuePriority,
    });
  }

  // . CreateInfo Device

  VkDeviceCreateInfo const deviceCreateInfo {
    .sType            = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pEnabledFeatures = &mInstance->physicalDeviceFeatures,
    // Queues info
    .pQueueCreateInfos    = queueCreateInfos.data(),
    .queueCreateInfoCount = vku__castsize(queueCreateInfos.size()),
    // Set device extensions
    .enabledExtensionCount   = vku__castsize(vo::sDeviceExtensions.size()),
    .ppEnabledExtensionNames = vo::sDeviceExtensions.data(),
    // Set device validation layers
    .enabledLayerCount   = vku__castsize(vo::sValidationLayers.size()),
    .ppEnabledLayerNames = vo::sValidationLayers.data(),
  };

  // . Create Device and Pick queues
  vku__check(vkCreateDevice(mInstance->physicalDevice, &deviceCreateInfo, nullptr, &this->handle));
  this->queues = vku::queuefamily::findQueues(this->handle, this->queuesIndex);

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

void Device::destroy()
{
  //   for (auto framebuffer : mSwapChainFramebuffers) {
  //     vkDestroyFramebuffer(this->handle, framebuffer, nullptr);
  //   }

  //   vkFreeCommandBuffers(
  //     this->handle,
  //     mCommandPool,
  //     VW_SIZE_CAST(mCommandBuffers.size()),
  //     mCommandBuffers.data());

  //   vkDestroyPipeline(this->handle, mGraphicsPipeline, nullptr);
  //   vkDestroyPipelineLayout(this->handle, mPipelineLayout, nullptr);
  //   vkDestroyRenderPass(this->handle, mRenderPass, nullptr);  // after: mPipelineLayout

  for (auto imageView : this->swapchainImageViews) { vkDestroyImageView(this->handle, imageView, nullptr); }
  vkDestroySwapchainKHR(this->handle, this->swapchain, nullptr);
  vkDestroyDevice(this->handle, nullptr);
}

//-----------------------------------------------
//-----------------------------------------------

}  // namespace vo::vulkan
