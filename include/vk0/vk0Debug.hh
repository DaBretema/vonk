#pragma once

#include "dac.hh"

#include "vk0.hh"
#include "vk0Settings.hh"

namespace vk0
{
struct DebugMessenger
{
  dac_ClassNotMoveCopy(DebugMessenger);

  void create(VkInstance const &instance);
  void destroy(VkInstance const &instance);

private:
  static vk::DebugUtilsMessengerCreateInfoEXT defaultCreateInfo();

  VkDebugUtilsMessengerEXT mDebugMessenger;
};
}  // namespace vk0
