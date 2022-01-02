#pragma once

#include <stack>
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
  void addPipeline(DrawPipelineData_t const &ci);

  inline auto currentFormat() const { return mSwapChain.colorFormat; }

  inline void iterScenes() { mActivePipeline = (mActivePipeline + 1) % mPipelines.size(); }

  // . Meshes
  Mesh_t const &createMesh(std::vector<uint32_t> const &indices, std::vector<Vertex_t> const &vertices);
  void          drawMesh(VkCommandBuffer cmd, Mesh_t const &mesh);

  // . Shaders
  DrawShader_t const &
    createDrawShader(std::string const &keyName, std::string const &vertexName, std::string const &fragmentName);
  DrawShader_t const &getDrawShader(std::string const &keyName);
  Shader_t const &    createComputeShader(std::string const &name);
  Shader_t const &    getComputeShader(std::string const &name);

private:
  void recreateSwapChain();
  void destroySwapChainDependencies();

  // Context:
  Instance_t  mInstance;
  Gpu_t       mGpu;
  Device_t    mDevice;
  SwapChain_t mSwapChain;

  // Meshes:
  std::unordered_map<uint32_t, Mesh_t> mMeshes;
  std::stack<uint32_t>                 mRemovedMeshes;

  // Shaders:
  std::unordered_map<std::string, DrawShader_t> mDrawShaders;
  std::unordered_map<std::string, Shader_t>     mComputeShaders;

  // Pipelines:
  std::vector<DrawPipeline_t>     mPipelines;
  std::vector<DrawPipelineData_t> mPipelinesCI;
  uint32_t                        mActivePipeline = 0u;

  // Settings:
  uint32_t sInFlightMaxFrames = 3;

  // Resources:

};  // class Context
}  // namespace vonk
