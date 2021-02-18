#include "App.hh"

#include <tuple>
#include <algorithm>
#include <fmt/core.h>

#ifndef NDEBUG
#  define VO_TRACE(s) fmt::print(s);
#else
#  define VO_TRACE(s)
#endif

namespace Vonsai
{
//-----------------------------------------------------------------------------

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//-----------------------------------------------------------------------------

void App::run()
{
  VO_TRACE("> INIT WINDOW\n");
  initWindow();
  VO_TRACE("> INIT VULKAN\n");
  initVulkan();
  VO_TRACE("> MAIN LOOP\n");
  mainLoop();
  VO_TRACE("> CLEAN UP\n");
  cleanup();
}

//-----------------------------------------------------------------------------

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//-----------------------------------------------------------------------------

void App::initWindow()
{
  VO_TRACE(">> GLFW Init\n");
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Avoid OpenGL context creation
  glfwWindowHint(GLFW_RESIZABLE,
                 GLFW_FALSE);  // Handling resized windows takes special care

  VO_TRACE(">> GLFW Create Window\n");
  mWindow = glfwCreateWindow(mW, mH, mTitle.c_str(), nullptr, nullptr);
}

//-----------------------------------------------------------------------------

void App::initVulkan()
{
  VO_TRACE(">> Create Instance\n");
  createInstance();

  VO_TRACE(">> Setup Debug callback\n");
  if (!vk0::LAYERS_ON) return;
  // NOTE: Vulkan-hpp has methods for this, but they trigger linking errors...
  // instance->createDebugUtilsMessengerEXT(createInfo);
  // instance->createDebugUtilsMessengerEXTUnique(createInfo);
  auto const createInfo  = vk0::defaultDebugUtilsMessengerCreateInfo();
  auto const pCreateInfo = reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(&createInfo);
  // NOTE: reinterpret_cast is also used by vulkan.hpp internally for all these structs
  VK0_Test_Fn_C(vk0::CreateDebugUtilsMessengerEXT(*mInstance, pCreateInfo, nullptr, &mDebugMessenger));
}

//-----------------------------------------------------------------------------

void App::mainLoop()
{
  while (!glfwWindowShouldClose(mWindow)) {
    glfwWaitEvents();  // glfwPollEvents();
  }
}

//-----------------------------------------------------------------------------

void App::cleanup()
{
  // NOTE: Comment line below to verify that VALIDATION LAYERS are working...
  if (vk0::LAYERS_ON) { vk0::DestroyDebugUtilsMessengerEXT(*mInstance, mDebugMessenger, nullptr); }

  // NOTE: instance destruction is handled by UniqueInstance

  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

//-----------------------------------------------------------------------------

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//-----------------------------------------------------------------------------

void App::createInstance()
{
  if (vk0::LAYERS_ON && !checkValidationLayerSupport()) {
    VO_TRACE(">>> Checking validation layers\n");
    VK0_Alert("Validation layers NOT available!");
  }

  auto constexpr v   = VK_API_VERSION_1_0;
  auto const appInfo = vk::ApplicationInfo { mTitle.c_str(), v, "Vonsai", v, v };

  // . Window Extensions
  auto const exts = getRequiredExtensions();

  // . Creating instance = (ic - InstanceCreate)
  auto const icFlags = vk::InstanceCreateFlags();
  auto const icInfo  = vk::InstanceCreateInfo { icFlags, &appInfo, VK0_SizeData(vk0::LAYERS), VK0_SizeData(exts) };

  mInstance = VK0_Call(createInstanceUnique(icInfo, nullptr));

  // . Getting extensions
  auto const extensions = VK0_Call(enumerateInstanceExtensionProperties());

  // . Show info about extensions
  fmt::print("[*] Available Extensions\n");
  for (auto const &ext : extensions) { fmt::print(" ↳ {}\n", ext.extensionName); }
}

//-----------------------------------------------------------------------------

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//-----------------------------------------------------------------------------

bool App::checkValidationLayerSupport()
{
  auto const availableLayers = vk::enumerateInstanceLayerProperties().value;

  for (auto layerName : vk0::LAYERS) {
    auto const found = std::any_of(availableLayers.begin(), availableLayers.end(), [&](auto const layer) {
      return std::string_view(layerName) == layer.layerName;
    });

    if (!found) {
      VK0_AlertFmt("Layer {} not found.", layerName);
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

std::vector<const char *> App::getRequiredExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (vk0::LAYERS_ON) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

  return extensions;
}

// std::tuple<uint32_t, const char **> App::getRequiredExtensions()
// {
//   uint32_t     glfwExtensionsCount = 0;
//   const char **glfwExtensions      = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

//   fmt::print(">>>>>>>>>>>>>>>>>>>> no issues here");
//   std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

//   if (vk0::LAYERS_ON) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

//   return { extensions.size(), extensions.data() };  //? Whats wrong??
// }

//-----------------------------------------------------------------------------

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//-----------------------------------------------------------------------------

}  // namespace Vonsai
