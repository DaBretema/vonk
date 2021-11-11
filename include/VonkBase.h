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
  void recreateSwapChain();
  void destroySwapChainDependencies();

  Instance_t  mInstance;
  Gpu_t       mGpu;
  Device_t    mDevice;
  SwapChain_t mSwapChain;
  // SyncBase_t  mSync;

  std::vector<Pipeline_t>     mPipelines;
  std::vector<PipelineData_t> mPipelinesCI;
  uint32_t                    mActivePipeline = 0u;

  // Settings:
  uint32_t sInFlightMaxFrames = 3;

  // Resources:

};  // class Base
}  // namespace vonk
