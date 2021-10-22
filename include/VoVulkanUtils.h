#pragma once

#include "_vulkan.h"
#include "Macros.h"
#include "VoWindow.h"
#include "VoVulkanTypes.h"

#include <set>
#include <vector>
#include <optional>

namespace vo::vulkan
{  //

//

//=============================================================================
// ---- Others ----
//=============================================================================
namespace others
{  //

  std::vector<char const *> getInstanceExtensions();
  bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, std::vector<char const *> const &exts);
  bool checkValidationLayersSupport(std::vector<char const *> const &layers);

}  // namespace others

//=============================================================================

//

//=============================================================================
// ---- Debug Messenger ----
//=============================================================================

namespace debugmessenger
{  //

  void create(VkInstance const &instance, VkDebugUtilsMessengerEXT &debugHandle);
  void destroy(VkInstance const &instance, VkDebugUtilsMessengerEXT &debugHandle);

}  // namespace debugmessenger

//=============================================================================

//

//=============================================================================
// ---- Queue Families ----
//=============================================================================

namespace queue
{  //

  enum Type
  {
    graphics = 0,
    present,
  };

  struct IndicesOpt
  {
    std::optional<uint32_t> graphics, present;  //, compute, transfer;
  };

  // struct Indices
  // {
  //   uint32_t graphics, present;  //, compute, transfer;
  // };

  // struct Queues
  // {
  //   VkQueue graphics, present;  //, compute, transfer;
  // };

  bool           isComplete(IndicesOpt const &indices);
  QueueIndices_t unrollOptionals(IndicesOpt const &indices);  // Call after: isComplete

  std::vector<uint32_t> getUniqueIndices(QueueIndices_t const &indices);
  IndicesOpt            findIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

  Queues_t findQueues(VkDevice device, QueueIndices_t const &indices);

}  // namespace queue

//=============================================================================

//

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

}  // namespace vo::vulkan

namespace vku = vo::vulkan;
