#pragma once

#include "_vulkan.h"
#include <vector>
#include <unordered_map>
#include <VoVulkanTypes.h>
#include <VoVulkanUtils.h>

namespace vo::vulkan
{  //

class Base
{
public:
  void init();
  void cleanup();
  void drawFrame();
  void waitDevice();

  inline void addPipeline(
    FixedFuncs_t         fixed,
    RenderPassData_t     rpd,
    PipelineLayoutData_t pld,
    ShadersData_t        shadersdata,
    CommandBuffersData_t commandBuffersData)
  {
    // . Create the scene/pipeline
    auto &&pipeline = mScenes.emplace_back();

    // . Render Pass things
    for (auto &att : rpd.attachments) { att.format = mSwapChain.settings.surfaceFormat.format; }

    VkRenderPassCreateInfo const renderpassCI {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = vku__getSize(rpd.attachments),
      .pAttachments    = vku__getData(rpd.attachments),
      .subpassCount    = vku__getSize(rpd.subpassDescs),
      .pSubpasses      = vku__getData(rpd.subpassDescs),
      .dependencyCount = vku__getSize(rpd.subpassDeps),
      .pDependencies   = vku__getData(rpd.subpassDeps),
    };

    vku__check(vkCreateRenderPass(mDevice.handle, &renderpassCI, nullptr, &pipeline.renderpass));

    // . Set framebuffers
    pipeline.frameBuffers.resize(mSwapChain.views.size());

    for (size_t i = 0; i < mSwapChain.views.size(); ++i) {
      VkImageView const             attachments[] = { mSwapChain.views[i] };
      VkFramebufferCreateInfo const framebufferCI {
        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass      = pipeline.renderpass,
        .attachmentCount = 1,  // Modify this for MRT ??
        .pAttachments    = attachments,
        .width           = mSwapChain.settings.extent2D.width,
        .height          = mSwapChain.settings.extent2D.height,
        .layers          = 1,
      };
      vku__check(vkCreateFramebuffer(mDevice.handle, &framebufferCI, nullptr, &pipeline.frameBuffers[i]));
    }

    // . Shaders
    for (auto const &sd : shadersdata) {
      auto const data = vku::shaders::create(mDevice.handle, sd.first, sd.second);
      pipeline.shaderModules.emplace(data.path, data.module);
      pipeline.stagesCI.emplace_back(data.stageCreateInfo);
    }

    // . Pipeline itself : @DANI research about PipelineCache object

    vku__check(vkCreatePipelineLayout(mDevice.handle, &pld.pipelineLayoutCI, nullptr, &pipeline.layout));

    VkGraphicsPipelineCreateInfo const graphicsPipelineCI {
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount          = vku__getSize(pipeline.stagesCI),
      .pStages             = vku__getData(pipeline.stagesCI),
      .pVertexInputState   = &pld.inputstateVertexCI,
      .pInputAssemblyState = &pld.inputstateAssemblyCI,
      .pViewportState      = &fixed.viewportStateCI,
      .pRasterizationState = &fixed.rasterizationStateCI,
      .pMultisampleState   = &fixed.multisamplingCI,
      .pDepthStencilState  = &fixed.depthstencilCI,
      .pColorBlendState    = &fixed.blendingCI,
      // .pDynamicState       = &dynamicStateCI,  // Optional
      .layout             = pipeline.layout,
      .renderPass         = pipeline.renderpass,
      .subpass            = 0,               // index of subpass (or first subpass, not sure yet...)
      .basePipelineHandle = VK_NULL_HANDLE,  // Optional
      .basePipelineIndex  = -1,              // Optional
    };
    vku__check(
      vkCreateGraphicsPipelines(mDevice.handle, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &pipeline.handle));

    // . Commad Buffers Allocation
    pipeline.commandBuffers.resize(pipeline.frameBuffers.size());
    VkCommandBufferAllocateInfo const commandBufferAllocInfo {
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = mDevice.commandPool,
      .commandBufferCount = vku__getSize(pipeline.commandBuffers),
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };
    vku__check(
      vkAllocateCommandBuffers(mDevice.handle, &commandBufferAllocInfo, vku__getData(pipeline.commandBuffers)));

    // . Commad Buffers Recording

    for (auto const &commandBuffesData : commandBuffersData) {
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
        .renderArea.extent = mSwapChain.settings.extent2D,
      };

      for (size_t i = 0; i < pipeline.commandBuffers.size(); ++i) {
        auto const commandBuffer = pipeline.commandBuffers[i];
        renderpassBI.framebuffer = pipeline.frameBuffers[i];
        vku__check(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));
        vkCmdBeginRenderPass(commandBuffer, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
        // vkCmdSetViewport(commandBuffer, 0, 1, &viewport);  // -> If Dynamic Viewport
        if (commandBuffesData.commands) { commandBuffesData.commands(commandBuffer); }
        vkCmdEndRenderPass(commandBuffer);
        vku__check(vkEndCommandBuffer(commandBuffer));
      }
    }

    // Store all data to be ables to recreate the pipeline.
    // NOTE: Otherwise on swapchain recreation we will got this error:
    // [UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImageView] : You are adding vkQueueSubmit to
    // VkCommandBuffer [...] that is invalid because bound VkImageView [...] was destroyed.

    //.
  };

private:
  void swapchainCreate();
  void swapchainReCreate();
  void swapchainCleanUp();

  static const inline uint32_t sInFlightMaxFrames = 3;

  Instance_t  mInstance;
  Gpu_t       mGpu;
  Device_t    mDevice;
  SwapChain_t mSwapChain;
  SyncBase_t  mSync;
  // Pipeline_t  mDrawPipeline;
  Scenes_t mScenes;
  uint32_t activeScene = 0u;

};  // class Base
}  // namespace vo::vulkan
