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

uint32_t
  getMemoryType(VkPhysicalDeviceMemoryProperties memProps, uint32_t typeBits, VkMemoryPropertyFlags requestedProps);

SurfaceSupport_t getSurfaceSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface);

bool checkGpuExtensionsSupport(Gpu_t const &gpu);
bool checkValidationLayersSupport(std::vector<char const *> const &layers);

std::vector<uint32_t> getUniqueQueueFamilies(Gpu_t const &gpu);

}  // namespace vonk
