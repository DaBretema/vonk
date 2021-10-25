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
  void addPipeline(PipelineCreateInfo_t const &ci);

  inline auto currentFormat() { return mSwapChain.settings.surfaceFormat.format; }

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

  std::vector<Pipeline_t>           mPipelines;
  std::vector<PipelineCreateInfo_t> mPipelinesCI;
  uint32_t                          mActivePipeline = 0u;

};  // class Base
}  // namespace vo::vulkan
