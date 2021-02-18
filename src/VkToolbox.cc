#include "VkToolbox.hh"
#include <string>
#include <string_view>
#include <unordered_map>

namespace Vonsai::vk0
{
//-----------------------------------------------------------------------------

VkResult CreateDebugUtilsMessengerEXT(
  VkInstance                                instance,
  const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks *             pAllocator,
  VkDebugUtilsMessengerEXT *                pCallback)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

//-----------------------------------------------------------------------------

void DestroyDebugUtilsMessengerEXT(
  VkInstance                   instance,
  VkDebugUtilsMessengerEXT     callback,
  const VkAllocationCallbacks *pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) { func(instance, callback, pAllocator); }
}

//-----------------------------------------------------------------------------

vk::DebugUtilsMessengerCreateInfoEXT defaultDebugUtilsMessengerCreateInfo()
{
  auto SeverityFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                       | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                       | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

  auto TypeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                   | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

  return vk::DebugUtilsMessengerCreateInfoEXT(
    vk::DebugUtilsMessengerCreateFlagsEXT(),
    SeverityFlags,
    TypeFlags,
    debugCallback,
    nullptr);
}

//-----------------------------------------------------------------------------

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  void *                                      pUserData)
{
  static const std::unordered_map<int, std::string_view> severityToStr {
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "Verbose" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "Info" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "Warning" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "Error" },
  };
  static const std::unordered_map<int, std::string_view> typeToStr {
    { VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, "General" },
    { VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, "Validation" },
    { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, "Performance" },
  };

  if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    fmt::print(
      "\nVALIDATION LAYERS [{} / {} - {}] \n----------------------------\n{}\n",
      pUserData,
      severityToStr.at(messageSeverity),
      typeToStr.at(messageType),
      pCallbackData->pMessage);
  }

  return VK_FALSE;
}

//-----------------------------------------------------------------------------

}  // namespace Vonsai::vk0
