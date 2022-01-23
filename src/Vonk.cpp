#include "Vonk.h"
#include "VonkResources.h"
#include "VonkTools.h"
#include "VonkWindow.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/config.h.in>

#include <filesystem>

namespace vonk
{  //

//=============================================================================

// === MESHEs

//-------------------------------------

std::vector<Mesh_t> Vonk::read3DFile(
  std::string const &filepath,
  uint32_t           optimizationLevel,
  bool               recalculateUVs,
  bool               recalculateNormals,
  bool               recalculateTangentsAndBitangets)
{
  static Assimp::Importer importer = {};  // Expensive to initialize so create only once

  auto const fp = std::filesystem::absolute(filepath);
  LogInfof("Loading File: '{}' ...", fp.string());

  // ::: Validate the path
  if (!std::filesystem::exists(fp)) {
    LogErrorf("File {} doesn't exist", fp.string());
    return {};
  }

  // ::: Prepare flags

  bool const recalc = recalculateUVs || recalculateNormals || recalculateTangentsAndBitangets;

  uint32_t const recalcComps = ((recalculateUVs) ? aiComponent_TEXCOORDS : 0)
                               | ((recalculateNormals) ? aiComponent_NORMALS : 0)
                               | ((recalculateTangentsAndBitangets) ? aiComponent_TANGENTS_AND_BITANGENTS : 0);

  uint32_t const recalcFlags = (recalc) ? aiProcess_RemoveComponent | ((recalculateUVs) ? aiProcess_GenUVCoords : 0)
                                            | ((recalculateNormals) ? aiProcess_GenSmoothNormals : 0)
                                            | ((recalculateTangentsAndBitangets) ? aiProcess_CalcTangentSpace : 0)
                                        : 0;

  uint32_t const flags =  aiProcess_ConvertToLeftHanded | aiProcess_GenBoundingBoxes | recalcFlags | (
                           optimizationLevel==1 ? aiProcessPreset_TargetRealtime_Fast       :
                           optimizationLevel==2 ? aiProcessPreset_TargetRealtime_Quality    :
                           optimizationLevel==3 ? aiProcessPreset_TargetRealtime_MaxQuality : 0
                         );

  // ::: Set importer settings
  // - http://assimp.sourceforge.net/lib_html/postprocess_8h.html

  static int constexpr toIgnore = aiPrimitiveType::aiPrimitiveType_POINT | aiPrimitiveType::aiPrimitiveType_LINE;

  importer.SetPropertyBool(AI_CONFIG_FAVOUR_SPEED, 1);
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, toIgnore);
  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, recalcComps);

  if (recalc && (recalcComps & aiComponent_NORMALS)) {
    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 175.f);
  }

  // ::: Prepare scene  // @DANI Fails with .glb scenes...
  auto const scene = importer.ReadFile(filepath, aiProcess_Triangulate /*flags*/);
  if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
    LogErrorf("[ASSIMP] - Scene {} : {}", filepath, importer.GetErrorString());
    return {};
  }
  LogInfof("NNNNNNNNNNNN {}", scene->mNumMeshes);
  // return {};  // @DANI

  // ::: Get Meshes
  std::vector<Mesh_t> meshes;
  meshes.reserve(scene->mNumMeshes);
  for (size_t meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
    aiMesh const *mesh = scene->mMeshes[meshIdx];
    Assert(mesh);

    auto const &v = mesh->mVertices;
    auto const &u = mesh->mTextureCoords[0];
    auto const &n = mesh->mNormals;
    auto const &t = mesh->mTangents;
    auto const &b = mesh->mBitangents;
    auto const &c = mesh->mColors[0];

    LogInfof("AAAAAAAAAAAAA {}", mesh->mName.C_Str());

    // SXMesh_Info mde;
    // mde.idx     = i;
    // mde.name    = mesh->mName.C_Str();
    // mde.matUUID = out.materials.at(mesh->mMaterialIndex).UUID;

    std::vector<uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3u);
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
      if (mesh->mFaces[i].mNumIndices == 3)
        for (auto j : { 0, 1, 2 }) indices.push_back(mesh->mFaces[i].mIndices[j]);

    std::vector<Vertex_t> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
      Vertex_t out;
      if (v) out.vertex = { v[i].x, v[i].y, v[i].z };
      if (u) out.uv = { u[i].x, u[i].y };
      if (n) out.normal = { n[i].x, n[i].y, n[i].z };
      if (t) out.tangent = { t[i].x, t[i].y, t[i].z };
      if (b) out.bitangent = { b[i].x, b[i].y, b[i].z };
      if (c) out.color = { c[i].r, c[i].g, c[i].b };
      vertices.push_back(out);
    }

    LogInfof(
      "AAAAAAAAAAAAA 2 : {} / {} : {} / {}",
      mesh->mNumFaces * 3,
      mesh->mNumVertices,
      indices.size(),
      vertices.size());
    // return {};
    meshes.push_back(createMesh(indices, vertices));
  }

  // LogInfof("AAAAAAAAAAAAA 2 {}", meshes.size());
  // LogInfof("AAAAAAAAAAAAA 3 {}", meshes.size()>0 ? meshes.at(1).);
  return meshes;
}

