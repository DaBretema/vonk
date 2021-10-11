#include "VoVulkanInstance.h"

#include "Macros.h"
#include "VoWindow.h"

namespace vo::vulkan
{  //

//-----------------------------------------------

void Instance::create()
{
  // . Create instance
  if (!vku::others::checkValidationLayersSupport(vo::sValidationLayers)) {
    vo__abort("Validation layers requested, but not available!");
  }

  auto const engineName = std::string(vo::window::title + " Engine");

  VkApplicationInfo const appInfo {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = vo::window::title.c_str(),
    .applicationVersion = VK_API_VERSION_1_2,
    .pEngineName        = engineName.c_str(),
    .engineVersion      = VK_API_VERSION_1_2,
    .apiVersion         = VK_API_VERSION_1_2,
  };

  auto const instanceExtensions = vku::others::getInstanceExtensions();

  VkInstanceCreateInfo const createInfo {
    .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    // Esential extensions
    .enabledExtensionCount   = vku__castsize(instanceExtensions.size()),
    .ppEnabledExtensionNames = instanceExtensions.data(),
    // Layers
    .enabledLayerCount   = static_cast<uint32_t>(vo::sValidationLayers.size()),
    .ppEnabledLayerNames = vo::sValidationLayers.data(),
  };

  vku__check(vkCreateInstance(&createInfo, nullptr, &this->handle));

  // ...

  // . Create debug messenger
  vku::debugmessenger::create(this->handle, this->debugMessenger);

  // ...

  // . Create surface
  this->surface = vo::window::createSurface(this->handle);

  // ...

  // . Pick physical device

  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(this->handle, &physicalDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(this->handle, &physicalDeviceCount, physicalDevices.data());

  uint32_t maxScore = 0;
  for (const auto &physicalDevice : physicalDevices) {  //

    VkPhysicalDeviceFeatures   features;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    auto const queueIndices = vku::queuefamily::findIndices(physicalDevice, this->surface);

    const int highValue = (                                            //
      (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)  //
    );
    const int midValue  = (                   //
      properties.limits.maxImageDimension2D  //
    );
    const int nullValue = { !vku::queuefamily::isComplete(queueIndices)                                           //
                            or vku::swapchain::isEmpty(physicalDevice, this->surface)                             //
                            or !vku::others::checkDeviceExtensionsSupport(physicalDevice, vo::sDeviceExtensions)  //
                            or !features.geometryShader };

    uint32_t const finalScore = nullValue * ((1000 * highValue) + midValue);

    if (finalScore > maxScore) {
      this->physicalDevice           = physicalDevice;
      this->physicalDeviceFeatures   = features;
      this->physicalDeviceProperties = properties;
      this->queuesIndex              = vku::queuefamily::unrollOptionals(queueIndices);
      maxScore                       = finalScore;
    }
  }
  if (maxScore < 1) { vo__abort("Suitable GPU not found!"); }

  // ...
}

//-----------------------------------------------

void Instance::destroy()
{
  vku::debugmessenger::destroy(this->handle, this->debugMessenger);
  vkDestroySurfaceKHR(this->handle, this->surface, nullptr);
  vkDestroyInstance(this->handle, nullptr);
}

//-----------------------------------------------

}  // namespace vo::vulkan
