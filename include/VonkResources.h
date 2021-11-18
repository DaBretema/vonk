#pragma once
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "Macros.h"
#include "Utils.h"
#include "VonkToStr.h"
#include "VonkTypes.h"
#include "VonkTools.h"
#include "VonkWindow.h"
#include "_vulkan.h"

namespace vonk
{  //

// INSTANCEs
//-----------------------------------------------
Instance_t createInstance(const char *title = "VONK", uint32_t apiVersion = VK_API_VERSION_1_2);
void       destroyInstance(Instance_t &instance);

// GPUs (PHYSICAL DEVICEs)
//-----------------------------------------------
Gpu_t pickGpu(Instance_t &instance, bool enableGraphics, bool enablePresent, bool enableCompute, bool enableTransfer);

// DEVICEs
//-----------------------------------------------
Device_t createDevice(Instance_t const &instance, Gpu_t const &gpu);
void     destroyDevice(Device_t &device);

// RENDER PASSes
//-----------------------------------------------
VkRenderPass createRenderPass(VkDevice device, RenderPassData_t const &rpd);
VkRenderPass createDefaultRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat);

// TEXTUREs
//-----------------------------------------------
Texture_t createTexture(
  VkDevice                                device,
  VkPhysicalDeviceMemoryProperties const &memProps,
  VkExtent2D const &                      extent2D,
  VkFormat const &                        format,
  VkSampleCountFlagBits const &           samples,
  VkImageUsageFlags const &               usage,
  VkImageAspectFlagBits const &           aspectMaskBits);
void destroyTexture(VkDevice device, Texture_t const &tex);
bool isEmptyTexture(Texture_t const &tex);

// SEMAPHOREs
//-----------------------------------------------
VkSemaphore createSemaphore(VkDevice device);

// FENCEs
//-----------------------------------------------
VkFence createFence(VkDevice device);

// SWAP CHAIN
//-----------------------------------------------
SwapChain_t createSwapChain(Device_t const &device, SwapChain_t oldSwapChain);
void        destroySwapChain(SwapChain_t &swapchain, bool justForRecreation = false);

// SHADERs
//-----------------------------------------------
#define VONK_SHADER_CACHE 0
Shader_t     createShader(VkDevice device, std::string const &name, VkShaderStageFlagBits stage);
DrawShader_t createDrawShader(
  Device_t const &   device,
  std::string const &vert,
  std::string const &frag,
  std::string const &tesc,
  std::string const &tese,
  std::string const &geom);
void destroyDrawShader(VkDevice device, DrawShader_t const &ds);
// inline void destroyShader(Shader_t const &shader) {
//   vkDestroyShaderModule(shader.pDevice->handle, shader.module, nullptr);
// }

// PIPELINEs
//-----------------------------------------------
DrawPipeline_t createPipeline(
  DrawPipeline_t const &            oldPipeline,
  DrawPipelineData_t const &        ci,
  SwapChain_t const &               swapchain,
  VkDevice                          device,
  VkCommandPool                     commandPool,
  VkRenderPass                      renderpass,
  std::vector<VkFramebuffer> const &frameBuffers);
void destroyPipeline(SwapChain_t const &swapchain, DrawPipeline_t const &pipeline);

}  // namespace vonk
