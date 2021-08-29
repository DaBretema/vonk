#include <vk0/vk0Debug.hh>
#include <vk0/vk0Macros.hh>

#include <fmt/core.h>

#include <unordered_map>

namespace vk0
{  //

//---------------------------------------------------------------------------

VKAPI_ATTR VkBool32 VKAPI_CALL callback(
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
  static const std::unordered_map<uint32_t, std::string_view> typeToStr {
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

//---------------------------------------------------------------------------

vk::DebugUtilsMessengerCreateInfoEXT DebugMessenger::defaultCreateInfo()
{
  auto const severityFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                             | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                             | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

  auto const typeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                         | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                         | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

  return vk::DebugUtilsMessengerCreateInfoEXT { vk::DebugUtilsMessengerCreateFlagsEXT(),
                                                severityFlags,
                                                typeFlags,
                                                callback,
                                                nullptr };
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void DebugMessenger::create(VkInstance const &instance)
{
  if (!vk0::hasLayers) { return; }

  auto const createInfo  = defaultCreateInfo();
  auto const pCreateInfo = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT const *>(&createInfo);
  vk0InstanceResFn(instance, vkCreateDebugUtilsMessengerEXT, pCreateInfo, nullptr, &mDebugMessenger)
}

//---------------------------------------------------------------------------

void DebugMessenger::destroy(VkInstance const &instance)
{
  if (!vk0::hasLayers) { return; }
  vk0InstanceFn(instance, vkDestroyDebugUtilsMessengerEXT, mDebugMessenger, nullptr)
}

//---------------------------------------------------------------------------

}  // namespace vk0
