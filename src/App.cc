#include "App.hh"
#include "VkToolbox.hh"

#include <fmt/core.h>
#include <algorithm>

namespace Vonsai
{
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void App::run()
{
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void App::initWindow()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Avoid OpenGL context creation
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // Handling resized windows takes special care
  mWindow = glfwCreateWindow(mW, mH, mTitle.c_str(), nullptr, nullptr);
}

//---------------------------------------------------------------------------

void App::initVulkan() { createInstance(); }

//---------------------------------------------------------------------------

void App::mainLoop()
{
  while (!glfwWindowShouldClose(mWindow)) {
    glfwWaitEvents();  // glfwPollEvents();
  }
}

//---------------------------------------------------------------------------

void App::cleanup()
{
  // NOTE: instance destruction is handled by UniqueInstance

  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void App::createInstance()
{
  if (vk0::LAYERS_ON && !checkValidationLayerSupport()) { vkErr_("Validation layers NOT available!"); }

  auto constexpr kVer     = VK_API_VERSION_1_0;
  auto constexpr kAppInfo = vk::ApplicationInfo { "Hello Triangle", kVer, "No Engine", kVer, kVer };

  // . Window Extensions
  // auto         extensions   = getRequiredExtensions();
  uint32_t     glfwExtCount = 0;
  const char **glfwExts     = glfwGetRequiredInstanceExtensions(&glfwExtCount);

  // . Creating instance
  auto constexpr kICLayers = nullptr;
  auto const kICFlags      = vk::InstanceCreateFlags();
  auto       kICInfo       = vk::InstanceCreateInfo { kICFlags, &kAppInfo, 0, kICLayers, glfwExtCount, glfwExts };
  if (vk0::LAYERS_ON) {
    kICInfo.enabledLayerCount   = static_cast<uint32_t>(vk0::LAYERS.size());
    kICInfo.ppEnabledLayerNames = vk0::LAYERS.data();
  } else {
    kICInfo.enabledLayerCount = 0;
  }

  auto [err0, mainInstance] = vk::createInstanceUnique(kICInfo, nullptr);
  vkErr(err0, "createInstanceUnique");

  mInstance = std::move(mainInstance);

  // . Getting extensions
  auto const [err1, extensions] = vk::enumerateInstanceExtensionProperties();
  vkErr(err1, "enumerateInstanceExtensionProperties");

  // . Show info about extensions
  fmt::print("Available extensions:\n");
  for (auto const &ext : extensions) { fmt::print("\t{}\n", ext.extensionName); }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool App::checkValidationLayerSupport()
{
  auto const availableLayers = vk::enumerateInstanceLayerProperties().value;

  for (auto layerName : vk0::LAYERS) {
    auto const found = std::any_of(availableLayers.begin(), availableLayers.end(), [&](auto const layer) {
      return std::string_view(layerName) == layer.layerName;
    });
    if (!found) return false;
  }

  return true;
}

//---------------------------------------------------------------------------

}  // namespace Vonsai
