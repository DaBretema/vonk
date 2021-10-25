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

  //===== Vulkan-Window linking
  vo::window::setUserPointer(&mVulkan);

  //===== Vulkan-Window setup
  // . Keyboard input
  vo::window::setCallbackKeyboard(
    [](MBU GLFWwindow *windowHandle, int key, MBU int scancode, int action, MBU int mods) {
      auto *vku = static_cast<vku::Base *>(vo::window::userPtr);  // Cast UserPtr before use it.

      if (key == GLFW_KEY_1 and action == GLFW_PRESS) { vku->iterScenes(); }
    });

  //===== Define actions

  vku::RenderPassData_t rpd;
  rpd.attachments    = { {
    .format         = mVulkan.currentFormat(),  // @DANI : ATENTION!!!
    .samples        = VK_SAMPLE_COUNT_1_BIT,    // VK_SAMPLE_COUNT_1_BIT for No-Multisampling
    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  } };
  rpd.attachmentRefs = { {
    .attachment = 0,
    .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  } };
  rpd.subpassDescs   = { {
    .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = vku__castSize(1),        // we don't want all of them in every subpass
    .pColorAttachments    = &rpd.attachmentRefs[0],  // we don't want all of them in every subpass
  } };
  rpd.subpassDeps    = { {
    .srcSubpass    = VK_SUBPASS_EXTERNAL,
    .dstSubpass    = 0,
    .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  } };

  vku::PipelineCreateInfo_t const pipelineCI {
    // . Static
    .fixedFuncs         = vku::creators::fixedFuncs(),
    .shadersData        = { { "base", VK_SHADER_STAGE_VERTEX_BIT }, { "base", VK_SHADER_STAGE_FRAGMENT_BIT } },
    .renderPassData     = rpd,
    .pipelineLayoutData = {},

    // . Dynamic
    .viewports          = { {
      .x        = 0.0f,
      .y        = 0.0f,
      .width    = -100.f,
      .height   = -100.f,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    } },
    .scissors           = { {
      .offset = { 0, 0 },
      .extent = { UINT32_MAX, UINT32_MAX },
    } },
    .commandBuffersData = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 3, 1, 0, 0); } } },
  };

  vku::PipelineCreateInfo_t pipelineCI2 = pipelineCI;
  pipelineCI2.shadersData = { { "base2", VK_SHADER_STAGE_VERTEX_BIT }, { "base", VK_SHADER_STAGE_FRAGMENT_BIT } },
  pipelineCI2.commandBuffersData = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 6, 1, 0, 0); } } },

  mVulkan.addPipeline(pipelineCI);
  mVulkan.addPipeline(pipelineCI2);

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
