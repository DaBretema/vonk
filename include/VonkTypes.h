#pragma once
#include "_glm.h"
#include "_vulkan.h"

#include "Macros.h"

#include <array>
#include <string>
#include <unordered_map>

namespace vonk
{  //

//-----------------------------------------------

struct SurfaceSupport_t
{
  VkSurfaceCapabilitiesKHR        caps;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   presentModes;
  VkFormat                        depthFormat;
};

//-----------------------------------------------

struct Instance_t
{
  VkInstance               handle   = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugger = VK_NULL_HANDLE;
  VkSurfaceKHR             surface  = VK_NULL_HANDLE;

  std::vector<const char *> layers = { "VK_LAYER_KHRONOS_validation" };
  std::vector<const char *> exts   = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
};

//-----------------------------------------------

struct Gpu_t
{
  VkPhysicalDevice handle = VK_NULL_HANDLE;

  SurfaceSupport_t                 surfSupp;
  VkPhysicalDeviceMemoryProperties memory;
  VkPhysicalDeviceFeatures         features;
  VkPhysicalDeviceProperties       properties;
  std::optional<uint32_t>          graphicsIdx = {}, presentIdx = {}, computeIdx = {}, transferIdx = {};

  std::vector<const char *> exts = { "VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  Instance_t const *pInstance = nullptr;
};

//-----------------------------------------------

struct Device_t
{
  VkDevice handle = VK_NULL_HANDLE;
  // VkCommandPool commandPool = VK_NULL_HANDLE;
  VkCommandPool graphicsCP = VK_NULL_HANDLE, presentCP = VK_NULL_HANDLE, computeCP = VK_NULL_HANDLE,
                transferCP = VK_NULL_HANDLE;
  VkQueue graphicsQ = VK_NULL_HANDLE, presentQ = VK_NULL_HANDLE, computeQ = VK_NULL_HANDLE, transferQ = VK_NULL_HANDLE;

  Gpu_t const *pGpu = nullptr;
};

//-----------------------------------------------

struct Texture_t
{
  VkImageView    view   = VK_NULL_HANDLE;
  VkImage        image  = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
};

//-----------------------------------------------

struct SwapChain_t
{
  VkSwapchainKHR           handle = VK_NULL_HANDLE;
  std::vector<VkImage>     images;
  std::vector<VkImageView> views;

  std::vector<VkFramebuffer> defaultFrameBuffers;
  Texture_t                  defaultDepthTexture;
  VkRenderPass               defaultRenderPass = VK_NULL_HANDLE;

  bool vsync = true;  // ! This may come from settings

  VkExtent2D                    extent2D             = { 1280, 720 };
  VkPresentModeKHR              presentMode          = VK_PRESENT_MODE_FIFO_KHR;
  uint32_t                      minImageCount        = 0u;
  VkFormat                      depthFormat          = VK_FORMAT_UNDEFINED;
  VkFormat                      colorFormat          = VK_FORMAT_B8G8R8A8_SRGB;
  VkColorSpaceKHR               colorSpace           = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  VkSurfaceTransformFlagBitsKHR preTransformFlag     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  VkCompositeAlphaFlagBitsKHR   compositeAlphaFlag   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VkImageUsageFlags             extraImageUsageFlags = {};

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

  static constexpr uint32_t sInFlightMaxFrames = 3;

  Device_t const *pDevice = nullptr;
};

//-----------------------------------------------

uint32_t constexpr VONK_COLOR_COMPONENT_RGBA_BIT =
  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

//-----------------------------------------------

namespace BlendType
{
  VkPipelineColorBlendAttachmentState const None = {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = VONK_COLOR_COMPONENT_RGBA_BIT,
  };
  VkPipelineColorBlendAttachmentState const Additive = {
    .blendEnable         = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = VONK_COLOR_COMPONENT_RGBA_BIT,
  };
  VkPipelineColorBlendAttachmentState const StraightAlpha = {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = VONK_COLOR_COMPONENT_RGBA_BIT,
  };
  VkPipelineColorBlendAttachmentState const StraightColor = {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = VONK_COLOR_COMPONENT_RGBA_BIT,
  };
}  // namespace BlendType

//-----------------------------------------------

struct RenderPassData_t
{
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkAttachmentReference>   attachmentRefs;
  std::vector<VkSubpassDescription>    subpassDescs;
  std::vector<VkSubpassDependency>     subpassDeps;
};

//-----------------------------------------------

struct FrameBuffer_t
{
  VkFramebuffer          framebuffer = VK_NULL_HANDLE;
  std::vector<Texture_t> attachments;
  // VkDescriptorImageInfo                descriptor;
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

struct Shader_t
{
  std::string                     path;
  VkShaderModule                  module;
  VkPipelineShaderStageCreateInfo stageCI;
};

//-----------------------------------------------

struct DrawShader_t
{
  Shader_t vert;
  Shader_t frag;
  Shader_t tesc;
  Shader_t tese;
  Shader_t geom;
};

//-----------------------------------------------

using ComputeShader_t = Shader_t;

//-----------------------------------------------

struct PipelineLayoutData_t
{
  // . Used for: VkCheck(vkCreatePipelineLayout(mDevice.handle, &pipelineLayoutCI, nullptr, &mPipeline.layout));
  VkPipelineLayoutCreateInfo pipelineLayoutCI {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount         = 0,        // Optional
    .pSetLayouts            = nullptr,  // Optional
    .pushConstantRangeCount = 0,        // Optional
    .pPushConstantRanges    = nullptr,  // Optional
  };
};

//-----------------------------------------------

struct DrawPipelineData_t
{
  bool useMeshes = true;