//-------------------------------------

Mesh_t const &Vonk::createMesh(std::vector<uint32_t> const &indices, std::vector<Vertex_t> const &vertices)
{
  // @DANI : Change this for a "trace and fill the gaps"-pattern, basically a stack of removed IDs;
  static uint32_t meshCountID = 0;
  uint32_t        meshID      = 0;

  if (!mRemovedMeshes.empty()) {
    meshID = mRemovedMeshes.top();
    mRemovedMeshes.pop();
  } else {
    meshID = meshCountID++;
  }

  mMeshes[meshID] = vonk::createMesh(mDevice, indices, vertices);
  return mMeshes[meshID];
}

//-------------------------------------

void Vonk::drawMesh(VkCommandBuffer cmd, Mesh_t const &mesh) { vonk::drawMesh(cmd, mesh); }
void Vonk::drawMeshes(VkCommandBuffer cmd, std::vector<Mesh_t> const &meshes)
{
  for (auto const &mesh : meshes) vonk::drawMesh(cmd, mesh);
}

//-------------------------------------

//=============================================================================

// === SHADERs

//-------------------------------------

DrawShader_t const &
  Vonk::createDrawShader(std::string const &keyName, std::string const &vertexName, std::string const &fragmentName)
{
  mDrawShaders[keyName] = vonk::createDrawShader(mDevice, vertexName, fragmentName, "", "", "");
  return mDrawShaders[keyName];
}

//-------------------------------------

DrawShader_t const &Vonk::getDrawShader(std::string const &keyName)
{
  AbortIfMsg(mDrawShaders.count(keyName) < 1, "Draw Shader Not Found!");
  return mDrawShaders[keyName];
}

//-------------------------------------

Shader_t const &Vonk::createComputeShader(std::string const &name)
{
  mComputeShaders[name] = vonk::createShader(mDevice, name, VK_SHADER_STAGE_COMPUTE_BIT);
  return mComputeShaders[name];
}

//-------------------------------------

Shader_t const &Vonk::getComputeShader(std::string const &name)
{
  AbortIfMsg(mComputeShaders.count(name) < 1, "Compute Shader Not Found!");
  return mComputeShaders[name];
}

//-------------------------------------

//=============================================================================

// === PIPELINEs

//-------------------------------------

void Vonk::addPipeline(DrawPipelineData_t const &ci)
{
  mPipelinesCI.push_back(ci);
  mPipelines.push_back(vonk::createPipeline(
    {},
    mPipelinesCI.back(),
    mSwapChain,
    mDevice.handle,
    mDevice.cmdpool.graphics,
    mSwapChain.defaultRenderPass,
    mSwapChain.defaultFrameBuffers));
};

//-------------------------------------

//=============================================================================

// === FRAME OPs

//-------------------------------------

void Vonk::waitDevice() { vkDeviceWaitIdle(mDevice.handle); }

//-------------------------------------

