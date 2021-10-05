#pragma once
#include "Macros.h"
#include "VwVulkan.h"

#include <set>
#include <string>
#include <optional>
#include <unordered_map>

namespace vku
{
//

//-----------------------------------------------

enum class QueueType
{
  graphics,
  present,
};

//-----------------------------------------------

struct QueueData
{
  std::optional<uint32_t> index = {};
  VkQueue                 queue = VK_NULL_HANDLE;
};

inline static QueueData const QueueDataNull { {}, VK_NULL_HANDLE };

//-----------------------------------------------

class QueueFamily
{
public:
  void findIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
  void findQueues(VkDevice logicalDevice);

  bool                  isComplete();
  std::vector<uint32_t> getUniqueIndices();

  VkQueue  getQueueVal(QueueType t) const { return mQueues.at(t).queue; }
  uint32_t getIndexVal(QueueType t) const { return mQueues.at(t).index.value(); }

private:
  std::unordered_map<QueueType, QueueData> mQueues { { QueueType::graphics, QueueDataNull },
                                                     { QueueType::present, QueueDataNull } };

  void     resetQueues();
  VkQueue &getQueueRef(QueueType t) { return mQueues.at(t).queue; }

  void                     resetIndices();
  std::optional<uint32_t> &getIndexRef(QueueType t) { return mQueues.at(t).index; }
};

//-----------------------------------------------

}  // namespace vku
