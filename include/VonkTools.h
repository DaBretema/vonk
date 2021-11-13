#pragma once

#include "_vulkan.h"
#include "VonkWindow.h"
#include "VonkTypes.h"

#include "Macros.h"

#include <set>
#include <vector>
#include <optional>

namespace vonk
{  //

bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, std::vector<char const *> const &exts);
bool checkValidationLayersSupport(std::vector<char const *> const &layers);

uint32_t
  getMemoryType(VkPhysicalDeviceMemoryProperties memProps, uint32_t typeBits, VkMemoryPropertyFlags requestedProps);

//=============================================================================
// ---- SWAP CHAIN ----
//=============================================================================

namespace swapchain
{  //
  struct SupportSurface
  {
    VkSurfaceCapabilitiesKHR        caps;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
    VkFormat                        depthFormat;
  };

  bool                isEmpty(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
  SwapShainSettings_t getSettings(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapShainSettings_t rs);

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

  ShaderData create(VkDevice logicalDevice, std::string const &name, VkShaderStageFlagBits stage);

}  // namespace shaders

//=============================================================================

//=============================================================================
// --- ---
//=============================================================================

}  // namespace vonk
