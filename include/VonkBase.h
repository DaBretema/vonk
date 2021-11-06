#pragma once

#include <vector>
#include <unordered_map>

#include "_vulkan.h"
#include "VonkTypes.h"
#include "VonkTools.h"

namespace vonk
{  //

class Base
{
public:
  Base() = default;

  void init();
  void cleanup();
  void drawFrame();
  void waitDevice();
  void addPipeline(PipelineData_t const &ci);

  inline auto currentFormat() const { return mSwapChain.settings.colorFormat; }

  inline void iterScenes() { mActivePipeline = (mActivePipeline + 1) % mPipelines.size(); }

private:
  void swapchainCreate();
  void swapchainReCreate();
  void swapchainCleanUp();

  Instance_t  mInstance;
  Gpu_t       mGpu;
  Device_t    mDevice;
  SwapChain_t mSwapChain;
  SyncBase_t  mSync;

  Texture_t                  mDefaultDepthTexture;
  VkRenderPass               mDefaultRenderPass;
  std::vector<VkFramebuffer> mDefaultFrameBuffers;

  std::vector<Pipeline_t>     mPipelines;
  std::vector<PipelineData_t> mPipelinesCI;
  uint32_t                    mActivePipeline = 0u;

  // Settings:
  uint32_t                  sInFlightMaxFrames  = 3;
  std::vector<const char *> sValidationLayers   = { "VK_LAYER_KHRONOS_validation" };
  std::vector<const char *> sInstanceExtensions = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
  std::vector<const char *> sDeviceExtensions   = { "VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME };

};  // class Base
}  // namespace vonk
