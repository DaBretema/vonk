#pragma once

#include <string>
#include <vector>

#include "VoWindow.h"
#include "VoVulkan.h"
#include "VwUtils.h"

// #include "VwDebugMessenger.h"

namespace vo
{  //

class Vonsai
{
public:
  Vonsai(uint32_t w, uint32_t h, std::string title) : mW(w), mH(h), mTitle(std::move(title)) {}
  void run();

private:
  //

  // ::: App variables
  uint32_t    mW { 800 };
  uint32_t    mH { 600 };
  std::string mTitle { "Vonsai!" };

  void initWindow();
  void initVulkan();
  void mainLoop();
  void drawFrame();
  void cleanup();

  // ::: Vulkan variables

  vo::vulkan::Instance mInstance;
  vo::vulkan::Device   mDevice;

  //.
  //.
  //.

  // ::: DEVICE
  //-------------------------
  VkDevice mLogicalDevice = VK_NULL_HANDLE;

  VkSwapchainKHR           mSwapChain    = VK_NULL_HANDLE;
  VkSwapchainKHR           mOldSwapChain = VK_NULL_HANDLE;
  std::vector<VkImage>     mSwapChainImages;
  vku::swapchain::Settings mSwapChainSettings;
  std::vector<VkImageView> mSwapChainImageViews;

  std::unordered_map<std::string, VkShaderModule> mShaderModules;
  std::vector<VkPipelineShaderStageCreateInfo>    mPipelineShaderStageCreateInfos;

  VkRenderPass     mRenderPass     = VK_NULL_HANDLE;  // This could grow, might be a std::vector<VkRenderPass>
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;  // This could grow, might be a std::vector<VkPipelineLayout> (??)
  VkPipeline       mGraphicsPipeline = VK_NULL_HANDLE;  // This could grow, might be a std::vector<VkPipeline> (??)

  std::vector<VkFramebuffer> mSwapChainFramebuffers;

  VkCommandPool mCommandPool = VK_NULL_HANDLE;  // This have been meant to be one per thread,
                                                // so extract it to a pool of pools ?
  std::vector<VkCommandBuffer> mCommandBuffers;

  std::vector<VkSemaphore> mImageSemaphores;
  std::vector<VkSemaphore> mRenderSempahores;
  uint32_t                 mMaxFlightFrames;
  std::vector<VkFence>     mInFlightFencesSubmit;
  std::vector<VkFence>     mInFlightFencesAcquire;
  //-------------------------

  //.
  //.
  //.

  // ::: Vulkan initialization

  // void createInstance();
  // void createSurface();
  // void pickPhysicalDevice();
  // void createLogicalDevice();
  // void createSwapChain();
  // void createImageViews();
  // void createRenderPass();
  // void createShaders();
  // void createGraphicsPipeline();
  // void createFramebuffers();
  // void createCommandPool();
  // void createCommandBuffers();
  // void createSyncObjects();
  // void recreateSwapChain();
  // void cleanupSwapChain();

  //
};

}  // namespace vo
