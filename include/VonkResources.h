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
Shader_t     createShader(Device_t const &device, std::string const &name, VkShaderStageFlagBits stage);
DrawShader_t createDrawShader(
  Device_t const &   device,
  std::string const &vert,
  std::string const &frag,
  std::string const &tesc,
  std::string const &tese,
  std::string const &geom);
void destroyDrawShader(Device_t const &device, DrawShader_t const &ds);
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

// VERTEX BUFFERs
//-----------------------------------------------
inline Mesh_t createMesh(Device_t const &device, std::vector<Vertex_t> const &vertices)
{
  Assert(device.pGpu);

  Mesh_t mesh;
  mesh.data = vertices;  // std::move(vertices);

  // . Buffer
  VkBufferCreateInfo buffCI {
    .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size        = GetSizeOfU32(mesh.data),
    .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  VkCheck(vkCreateBuffer(device.handle, &buffCI, nullptr, &mesh.buffer));

  // . Memory
  // HOST_COHERENT_BIT = Guaranteed transfer completeness before the next call to vkQueueSubmit
  VkMemoryRequirements memReqs;
  vkGetBufferMemoryRequirements(device.handle, mesh.buffer, &memReqs);
  static constexpr uint32_t  memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkMemoryAllocateInfo const memAlloc {
    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize  = memReqs.size,
    .memoryTypeIndex = vonk::getMemoryType(device.pGpu->memory, memReqs.memoryTypeBits, memFlags),
  };
  vkAllocateMemory(device.handle, &memAlloc, nullptr, &mesh.memory);
  vkBindBufferMemory(device.handle, mesh.buffer, mesh.memory, 0);

  // @DANI issues here!!!!

  // . Copy data into the buffer

  void *data = nullptr;
  vkMapMemory(device.handle, mesh.memory, 0, sizeof(Vertex_t) * mesh.data.size(), 0, &data);
  memcpy(data, mesh.data.data(), sizeof(Vertex_t) * mesh.data.size());

  LogInfof("data ptr = {} / Is null? {}", PtrStr(data), data == nullptr);

  LogInfof(
    "\nREAL DATA >>>\n 0 - P : {} C : {}\n 1 - P : {} C : {}\n 2 - P : {} C : {}",
    glm::to_string(mesh.data[0].pos),
    glm::to_string(mesh.data[0].color),
    glm::to_string(mesh.data[1].pos),
    glm::to_string(mesh.data[1].color),
    glm::to_string(mesh.data[2].pos),
    glm::to_string(mesh.data[2].color));

  auto const d0 = static_cast<Vertex_t *>(data);
  auto const d1 = d0 + sizeof(Vertex_t);
  auto const d2 = d1 + sizeof(Vertex_t);
  LogInfof(
    "\nRAW  DATA >>>\n 0 - P : {} C : {}\n 1 - P : {} C : {}\n 2 - P : {} C : {}",
    glm::to_string(d0->pos),
    glm::to_string(d0->color),
    glm::to_string(d1->pos),
    glm::to_string(d1->color),
    glm::to_string(d2->pos),
    glm::to_string(d2->color));

  vkUnmapMemory(device.handle, mesh.memory);

  return mesh;
}
inline void destroyMesh(Device_t const &device, Mesh_t &mesh)
{
  vkDestroyBuffer(device.handle, mesh.buffer, nullptr);
  vkFreeMemory(device.handle, mesh.memory, nullptr);
  mesh.data.clear();
}
inline void drawMesh(VkCommandBuffer cmd, Mesh_t const &mesh)
{
  VkBuffer     buffers[] = { mesh.buffer };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
  vkCmdDraw(cmd, GetSizeU32(mesh.data), 1, 0, 0);
}

}  // namespace vonk
