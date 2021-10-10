#include "Vonsai.h"

#include <map>
#include <vector>

#include <fmt/core.h>

#include "Utils.h"
#include "Macros.h"
#include "Settings.h"

namespace vo
{
//-----------------------------------------------------------------------------
// === ENTRY POINT
//-----------------------------------------------------------------------------

void Vonsai::run()
{
  // ::: INIT WINDOW
  vo::window::init(1280, 720, "Vonsai");

  // ::: INIT VULKAN INSTANCE
  initVulkan();

  // ::: INIT VULKAN DEVICE

  // ::: MAIN LOOP
  // vo::window::loop([&]() { drawFrame(); }, [&]() { vkDeviceWaitIdle(mDevice.handle); });

  // ::: CLEAN UP
  // cleanup();
}

//-----------------------------------------------------------------------------
// === TO RUN
//-----------------------------------------------------------------------------

void Vonsai::initVulkan()
{
  vku::createInstance(mInstance);
  vo__check(mInstance.handle);

  vku::createDevice(mDevice, mInstance);
  vo__check(mDevice.handle);

  vku::destroyDevice(mDevice);
  vku::destroyInstance(mInstance);

  //   createInstance();
  // // vku::debugmessenger::create(mInstance, &??mDebugMessenger);
  // mDebugMessenger.create(mInstance);

  // createSurface();

  // pickPhysicalDevice();

  // createLogicalDevice();

  // createSwapChain();

  // createImageViews();

  // createRenderPass();

  // createShaders();

  // createGraphicsPipeline();

  // createFramebuffers();

  // createCommandPool();

  // createCommandBuffers();

  // createSyncObjects();
}

//-----------------------------------------------------------------------------

// void Vonsai::drawFrame()
// {
//   size_t currFrame = 0;

//   // . Fence management
//   vkWaitForFences(vo::vulkan::device.handle, 1, &mInFlightFencesSubmit[currFrame], VK_TRUE, UINT64_MAX);

//   // . Acquiere next image
//   uint32_t   imageIndex;
//   auto const acquireRet = vkAcquireNextImageKHR(
//     vo::vulkan::device.handle,
//     mSwapChain,
//     UINT64_MAX,
//     mImageSemaphores[currFrame],
//     VK_NULL_HANDLE,
//     &imageIndex);

//   // .. Validate the SwapChain
//   if (acquireRet == VK_ERROR_OUT_OF_DATE_KHR) {
//     recreateSwapChain();
//     return;
//   } else if (acquireRet != VK_SUCCESS && acquireRet != VK_SUBOPTIMAL_KHR) {
//     VO_ERR("Failed to acquire swap chain image!");
//   }

//   // .. Check if a previous frame is using this image (i.e. there is its fence to wait on)
//   if (mInFlightFencesAcquire[imageIndex] != VK_NULL_HANDLE) {
//     vkWaitForFences(vo::vulkan::device.handle, 1, &mInFlightFencesAcquire[imageIndex], VK_TRUE, UINT64_MAX);
//   }
//   // .. Mark the image as now being in use by this frame
//   mInFlightFencesAcquire[imageIndex] = mInFlightFencesSubmit[currFrame];

//   // . Submitting the command buffer
//   VkSubmitInfo submitInfo {};
//   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

//   VkSemaphore          waitSemaphores[] = { mImageSemaphores[currFrame] };
//   VkPipelineStageFlags waitStages[]     = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//   submitInfo.waitSemaphoreCount         = 1;
//   submitInfo.pWaitSemaphores            = waitSemaphores;
//   submitInfo.pWaitDstStageMask          = waitStages;
//   submitInfo.commandBufferCount         = 1;
//   submitInfo.pCommandBuffers            = &mCommandBuffers[imageIndex];

//   VkSemaphore signalSemaphores[]  = { mRenderSempahores[currFrame] };
//   submitInfo.signalSemaphoreCount = 1;
//   submitInfo.pSignalSemaphores    = signalSemaphores;

//   vkResetFences(vo::vulkan::device.handle, 1, &mInFlightFencesSubmit[currFrame]);
//   VW_CHECK(vkQueueSubmit(vo::vulkan::device.queueGraphics, 1, &submitInfo, mInFlightFencesSubmit[currFrame]));

//   // . Presentation
//   VkPresentInfoKHR presentInfo {};
//   presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//   presentInfo.waitSemaphoreCount = 1;
//   presentInfo.pWaitSemaphores    = signalSemaphores;

//   VkSwapchainKHR swapChains[] = { mSwapChain };
//   presentInfo.swapchainCount  = 1;
//   presentInfo.pSwapchains     = swapChains;
//   presentInfo.pImageIndices   = &imageIndex;
//   presentInfo.pResults        = nullptr;  // Optional

//   auto const presentRet = vkQueuePresentKHR(vo::vulkan::device.queuePresent, &presentInfo);
//   if (presentRet == VK_ERROR_OUT_OF_DATE_KHR || presentRet == VK_SUBOPTIMAL_KHR) {
//     recreateSwapChain();
//   } else if (presentRet != VK_SUCCESS) {
//     VO_ERR("Failed to present swap chain image!");
//   }

//   // . Iter flight-frame
//   currFrame = (currFrame + 1) % mMaxFlightFrames;
// }

// //-----------------------------------------------------------------------------

// void Vonsai::cleanup()
// {
//   // . Swap-Chain
//   cleanupSwapChain();
//   vkDestroySwapchainKHR(vo::vulkan::device.handle, mSwapChain, nullptr);

//   // . Logical Device
//   // .. Dependencies
//   for (size_t i = 0; i < mMaxFlightFrames; i++) {
//     vkDestroySemaphore(vo::vulkan::device.handle, mImageSemaphores[i], nullptr);
//     vkDestroySemaphore(vo::vulkan::device.handle, mRenderSempahores[i], nullptr);
//     vkDestroyFence(vo::vulkan::device.handle, mInFlightFencesSubmit[i], nullptr);
//   }

//   vkDestroyCommandPool(vo::vulkan::device.handle, mCommandPool, nullptr);

//   for (auto [name, shaderModule] : mShaderModules) {
//     vkDestroyShaderModule(vo::vulkan::device.handle, shaderModule, nullptr);
//   }

//   // .. Itself
//   vkDestroyDevice(vo::vulkan::device.handle, nullptr);

//   // . Instance
//   // .. Dependencies
//   // vku::debugmessenger::destroy(mInstance, mDebugMessenger);
//   // mDebugMessenger.destroy(vo::vulkan::instance.handle);
//   vkDestroySurfaceKHR(vo::vulkan::instance.handle, vo::vulkan::instance.surface, nullptr);
//   // .. Itself
//   vkDestroyInstance(vo::vulkan::instance.handle, nullptr);

//   // . Window
//   vo::window::cleanup();
// }

//=====================================================================================================================

// //-----------------------------------------------------------------------------

// void Vonsai::createRenderPass()
// {
//   /*
//    * VkAttachmentDescription.loadOp :
//        - VK_ATTACHMENT_LOAD_OP_LOAD      : Preserve the existing contents of the attachment.
//        - VK_ATTACHMENT_LOAD_OP_CLEAR     : Clear the values to a constant at the start.
//        - VK_ATTACHMENT_LOAD_OP_DONT_CARE : Existing contents are undefined; we don't care about them.
//    * VkAttachmentDescription.storeOp :
//        - VK_ATTACHMENT_STORE_OP_STORE     : Rendered contents will be stored in memory and can be read later.
//        - VK_ATTACHMENT_STORE_OP_DONT_CARE : Contents of the framebuffer will be undefined after the rendering
//        operation.

//    * Every subpass references one or more of the attachments using VkAttachmentReference. This allows attachment
//    resuse between render-subpasses.

//    * VkSubpassDescription.[...]
//        - pInputAttachments       : Attachments that are read from a shader.
//        - pDepthStencilAttachment : Attachment for depth and stencil data.
//        - pResolveAttachments     : Attachments used for multisampling color attachments.
//        - pPreserveAttachments    : Attachments that are not used by this subpass, but data must be preserved.
//    */

//   // . Attachment
//   VkAttachmentDescription colorAttachment {};
//   colorAttachment.format  = mSwapChainSettings.surfaceFormat.format;
//   colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // VK_SAMPLE_COUNT_1_BIT for No-Multisampling
//   colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
//   colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//   // .. stencil
//   colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//   colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//   // .. init and final
//   colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//   colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

//   // . Subpass
//   // .. References
//   VkAttachmentReference colorAttachmentRef {};
//   colorAttachmentRef.attachment = 0;
//   colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//   // .. Description
//   VkSubpassDescription subpass {};
//   subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;  // also could be: Compute or Raytracing
//   subpass.colorAttachmentCount = 1;
//   subpass.pColorAttachments    = &colorAttachmentRef;

//   // . [Create-Info] - RenderPass
//   VkRenderPassCreateInfo renderPassInfo {};
//   renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//   renderPassInfo.attachmentCount = 1;
//   renderPassInfo.pAttachments    = &colorAttachment;
//   renderPassInfo.subpassCount    = 1;
//   renderPassInfo.pSubpasses      = &subpass;

//   // . Subpass dependencies (required for user-defined subpasses and 'implicit' ones)
//   VkSubpassDependency dependency {};
//   dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
//   dependency.dstSubpass          = 0;
//   dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//   dependency.srcAccessMask       = 0;
//   dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//   dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//   renderPassInfo.dependencyCount = 1;
//   renderPassInfo.pDependencies   = &dependency;

//   VW_CHECK(vkCreateRenderPass(vo::vulkan::device.handle, &renderPassInfo, nullptr, &mRenderPass));
// }

// //-----------------------------------------------------------------------------

// void Vonsai::createShaders()
// {
//   static auto const createShadersSameName =
//     [&](std::string const &name, std::initializer_list<VkShaderStageFlagBits> stages) {
//       for (auto const stage : stages) {
//         auto const data = vku::shaders::create(vo::vulkan::device.handle, name, stage);
//         mShaderModules.emplace(data.path, data.module);
//         mPipelineShaderStageCreateInfos.emplace_back(data.stageCreateInfo);
//       }
//     };

//   createShadersSameName("base", { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT });
// }

// //-----------------------------------------------------------------------------

// void Vonsai::createGraphicsPipeline()
// {
//   /*
//    * Maximum line width: Depends on the hardware and any line thicker than 1.0f requires you to enable the {
//    wideLines }
//    * GPU feature.
//    */

//   // . Viewport

//   // .. Base
//   VkViewport viewport {};
//   viewport.x        = 0.0f;
//   viewport.y        = 0.0f;
//   viewport.width    = (float)mSwapChainSettings.extent2D.width;
//   viewport.height   = (float)mSwapChainSettings.extent2D.height;
//   viewport.minDepth = 0.0f;
//   viewport.maxDepth = 1.0f;

//   // .. Scissor
//   VkRect2D scissor {};
//   scissor.offset = { 0, 0 };
//   scissor.extent = mSwapChainSettings.extent2D;

//   // .. CreateInfo
//   VkPipelineViewportStateCreateInfo viewportState {};
//   viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//   viewportState.viewportCount = 1;
//   viewportState.pViewports    = &viewport;
//   viewportState.scissorCount  = 1;
//   viewportState.pScissors     = &scissor;

//   // . Rasterizer
//   VkPipelineRasterizationStateCreateInfo rasterizer {};
//   rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

//   rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // or _LINE / _POINT as always
//   rasterizer.cullMode    = VK_CULL_MODE_BACK_BIT;
//   rasterizer.frontFace   = VK_FRONT_FACE_CLOCKWISE;

//   // .. Disable any output
//   rasterizer.rasterizerDiscardEnable = VK_FALSE;

//   // .. Usefull for Shadow-Maps
//   rasterizer.depthClampEnable        = VK_FALSE;
//   rasterizer.depthBiasEnable         = VK_FALSE;
//   rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
//   rasterizer.depthBiasClamp          = 0.0f;  // Optional
//   rasterizer.depthBiasSlopeFactor    = 0.0f;  // Optional
//   rasterizer.lineWidth               = 1.0f;

//   // . Multisampling : for now disabled
//   VkPipelineMultisampleStateCreateInfo multisampling {};
//   multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//   multisampling.sampleShadingEnable   = VK_FALSE;
//   multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
//   multisampling.minSampleShading      = 1.0f;      // Optional
//   multisampling.pSampleMask           = nullptr;   // Optional
//   multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
//   multisampling.alphaToOneEnable      = VK_FALSE;  // Optional

//   // . Depth / Stencil
//   VkPipelineDepthStencilStateCreateInfo depthStencil {};
//   depthStencil.sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//   depthStencil.stencilTestEnable = VK_FALSE;
//   depthStencil.depthWriteEnable  = VK_TRUE;
//   depthStencil.depthTestEnable   = VK_TRUE;
//   depthStencil.depthCompareOp    = VkCompareOp::VK_COMPARE_OP_LESS;

//   // . Blending

//   // .. Attacment state
//   VkPipelineColorBlendAttachmentState colorBlendAttachment {};
//   colorBlendAttachment.colorWriteMask =
//     VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//   // ... Color pass through
//   colorBlendAttachment.blendEnable         = VK_FALSE;
//   colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
//   colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
//   colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;       // Optional
//   colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
//   colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
//   colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;       // Optional
//   // ... Classic alpha blending
//   // colorBlendAttachment.blendEnable         = VK_TRUE;
//   // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//   // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//   // colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
//   // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//   // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//   // colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

//   // .. Create info
//   VkPipelineColorBlendStateCreateInfo colorBlending {};
//   colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//   colorBlending.logicOpEnable     = VK_FALSE;
//   colorBlending.logicOp           = VK_LOGIC_OP_COPY;  // Optional
//   colorBlending.attachmentCount   = 1;
//   colorBlending.pAttachments      = &colorBlendAttachment;
//   colorBlending.blendConstants[0] = 0.0f;  // Optional
//   colorBlending.blendConstants[1] = 0.0f;  // Optional
//   colorBlending.blendConstants[2] = 0.0f;  // Optional
//   colorBlending.blendConstants[3] = 0.0f;  // Optional

//   // . Dynamic state
//   VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

//   VkPipelineDynamicStateCreateInfo dynamicState {};
//   dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//   dynamicState.dynamicStateCount = 2;
//   dynamicState.pDynamicStates    = dynamicStates;

//   // . Layout
//   VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
//   pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//   pipelineLayoutInfo.setLayoutCount         = 0;        // Optional
//   pipelineLayoutInfo.pSetLayouts            = nullptr;  // Optional
//   pipelineLayoutInfo.pushConstantRangeCount = 0;        // Optional
//   pipelineLayoutInfo.pPushConstantRanges    = nullptr;  // Optional

//   VW_CHECK(vkCreatePipelineLayout(vo::vulkan::device.handle, &pipelineLayoutInfo, nullptr, &mPipelineLayout));

//   // . GRAPHCIS PIPELINE
//   VkGraphicsPipelineCreateInfo pipelineInfo {};
//   pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//   pipelineInfo.stageCount          = VW_SIZE_CAST(mPipelineShaderStageCreateInfos.size());
//   pipelineInfo.pStages             = mPipelineShaderStageCreateInfos.data();
//   pipelineInfo.pVertexInputState   = &mVertexInputInfo;
//   pipelineInfo.pInputAssemblyState = &mInputAssembly;
//   pipelineInfo.pViewportState      = &viewportState;
//   pipelineInfo.pRasterizationState = &rasterizer;
//   pipelineInfo.pMultisampleState   = &multisampling;
//   pipelineInfo.pDepthStencilState  = &depthStencil;  // Optional
//   pipelineInfo.pColorBlendState    = &colorBlending;
//   // pipelineInfo.pDynamicState       = &dynamicState;  // Optional
//   pipelineInfo.layout             = mPipelineLayout;
//   pipelineInfo.renderPass         = mRenderPass;
//   pipelineInfo.subpass            = 0;               // index of subpass (or first subpass, not sure yet...)
//   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
//   pipelineInfo.basePipelineIndex  = -1;              // Optional

//   VW_CHECK(vkCreateGraphicsPipelines(
//     vo::vulkan::device.handle,
//     VK_NULL_HANDLE,
//     1,
//     &pipelineInfo,
//     nullptr,
//     &mGraphicsPipeline));
// }

// //-----------------------------------------------------------------------------

// void Vonsai::createFramebuffers()
// {
//   /*
//   * VkFramebufferCreateInfo.attachmentCount could vary between techniques,
//       I mean this is the point where you define MRT right?
//       So i.e. for Deferred rendering here, we wil need something like 5 attachmentCount, previously created, for
//       sure.
//   */

//   mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

//   for (size_t i = 0; i < mSwapChainImageViews.size(); ++i) {
//     // . Get its VkImageView
//     VkImageView attachments[] = { mSwapChainImageViews[i] };

//     // . [Create-Info] - FrameBuffer
//     VkFramebufferCreateInfo framebufferInfo {};
//     framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//     framebufferInfo.renderPass      = mRenderPass;
//     framebufferInfo.attachmentCount = 1;
//     framebufferInfo.pAttachments    = attachments;
//     framebufferInfo.width           = mSwapChainSettings.extent2D.width;
//     framebufferInfo.height          = mSwapChainSettings.extent2D.height;
//     framebufferInfo.layers          = 1;

//     // . Creation
//     VW_CHECK(vkCreateFramebuffer(vo::vulkan::device.handle, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]));
//   }
// }

// //-----------------------------------------------------------------------------

// void Vonsai::createCommandPool()
// {
//   VkCommandPoolCreateInfo poolInfo {};
//   poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//   poolInfo.queueFamilyIndex = vo::vulkan::device.queueGraphicsIndex;
//   poolInfo.flags            = 0;  // Optional

//   VW_CHECK(vkCreateCommandPool(vo::vulkan::device.handle, &poolInfo, nullptr, &mCommandPool));
// }

// //-----------------------------------------------------------------------------

// void Vonsai::createCommandBuffers()
// {
//   /*
//   * The VkCommandBufferAllocateInfo.level parameter specifies:
//       - VK_COMMAND_BUFFER_LEVEL_PRIMARY:
//           [V] Submitted to a queue for execution.
//           [X] Called from other command buffers.
//       - VK_COMMAND_BUFFER_LEVEL_SECONDARY:
//           [V] Called from primary command buffers.
//           [X] Submitted directly.
//   */

//   // . Allocating command-buffers
//   mCommandBuffers.resize(mSwapChainFramebuffers.size());

//   VkCommandBufferAllocateInfo allocInfo {};
//   allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//   allocInfo.commandPool        = mCommandPool;
//   allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//   allocInfo.commandBufferCount = VW_SIZE_CAST(mCommandBuffers.size());

//   VW_CHECK(vkAllocateCommandBuffers(vo::vulkan::device.handle, &allocInfo, mCommandBuffers.data()));

//   // . Recording
//   VkClearValue const clearColor = { { { 0.0175f, 0.0f, 0.0175f, 1.0f } } };

//   for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
//     VkCommandBufferBeginInfo beginInfo {};
//     beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//     // beginInfo.flags            = 0;        // Optional
//     beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
//     beginInfo.pInheritanceInfo = nullptr;  // Optional

//     VW_CHECK(vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo));

//     // .. Renderpass begin
//     VkRenderPassBeginInfo renderPassInfo {};
//     renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//     renderPassInfo.renderPass        = mRenderPass;
//     renderPassInfo.framebuffer       = mSwapChainFramebuffers[i];
//     renderPassInfo.renderArea.offset = { 0, 0 };
//     renderPassInfo.renderArea.extent = mSwapChainSettings.extent2D;
//     renderPassInfo.clearValueCount   = 1;
//     renderPassInfo.pClearValues      = &clearColor;

//     vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//     vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
// #if VO_HARDCODED_SHAPE == 0
//     vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);
// #else
//     vkCmdDraw(mCommandBuffers[i], 6, 1, 0, 0);
// #endif
//     // vkCmdDrawIndexed(mCommandBuffers[i], 6, 1, 0, 3, 0);
//     vkCmdEndRenderPass(mCommandBuffers[i]);

//     VW_CHECK(vkEndCommandBuffer(mCommandBuffers[i]));
//   }
// }

// //-----------------------------------------------------------------------------

// void Vonsai::createSyncObjects()
// {
//   mImageSemaphores.resize(mMaxFlightFrames);
//   mRenderSempahores.resize(mMaxFlightFrames);
//   mInFlightFencesSubmit.resize(mMaxFlightFrames);
//   mInFlightFencesAcquire.resize(mSwapChainImages.size(), VK_NULL_HANDLE);

//   VkSemaphoreCreateInfo semaphoreInfo {};
//   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

//   VkFenceCreateInfo fenceInfo {};
//   fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Initialize on creation

//   for (size_t i = 0; i < mMaxFlightFrames; ++i) {
//     VW_CHECK(vkCreateSemaphore(vo::vulkan::device.handle, &semaphoreInfo, nullptr, &mImageSemaphores[i]));
//     VW_CHECK(vkCreateSemaphore(vo::vulkan::device.handle, &semaphoreInfo, nullptr, &mRenderSempahores[i]));
//     VW_CHECK(vkCreateFence(vo::vulkan::device.handle, &fenceInfo, nullptr, &mInFlightFencesSubmit[i]));
//   }
// }

// //-----------------------------------------------------------------------------

// void Vonsai::cleanupSwapChain()
// {
//   VO_TRACE(2, "Destroy FrameBuffers");
//   for (auto framebuffer : mSwapChainFramebuffers) {
//     VO_TRACE(3, PtrStr(framebuffer));
//     vkDestroyFramebuffer(vo::vulkan::device.handle, framebuffer, nullptr);
//   }

//   VO_TRACE(2, "Free CommandBuffers");
//   vkFreeCommandBuffers(
//     vo::vulkan::device.handle,
//     mCommandPool,
//     VW_SIZE_CAST(mCommandBuffers.size()),
//     mCommandBuffers.data());

//   VO_TRACE(2, "Destroy Graphics-Pipeline : Pipeline Itself");
//   VO_TRACE(3, PtrStr(mGraphicsPipeline));
//   vkDestroyPipeline(vo::vulkan::device.handle, mGraphicsPipeline, nullptr);

//   VO_TRACE(2, "Destroy Graphics-Pipeline : Pipeline Layout");
//   VO_TRACE(3, PtrStr(mPipelineLayout));
//   vkDestroyPipelineLayout(vo::vulkan::device.handle, mPipelineLayout, nullptr);

//   VO_TRACE(2, "Destroy Graphics-Pipeline : Render Pass");
//   VO_TRACE(3, PtrStr(mRenderPass));
//   vkDestroyRenderPass(vo::vulkan::device.handle, mRenderPass, nullptr);  // after: mPipelineLayout

//   VO_TRACE(2, "Destroy Image-Views");
//   for (auto imageView : mSwapChainImageViews) {
//     VO_TRACE(3, PtrStr(imageView));
//     vkDestroyImageView(vo::vulkan::device.handle, imageView, nullptr);
//   }

//   mOldSwapChain = mSwapChain;
// }

// //-----------------------------------------------

// void Vonsai::recreateSwapChain()
// {
//   VO_TRACE(1, "Re-Create SWAP-CHAIN");

//   vkDeviceWaitIdle(vo::vulkan::device.handle);

//   cleanupSwapChain();

//   createSwapChain();
//   createImageViews();
//   createRenderPass();
//   createGraphicsPipeline();
//   createFramebuffers();
//   createCommandBuffers();
// }

// //-----------------------------------------------------------------------------

}  // namespace vo
