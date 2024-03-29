#pragma once

#include "_vulkan.h"

#ifndef DC_ENABLED_HEADLESS
#  include <GLFW/glfw3.h>
#  include <GLFW/glfw3native.h>
#endif

#include <string>
#include <functional>

namespace vonk::window
{  //

//-----------------------------------------------

void init(int32_t w_, int32_t h_, char const *title_);
void cleanup();

void loop(std::function<void(void)> perFrame, std::function<void(void)> onClose, bool polling = false);

VkSurfaceKHR createSurface(VkInstance instance);

VkExtent2D                getFramebufferSize();
std::vector<char const *> getInstanceExts();

void setCallbackKeyboard(GLFWkeyfun keyboardCallback);

//-----------------------------------------------

static inline GLFWwindow *handle  = nullptr;
static inline void *      userPtr = nullptr;

static inline uint32_t    w     = 640 * 2;
static inline uint32_t    h     = 360 * 2;
static inline std::string title = "<>";

static inline bool framebufferResized = false;

//-----------------------------------------------

static auto const setUserPointer = [](auto *ptr) { userPtr = reinterpret_cast<void *>(ptr); };

//-----------------------------------------------

}  // namespace vonk::window
