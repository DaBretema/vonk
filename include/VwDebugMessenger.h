#pragma once
#include "VwVulkan.h"

namespace vku
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

}  // namespace vku
