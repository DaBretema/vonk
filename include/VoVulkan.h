#pragma once

#include "_vulkan.h"
#include "VwUtils.h"

#include <optional>

#include <set>
#include <vector>
#include <unordered_map>

namespace vo::vulkan
{  //

//-----------------------------------------------

struct Instance
{
  VkInstance handle = VK_NULL_HANDLE;

  VkDebugUtilsMessengerEXT debugMessenger;

  VkPhysicalDevice           physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures   physicalDeviceFeatures;
  VkPhysicalDeviceProperties physicalDeviceProperties;

  std::vector<uint32_t> queuesIndex;

  VkSurfaceKHR surface;
};

//-----------------------------------------------

struct Device
{
  VkDevice handle = VK_NULL_HANDLE;

  std::vector<VkQueue>  queues;
  std::vector<uint32_t> queuesIndex;
  inline VkQueue        queueGraphics() { return queues.at(vku::queuefamily::Type::graphics); }
  inline uint32_t       queueGraphicsIndex() { return queuesIndex.at(vku::queuefamily::Type::graphics); }
  inline VkQueue        queuePresent() { return queues.at(vku::queuefamily::Type::present); }
  inline uint32_t       queuePresentIndex() { return queuesIndex.at(vku::queuefamily::Type::present); }

  VkSwapchainKHR           swapChain = VK_NULL_HANDLE;
  std::vector<VkImage>     swapChainImages;
  vku::swapchain::Settings swapChainSettings;
  std::vector<VkImageView> swapChainImageViews;

  std::unordered_map<std::string, VkShaderModule> mShaderModules;
  std::vector<VkPipelineShaderStageCreateInfo>    mPipelineShaderStageCreateInfos;

  VkRenderPass     mRenderPass       = VK_NULL_HANDLE;  // This could grow, might be a std::vector<VkRenderPass>
  VkPipelineLayout mPipelineLayout   = VK_NULL_HANDLE;  // ... std::vector<VkPipelineLayout> (???)
  VkPipeline       mGraphicsPipeline = VK_NULL_HANDLE;  // ... std::vector<VkPipeline> (???)

  std::vector<VkFramebuffer> mSwapChainFramebuffers;

  VkCommandPool mCommandPool = VK_NULL_HANDLE;  // This have been meant to be one per thread,
                                                // so extract it to a pool of pools ?
  std::vector<VkCommandBuffer> mCommandBuffers;

  std::vector<VkSemaphore> mImageSemaphores;
  std::vector<VkSemaphore> mRenderSempahores;

  uint32_t             maxFlightFrames;
  std::vector<VkFence> inFlightFencesSubmit;
  std::vector<VkFence> inFlightFencesAcquire;
};

//-----------------------------------------------

std::vector<char const *> getInstanceExtensions();
bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, std::vector<char const *> const &exts);
bool checkValidationLayersSupport(std::vector<char const *> const &layers);

//-----------------------------------------------
//-----------------------------------------------

// . initInstance( VulkanInstance, VulkanDevice );
// .. createInstance ( VulkanInstance )
// .. vku::debugmessenger::create( vulkanInstance );  // NOW: mDebugMessenger.create(mInstance)
// .. createSurface( vulkanInstance )
// .. pickPhysicalDevice( vulkanInstance )
// .. createLogicalDevice( vulkanInstanceState, vulkanDeviceState )
void createInstance(Instance &instance);
void destroyInstance(Instance &instance);
// void createInstance();
// // vku::debugmessenger::create(mInstance, &??mDebugMessenger);
// // mDebugMessenger.create(mInstance);
// void createSurface();
// void pickPhysicalDevice();
// void createLogicalDevice();

//-----------------------------------------------
//-----------------------------------------------

// . initDevice( VulkanDevice );
void createDevice(Device &device, Instance const &instance);
void destroyDevice(Device &device);
// .. createSwapChain( vulkanDevice )  V
// .. createImageViews( vulkanDevice ) V

// .. createRenderPass( vulkanDevice )
// .. createShaders( vulkanDevice )
// .. createGraphicsPipeline( vulkanDevice )

// .. createFramebuffers( vulkanDevice )

// .. createCommandPool( vulkanDevice )
// .. createCommandBuffers( vulkanDevice )

// .. createSyncObjects( vulkanDevice )

//-----------------------------------------------
//-----------------------------------------------

// . OnEvents
// .. recreateSwapChain( vulkanDevice )  :  OnWindowResize, AfterAcquireChecks, AfterPresentChecks

//-----------------------------------------------
//-----------------------------------------------

// . Draw frame ( vulkanDevice )
// .. Is there a better way of manage create-info structures and
// swapchain-recreation conditions? (See 'OnEvents')

//-----------------------------------------------
//-----------------------------------------------

// . Clean up ( vulkanInstanceState, vulkanDevice )

//-----------------------------------------------
//-----------------------------------------------

}  // namespace vo::vulkan
