#pragma once

#include "_vulkan.h"
#include <vector>
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

  // void onRecreationNeeded();

private:
  void swapchainCreate();
  void swapchainReCreate();
  void swapchainCleanUp();
  void graphicspipelineCreateHARDCODED();

  static const inline uint32_t sInFlightMaxFrames = 3;

  Instance_t         mInstance;
  Gpu_t              mGpu;
  Device_t           mDevice;
  SwapChain_t        mSwapChain;
  SyncBase_t         mSync;
  GraphicsPipeline_t mDrawPipeline;

};  // class Base
}  // namespace vo::vulkan
