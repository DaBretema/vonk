#include "VwQueueFamily.h"

#include <vector>

namespace vo
{
bool QueueFamily::isComplete()
{
  return                        //
    mGraphicsIndex.has_value()  //
    //&& mPresentQueueIndex.has_value()
    ;
}

void QueueFamily::findQueues(VkDevice logicalDevice)
{
  vkGetDeviceQueue(logicalDevice, getGraphicsIndex(), 0, &mGraphicsQueue);
  // vkGetDeviceQueue(logicalDevice, getPresentIndex(), 0, &mPresentQueue);
}

void QueueFamily::findIndices(VkPhysicalDevice physicalDevice)
{
  // . Get device families
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  // . Reset (in case that reuse this object and one device write previously a fam-index)
  mGraphicsIndex = {};

  // . Find again
  for (uint32_t i = 0u; (!isComplete() and i < queueFamilies.size()); ++i) {
    if (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) { mGraphicsIndex = i; }
  }
}

}  // namespace vo
