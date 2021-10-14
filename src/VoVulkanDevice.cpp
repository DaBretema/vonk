#include "VoVulkanDevice.h"

#include "Macros.h"
#include "VoWindow.h"

namespace vo::vulkan
{  //

//-----------------------------------------------

void Device::createSwapChain()
{
  //=====

  vo__assert(mInstance);

  //=====

  // . Get old settings
  this->swapchainOld = this->swapchain;

  //=====

  // . Get current settings
  this->swapchainSettings = vku::swapchain::getSettings(
    mInstance->physicalDevice,
    mInstance->surface,
    vku::swapchain::Settings { vo::window::getFramebufferSize() });

  auto const &s = this->swapchainSettings;
  // this->inFlightMaxFrames = s.minImageCount;

  //=====

  // . Create swapchain
  bool const        gpSameQueue = this->queueGraphics() == this->queuePresent();
  std::vector const gpIndices   = { this->queueGraphicsIndex(), this->queuePresentIndex() };

  VkSwapchainCreateInfoKHR const swapchainCreateInfo {
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface               = mInstance->surface,
    .minImageCount         = s.minImageCount,
    .imageFormat           = s.surfaceFormat.format,
    .imageColorSpace       = s.surfaceFormat.colorSpace,
    .imageExtent           = s.extent2D,
    .presentMode           = s.presentMode,
    .imageArrayLayers      = 1,  // .. Always 1 unless you are developing a stereoscopic 3D application.
    .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // On swap-chain-present don't need more... (???)
    .imageSharingMode      = gpSameQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
    .queueFamilyIndexCount = gpSameQueue ? 0 : vku__castsize(gpIndices.size()),
    .pQueueFamilyIndices   = gpSameQueue ? nullptr : gpIndices.data(),
    .preTransform          = s.capabilities.currentTransform,    // i.e. globally flips 90degrees
    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,  // blending with other windows
    .clipped               = VK_TRUE,            // VK_TRUE : we don't care about the color of pixels that are obscured
    .oldSwapchain          = this->swapchainOld  // Better cleanups
  };

  vku__check(vkCreateSwapchainKHR(this->handle, &swapchainCreateInfo, nullptr, &this->swapchain));

  //=====

  // . Capture swapchain 'internal' images
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(this->handle, this->swapchain, &imageCount, nullptr);
  this->swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(this->handle, this->swapchain, &imageCount, this->swapchainImages.data());

  // . Init things that depends on it
  this->inFlightFencesAcquire.resize(this->swapchainImages.size(), VK_NULL_HANDLE);

  //=====

  // . Create a image-view per 'internal' image
  this->swapchainImageViews.resize(this->swapchainImages.size());

  for (size_t i = 0; i < this->swapchainImages.size(); i++) {
    VkImageViewCreateInfo imageViewCreateInfo {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = this->swapchainImages[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = this->swapchainSettings.surfaceFormat.format,
      // How to read RGBA
      .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
      // The subresourceRange field describes what the image's purpose is and which part of the image should be
      // accessed. **For now** set it as color targets without any mipmapping levels or multiple layers.
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,  // MIP-MAPing the texture [TODO]
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };

    vku__check(vkCreateImageView(this->handle, &imageViewCreateInfo, nullptr, &this->swapchainImageViews[i]));
  }

  //=====

}  // void createSwapChain(Device &device, Instance const &instance, Device const &oldDevice)

//-----------------------------------------------

void Device::createGraphicsPipeline()
{
  //=====

  // . Create RenderPass

  // ..  Attachment
  VkAttachmentDescription const colorAtt {
    .format  = this->swapchainSettings.surfaceFormat.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,  // VK_SAMPLE_COUNT_1_BIT for No-Multisampling
    // Ops
    //  VkAttachmentDescription.loadOp :
    //  - VK_ATTACHMENT_LOAD_OP_LOAD      : Preserve the existing contents of the attachment.
    //  - VK_ATTACHMENT_LOAD_OP_CLEAR     : Clear the values to a constant (clear color) at the start.
    //  - VK_ATTACHMENT_LOAD_OP_DONT_CARE : Existing contents are undefined; we don't care about them.
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    //  VkAttachmentDescription.storeOp :
    //  - VK_ATTACHMENT_STORE_OP_STORE     : Rendered contents will be stored in memory and can be read later.
    //  - VK_ATTACHMENT_STORE_OP_DONT_CARE : Contents of the framebuffer will be undefined after the rendering op.
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    // Stencil
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    // Layouts
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  // ..  Subpass
  // ....  References
  VkAttachmentReference const colorAttRef {
    .attachment = 0,
    .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  // ....  Description
  VkSubpassDescription const subpassDesc {
    // Every subpass references one or more of the attachments using VkAttachmentReference.
    // This allows attachment resuse between render-subpasses.
    .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,  // also could be: Compute or Raytracing
    .colorAttachmentCount = 1,
    .pColorAttachments    = &colorAttRef,
  };

  // ..  Subpass dependencies (required for user-defined subpasses and 'implicit' ones)
  VkSubpassDependency dependency {
    .srcSubpass    = VK_SUBPASS_EXTERNAL,
    .dstSubpass    = 0,
    .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  // ..  RenderPass create info
  VkRenderPassCreateInfo const renderpassCI {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments    = &colorAtt,
    .subpassCount    = 1,
    .pSubpasses      = &subpassDesc,
    .dependencyCount = 1,
    .pDependencies   = &dependency,
  };

  vku__check(vkCreateRenderPass(this->handle, &renderpassCI, nullptr, &this->renderpass));

  //=====

  // . Create FrameBuffers

  this->swapchainFramebuffers.resize(this->swapchainImageViews.size());
  for (size_t i = 0; i < this->swapchainImageViews.size(); ++i) {
    VkImageView             attachments[] = { this->swapchainImageViews[i] };
    VkFramebufferCreateInfo framebufferCreateInfo {
      .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass      = this->renderpass,
      .attachmentCount = 1,  // Modify this for MRT ??
      .pAttachments    = attachments,
      .width           = this->swapchainSettings.extent2D.width,
      .height          = this->swapchainSettings.extent2D.height,
      .layers          = 1,
    };
    vku__check(vkCreateFramebuffer(this->handle, &framebufferCreateInfo, nullptr, &this->swapchainFramebuffers[i]));
  }

  //=====

  // . Fixed Functions

  // ..  Viewport
  // ....  Base
  VkViewport const viewport {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = (float)this->swapchainSettings.extent2D.width,
    .height   = (float)this->swapchainSettings.extent2D.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  // ....  Scissor
  VkRect2D const scissor {
    .offset = { 0, 0 },
    .extent = this->swapchainSettings.extent2D,
  };
  // ....  CreateInfo
  VkPipelineViewportStateCreateInfo const viewportState {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports    = &viewport,
    .scissorCount  = 1,
    .pScissors     = &scissor,
  };

  // ..  Rasterizer
  VkPipelineRasterizationStateCreateInfo const rasterizer {
    .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,  // or _LINE / _POINT as always
    .cullMode    = VK_CULL_MODE_BACK_BIT,
    .frontFace   = VK_FRONT_FACE_CLOCKWISE,
    // .. Disable any output
    .rasterizerDiscardEnable = VK_FALSE,
    // .. Usefull for Shadow-Maps
    .depthClampEnable        = VK_FALSE,
    .depthBiasEnable         = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,  // Optional
    .depthBiasClamp          = 0.0f,  // Optional
    .depthBiasSlopeFactor    = 0.0f,  // Optional
    .lineWidth               = 1.0f,
  };

  // ..  Multisampling : for now disabled
  VkPipelineMultisampleStateCreateInfo const multisampling {
    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable   = VK_FALSE,
    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
    .minSampleShading      = 1.0f,      // Optional
    .pSampleMask           = nullptr,   // Optional
    .alphaToCoverageEnable = VK_FALSE,  // Optional
    .alphaToOneEnable      = VK_FALSE,  // Optional
  };

  // ..  Depth / Stencil
  VkPipelineDepthStencilStateCreateInfo const depthStencil {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .stencilTestEnable = VK_FALSE,
    .depthWriteEnable  = VK_TRUE,
    .depthTestEnable   = VK_TRUE,
    .depthCompareOp    = VkCompareOp::VK_COMPARE_OP_LESS,
  };

  // ..  Blending
  // ....  Attacment State
  VkPipelineColorBlendAttachmentState const colorBlendAttachment {
    .colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    // Color pass through
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional
    .colorBlendOp        = VK_BLEND_OP_ADD,       // Optional
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional
    .alphaBlendOp        = VK_BLEND_OP_ADD,       // Optional
    // Classic alpha blending
    // .blendEnable         = VK_TRUE,
    // .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    // .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    // .colorBlendOp        = VK_BLEND_OP_ADD,
    // .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    // .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    // .alphaBlendOp        = VK_BLEND_OP_ADD,
  };
  // ....  Create info
  VkPipelineColorBlendStateCreateInfo const colorBlending {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable     = VK_FALSE,
    .logicOp           = VK_LOGIC_OP_COPY,  // Optional
    .attachmentCount   = 1,
    .pAttachments      = &colorBlendAttachment,
    .blendConstants[0] = 0.0f,  // Optional
    .blendConstants[1] = 0.0f,  // Optional
    .blendConstants[2] = 0.0f,  // Optional
    .blendConstants[3] = 0.0f,  // Optional
  };

  // ..  Dynamic state
  VkDynamicState const dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,   //
    VK_DYNAMIC_STATE_LINE_WIDTH  //
  };
  MBU VkPipelineDynamicStateCreateInfo const dynamicState {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates    = dynamicStates,
  };

  // ..  Pipeline Layout - This may grow up
  VkPipelineLayoutCreateInfo const pipelineLayoutInfo {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount         = 0,        // Optional
    .pSetLayouts            = nullptr,  // Optional
    .pushConstantRangeCount = 0,        // Optional
    .pPushConstantRanges    = nullptr,  // Optional
  };
  vku__check(vkCreatePipelineLayout(this->handle, &pipelineLayoutInfo, nullptr, &this->pipelineLayout));

  //=====

  // . Graphics Pipeline

  // ..  Input-State vertex
  VkPipelineVertexInputStateCreateInfo const sVertexInputInfo {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount   = 0,
    .pVertexBindingDescriptions      = nullptr,  // Optional
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions    = nullptr,  // Optional
  };

  // ..  Input-Assembly
  VkPipelineInputAssemblyStateCreateInfo const sInputAssembly {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  // ..  Create Info
  VkGraphicsPipelineCreateInfo pipelineInfo {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount          = vku__castsize(this->pipelineShaderStageCreateInfos.size()),
    .pStages             = this->pipelineShaderStageCreateInfos.data(),
    .pVertexInputState   = &sVertexInputInfo,
    .pInputAssemblyState = &sInputAssembly,
    .pViewportState      = &viewportState,
    .pRasterizationState = &rasterizer,
    .pMultisampleState   = &multisampling,
    .pDepthStencilState  = &depthStencil,  // Optional
    .pColorBlendState    = &colorBlending,
    // .pDynamicState       = &dynamicState,  // Optional
    .layout             = this->pipelineLayout,
    .renderPass         = this->renderpass,
    .subpass            = 0,               // index of subpass (or first subpass, not sure yet...)
    .basePipelineHandle = VK_NULL_HANDLE,  // Optional
    .basePipelineIndex  = -1,              // Optional
  };

  // ..  Create graphics-pipeline
  vku__check(
    vkCreateGraphicsPipelines(this->handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline));

  //=====

  // . Command Buffers

  /*
  * The VkCommandBufferAllocateInfo.level parameter specifies:
      - VK_COMMAND_BUFFER_LEVEL_PRIMARY:
          [V] Submitted to a queue for execution.
          [X] Called from other command buffers.
      - VK_COMMAND_BUFFER_LEVEL_SECONDARY:
          [V] Called from primary command buffers.
          [X] Submitted directly.
  */

  // ..  Allocation
  this->commandBuffers.resize(this->swapchainFramebuffers.size());
  VkCommandBufferAllocateInfo const allocInfo {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool        = this->commandpool,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = vku__castsize(this->commandBuffers.size()),
  };
  vku__check(vkAllocateCommandBuffers(this->handle, &allocInfo, this->commandBuffers.data()));

  // ..  Recording
  VkClearValue const             clearColor { { { 0.0175f, 0.0f, 0.0175f, 1.0f } } };
  VkCommandBufferBeginInfo const beginInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    // .flags            = 0,        // Optional
    .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
    .pInheritanceInfo = nullptr,  // Optional
  };

  for (size_t i = 0; i < this->commandBuffers.size(); ++i) {
    vku__check(vkBeginCommandBuffer(this->commandBuffers[i], &beginInfo));

    // .. Renderpass begin
    VkRenderPassBeginInfo const renderPassInfo {
      .sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass        = this->renderpass,
      .framebuffer       = this->swapchainFramebuffers[i],
      .renderArea.offset = { 0, 0 },
      .renderArea.extent = this->swapchainSettings.extent2D,
      .clearValueCount   = 1,
      .pClearValues      = &clearColor,
    };

    vkCmdBeginRenderPass(this->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(this->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);
#if 1
    vkCmdDraw(this->commandBuffers[i], 3, 1, 0, 0);
#else
    vkCmdDraw(this->commandBuffers[i], 6, 1, 0, 0);
#endif
    vkCmdEndRenderPass(this->commandBuffers[i]);

    vku__check(vkEndCommandBuffer(this->commandBuffers[i]));
  }

  //=====

}  // void Device::createGraphicsPipeline()

//-----------------------------------------------

void Device::create(Instance const &instance)
{
  // . Create (Logical) Device

  // ..  Copy data from INSTANCE
  mInstance         = &instance;
  this->queuesIndex = mInstance->queuesIndex;

  // ..  Queues' Create Infos
  float const                          queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  for (uint32_t queueFamily : vku::queuefamily::getUniqueIndices(this->queuesIndex)) {
    queueCreateInfos.push_back(VkDeviceQueueCreateInfo {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount       = 1,
      .pQueuePriorities = &queuePriority,
    });
  }

  // ..  Device's Create Info
  VkDeviceCreateInfo const deviceCreateInfo {
    .sType            = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pEnabledFeatures = &mInstance->physicalDeviceFeatures,
    // Queues info
    .pQueueCreateInfos    = queueCreateInfos.data(),
    .queueCreateInfoCount = vku__castsize(queueCreateInfos.size()),
    // Set device extensions
    .enabledExtensionCount   = vku__castsize(vo::sDeviceExtensions.size()),
    .ppEnabledExtensionNames = vo::sDeviceExtensions.data(),
    // Set device validation layers
    .enabledLayerCount   = vku__castsize(vo::sValidationLayers.size()),
    .ppEnabledLayerNames = vo::sValidationLayers.data(),
  };

  // ..  Create Device
  vku__check(vkCreateDevice(mInstance->physicalDevice, &deviceCreateInfo, nullptr, &this->handle));

  // ..  Pick required Queues
  this->queues = vku::queuefamily::findQueues(this->handle, this->queuesIndex);

  //=====

  // . Create CommandPool
  /// @DANI Maybe move this out and create one per thread

  VkCommandPoolCreateInfo commandpoolCreateInfo {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = this->queueGraphicsIndex(),
    .flags            = 0,  // Optional
  };
  vku__check(vkCreateCommandPool(this->handle, &commandpoolCreateInfo, nullptr, &this->commandpool));

  //=====

  // . Sync objects

  this->imageSemaphores.resize(this->inFlightMaxFrames);
  this->renderSempahores.resize(this->inFlightMaxFrames);
  this->inFlightFencesSubmit.resize(this->inFlightMaxFrames);
  // this->inFlightFencesAcquire.resize(this->swapchainImages.size(), VK_NULL_HANDLE);

  static VkSemaphoreCreateInfo const semaphoreInfo {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  VkFenceCreateInfo fenceInfo {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,  // Initialize on creation to avoid 'freezes'
  };

  for (size_t i = 0; i < this->inFlightMaxFrames; ++i) {
    vku__check(vkCreateSemaphore(this->handle, &semaphoreInfo, nullptr, &this->imageSemaphores[i]));
    vku__check(vkCreateSemaphore(this->handle, &semaphoreInfo, nullptr, &this->renderSempahores[i]));
    vku__check(vkCreateFence(this->handle, &fenceInfo, nullptr, &this->inFlightFencesSubmit[i]));
  }

  //=====

  createSwapChain();
  // createGraphicsPipeline();

  //=====
}

//-----------------------------------------------

void Device::reCreateSwapChain()
{
  vkDeviceWaitIdle(this->handle);

  destroySwapChainRelated();

  createSwapChain();
  createGraphicsPipeline();
  //   createCommandBuffers();
}

//-----------------------------------------------

void Device::destroySwapChainRelated()
{
  for (auto framebuffer : this->swapchainFramebuffers) { vkDestroyFramebuffer(this->handle, framebuffer, nullptr); }

  if (this->commandBuffers.size() > 0) {
    vkFreeCommandBuffers(
      this->handle,
      this->commandpool,
      vku__castsize(this->commandBuffers.size()),
      this->commandBuffers.data());
  }

  vkDestroyPipeline(this->handle, this->graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(this->handle, this->pipelineLayout, nullptr);
  vkDestroyRenderPass(this->handle, this->renderpass, nullptr);  // after: mPipelineLayout

  for (auto imageView : this->swapchainImageViews) { vkDestroyImageView(this->handle, imageView, nullptr); }

  // NOTE: Do not destroy here the swap-chain in order to use it as .swapchainOld parameter and have a better exchange
  // on swapchain recreations.
}

//-----------------------------------------------

void Device::destroy()
{
  destroySwapChainRelated();
  vkDestroySwapchainKHR(this->handle, this->swapchain, nullptr);

  for (size_t i = 0; i < this->inFlightMaxFrames; i++) {
    vkDestroySemaphore(this->handle, this->imageSemaphores[i], nullptr);
    vkDestroySemaphore(this->handle, this->renderSempahores[i], nullptr);
    vkDestroyFence(this->handle, this->inFlightFencesSubmit[i], nullptr);
  }

  for (auto [name, shaderModule] : this->shaderModules) { vkDestroyShaderModule(this->handle, shaderModule, nullptr); }

  vkDestroyCommandPool(this->handle, this->commandpool, nullptr);

  vkDestroyDevice(this->handle, nullptr);
}

//-----------------------------------------------
//-----------------------------------------------

}  // namespace vo::vulkan
