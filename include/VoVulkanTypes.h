#pragma once
#include "_vulkan.h"

#include "Macros.h"

#include <string>
#include <unordered_map>

namespace vo::vulkan
{  //

//=============================================================================

struct QueueIndices_t
{
  uint32_t graphics, present;  //, compute, transfer;
};

//-----------------------------------------------

struct Queues_t
{
  VkQueue graphics, present;  //, compute, transfer;
};

//-----------------------------------------------

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
};

//=============================================================================

struct Instance_t
{
  VkInstance               handle = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugger;
  VkSurfaceKHR             surface;
};

//-----------------------------------------------

struct Gpu_t
{
  uint32_t                         score  = 0u;
  VkPhysicalDevice                 handle = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties memory;
  VkPhysicalDeviceFeatures         features;
  VkPhysicalDeviceProperties       properties;
  QueueIndices_t                   queuesIndices;
};

//-----------------------------------------------

struct Device_t
{
  VkDevice handle = VK_NULL_HANDLE;

  Queues_t       queues;
  QueueIndices_t queuesIndices;

  VkCommandPool commandPool = VK_NULL_HANDLE;
};

//-----------------------------------------------

struct SwapChain_t
{
  VkSwapchainKHR           handle = VK_NULL_HANDLE;
  SwapShainSettings_t      settings;
  std::vector<VkImage>     images;
  std::vector<VkImageView> views;
};

//-----------------------------------------------

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

//-----------------------------------------------

struct RenderPassData_t
{
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkAttachmentReference>   attachmentRefs;
  std::vector<VkSubpassDescription>    subpassDescs;
  std::vector<VkSubpassDependency>     subpassDeps;
};

//-----------------------------------------------

struct FixedFuncs_t
{
  // . ViewportState : MOVED TO DYNAMIC
  // std::vector<VkViewport>           viewports;
  // std::vector<VkRect2D>             scissors;
  // VkPipelineViewportStateCreateInfo viewportStateCI;
  // . Rasterization
  VkPipelineRasterizationStateCreateInfo rasterizationStateCI;
  // . Multisampling : Default OFF
  VkPipelineMultisampleStateCreateInfo multisamplingCI;
  // . Depth / Stencil
  VkPipelineDepthStencilStateCreateInfo depthstencilCI;
  // . Blending
  std::vector<VkPipelineColorBlendAttachmentState> blendingPerAttachment;
  VkPipelineColorBlendStateCreateInfo              blendingCI;
};

//-----------------------------------------------

struct CommandBufferData_t
{
  VkClearColorValue                    clearColor        = { { 0.0175f, 0.0f, 0.0175f, 1.0f } };
  VkClearDepthStencilValue             clearDephtStencil = { 1.f, 0 };
  MBU uint32_t                         renderPassIdx     = 0u;
  std::function<void(VkCommandBuffer)> commands          = nullptr;
};
using CommandBuffersData_t = std::vector<CommandBufferData_t>;

//-----------------------------------------------

using ShadersData_t = std::vector<std::pair<std::string, VkShaderStageFlagBits>>;

//-----------------------------------------------

struct PipelineLayoutData_t
{
  // . Used for: vku__check(vkCreatePipelineLayout(mDevice.handle, &pipelineLayoutCI, nullptr, &mPipeline.layout));
  VkPipelineLayoutCreateInfo pipelineLayoutCI {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount         = 0,        // Optional
    .pSetLayouts            = nullptr,  // Optional
    .pushConstantRangeCount = 0,        // Optional
    .pPushConstantRanges    = nullptr,  // Optional
  };

  // . Input-State Vertex : Probably tends to grow with the 'pipelineLayout'
  VkPipelineVertexInputStateCreateInfo inputstateVertexCI {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount   = 0,
    .pVertexBindingDescriptions      = nullptr,  // Optional
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions    = nullptr,  // Optional
  };

  // . Input-State Assembly : Probably could be 'static'
  VkPipelineInputAssemblyStateCreateInfo inputstateAssemblyCI {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };
};

struct PipelineCreateInfo_t
{
  // . Static
  FixedFuncs_t         fixedFuncs;
  ShadersData_t        shadersData;
  RenderPassData_t     renderPassData;
  PipelineLayoutData_t pipelineLayoutData;
  // . Dynamic
  std::vector<VkViewport> viewports;
  std::vector<VkRect2D>   scissors;
  CommandBuffersData_t    commandBuffersData;
};

struct Pipeline_t
{
  VkPipeline handle;
  // . Static
  VkPipelineLayout                                layout;
  std::vector<VkPipelineShaderStageCreateInfo>    stagesCI;
  VkRenderPass                                    renderpass;
  std::unordered_map<std::string, VkShaderModule> shaderModules;
  // . Dynamic
  std::vector<VkFramebuffer>   frameBuffers;
  std::vector<VkCommandBuffer> commandBuffers;
};

//-----------------------------------------------

}  // namespace vo::vulkan
