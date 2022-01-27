#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

//-----------------------------------------------

//
// // === C++ Helpers
//

//-----------------------------------------------

#define MBU [[maybe_unused]]

//-----------------------------------------------

// Logging w/o format
#define LogInfo(msg)  fmt::print("ℹ️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define LogWarn(msg)  fmt::print("⚠️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define LogError(msg) fmt::print("⛔️ ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define Abort(msg) \
  LogError(msg);   \
  abort();

//-----------------------------------------------

// Logging w/ format
#define LogInfof(msg, ...)  LogInfo(fmt::format(msg, __VA_ARGS__))
#define LogWarnf(msg, ...)  LogWarn(fmt::format(msg, __VA_ARGS__))
#define LogErrorf(msg, ...) LogError(fmt::format(msg, __VA_ARGS__))
#define Abortf(msg, ...)       \
  LogErrorf(msg, __VA_ARGS__); \
  abort();

//-----------------------------------------------

// Vector C++ to C helpers
#define GetSizeOfFirst(v)            (v.size() < 1 ? 0u : sizeof(v.at(0)))
#define GetSizeOfFirstU32(v)         static_cast<uint32_t>(GetSizeOfFirst(v))
#define GetSizeOfFirstAs(newType, v) static_cast<newType>(GetSizeOfFirst(v))
#define GetSizeOf(v)                 (v.size() < 1 ? v.size() : v.size() * sizeof(v.at(0)))
#define GetSizeOfU32(v)              static_cast<uint32_t>(GetSizeOf(v))
#define GetSizeOfAs(newType, v)      static_cast<newType>(GetSizeOf(v))
#define GetCount(v)                  v.size()
#define GetCountU32(v)               static_cast<uint32_t>(GetCount(v))
#define GetCountAs(newType, v)       static_cast<newType>(GetCount(v))
#define GetData(v)                   v.data()
#define GetDataAs(newType, v)        reinterpret_cast<newType>(GetData(v))

//-----------------------------------------------

// Evaluate critical condition

#define AbortIf(conditionCode) \
  if (conditionCode) { Abort(#conditionCode); }

#define AbortIfMsg(conditionCode, msg) \
  if (conditionCode) { Abortf("{} --> {}", #conditionCode, msg); }

#define AbortIfMsgf(conditionCode, msg, ...) \
  if (conditionCode) { Abortf("{} --> {}", #conditionCode, fmt::format(msg, __VA_ARGS__)); }

//-----------------------------------------------

// Custom Assert
#define Assert(cond) assert(cond)

//-----------------------------------------------

// Fmt ptr
#define PtrStr(p) fmt::format("{}", fmt::ptr(p))

//-----------------------------------------------

#define AssertLogReturn(toRet, ...) \
  {                                 \
    LogErrorf(__VA_ARGS__);         \
    Assert(0);                      \
    return toRet;                   \
  }

//-----------------------------------------------

//
// // === VULKAN MACROS = vonk__
//

//-----------------------------------------------

// Validate api calls
#define VkCheck(vulkanCode) \
  if (VkResult res = vulkanCode; res != VK_SUCCESS) { Abortf("{} : {}", res, #vulkanCode); }

// Get instance functions
#define VkInstanceFn(instance, extName, ...)                                      \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { \
    fn(instance, __VA_ARGS__);                                                    \
  } else {                                                                        \
    LogErrorf("Function {} is not available", #extName);                          \
  }

//-----------------------------------------------

// Vulkan things, move out of here...
struct DataInfo_t
{
  uint32_t    elemSize = 0u;
  uint32_t    count    = 0;
  void const *data     = nullptr;
};
DataInfo_t GetDataInfo(auto const &v) { return { GetSizeOfFirstU32(v), GetCountU32(v), GetDataAs(void const *, v) }; }
// #define GetDataInfo(v) std::forward_as_tuple(GetSizeOfFirst(v), GetCountU32(v), GetData(v))

//-----------------------------------------------