void Vonk::drawFrame()
{
  if (mPipelines.empty()) return;

  static size_t currFrame = 0;

  // ::: Preconditions
  vkWaitForFences(mDevice.handle, 1, &mSwapChain.fences.submit[currFrame], VK_TRUE, UINT64_MAX);

  // ::: 1. Get next image to process
  // 1.1 : Acquiere next image
  uint32_t   imageIndex;
  auto const acquireRet = vkAcquireNextImageKHR(
    mDevice.handle,
    mSwapChain.handle,
    UINT64_MAX,
    mSwapChain.semaphores.present[currFrame],
    VK_NULL_HANDLE,
    &imageIndex);
  // 1.2 : Validate the swapchain state
  if (acquireRet == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  } else if (acquireRet != VK_SUCCESS && acquireRet != VK_SUBOPTIMAL_KHR) {
    LogError("Failed to acquire swap chain image!");
  }
  // 1.3 : Check if a previous frame is using this image (i.e. there is its fence to wait on)
  if (mSwapChain.fences.acquire[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(mDevice.handle, 1, &mSwapChain.fences.acquire[imageIndex], VK_TRUE, UINT64_MAX);
  }
  // 1.4 : Mark the image as now being in use by this frame
  mSwapChain.fences.acquire[imageIndex] = mSwapChain.fences.submit[currFrame];

  // ::: 2. Draw ( Graphics Queue )
  // 2.1 : Sync objects
  VkPipelineStageFlags const waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore const          waitSemaphores[]   = { mSwapChain.semaphores.present[currFrame] };
  VkSemaphore const          signalSemaphores[] = { mSwapChain.semaphores.render[currFrame] };
  // 2.2 : Submit info
  VkSubmitInfo const submitInfo {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pWaitDstStageMask    = waitStages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &mPipelines[mActivePipeline].commandBuffers[imageIndex],
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = waitSemaphores,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = signalSemaphores,
  };
  // 2.3 : Reset fences right before asking for draw
  vkResetFences(mDevice.handle, 1, &mSwapChain.fences.submit[currFrame]);
  VkCheck(vkQueueSubmit(mDevice.queue.graphics, 1, &submitInfo, mSwapChain.fences.submit[currFrame]));

  // ::: 3. Dump to screen ( Present Queue )
  // 3.1 : Info
  VkSwapchainKHR const   swapChains[] = { mSwapChain.handle };
  VkPresentInfoKHR const presentInfo {
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = signalSemaphores,
    .swapchainCount     = 1,
    .pSwapchains        = swapChains,
    .pImageIndices      = &imageIndex,
    .pResults           = nullptr,  // Optional
  };
  // 3.2 : Ask for dump into screen
  auto const presentRet = vkQueuePresentKHR(mDevice.queue.present, &presentInfo);
  // 3.3 : Validate swapchain state
  if (presentRet == VK_ERROR_OUT_OF_DATE_KHR || presentRet == VK_SUBOPTIMAL_KHR || vonk::window::framebufferResized) {
    vonk::window::framebufferResized = false;  // move this variable to vonk::Vonk
    recreateSwapChain();
  } else if (presentRet != VK_SUCCESS) {
    LogError("Failed to present swap chain image!");
  }

  // ::: Extra tasks
  currFrame = (currFrame + 1) % sInFlightMaxFrames;
}

//-------------------------------------

//=============================================================================

// === SWAPCHAIN

//-------------------------------------

void Vonk::destroySwapChainDependencies()
{
  for (auto &pipeline : mPipelines) {
    // . Command Buffers
    auto &cb = pipeline.commandBuffers;
    if (cb.size() > 0) { vkFreeCommandBuffers(mDevice.handle, mDevice.cmdpool.graphics, GetCountU32(cb), GetData(cb)); }

    // // . Frame Buffers
    // for (size_t i = 0; i < pipeline.frameBuffers.size(); ++i) {
    //   if (pipeline.frameBuffers[i] != mSwapChain.defaultFrameBuffers[i]) {
    //     vkDestroyFramebuffer(mDevice.handle, pipeline.frameBuffers[i], nullptr);
    //   }
    // }
  }
}

//-------------------------------------

void Vonk::recreateSwapChain()
{
  vkDeviceWaitIdle(mDevice.handle);

  destroySwapChainDependencies();
  mSwapChain = vonk::createSwapChain(mDevice, mSwapChain);

  for (size_t i = 0; i < mPipelines.size(); ++i) {
    // vonk::createPipeline(mDevice, mPipelines[i], currSwapChainExtent2D, [renderpass], [framebuffer]);
    vonk::createPipeline(
      mPipelines[i],
      mPipelinesCI[i],
      mSwapChain,
      mDevice.handle,
      mDevice.cmdpool.graphics,
      mSwapChain.defaultRenderPass,
      mSwapChain.defaultFrameBuffers);
  }
}

//-------------------------------------

//=============================================================================

// === FLOW

//-------------------------------------

void Vonk::init()
{
  // . Validation layers support
  AbortIfMsg(!vonk::checkValidationLayersSupport(mInstance.layers), "Required Layers Not Found!");
  // . Create Instance : VkInstance, VkDebugMessenger, VkSurfaceKHR
  mInstance = vonk::createInstance(vonk::window::title.c_str(), VK_API_VERSION_1_2);
  // . Pick Gpu (aka: physical device)
  mGpu = vonk::pickGpu(mInstance, true, true, true, true);
  // . Create Device (aka: gpu-manager / logical-device)
  mDevice = vonk::createDevice(mInstance, mGpu);
  // . Create SwapChain
  mSwapChain = vonk::createSwapChain(mDevice, mSwapChain);
}

//-------------------------------------

void Vonk::cleanup()
{
  // . Pipelines
  for (auto &pipeline : mPipelines) { vonk::destroyPipeline(mSwapChain, pipeline); }

  // . Meshes
  for (auto &[k, m] : mMeshes) {
    mRemovedMeshes.push(k);
    vonk::destroyMesh(mDevice, m);
  }

  // . Shaders
  for (auto const &[k, ds] : mDrawShaders) { vonk::destroyDrawShader(mDevice, ds); }
  for (auto const &[k, cs] : mComputeShaders) { vkDestroyShaderModule(mDevice.handle, cs.module, nullptr); }

  // . Context ¿?
  vonk::destroySwapChain(mSwapChain, false);
  vonk::destroyDevice(mDevice);
  vonk::destroyInstance(mInstance);
}

//-------------------------------------

//=============================================================================

}  // namespace vonk
