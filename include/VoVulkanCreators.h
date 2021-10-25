#pragma once
#include "_vulkan.h"
#include "VoVulkanTypes.h"

#include "Macros.h"
#include <vector>
#include <unordered_map>

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

inline FixedFuncs_t fixedFuncs()
{
  FixedFuncs_t FF = {};

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

inline void pipelineRecreation(
  Pipeline_t &         pipeline,
  bool                 onlyRecreateImageViewDependencies,
  PipelineCreateInfo_t ci,
  SwapChain_t const &  swapchain,
  VkDevice             device,
  VkCommandPool        commandPool)
{
  if (!onlyRecreateImageViewDependencies) {
    // . Render Pass
    VkRenderPassCreateInfo const renderpassCI {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = vku__getSize(ci.renderPassData.attachments),
      .pAttachments    = vku__getData(ci.renderPassData.attachments),
      .subpassCount    = vku__getSize(ci.renderPassData.subpassDescs),
      .pSubpasses      = vku__getData(ci.renderPassData.subpassDescs),
      .dependencyCount = vku__getSize(ci.renderPassData.subpassDeps),
      .pDependencies   = vku__getData(ci.renderPassData.subpassDeps),
    };
    vku__check(vkCreateRenderPass(device, &renderpassCI, nullptr, &pipeline.renderpass));

    // . Shaders
    for (auto const &sd : ci.shadersData) {
      auto const data = vku::shaders::create(device, sd.first, sd.second);
      pipeline.shaderModules.emplace(data.path, data.module);
      pipeline.stagesCI.emplace_back(data.stageCreateInfo);
    }

    // . Pipeline Layout
    vku__check(vkCreatePipelineLayout(device, &ci.pipelineLayoutData.pipelineLayoutCI, nullptr, &pipeline.layout));

    // . Pipeline : @DANI research about PipelineCache object
    std::vector<VkDynamicState> const dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,  //
      VK_DYNAMIC_STATE_SCISSOR,   //
      // VK_DYNAMIC_STATE_LINE_WIDTH  //
    };
    MBU VkPipelineDynamicStateCreateInfo const dynamicStateCI {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = vku__getSize(dynamicStates),
      .pDynamicStates    = vku__getData(dynamicStates),
    };

    VkPipelineViewportStateCreateInfo const viewportStateCI {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = vku__getSize(ci.viewports),
      .pViewports    = vku__getData(ci.viewports),
      .scissorCount  = vku__getSize(ci.scissors),
      .pScissors     = vku__getData(ci.scissors),
    };

    VkGraphicsPipelineCreateInfo const graphicsPipelineCI {
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount          = vku__getSize(pipeline.stagesCI),
      .pStages             = vku__getData(pipeline.stagesCI),
      .pVertexInputState   = &ci.pipelineLayoutData.inputstateVertexCI,
      .pInputAssemblyState = &ci.pipelineLayoutData.inputstateAssemblyCI,
      .pViewportState      = &viewportStateCI,
      .pRasterizationState = &ci.fixedFuncs.rasterizationStateCI,
      .pMultisampleState   = &ci.fixedFuncs.multisamplingCI,
      .pDepthStencilState  = &ci.fixedFuncs.depthstencilCI,
      .pColorBlendState    = &ci.fixedFuncs.blendingCI,
      .pDynamicState       = &dynamicStateCI,  // Optional
      .layout              = pipeline.layout,
      .renderPass          = pipeline.renderpass,
      .subpass             = 0,               // index of subpass (or first subpass, not sure yet...)
      .basePipelineHandle  = VK_NULL_HANDLE,  // Optional
      .basePipelineIndex   = -1,              // Optional
    };
    vku__check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &pipeline.handle));
  }

  // . Set Viewports and Scissors.
  // NOTE_1: If the Viewport size is negative, read it as percentage of current swapchain-size
  // NOTE_2: If the Scissor size is UINT32_MAX, set the current swapchain-size
  float const             swapH = swapchain.settings.extent2D.height;
  float const             swapW = swapchain.settings.extent2D.width;
  std::vector<VkViewport> viewports;
  for (auto &viewport : ci.viewports) {
    auto &v = viewports.emplace_back(viewport);
    if (v.height < 0) { v.height = swapH * (-viewport.height * 0.01f); }
    if (v.width < 0) { v.width = swapW * (-viewport.width * 0.01f); }
  }
  std::vector<VkRect2D> scissors;
  for (auto &scissor : ci.scissors) {
    auto &s = scissors.emplace_back(scissor);
    if (s.extent.height == UINT32_MAX) { s.extent.height = swapH; }
    if (s.extent.width == UINT32_MAX) { s.extent.width = swapW; }
  }

  // . Set framebuffers
  pipeline.frameBuffers.resize(swapchain.views.size());
  for (size_t i = 0; i < swapchain.views.size(); ++i) {
    VkImageView const             attachments[] = { swapchain.views[i] };
    VkFramebufferCreateInfo const framebufferCI {
      .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass      = pipeline.renderpass,
      .attachmentCount = 1,  // Modify this for MRT ??
      .pAttachments    = attachments,
      .width           = swapchain.settings.extent2D.width,
      .height          = swapchain.settings.extent2D.height,
      .layers          = 1,
    };
    vku__check(vkCreateFramebuffer(device, &framebufferCI, nullptr, &pipeline.frameBuffers[i]));
  }

  // . Commad Buffers Allocation
  pipeline.commandBuffers.resize(pipeline.frameBuffers.size());
  VkCommandBufferAllocateInfo const commandBufferAllocInfo {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool        = commandPool,
    .commandBufferCount = vku__getSize(pipeline.commandBuffers),
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  };
  vku__check(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, vku__getData(pipeline.commandBuffers)));

  // . Commad Buffers Recording
  for (auto const &commandBuffesData : ci.commandBuffersData) {
    VkCommandBufferBeginInfo const commandBufferBI {
      .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,  // @DANI : Review
      .pInheritanceInfo = nullptr,                                       // Optional
    };

    std::vector<VkClearValue> const clearValues { { .color = commandBuffesData.clearColor },
                                                  { .depthStencil = commandBuffesData.clearDephtStencil } };

    VkRenderPassBeginInfo renderpassBI {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass      = pipeline.renderpass,  // with multiple renderpasses: commandBuffesData.renderPassIdx
      .clearValueCount = vku__getSize(clearValues),
      .pClearValues    = vku__getData(clearValues),
      // ?? Use this both for blitting.
      .renderArea.offset = { 0, 0 },
      .renderArea.extent = swapchain.settings.extent2D,
    };

    for (size_t i = 0; i < pipeline.commandBuffers.size(); ++i) {
      auto const commandBuffer = pipeline.commandBuffers[i];
      renderpassBI.framebuffer = pipeline.frameBuffers[i];
      vku__check(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));
      vkCmdBeginRenderPass(commandBuffer, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
      vkCmdSetViewport(commandBuffer, 0, vku__getSize(viewports), vku__getData(viewports));  // Dynamic Viewport
      vkCmdSetScissor(commandBuffer, 0, vku__getSize(scissors), vku__getData(scissors));     // Dynamic Scissors
      if (commandBuffesData.commands) { commandBuffesData.commands(commandBuffer); }
      vkCmdEndRenderPass(commandBuffer);
      vku__check(vkEndCommandBuffer(commandBuffer));
    }
  }
}

inline Pipeline_t
  pipeline(PipelineCreateInfo_t ci, SwapChain_t const &swapchain, VkDevice device, VkCommandPool commandPool)
{
  Pipeline_t pipeline;
  pipelineRecreation(pipeline, false, ci, swapchain, device, commandPool);

  return pipeline;
}

//-----------------------------------------------

}  // namespace vo::vulkan::creators

namespace vku = vo::vulkan;
