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

//-----------------------------------------------

// INSTANCEs

Instance_t createInstance(const char *title = "VONK", uint32_t apiVersion = VK_API_VERSION_1_2);

void destroyInstance(Instance_t &instance);

//-----------------------------------------------

// GPUs (PHYSICAL DEVICEs)

Gpu_t pickGpu(Instance_t &instance, bool enableGraphics, bool enablePresent, bool enableTransfer, bool enableCompute);

//-----------------------------------------------

// DEVICEs

Device_t createDevice(Instance_t const &instance, Gpu_t const &gpu);

void destroyDevice(Device_t &device);

//-----------------------------------------------

// COMMAND POOL

VkCommandPool createCommandPool(Device_t const &device, uint32_t idx);

//-----------------------------------------------

// RENDER PASSes

VkRenderPass createRenderPass(VkDevice device, RenderPassData_t const &rpd);

VkRenderPass createDefaultRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat);

//-----------------------------------------------

// TEXTUREs

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

//-----------------------------------------------

// SEMAPHOREs

VkSemaphore createSemaphore(VkDevice device);

//-----------------------------------------------

// FENCEs

VkFence createFence(VkDevice device);

//-----------------------------------------------

// SWAP CHAIN

SwapChain_t createSwapChain(Device_t const &device, SwapChain_t oldSwapChain);

void destroySwapChain(SwapChain_t &swapchain, bool justForRecreation = false);

//-----------------------------------------------

// SHADERs

#define VONK_SHADER_CACHE 0

Shader_t createShader(Device_t const &device, std::string const &name, VkShaderStageFlagBits stage);

DrawShader_t createDrawShader(
  Device_t const &   device,
  std::string const &vert,
  std::string const &frag,
  std::string const &tesc = "",
  std::string const &tese = "",
  std::string const &geom = "");

void destroyDrawShader(Device_t const &device, DrawShader_t const &ds);

// inline void destroyShader(Shader_t const &shader) {
//   vkDestroyShaderModule(shader.pDevice->handle, shader.module, nullptr);
// }

//-----------------------------------------------

// PIPELINEs

DrawPipeline_t createPipeline(
  DrawPipeline_t const &            oldPipeline,
  DrawPipelineData_t const &        ci,
  SwapChain_t const &               swapchain,
  VkDevice                          device,
  VkCommandPool                     commandPool,
  VkRenderPass                      renderpass,
  std::vector<VkFramebuffer> const &frameBuffers);

void destroyPipeline(SwapChain_t const &swapchain, DrawPipeline_t const &pipeline);

//-----------------------------------------------

// BUFFERs

inline Buffer_t createBuffer(
  Device_t const &      device,
  size_t                elemSize,
  uint32_t              count,
  VkBufferUsageFlags    usage,
  VkMemoryPropertyFlags properties)
{
  Buffer_t buffer;
  buffer.size  = elemSize * count;
  buffer.count = count;

  // . Buffer
  VkBufferCreateInfo bufferCI {
    .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size        = buffer.size,
    .usage       = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  VkCheck(vkCreateBuffer(device.handle, &bufferCI, nullptr, &buffer.handle));

  // . Memory
  VkMemoryRequirements memReqs;
  vkGetBufferMemoryRequirements(device.handle, buffer.handle, &memReqs);
  VkMemoryAllocateInfo const memAlloc {
    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize  = memReqs.size,
    .memoryTypeIndex = vonk::getMemoryType(device.pGpu->memory, memReqs.memoryTypeBits, properties),
  };
  vkAllocateMemory(device.handle, &memAlloc, nullptr, &buffer.memory);
  vkBindBufferMemory(device.handle, buffer.handle, buffer.memory, 0);

  return buffer;
}
//---
inline void copyBuffer(Device_t const &device, Buffer_t const &src, Buffer_t &dst)
{
  auto const pool = device.cmdpool.transfer ? device.cmdpool.transfer : device.cmdpool.graphics;

  // . Allocate
  VkCommandBufferAllocateInfo allocInfo {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandPool        = pool,
    .commandBufferCount = 1,
  };
  VkCommandBuffer cmd;
  vkAllocateCommandBuffers(device.handle, &allocInfo, &cmd);

  // . Record
  VkCommandBufferBeginInfo const beginInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(cmd, &beginInfo);
  VkBufferCopy const copyRegion {
    .srcOffset = 0,  // Optional
    .dstOffset = 0,  // Optional
    .size      = src.size,
  };
  vkCmdCopyBuffer(cmd, src.handle, dst.handle, 1, &copyRegion);
  vkEndCommandBuffer(cmd);

  // . Submit
  VkSubmitInfo const submitInfo {
    .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers    = &cmd,
  };
  auto const queue = device.queue.transfer ? device.queue.transfer : device.queue.graphics;
  vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);  // There is some room to improve using 'vkWaitForFences'-logic

  // . Free
  vkFreeCommandBuffers(device.handle, pool, 1, &cmd);
}
//---
inline void destroyBuffer(Device_t const &device, Buffer_t const &buff)
{
  vkDestroyBuffer(device.handle, buff.handle, nullptr);
  vkFreeMemory(device.handle, buff.memory, nullptr);
}
//---
inline Buffer_t createBufferStaging(Device_t const &device, DataInfo_t di, VkBufferUsageFlags usage)
{
  // . Create
  auto const hostUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  auto const hostProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  auto       buffHost  = createBuffer(device, di.elemSize, di.count, hostUsage, hostProps);

  auto const devUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
  auto const devProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  auto       buffDev  = createBuffer(device, di.elemSize, di.count, devUsage, devProps);

  // . Populate the host one
  void *data = nullptr;
  vkMapMemory(device.handle, buffHost.memory, 0, di.elemSize * di.count, 0, &data);
  memcpy(data, di.data, di.elemSize * di.count);
  vkUnmapMemory(device.handle, buffHost.memory);

  // . Copy from Host to Dev and clean the Host one
  copyBuffer(device, buffHost, buffDev);
  destroyBuffer(device, buffHost);

  return buffDev;
}

//-----------------------------------------------

// MESHes

inline Mesh_t
  createMesh(Device_t const &device, std::vector<uint32_t> const &indices, std::vector<Vertex_t> const &vertices)
{
  Mesh_t mesh;
  mesh.indices  = createBufferStaging(device, GetDataInfo(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  mesh.vertices = createBufferStaging(device, GetDataInfo(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  return mesh;
}

inline void destroyMesh(Device_t const &device, Mesh_t &mesh)
{
  destroyBuffer(device, mesh.indices);
  destroyBuffer(device, mesh.vertices);
}

inline void drawMesh(VkCommandBuffer cmd, Mesh_t const &mesh)
{
  // ROOM TO IMPROVEMENT : https://developer.nvidia.com/vulkan-memory-management

  /* @DANI
   You should store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in
   commands like vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because
   it's closer together. It is even possible to reuse the same chunk of memory for multiple resources if they are not
   used during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
   and some Vulkan functions have explicit flags to specify that you want to do this.
  */

  VkBuffer const     vertexBuffers[] = { mesh.vertices.handle };
  VkDeviceSize const offsets[]       = { 0 };
  vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
#if 1
  vkCmdBindIndexBuffer(cmd, mesh.indices.handle, 0, VK_INDEX_TYPE_UINT32);  // or... VK_INDEX_TYPE_UINT16
  vkCmdDrawIndexed(cmd, mesh.indices.count, 1, 0, 0, 0);
#else
  vkCmdDraw(cmd, mesh.vertices.count, 1, 0, 0);
#endif
}

//-----------------------------------------------

}  // namespace vonk
