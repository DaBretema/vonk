#include "VwUtils.h"

#include "Utils.h"

#include "VwToStr.h"

//

//=============================================================================
// --- SWAP CHAIN ---
//=============================================================================

namespace vku::swapchain
{  //

//---------------------------------------------

std::tuple<VkSurfaceCapabilitiesKHR, std::vector<VkPresentModeKHR>, std::vector<VkSurfaceFormatKHR>> getSupportData(
  VkPhysicalDevice physicalDevice,
  VkSurfaceKHR     surface)
{
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkPresentModeKHR>   presentModes;
  std::vector<VkSurfaceFormatKHR> surfaceFormats;

  // . Capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
  // . Surface Formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
  }
  // . Present modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
  }

  return { capabilities, presentModes, surfaceFormats };
}

//---------------------------------------------

bool isEmpty(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
  auto const [capabilities, presentModes, surfaceFormats] = getSupportData(physicalDevice, surface);
  return presentModes.empty() or surfaceFormats.empty();
}

//---------------------------------------------

Settings getSettings(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, Settings rs)
{
  Settings scs;
  auto const [capabilities, presentModes, surfaceFormats] = getSupportData(physicalDevice, surface);

  // . [capabilities] - Direct store
  scs.capabilities = capabilities;

  // . [capabilities] - Get max allowed image to get in queue
  scs.minImageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 and scs.minImageCount > capabilities.maxImageCount) {
    scs.minImageCount = capabilities.maxImageCount;
  }

  // . [capabilities] - Get best extent
  if (capabilities.currentExtent.width != UINT32_MAX) {
    scs.extent2D = capabilities.currentExtent;
  } else {
    uint32_t    w   = rs.extent2D.width;
    uint32_t    h   = rs.extent2D.height;
    auto const &min = capabilities.minImageExtent;
    auto const &max = capabilities.maxImageExtent;
    scs.extent2D    = VkExtent2D { std::clamp(w, min.width, max.width), std::clamp(h, min.height, max.height) };
  }

  // . [presentModes] - Get best
  // VK_PRESENT_MODE_MAILBOX_KHR : Render as fast as possible while still avoiding tearing : vsync + triple-buffer
  // VK_PRESENT_MODE_FIFO_KHR    : Guaranteed to be available : vsync + double-buffer
  scs.presentMode = VK_PRESENT_MODE_FIFO_KHR;  // fallback
  if (std::find(presentModes.begin(), presentModes.end(), rs.presentMode) != presentModes.end()) {
    scs.presentMode = rs.presentMode;
  }

  // . [surfaceFormat] - Get best
  for (const auto &available : surfaceFormats) {
    bool const sameFormat     = available.format == rs.surfaceFormat.format;
    bool const sameColorSpace = available.colorSpace == rs.surfaceFormat.colorSpace;
    if (sameFormat and sameColorSpace) { scs.surfaceFormat = available; }
  }

  return scs;
}

//---------------------------------------------

void Settings::dumpInfo() const
{
  VO_INFO("- SwapChainSettings -");
  VO_INFO_FMT("Image Count             : {}", minImageCount);
  VO_INFO_FMT("Selected Extent 2D      : ({},{})", extent2D.width, extent2D.height);
  VO_INFO_FMT("Selected Present Mode   : {}", vku::ToStr_PresentMode.at(presentMode));
  VO_INFO_FMT("Selected Surface Format : {}", vku::ToStr_Format.at(surfaceFormat.format));
}

//---------------------------------------------

}  // namespace vku::swapchain

//=============================================================================

//

//=============================================================================
// --- SHADERS ---
//=============================================================================

namespace vku::shaders
{  //

//---------------------------------------------

std::string getPath(char const *shaderName, VkShaderStageFlagBits stage)
{
  static std::unordered_map<VkShaderStageFlagBits, std::string> stageToExtension {
    { VK_SHADER_STAGE_VERTEX_BIT, "vert" },
    { VK_SHADER_STAGE_FRAGMENT_BIT, "frag" },
    { VK_SHADER_STAGE_COMPUTE_BIT, "comp" },
    { VK_SHADER_STAGE_GEOMETRY_BIT, "geom" },
    { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tesc" },
    { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tese" },
  };

  static auto const VwShadersPath = std::string("./assets/shaders/");  // get this path for a define
  return VwShadersPath + shaderName + "." + stageToExtension[stage] + ".spv";
};

//---------------------------------------------

// VkShaderModule createModule(VkDevice logicalDevice, const std::vector<char> &code)
// {
//   VkShaderModuleCreateInfo createInfo {};
//   createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//   createInfo.codeSize = code.size();
//   createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());

//   VkShaderModule shaderModule;
//   VW_CHECK(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule));

//   return shaderModule;
// }

//---------------------------------------------

ShaderData create(VkDevice logicalDevice, std::string const &name, VkShaderStageFlagBits stage)
{
  auto const path = vku::shaders::getPath(name.c_str(), stage);

  auto code = vo::files::read(path);
  if (code.empty()) { VO_ERR_FMT("Failed to open shader '{}'!", path); }

  // VkShaderModule shaderModule = vku::shaders::createModule(logicalDevice, shaderBinary);

  VkShaderModuleCreateInfo createInfo {};
  createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = VW_SIZE_CAST(code.size());
  createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  VW_CHECK(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule));

  VkPipelineShaderStageCreateInfo shaderStageInfo {
    .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage  = stage,
    .module = shaderModule,
    .pName  = "main",  // Entrypoint function name
  };

  return { path, shaderModule, shaderStageInfo };
}

//---------------------------------------------

//=============================================================================

//

//=============================================================================
// ---  ---
//=============================================================================

//=============================================================================

//

//=============================================================================
// ---  ---
//=============================================================================

//=============================================================================

//

//=============================================================================
// ---  ---
//=============================================================================

}  // namespace vku::shaders
