#pragma once
#include "_vulkan.h"

#include <string>
#include <unordered_map>

namespace vo::vulkan
{  //

struct QueueIndices_t
{
  uint32_t graphics, present;  //, compute, transfer;
};

struct Queues_t
{
  VkQueue graphics, present;  //, compute, transfer;
};

struct SwapShainSettings_t
{
  bool vsync = true;

  VkExtent2D               extent2D      = { 1280, 720 };
  VkPresentModeKHR         presentMode   = VK_PRESENT_MODE_FIFO_KHR;
  uint32_t                 minImageCount = 0u;
  VkSurfaceCapabilitiesKHR capabilities  = {};

  VkSurfaceFormatKHR            surfaceFormat        = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  VkSurfaceTransformFlagBitsKHR preTransformFlag     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  VkCompositeAlphaFlagBitsKHR   compositeAlphaFlag   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VkImageUsageFlags             extraImageUsageFlags = {};

  // void dumpInfo() const;
};

//----

struct Instance_t
{
  VkInstance               handle = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugger;
  VkSurfaceKHR             surface;
};

struct Gpu_t
{
  uint32_t                         score  = 0u;
  VkPhysicalDevice                 handle = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties memory;
  VkPhysicalDeviceFeatures         features;
  VkPhysicalDeviceProperties       properties;
  QueueIndices_t                   queuesIndices;
};

struct Device_t
{
  VkDevice handle = VK_NULL_HANDLE;

  Queues_t       queues;
  QueueIndices_t queuesIndices;

  VkCommandPool commandPool = VK_NULL_HANDLE;
};

struct SwapChain_t
{
  VkSwapchainKHR           handle = VK_NULL_HANDLE;
  SwapShainSettings_t      settings;
  std::vector<VkImage>     images;
  std::vector<VkImageView> views;
};

struct SyncBase_t
{
  struct
  {
    std::vector<VkSemaphore> render;
    std::vector<VkSemaphore> present;
  } semaphores;
  struct
  {
    std::vector<VkFence> acquire;
    std::vector<VkFence> submit;
  } fences;
};

struct GraphicsPipeline_t
{
  VkPipeline                                      handle;
  VkRenderPass                                    renderpass;
  std::vector<VkFramebuffer>                      frameBuffers;
  VkPipelineLayout                                layout;
  std::vector<VkPipelineShaderStageCreateInfo>    stagesCI;
  std::unordered_map<std::string, VkShaderModule> shaderModules;
  std::vector<VkCommandBuffer>                    commandBuffers;
};

}  // namespace vo::vulkan
