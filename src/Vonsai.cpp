#include "Vonsai.h"
#include "VoVulkanCreators.h"

namespace vo
{  //

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

void Vonsai::run()
{
  //===== Window

  vo::window::init(mW, mH, "Vonsai");

  //===== Vulkan Create
  mVulkan.init();

  //===== Define actions
  vku::RenderPassData_t const rpd {
    .attachments    = { {
      // .format  = mSwapChain.settings.surfaceFormat.format,
      .samples        = VK_SAMPLE_COUNT_1_BIT,  // VK_SAMPLE_COUNT_1_BIT for No-Multisampling
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    } },
    .attachmentRefs = { {
      .attachment = 0,
      .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    } },
    .subpassDescs   = { {
      .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = vku__castSize(1),        // we don't want all of them in every subpass
      .pColorAttachments    = &rpd.attachmentRefs[0],  // we don't want all of them in every subpass
    } },
    .subpassDeps    = { {
      .srcSubpass    = VK_SUBPASS_EXTERNAL,
      .dstSubpass    = 0,
      .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    } },
  };
  vku::FixedFuncs_t         fixed = vku::creators::fixedFuncs(vo::window::getFramebufferSize());
  vku::PipelineLayoutData_t pld;
  vku::ShadersData_t shaders = { { "base", VK_SHADER_STAGE_VERTEX_BIT }, { "base", VK_SHADER_STAGE_FRAGMENT_BIT } };

  vku::CommandBufferData_t cmd_0;
  cmd_0.commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 6, 1, 0, 0); };

  mVulkan.addPipeline(fixed, rpd, pld, shaders, { cmd_0 });

  //===== Loop
  vo::window::loop(
    [&]() { mVulkan.drawFrame(); },   //
    [&]() { mVulkan.waitDevice(); },  //
    false                             //
  );

  //===== Clean up
  mVulkan.cleanup();
  // mDevice.destroy();
  // mInstance.destroy();

  //=====
}

}  // namespace vo
