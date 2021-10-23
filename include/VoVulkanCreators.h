#pragma once
#include "_vulkan.h"
#include "VoVulkanTypes.h"

#include "Macros.h"
#include <vector>

namespace vo::vulkan::creators
{  //

//-----------------------------------------------

inline VkInstanceCreateInfo instance(
  const char *              title,               //
  std::vector<const char *> instanceExtensions,  //
  std::vector<const char *> validationLayers)
{
  VkApplicationInfo const appInfo {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = title,
    .pEngineName        = title,
    .applicationVersion = VK_API_VERSION_1_2,
    .engineVersion      = VK_API_VERSION_1_2,
    .apiVersion         = VK_API_VERSION_1_2,
  };

  VkInstanceCreateInfo const instanceCI {
    .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    // Esential extensions
    .enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size()),
    .ppEnabledExtensionNames = instanceExtensions.data(),
    // Layers
    .enabledLayerCount   = static_cast<uint32_t>(validationLayers.size()),
    .ppEnabledLayerNames = validationLayers.data(),
  };

  return instanceCI;
}

//-----------------------------------------------

inline FixedFuncs_t fixedFuncs(VkExtent2D extent2D)
{
  FixedFuncs_t FF = {};

  // . ViewportState
  FF.viewports       = { {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = (float)extent2D.width,
    .height   = (float)extent2D.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  } };
  FF.scissors        = { {
    .offset = { 0, 0 },
    .extent = extent2D,
  } };
  FF.viewportStateCI = {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = vku__getSize(FF.viewports),
    .pViewports    = vku__getData(FF.viewports),
    .scissorCount  = vku__getSize(FF.scissors),
    .pScissors     = vku__getData(FF.scissors),
  };

  // . Rasterization
  FF.rasterizationStateCI = {
    .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode             = VK_POLYGON_MODE_FILL,
    .cullMode                = VK_CULL_MODE_BACK_BIT,
    .frontFace               = VK_FRONT_FACE_CLOCKWISE,
    .lineWidth               = 1.0f,
    .rasterizerDiscardEnable = VK_FALSE,  // Disable any output
    .depthClampEnable        = VK_FALSE,  // Useful for shadows stuff
    .depthBiasEnable         = VK_FALSE,  // Useful for shadows stuff
    .depthBiasConstantFactor = 0.0f,      // Useful for shadows stuff // Optional
    .depthBiasClamp          = 0.0f,      // Useful for shadows stuff // Optional
    .depthBiasSlopeFactor    = 0.0f,      // Useful for shadows stuff // Optional
  };

  // . Multisampling : Default OFF
  FF.multisamplingCI = {
    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable   = VK_FALSE,
    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
    .minSampleShading      = 1.0f,      // Optional
    .pSampleMask           = nullptr,   // Optional
    .alphaToCoverageEnable = VK_FALSE,  // Optional
    .alphaToOneEnable      = VK_FALSE,  // Optional
  };

  // . Depth (default ON) / Stencil (default OFF)
  FF.depthstencilCI = {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .stencilTestEnable = VK_FALSE,
    .depthWriteEnable  = VK_TRUE,
    .depthTestEnable   = VK_TRUE,
    .depthCompareOp    = VkCompareOp::VK_COMPARE_OP_LESS,
  };

  // . Blending
  FF.blendingPerAttachment = { {
    // * Color pass through
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional, (bc is the default ??)
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional, (bc is the default ??)
    .colorBlendOp        = VK_BLEND_OP_ADD,       // Optional, (bc is the default ??)
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional, (bc is the default ??)
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional, (bc is the default ??)
    .alphaBlendOp        = VK_BLEND_OP_ADD,       // Optional, (bc is the default ??)
    .colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    // * Classic alpha blending
    // .blendEnable         = VK_TRUE,
    // .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    // .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    // .colorBlendOp        = VK_BLEND_OP_ADD,
    // .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    // .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    // .alphaBlendOp        = VK_BLEND_OP_ADD,
  } };
  FF.blendingCI            = {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable     = VK_FALSE,
    .logicOp           = VK_LOGIC_OP_COPY,  // Optional
    .attachmentCount   = vku__getSize(FF.blendingPerAttachment),
    .pAttachments      = vku__getData(FF.blendingPerAttachment),
    .blendConstants[0] = 0.0f,  // Optional
    .blendConstants[1] = 0.0f,  // Optional
    .blendConstants[2] = 0.0f,  // Optional
    .blendConstants[3] = 0.0f,  // Optional
  };

  return FF;
}  // inline FixedFuncs_t fixedFuncs(VkExtent2D extent2D)

//-----------------------------------------------

}  // namespace vo::vulkan::creators

namespace vku = vo::vulkan;
