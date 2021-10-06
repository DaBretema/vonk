#pragma once

#include "VwVulkan.h"
#include "Macros.h"

#include <vector>

//

//=============================================================================
// --- SWAP CHAIN ---
//=============================================================================

namespace vku::swapchain
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

}  // namespace vku::swapchain

//=============================================================================

//

//=============================================================================
// --- SHADERS ---
//=============================================================================

namespace vku::shaders
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
}  // namespace vku::shaders

//=============================================================================

//=============================================================================
// --- ---
//=============================================================================
