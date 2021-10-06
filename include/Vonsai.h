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

  // struct VulkanInstance
  // {
  //   VkInstance handle = VK_NULL_HANDLE;

  //   VkDebugUtilsMessengerEXT mDebugMessenger;

  //   VkPhysicalDevice           mPhysicalDevice = VK_NULL_HANDLE;
  //   VkPhysicalDeviceFeatures   mPhysicalDeviceFeatures;
  //   VkPhysicalDeviceProperties mPhysicalDeviceProperties;

  //   std::vector<std::optional<uint32_t>> mQueueFamilyIndices;

  //   VkSurfaceKHR mSurface;
  // };

  // struct VulkanDevice
  // {
  //   VkDevice handle = VK_NULL_HANDLE;

  //   VkQueue queueGraphics;
  //   VkQueue queuePresent;

  //   VkSwapchainKHR           mSwapChain    = VK_NULL_HANDLE;
  //   VkSwapchainKHR           mOldSwapChain = VK_NULL_HANDLE;
  //   std::vector<VkImage>     mSwapChainImages;
  //   vku::swapchain::Settings mSwapChainSettings;
  //   std::vector<VkImageView> mSwapChainImageViews;

  //   std::unordered_map<std::string, VkShaderModule> mShaderModules;
  //   std::vector<VkPipelineShaderStageCreateInfo>    mPipelineShaderStageCreateInfos;

  //   VkRenderPass     mRenderPass       = VK_NULL_HANDLE;  // This could grow, might be a std::vector<VkRenderPass>
  //   VkPipelineLayout mPipelineLayout   = VK_NULL_HANDLE;  // ... std::vector<VkPipelineLayout> (???)
  //   VkPipeline       mGraphicsPipeline = VK_NULL_HANDLE;  // ... std::vector<VkPipeline> (???)

  //   std::vector<VkFramebuffer> mSwapChainFramebuffers;

  //   VkCommandPool mCommandPool = VK_NULL_HANDLE;  // This have been meant to be one per thread,
  //                                                 // so extract it to a pool of pools ?
  //   std::vector<VkCommandBuffer> mCommandBuffers;

  //   std::vector<VkSemaphore> mImageSemaphores;
  //   std::vector<VkSemaphore> mRenderSempahores;
  //   uint32_t                 mMaxFlightFrames;
  //   std::vector<VkFence>     mInFlightFencesSubmit;
  //   std::vector<VkFence>     mInFlightFencesAcquire;
  // };

  // clang-format off

  // vo::window::...
      // . glfwWaitEvents
      // . glfwGetFramebufferSize
      // . glfwGetRequiredInstanceExtensions
      // . glfwCreateWindow
      // . glfwCreateWindowSurface
      // . glfwSetWindowUserPointer
      // . glfwSetFramebufferSizeCallback

  // vo::vulkan::...  ->>  DEFINE A STRUCT THAT CONTAIN THE VULKAN "STATE" TO SHARE BETWEEN HELPERS ???
      // . getInstanceExtensions(); ->> Will imply a call to vo::window::getRequiredInstanceExtensions
      // . getDeviceExtensions();
      // . initInstance( VulkanInstance, VulkanDevice );
          // .. createInstance ( VulkanInstance )
          // .. vku::debugmessenger::create( vulkanInstance );  // NOW: mDebugMessenger.create(mInstance)
          // .. createSurface( vulkanInstance )
          // .. pickPhysicalDevice( vulkanInstance )
          // .. createLogicalDevice( vulkanInstanceState, vulkanDeviceState )
      // . initDevice( VulkanDevice );
          // .. createSwapChain( vulkanDevice )
          // .. createImageViews( vulkanDevice )
          // .. createRenderPass( vulkanDevice )
          // .. createShaders( vulkanDevice )
          // .. createGraphicsPipeline( vulkanDevice )
          // .. createFramebuffers( vulkanDevice )
          // .. createCommandPool( vulkanDevice )
          // .. createCommandBuffers( vulkanDevice )
          // .. createSyncObjects( vulkanDevice )
      // . OnEvents
          // .. recreateSwapChain( vulkanDevice )  :  OnWindowResize, AfterAcquireChecks, AfterPresentChecks
      // . Draw frame ( vulkanDevice )
          // .. Is there a better way of manage create-info structures and
          // swapchain-recreation conditions? (See 'OnEvents')
      // . Clean up ( vulkanInstanceState, vulkanDevice )
  // clang-format on

  void initWindow();
  void initVulkan();
  void mainLoop();
  void drawFrame();
  void cleanup();

  // ::: Vulkan variables

  // ::: DEFAULTS
  //-------------------------

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

  // Move this flag to static data in vo::window::framebufferResized
  bool mFramebufferResized = false;

  //-------------------------

  //.
  //.
  //.

  // ::: INSTANCE
  //-------------------------
  VkInstance mInstance = VK_NULL_HANDLE;

  VkPhysicalDevice           mPhysicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures   mPhysicalDeviceFeatures;
  VkPhysicalDeviceProperties mPhysicalDeviceProperties;

  vku::QueueFamily    mQueueFamilies {};
  vku::DebugMessenger mDebugMessenger {};

  VkSurfaceKHR mSurface;
  //-------------------------

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
