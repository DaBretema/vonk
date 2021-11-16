#include "VonkWindow.h"
#include "Vonk.h"

#include "_glm.h"

// === Temporary code
//=====================================================================================================================

// struct Vertex
// {
//   glm::vec3 pos;
//   glm::vec3 color;

//   static VkVertexInputBindingDescription getBindingDescription()
//   {
//     VkVertexInputBindingDescription bindingDescription {};
//     bindingDescription.binding   = 0;
//     bindingDescription.stride    = sizeof(Vertex);
//     bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//     return bindingDescription;
//   }

//   static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
//   {
//     std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};
//     attributeDescriptions[0].binding  = 0;
//     attributeDescriptions[0].location = 0;
//     attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
//     attributeDescriptions[0].offset   = offsetof(Vertex, pos);
//     attributeDescriptions[1].binding  = 0;
//     attributeDescriptions[1].location = 1;
//     attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
//     attributeDescriptions[1].offset   = offsetof(Vertex, color);
//     return attributeDescriptions;
//   }
// };

// void createVertexBuffer(VkDevice device, std::vector<Vertex> vertices)
// {
//   VkBufferCreateInfo bufferInfo {};
//   bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//   bufferInfo.size        = sizeof(Vertex) * GetSizeU32(vertices);
//   bufferInfo.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//   VkBuffer vertexBuffer;
//   VkCheck(vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer));

//   return vertexBuffer;
// }

//=====================================================================================================================

int main()
{
  vonk::Vonk vonk;
  // mVulkan = vonk::init(1280, 720, "Vonk", /*...callbacks*/);

  // .
  // . Window

  vonk::window::init(1280, 720, "Vonk");

  vonk::window::setUserPointer(&vonk);

  vonk::window::setCallbackKeyboard(
    [](MBU GLFWwindow *windowHandle, int key, MBU int scancode, int action, MBU int mods) {
      auto *vonk = static_cast<vonk::Vonk *>(vonk::window::userPtr);  // Cast UserPtr before use it.

      if (key == GLFW_KEY_1 and action == GLFW_PRESS) { vonk->iterScenes(); }
    });

  // .
  // . Init

  vonk.init();
  MBU auto const &baseDrawShader = vonk.createDrawShader("base", "base2", "base");
  // MBU auto const &altDrawShader  = vonk.createDrawShader("alt", "base", "base");

  // .
  // . Setup

  // const std::vector<Vertex> vertices = { { { 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
  //                                        { { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
  //                                        { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };

  vonk::PipelineData_t const pipelineCI {
    // Static
    .pDrawShader = &baseDrawShader,
    // Dynamic
    .commandBuffersData = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 6, 1, 0, 0); } } },
  };

  vonk::PipelineData_t pipelineCI2 = pipelineCI;
  pipelineCI2.pDrawShader          = &baseDrawShader,
  pipelineCI2.commandBuffersData   = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 3, 1, 0, 0); } } },

  vonk.addPipeline(pipelineCI);
  vonk.addPipeline(pipelineCI2);

  // .
  // . Loop

  vonk::window::loop(
    [&]() { vonk.drawFrame(); },   //
    [&]() { vonk.waitDevice(); },  //
    false                          //
  );

  // .
  // . Clean up

  vonk.cleanup();
}
