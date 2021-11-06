#pragma once
#include "VonkTypes.h"
#include "_vulkan.h"

#include "Macros.h"
#include <unordered_map>
#include <vector>

namespace vonk::create
{  //

//-----------------------------------------------

inline VkInstance instance(
  const char *                     title,
  std::vector<const char *> const &instanceExtensions,
  std::vector<const char *> const &validationLayers,
  uint32_t                         apiVersion = VK_API_VERSION_1_2)
{
  VkApplicationInfo const appInfo {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = title,
    .pEngineName        = title,
    .applicationVersion = apiVersion,
    .engineVersion      = apiVersion,
    .apiVersion         = apiVersion,
  };

  auto const realInstanceExtensions = vonk::others::getInstanceExtensions(instanceExtensions, validationLayers);

  VkInstanceCreateInfo const instanceCI {
    .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    // Esential extensions
    .enabledExtensionCount   = static_cast<uint32_t>(realInstanceExtensions.size()),
    .ppEnabledExtensionNames = realInstanceExtensions.data(),
    // Layers
    .enabledLayerCount   = static_cast<uint32_t>(validationLayers.size()),
    .ppEnabledLayerNames = validationLayers.data(),
  };

  VkInstance instance;
  vonk__check(vkCreateInstance(&instanceCI, nullptr, &instance));

  return instance;
}

//-----------------------------------------------

// inline VkDevice device(){

// }

//-----------------------------------------------

inline VkRenderPass renderpass(VkDevice device, RenderPassData_t const &rpd)
{
  VkRenderPassCreateInfo const renderpassCI {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = vonk__getSize(rpd.attachments),
    .pAttachments    = vonk__getData(rpd.attachments),
    .subpassCount    = vonk__getSize(rpd.subpassDescs),
    .pSubpasses      = vonk__getData(rpd.subpassDescs),
    .dependencyCount = vonk__getSize(rpd.subpassDeps),
    .pDependencies   = vonk__getData(rpd.subpassDeps),
  };

  VkRenderPass renderpass;
  vonk__check(vkCreateRenderPass(device, &renderpassCI, nullptr, &renderpass));
  return renderpass;
}

//-----------------------------------------------

inline Texture_t texture(
  VkDevice                                device,
  VkPhysicalDeviceMemoryProperties const &memProps,
  VkExtent2D const &                      extent2D,
  VkFormat const &                        format,
  VkSampleCountFlagBits const &           samples,
  VkImageUsageFlags const &               usage,
  VkImageAspectFlagBits const &           aspectMaskBits)
{
  Texture_t tex;

  // . Image
  VkImageCreateInfo const imageCI {
    .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType   = VK_IMAGE_TYPE_2D,
    .format      = format,
    .extent      = { extent2D.width, extent2D.height, 1 },
    .mipLevels   = 1,
    .arrayLayers = 1,
    .samples     = samples,
    .tiling      = VK_IMAGE_TILING_OPTIMAL,
    .usage       = usage,
  };
  vonk__check(vkCreateImage(device, &imageCI, nullptr, &tex.image));

  // . Memory
  VkMemoryRequirements memReqs {};
  vkGetImageMemoryRequirements(device, tex.image, &memReqs);
  VkMemoryAllocateInfo const memAllloc {
    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize  = memReqs.size,
    .memoryTypeIndex = vonk::memory::getType(memProps, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };
  vonk__check(vkAllocateMemory(device, &memAllloc, nullptr, &tex.memory));
  vonk__check(vkBindImageMemory(device, tex.image, tex.memory, 0));

  // . View : add stencil bit if is depth texture and the format allows
  bool const needStencilBit = (VK_IMAGE_ASPECT_DEPTH_BIT & aspectMaskBits) && format >= VK_FORMAT_D16_UNORM_S8_UINT;
  auto const stencilBit     = (needStencilBit) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0u;
  VkImageViewCreateInfo const imageViewCI {
    .sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .viewType                        = VK_IMAGE_VIEW_TYPE_2D,
    .image                           = tex.image,
    .format                          = format,
    .subresourceRange.baseMipLevel   = 0,
    .subresourceRange.levelCount     = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount     = 1,
    .subresourceRange.aspectMask     = aspectMaskBits | stencilBit,
  };
  vonk__check(vkCreateImageView(device, &imageViewCI, nullptr, &tex.view));

  return tex;
}

//-----------------------------------------------

inline void pipelineRecreation(
  Pipeline_t &               pipeline,
  bool                       onlyRecreateImageViewDependencies,
  PipelineData_t             ci,
  SwapChain_t const &        swapchain,
  VkDevice                   device,
  VkCommandPool              commandPool,
  VkRenderPass               renderpass,
  std::vector<VkFramebuffer> frameBuffers)
{
  /*

  // Separated logic
    auto const createPipeline = [&]() {

    };
    auto const createCommands = [&]() {

    };

  // Behaviour
    if(!onlyRecreateImageViewDependencies) {
      createPipeline();
    }
    createCommands();

  */

  if (!onlyRecreateImageViewDependencies) {
    // . FIXED FUNCS - Rasterization
    VkPipelineRasterizationStateCreateInfo const rasterizationStateCI = {
      .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = ci.ffPolygonMode,
      .cullMode    = ci.ffCullMode,
      .frontFace   = ci.ffTriangleDirection,
      .lineWidth   = 1.0f,
    };

    // . FIXED FUNCS - Multisampling : Default OFF == 1_BIT
    VkPipelineMultisampleStateCreateInfo const multisamplingCI = {
      .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = ci.ffSamples,
      .sampleShadingEnable  = VK_FALSE,
      .minSampleShading     = 1.0f,
    };

    // . FIXED FUNCS - Depth (default ON) / Stencil (default OFF)
    VkPipelineDepthStencilStateCreateInfo const depthstencilCI = {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthWriteEnable  = (ci.ffDepthOp != VkCompareOp::VK_COMPARE_OP_MAX_ENUM),
      .depthTestEnable   = (ci.ffDepthOp != VkCompareOp::VK_COMPARE_OP_MAX_ENUM),
      .depthCompareOp    = ci.ffDepthOp,
      .stencilTestEnable = VK_FALSE,
    };

    // . FIXED FUNCS - Blending   @DANI NOTE : num of BlendTypes == num of renderpass' attachments.
    std::vector<VkPipelineColorBlendAttachmentState> const blendingPerAttachment = { { BlendType::None } };
    VkPipelineColorBlendStateCreateInfo const              blendingCI            = {
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable   = VK_FALSE,
      .logicOp         = VK_LOGIC_OP_COPY,  // Optional
      .attachmentCount = vonk__getSize(blendingPerAttachment),
      .pAttachments    = vonk__getData(blendingPerAttachment),
    };

    // . Render Pass
    pipeline.renderpass = renderpass;  // renderpass(device, ci.renderPassData);

    // . Shaders
    for (auto const &sd : ci.shadersData) {
      auto const data = vonk::shaders::create(device, sd.first, sd.second);
      pipeline.shaderModules.emplace(data.path, data.module);
      pipeline.stagesCI.emplace_back(data.stageCreateInfo);
    }

    // . Pipeline Layout
    vonk__check(vkCreatePipelineLayout(device, &ci.pipelineLayoutData.pipelineLayoutCI, nullptr, &pipeline.layout));

    // . Pipeline   @DANI NOTE : Research about PipelineCache object.
    std::vector<VkDynamicState> const dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,  //
      VK_DYNAMIC_STATE_SCISSOR,   //
    };
    VkPipelineDynamicStateCreateInfo const dynamicStateCI {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = vonk__getSize(dynamicStates),
      .pDynamicStates    = vonk__getData(dynamicStates),
    };

    VkPipelineViewportStateCreateInfo const viewportStateCI {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = vonk__getSize(ci.viewports),
      .pViewports    = vonk__getData(ci.viewports),
      .scissorCount  = vonk__getSize(ci.scissors),
      .pScissors     = vonk__getData(ci.scissors),
    };

    VkGraphicsPipelineCreateInfo const graphicsPipelineCI {
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount          = vonk__getSize(pipeline.stagesCI),
      .pStages             = vonk__getData(pipeline.stagesCI),
      .pVertexInputState   = &ci.pipelineLayoutData.inputstateVertexCI,
      .pInputAssemblyState = &ci.pipelineLayoutData.inputstateAssemblyCI,
      .pViewportState      = &viewportStateCI,
      .pRasterizationState = &rasterizationStateCI,
      .pMultisampleState   = &multisamplingCI,
      .pDepthStencilState  = &depthstencilCI,
      .pColorBlendState    = &blendingCI,
      .pDynamicState       = &dynamicStateCI,  // Optional
      .layout              = pipeline.layout,
      .renderPass          = pipeline.renderpass,
      .subpass             = 0,               // index of subpass (or first subpass, not sure yet...)
      .basePipelineHandle  = VK_NULL_HANDLE,  // Optional
      .basePipelineIndex   = -1,              // Optional
    };
    vonk__check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &pipeline.handle));
  }

  // . Set Viewports and Scissors.
  // NOTE_1: If the Viewport size is negative, read it as percentage of current
  // swapchain-size NOTE_2: If the Scissor size is UINT32_MAX, set the current
  // swapchain-size
  float const             swapH = swapchain.settings.extent2D.height;
  float const             swapW = swapchain.settings.extent2D.width;
  std::vector<VkViewport> viewports;
  for (auto &viewport : ci.viewports) {
    auto &v = viewports.emplace_back(viewport);
    if (v.x < 0) { v.x = swapW * (-viewport.x * 0.005f); }
    if (v.y < 0) { v.y = swapH * (-viewport.y * 0.005f); }
    if (v.width < 0) { v.width = swapW * (-viewport.width * 0.01f); }
    if (v.height < 0) { v.height = swapH * (-viewport.height * 0.01f); }
  }
  std::vector<VkRect2D> scissors;
  for (auto &scissor : ci.scissors) {
    auto &s = scissors.emplace_back(scissor);
    if (s.extent.height == UINT32_MAX) { s.extent.height = swapH; }
    if (s.extent.width == UINT32_MAX) { s.extent.width = swapW; }
  }

  // // . Set framebuffers
  // if (useAsOutput) {
  //   pipeline.frameBuffers.resize(swapchain.views.size());
  //   for (size_t i = 0; i < swapchain.views.size(); ++i) {
  //     VkImageView const             attachments[] = { swapchain.views[i] };
  //     VkFramebufferCreateInfo const framebufferCI {
  //       .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
  //       .renderPass      = pipeline.renderpass,
  //       .attachmentCount = 1,  // Modify this for MRT ??
  //       .pAttachments    = attachments,
  //       .width           = swapchain.settings.extent2D.width,
  //       .height          = swapchain.settings.extent2D.height,
  //       .layers          = 1,
  //     };
  //     vonk__check(vkCreateFramebuffer(device, &framebufferCI, nullptr,
  //     &pipeline.frameBuffers[i]));
  //   }
  // }

  // . Commad Buffers Allocation
  pipeline.commandBuffers.resize(frameBuffers.size() /* pipeline.frameBuffers.size() */);
  VkCommandBufferAllocateInfo const commandBufferAllocInfo {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool        = commandPool,
    .commandBufferCount = vonk__getSize(pipeline.commandBuffers),
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  };
  vonk__check(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, vonk__getData(pipeline.commandBuffers)));

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
      .sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = pipeline.renderpass,  // with multiple renderpasses:
      // commandBuffesData.renderPassIdx
      .clearValueCount = vonk__getSize(clearValues),
      .pClearValues    = vonk__getData(clearValues),
      // ?? Use this both for blitting.
      .renderArea.offset = { 0, 0 },
      .renderArea.extent = swapchain.settings.extent2D,
    };

    for (size_t i = 0; i < pipeline.commandBuffers.size(); ++i) {
      auto const commandBuffer = pipeline.commandBuffers[i];
      renderpassBI.framebuffer = frameBuffers.at(i);  // pipeline.frameBuffers[i];
      vonk__check(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));
      vkCmdBeginRenderPass(commandBuffer, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
      vkCmdSetViewport(commandBuffer, 0, vonk__getSize(viewports), vonk__getData(viewports));  // Dynamic Viewport
      vkCmdSetScissor(commandBuffer, 0, vonk__getSize(scissors), vonk__getData(scissors));     // Dynamic Scissors
      if (commandBuffesData.commands) { commandBuffesData.commands(commandBuffer); }
      vkCmdEndRenderPass(commandBuffer);
      vonk__check(vkEndCommandBuffer(commandBuffer));
    }
  }
}

//-----------------------------------------------

inline Pipeline_t pipeline(
  PipelineData_t             ci,
  SwapChain_t const &        swapchain,
  VkDevice                   device,
  VkCommandPool              commandPool,
  VkRenderPass               renderpass,
  std::vector<VkFramebuffer> frameBuffers)
{
  Pipeline_t pipeline;
  pipelineRecreation(pipeline, false, ci, swapchain, device, commandPool, renderpass, frameBuffers);

  return pipeline;
}

//-----------------------------------------------

}  // namespace vonk::create
