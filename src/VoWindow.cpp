#include "VoWindow.h"

#include "Macros.h"

namespace vo::window
{  //

// //-----------------------------------------------

// void waitEvents() { glfwWaitEvents(); }
// void pollEvents() { glfwPollEvents(); }

//-----------------------------------------------

void init(int32_t w_, int32_t h_, char const *title_)
{
  w     = w_;
  h     = h_;
  title = title_;

  vo__check(glfwInit());
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);     // Resize windows takes special care
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Avoid OpenGL context creation

  handle = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
  if (!handle) { vo__abort("Unable to create a window using GLFW"); }

  // . Callbacks setup

  // .. On FrameBuffer Resized
  glfwSetFramebufferSizeCallback(handle, [](MBU GLFWwindow *window, MBU int w, MBU int h) {
    framebufferResized = true;
  });
}

//-----------------------------------------------

void cleanup()
{
  glfwDestroyWindow(handle);
  glfwTerminate();
}

//-----------------------------------------------

void loop(std::function<void(void)> perFrame, std::function<void(void)> onClose, bool polling)
{
  while (!glfwWindowShouldClose(handle)) {  //

    // ::: Handle window minimization
    if (polling) {
      auto size = vo::window::getFramebufferSize();
      while (size.width == 0 || size.height == 0) {
        size = vo::window::getFramebufferSize();
        glfwWaitEvents();
      }
    }

    polling ? glfwPollEvents() : glfwWaitEvents();
    perFrame();
  }

  onClose();
}

//-----------------------------------------------

VkSurfaceKHR createSurface(VkInstance instance)
{
  VkSurfaceKHR surface;
  vku__check(glfwCreateWindowSurface(instance, handle, nullptr, &surface));
  return surface;
}

//-----------------------------------------------

VkExtent2D getFramebufferSize()
{
  int w, h;
  glfwGetFramebufferSize(handle, &w, &h);
  return { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
}

//-----------------------------------------------

std::vector<char const *> getRequiredInstanceExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  char const **glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  return std::vector<char const *> { glfwExtensions, glfwExtensions + glfwExtensionCount };
}

//-----------------------------------------------

}  // namespace vo::window