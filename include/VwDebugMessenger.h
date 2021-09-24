#pragma once

#include <unordered_map>

#include "VwVulkan.h"

namespace vo
{
class DebugMessenger
{
public:
  DebugMessenger();

  void create(VkInstance const &instance);
  void destroy(VkInstance const &instance);

  VkDebugUtilsMessengerCreateInfoEXT const *getCreateInfo() { return &createInfo; }

private:
  VkDebugUtilsMessengerEXT           debugMessenger = VK_NULL_HANDLE;
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
};

}  // namespace vo
