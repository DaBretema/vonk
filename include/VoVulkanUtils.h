#pragma once

#include "_vulkan.h"
#include "Macros.h"

#include <set>
#include <vector>
#include <optional>

namespace vo::vulkan
{  //

//

//=============================================================================
// ---- Debug Messenger ----
//=============================================================================

namespace debugmessenger
{  //

  void create(VkInstance const &instance, VkDebugUtilsMessengerEXT &debugHandle);
  void destroy(VkInstance const &instance, VkDebugUtilsMessengerEXT &debugHandle);

}  // namespace debugmessenger

//

//=============================================================================
// ---- Queue Families ----
//=============================================================================

namespace queuefamily
{  //

  enum Type
  {
    graphics = 0,
    present,
  };

  bool                  isComplete(std::vector<std::optional<uint32_t>> const indices);
  std::vector<uint32_t> unrollOptionals(std::vector<std::optional<uint32_t>> const indices);  // Call after: isComplete

  std::vector<uint32_t>                getUniqueIndices(std::vector<uint32_t> const &indices);
  std::vector<std::optional<uint32_t>> findIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

  std::vector<VkQueue> findQueues(VkDevice device, std::vector<uint32_t> const &indices);

}  // namespace queuefamily

//

//=============================================================================
// ---- SWAP CHAIN ----
//=============================================================================

namespace swapchain
{  //
  struct Settings
  {
    VkExtent2D               extent2D      = { 1280, 720 };
    VkPresentModeKHR         presentMode   = VK_PRESENT_MODE_FIFO_KHR;
    VkSurfaceFormatKHR       surfaceFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    uint32_t                 minImageCount = 0u;
    VkSurfaceCapabilitiesKHR capabilities  = {};
    void                     dumpInfo() const;
  };
  bool     isEmpty(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
  Settings getSettings(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, Settings rs);

}  // namespace swapchain

//=============================================================================

//

//=============================================================================
// --- SHADERS ---
//=============================================================================

namespace shaders
{  //

  struct ShaderData
  {
    std::string                     path;
    VkShaderModule                  module;
    VkPipelineShaderStageCreateInfo stageCreateInfo;
  };

  std::string getPathVert(char const *shaderName);
  std::string getPathFrag(char const *shaderName);
  std::string getPathComp(char const *shaderName);

  ShaderData create(VkDevice mLogicalDevice, std::string const &name, VkShaderStageFlagBits stage);
  // VkShaderModule createModule(VkDevice logicalDevice, const std::vector<char> &code);

}  // namespace shaders

//=============================================================================

//=============================================================================
// --- ---
//=============================================================================

}  // namespace vo::vulkan

namespace vku = vo::vulkan;
