#include "App.hh"

#include <fmt/core.h>

namespace Vonsai
{
// #define ALG(algo, container, body, ...) std::algo(container.begin(), container.end(), [](__VA_ARGS__) { body; });
#define ALG(algo, container, fn) std::algo(container.begin(), container.end(), fn);

//===============================================

#ifdef NDEBUG
constexpr bool kVkLayersEnabled = false;
#else
constexpr bool kVkLayersEnabled = true;
#endif

//===============================================

constexpr uint32_t             WIDTH { 800 }, HEIGHT { 600 };
const std::array kVkLayers { "VK_LAYER_KHRONOS_validation" };

//===============================================

// fmt::print("[VK - ERR] ({}:{}) on '{}'", __FILE__, __LINE__, msg)

#define vkErr_(msg) fmt::print("[VK - ERR] ({}:{}) :: '{}'", __FILE__, __LINE__, msg)
#define vkErr(r, msg) \
  if (r != vk::Result::eSuccess) vkErr_(msg)

//===============================================

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

  // Avoid OpenGL context creation
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // Handling resized windows takes special care
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Hello Vulkan!", nullptr, nullptr);
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
  if (kVkLayersEnabled && !checkValidationLayerSupport()) { vkErr_("Validation layers NOT available!"); }

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
  if (kVkLayersEnabled) {
    kICInfo.enabledLayerCount   = static_cast<uint32_t>(kVkLayers.size());
    kICInfo.ppEnabledLayerNames = kVkLayers.data()->data();
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

bool checkValidationLayerSupport()
{
  auto const availableLayers = vk::enumerateInstanceLayerProperties().value;

  for (auto layerName : kVkLayers)
    auto const found = ALG(any_of, availableLayers, [](auto const layerProps) { layerName == layerProps.layerName });
}

//---------------------------------------------------------------------------

}  // namespace Vonsai
