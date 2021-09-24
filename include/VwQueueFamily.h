#pragma once
#include "Macros.h"
#include "VwVulkan.h"
#include <optional>

namespace vo
{
class QueueFamily
{
public:
  void findIndices(VkPhysicalDevice physicalDevice);
  void findQueues(VkDevice logicalDevice);
  bool isComplete();

  // --- Graphics

public:
  uint32_t getGraphicsIndex() { return mGraphicsIndex.value(); }

private:
  std::optional<uint32_t> mGraphicsIndex;
  VkQueue                 mGraphicsQueue = VK_NULL_HANDLE;

  // --- Graphics

public:
  uint32_t getPresentIndex() { return mPresentIndex.value(); }

private:
  std::optional<uint32_t> mPresentIndex;
  MBU VkQueue             mPresentQueue = VK_NULL_HANDLE;

  //
};

}  // namespace vo
