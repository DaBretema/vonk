#include "VwQueueFamily.h"

#include <vector>

namespace vku
{
//-----------------------------------------------

void QueueFamily::resetIndices()
{
  for (auto &&[type, q] : mQueues) {
    (void)(type);
    q.index = {};
  }
}

//-----------------------------------------------

void QueueFamily::resetQueues()
{
  for (auto &&[type, q] : mQueues) {
    (void)(type);
    q.queue = VK_NULL_HANDLE;
  }
}

//-----------------------------------------------

bool QueueFamily::isComplete()
{
  for (auto &&[type, q] : mQueues) {
    (void)(type);
    if (!q.index.has_value()) return false;
  }
  return true;
}

//-----------------------------------------------

std::vector<uint32_t> QueueFamily::getUniqueIndices()
{
  std::set<uint32_t> uniqueIndices;
  for (auto &&[type, q] : mQueues) {
    (void)(type);
    uniqueIndices.emplace(q.index.value());
  }

  std::vector<uint32_t> indices;
  indices.reserve(uniqueIndices.size());
  for (auto &&i : uniqueIndices) { indices.emplace_back(i); }

  return indices;
}

//-----------------------------------------------

void QueueFamily::findQueues(VkDevice logicalDevice)
{
  for (auto &&[type, q] : mQueues) { vkGetDeviceQueue(logicalDevice, getIndexVal(type), 0, &getQueueRef(type)); }
}

//-----------------------------------------------

void QueueFamily::findIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
  if (physicalDevice == VK_NULL_HANDLE or surface == VK_NULL_HANDLE) {
    assert(0);
    return;
  }

  // . Get device families
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  // . Reset (in case that reuse this object and one device write previously a fam-index)
  resetIndices();

  // . Find again
  // ??? Have sense prioritize queueFamilies that contains many desired-queues over others with only one ?
  for (uint32_t i = 0u; (!isComplete() and i < queueFamilies.size()); ++i) {
    for (auto &&[type, q] : mQueues) {
      switch (type) {
          // .. Graphics
        case QueueType::graphics: {
          if (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) { getIndexRef(QueueType::graphics) = i; }
        } break;
          // .. Present
        case QueueType::present: {
          if (surface == VK_NULL_HANDLE) { break; }
          VkBool32 presentSupport = false;
          vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
          if (presentSupport) { getIndexRef(QueueType::present) = i; }
        } break;
        //...
        default: {
          assert(0);
        } break;
      }
    }
  }
}

//-----------------------------------------------

}  // namespace vku
