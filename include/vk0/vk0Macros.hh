#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

namespace vk0
{

//-----------------------------------------------------------------------------

// ALERT SYSTEM  ( https://stackoverflow.com/a/3048361 )

#define vk0DefaultErrorMsg "[ERR] - Vulkan - {}:{}\n↪ {}\n"

#define vk0Alert(msg) fmt::print(vk0DefaultErrorMsg, __FILE__, __LINE__, msg)

#define vk0AlertFmt(msg, ...) vk0Alert(fmt::format(msg, __VA_ARGS__))

#define vk0AlertAbort(msg) \
  vk0Alert(msg);           \
  abort();

//-----------------------------------------------------------------------------

// FUNCTION INVOKATION WITH ERROR CONTROL

#define vk0Test(res, msg)            \
  if (res != vk::Result::eSuccess) { \
    vk0Alert(msg);                   \
    abort();                         \
  }
#define vk0TestC(res, msg) \
  if (res != VK_SUCCESS) { \
    vk0Alert(msg);         \
    abort();               \
  }
#define vk0TestFn(fn)               \
  if (fn != vk::Result::eSuccess) { \
    vk0Alert(#fn);                  \
    abort();                        \
  }
#define vk0TestFnC(fn)    \
  if (fn != VK_SUCCESS) { \
    vk0Alert(#fn);        \
    abort();              \
  }

#define vk0Call(fn)        \
  [&]() -> auto            \
  {                        \
    auto [res, val] = fn;  \
    vk0Test(res, #fn);     \
    return std::move(val); \
  }                        \
  ()

//-----------------------------------------------------------------------------

// Get C-like data from container

#define vk0SizeData(container) static_cast<uint32_t>(container.size()), container.data()

//-----------------------------------------------------------------------------

// Get EXT functions

#define vk0InstanceFn(instance, extName, ...) \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { fn(instance, __VA_ARGS__); }

#define vk0InstanceResFn(instance, extName, ...)                                  \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { \
    vk0TestFnC(fn(instance, __VA_ARGS__));                                        \
  }

//-----------------------------------------------------------------------------

}  // namespace vk0
