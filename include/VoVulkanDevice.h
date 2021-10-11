#pragma once

#include "_vulkan.h"
#include "VoVulkanUtils.h"

#include <optional>

#include <set>
#include <vector>
#include <unordered_map>

#include "VoVulkanInstance.h"

namespace vo::vulkan
{  //

struct Device
{
  VkDevice handle = VK_NULL_HANDLE;

  std::vector<VkQueue>  queues;
  std::vector<uint32_t> queuesIndex;
  inline VkQueue        queueGraphics() { return queues.at(vku::queuefamily::Type::graphics); }
  inline uint32_t       queueGraphicsIndex() { return queuesIndex.at(vku::queuefamily::Type::graphics); }
  inline VkQueue        queuePresent() { return queues.at(vku::queuefamily::Type::present); }
  inline uint32_t       queuePresentIndex() { return queuesIndex.at(vku::queuefamily::Type::present); }

  VkSwapchainKHR             swapchain    = VK_NULL_HANDLE;
  VkSwapchainKHR             swapchainOld = VK_NULL_HANDLE;
  std::vector<VkImage>       swapchainImages;
  vku::swapchain::Settings   swapchainSettings;
  std::vector<VkImageView>   swapchainImageViews;
  std::vector<VkFramebuffer> swapchainFramebuffers;

  std::unordered_map<std::string, VkShaderModule> shaderModules;
  std::vector<VkPipelineShaderStageCreateInfo>    pipelineShaderStageCreateInfos;

  VkRenderPass     renderPass      = VK_NULL_HANDLE;  // This could grow, might be a std::vector<VkRenderPass>
  VkPipelineLayout pipelineLayout  = VK_NULL_HANDLE;  // ... std::vector<VkPipelineLayout> (???)
  VkPipeline       gaphicsPipeline = VK_NULL_HANDLE;  // ... std::vector<VkPipeline> (???)

  VkCommandPool commandpool = VK_NULL_HANDLE;  // This have been meant to be one per thread,
                                               // so extract it to a pool of pools ?
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageSemaphores;
  std::vector<VkSemaphore> renderSempahores;

  uint32_t             inFlightMaxFrames;
  std::vector<VkFence> inFlightFencesSubmit;
  std::vector<VkFence> inFlightFencesAcquire;

  void create(Instance const &instance);
  void destroy();

private:
  Instance const *mInstance = nullptr;
  void            createSwapChain();
};

//......
//......
//......

// . initDevice( VulkanDevice );
// void createDevice(Device &device, Instance const &instance);
// void destroyDevice(Device &device);
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

namespace vku = vo::vulkan;
