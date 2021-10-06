#pragma once

#include <string>
#include <vector>

#include "VwVulkan.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "VwUtils.h"

#include "VwQueueFamily.h"
#include "VwDebugMessenger.h"
// #include "VwSwapChain.h"

namespace vo
{
//===============
//===============
//===============
//===============
//===============

// VONSAI

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

  GLFWwindow *mWindow { nullptr };

  // ::: App flow

  void initWindow();
  void initVulkan();
  void mainLoop();
  void drawFrame();
  void cleanup();

  // ::: Vulkan variables

  VkInstance mInstance = VK_NULL_HANDLE;

  VkPhysicalDevice           mPhysicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures   mPhysicalDeviceFeatures;
  VkPhysicalDeviceProperties mPhysicalDeviceProperties;

  VkDevice mLogicalDevice = VK_NULL_HANDLE;

  vku::QueueFamily    mQueueFamilies {};
  vku::DebugMessenger mDebugMessenger {};

  VkSurfaceKHR mSurface;

  VkSwapchainKHR           mSwapChain    = VK_NULL_HANDLE;
  VkSwapchainKHR           mOldSwapChain = VK_NULL_HANDLE;
  std::vector<VkImage>     mSwapChainImages;
  vku::swapchain::Settings mSwapChainSettings;
  std::vector<VkImageView> mSwapChainImageViews;

  std::unordered_map<std::string, VkShaderModule> mShaderModules;
  std::vector<VkPipelineShaderStageCreateInfo>    mPipelineShaderStageCreateInfos;

  VkRenderPass     mRenderPass     = VK_NULL_HANDLE;
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

  // . Input-State vertex
  VkPipelineVertexInputStateCreateInfo mVertexInputInfo {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount   = 0,
    .pVertexBindingDescriptions      = nullptr,  // Optional
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions    = nullptr,  // Optional
  };

  // . Input-Assembly
  VkPipelineInputAssemblyStateCreateInfo mInputAssembly {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

  std::vector<VkFramebuffer> mSwapChainFramebuffers;

  VkCommandPool                mCommandPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> mCommandBuffers;

  std::vector<VkSemaphore> mImageSemaphores;
  std::vector<VkSemaphore> mRenderSempahores;
  uint32_t                 mMaxFlightFrames;
  std::vector<VkFence>     mInFlightFences;
  std::vector<VkFence>     mInFlightImages;

  bool mFramebufferResized = false;

  // ::: Vulkan initialization

  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();
  void createImageViews();
  void createRenderPass();
  void createShaders();
  void createGraphicsPipeline();
  void createFramebuffers();
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  void recreateSwapChain();
  void cleanupSwapChain();

  //
};

}  // namespace vo