  // . Static
  VkPolygonMode         ffPolygonMode       = VK_POLYGON_MODE_FILL;
  VkCullModeFlags       ffCullMode          = VK_CULL_MODE_BACK_BIT;
  VkFrontFace           ffTriangleDirection = VK_FRONT_FACE_CLOCKWISE;
  VkSampleCountFlagBits ffSamples           = VK_SAMPLE_COUNT_1_BIT;
  VkCompareOp           ffDepthOp           = VK_COMPARE_OP_LESS;

  DrawShader_t const *pDrawShader = nullptr;
  // ComputeShader_t *    pComputeShader;
  RenderPassData_t     renderPassData;
  PipelineLayoutData_t pipelineLayoutData;

  // . Dynamic
  std::vector<VkViewport> viewports = { {
    .x        = 0.f,
    .y        = 0.f,
    .width    = -100.f,
    .height   = -100.f,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  } };
  std::vector<VkRect2D>   scissors  = { {
    .offset = { 0, 0 },
    .extent = { UINT32_MAX, UINT32_MAX },
  } };
  CommandBuffersData_t    commandBuffersData;
};

//-----------------------------------------------

struct DrawPipeline_t  // @DANI Change this to DrawPipeline_t and create ComputePipeline_t
{
  VkPipeline handle    = VK_NULL_HANDLE;
  bool       useMeshes = true;
  // . Static
  VkPipelineLayout                             layout = VK_NULL_HANDLE;
  std::vector<VkPipelineShaderStageCreateInfo> stagesCI;
  VkRenderPass                                 renderpass = VK_NULL_HANDLE;
  // . Dynamic
  std::vector<VkFramebuffer>   frameBuffers;
  std::vector<VkCommandBuffer> commandBuffers;
};

//-----------------------------------------------

struct Buffer_t
{
  VkBuffer       handle = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkDeviceSize   size   = 0u;
  uint32_t       count  = 0u;
};

//-----------------------------------------------

struct Vertex_t
{
  glm::vec3 pos;
  glm::vec3 color;
};

//---

auto static inline InputStateVertex(bool empty = false)
{
  static std::vector<VkVertexInputBindingDescription> const bindings = {
    { .binding = 0, .stride = sizeof(Vertex_t), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
  };
  static std::vector<VkVertexInputAttributeDescription> const attribs = {
    {
      .binding  = 0,
      .location = 0,
      .format   = VK_FORMAT_R32G32B32_SFLOAT,
      .offset   = offsetof(Vertex_t, pos),
    },
    {
      .binding  = 0,
      .location = 1,
      .format   = VK_FORMAT_R32G32B32_SFLOAT,
      .offset   = offsetof(Vertex_t, color),
    },
  };
  static VkPipelineVertexInputStateCreateInfo const vertexFilled {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount   = GetCountU32(bindings),
    .pVertexBindingDescriptions      = GetData(bindings),
    .vertexAttributeDescriptionCount = GetCountU32(attribs),
    .pVertexAttributeDescriptions    = GetData(attribs),
  };
  static VkPipelineVertexInputStateCreateInfo const vertexEmpty {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount   = 0,
    .pVertexBindingDescriptions      = nullptr,
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions    = nullptr,
  };
  return !empty ? &vertexFilled : &vertexEmpty;
}

//---

auto static inline InputStateAssembly()
{
  static VkPipelineInputAssemblyStateCreateInfo const assembly {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  return &assembly;
}

//-----------------------------------------------

struct Mesh_t
{
  // std::vector<Vertex_t> data = {};
  Buffer_t vertices;
};

//-----------------------------------------------

}  // namespace vonk
