#pragma once

#include <vector>
#include <unordered_map>

#include "_vulkan.h"
#include "VonkTypes.h"

namespace vonk
{  //

class Vonk
{
public:
  Vonk() = default;

  void init();
  void cleanup();
  void drawFrame();
  void waitDevice();
  void addPipeline(PipelineData_t const &ci);

  inline auto currentFormat() const { return mSwapChain.colorFormat; }

  inline void iterScenes() { mActivePipeline = (mActivePipeline + 1) % mPipelines.size(); }

  // . Shaders
  DrawShader_t const &
    createDrawShader(std::string const &keyName, std::string const &vertexName, std::string const &fragmentName);
  DrawShader_t const &getDrawShader(std::string const &keyName);
  Shader_t const &    createComputeShader(std::string const &name);
  Shader_t const &    getComputeShader(std::string const &name);

private:
  void recreateSwapChain();
  void destroySwapChainDependencies();

  Instance_t  mInstance;
  Gpu_t       mGpu;
  Device_t    mDevice;
  SwapChain_t mSwapChain;

  std::unordered_map<std::string, DrawShader_t> mDrawShaders;
  std::unordered_map<std::string, Shader_t>     mComputeShaders;

  std::vector<Pipeline_t>     mPipelines;
  std::vector<PipelineData_t> mPipelinesCI;
  uint32_t                    mActivePipeline = 0u;

  // Settings:
  uint32_t sInFlightMaxFrames = 3;

  // Resources:

};  // class Vonk
}  // namespace vonk
