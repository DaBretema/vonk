#pragma once
#include "_vulkan.h"

#include <vector>

namespace vo::vulkan::init
{  //

//-----------------------------------------------

inline VkInstanceCreateInfo instance(
  const char *              title,               //
  std::vector<const char *> instanceExtensions,  //
  std::vector<const char *> validationLayers)
{
  VkApplicationInfo const appInfo {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = title,
    .pEngineName        = title,
    .applicationVersion = VK_API_VERSION_1_2,
    .engineVersion      = VK_API_VERSION_1_2,
    .apiVersion         = VK_API_VERSION_1_2,
  };

  VkInstanceCreateInfo const instanceCI {
    .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    // Esential extensions
    .enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size()),
    .ppEnabledExtensionNames = instanceExtensions.data(),
    // Layers
    .enabledLayerCount   = static_cast<uint32_t>(validationLayers.size()),
    .ppEnabledLayerNames = validationLayers.data(),
  };

  return instanceCI;
}
}  // namespace vo::vulkan::init

namespace vku = vo::vulkan;
