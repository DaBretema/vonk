#pragma once

#include "vk0.hh"

#include <vector>
#include <optional>

namespace vk0
{  //

struct QueueFamilyIndices
{
  QueueFamilyIndices() = default;
  QueueFamilyIndices(vk::PhysicalDevice const &device, vk::SurfaceKHR const &surface);

  std::optional<uint32_t> graphicsFamily {};
  std::optional<uint32_t> presentFamily {};
  bool                    isComplete() const;

  std::vector<vk::DeviceQueueCreateInfo> getCreateInfo();
};

}  // namespace vk0
