#pragma once

#include "_vulkan.h"
#include "VoVulkanUtils.h"

#include <optional>

#include <set>
#include <vector>
#include <unordered_map>

namespace vo::vulkan
{  //

struct Instance
{
  VkInstance handle = VK_NULL_HANDLE;

  VkDebugUtilsMessengerEXT debugMessenger;

  VkPhysicalDevice           physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures   physicalDeviceFeatures;
  VkPhysicalDeviceProperties physicalDeviceProperties;

  std::vector<uint32_t> queuesIndex;

  VkSurfaceKHR surface;

  void create();
  void destroy();
};

}  // namespace vo::vulkan

namespace vku = vo::vulkan;
