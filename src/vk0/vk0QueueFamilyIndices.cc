#include "vk0/vk0QueueFamilyIndices.hh"

#include "vk0/vk0Macros.hh"

#include <set>

namespace vk0
{
//---------------------------------------------------------------------------

bool QueueFamilyIndices::isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }

//---------------------------------------------------------------------------

QueueFamilyIndices::QueueFamilyIndices(vk::PhysicalDevice const &device, vk::SurfaceKHR const &surface)
{
  auto const queueFamily { device.getQueueFamilyProperties() };

  for (uint32_t i = 0u; (!isComplete() && i < queueFamily.size()); ++i) {
    bool const qc = queueFamily[i].queueCount > 0;
    if (qc && vk0Call(device.getSurfaceSupportKHR(i, surface))) { presentFamily = i; }
    if (qc && (queueFamily[i].queueFlags & vk::QueueFlagBits::eGraphics)) { graphicsFamily = i; }
  }
}

//---------------------------------------------------------------------------

std::vector<vk::DeviceQueueCreateInfo> QueueFamilyIndices::getCreateInfo()
{
  float constexpr defaultPriority        = 1.0f;
  std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };

  std::vector<vk::DeviceQueueCreateInfo> createInfos;

  for (uint32_t queueFamily : uniqueQueueFamilies) {
    createInfos.push_back({ vk::DeviceQueueCreateFlags(),
                            queueFamily,
                            1,  // queueCount
                            &defaultPriority });
  }

  return createInfos;
}

}  // namespace vk0
