#include "App.hh"

int main()
{
  Vonsai::App(800, 600, "Hello Vulkan").run();

  // static const char *  exts[] = { "VK_KHR_get_physical_device_properties2" };
  // VkInstance           inst;
  // VkInstanceCreateInfo createInst;
  // createInst.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  // createInst.pNext                   = NULL;
  // createInst.flags                   = 0;
  // createInst.pApplicationInfo        = NULL;
  // createInst.enabledLayerCount       = 0;
  // createInst.ppEnabledLayerNames     = NULL;
  // createInst.enabledExtensionCount   = 1;
  // createInst.ppEnabledExtensionNames = exts;
  // VkResult res                       = vkCreateInstance(&createInst, NULL, &inst);
  // if (res != VK_SUCCESS) {
  //   fprintf(stderr, "Failed to create instance, error %u\n", res);
  //   return 1;
  // }

  // uint32_t devCount = 0;
  // res               = vkEnumeratePhysicalDevices(inst, &devCount, NULL);
  // if (res != VK_SUCCESS && res != VK_INCOMPLETE) {
  //   fprintf(stderr, "Failed to enumerate physdevs, error %u\n", res);
  //   return 1;
  // }
  // // void *            a        = malloc(devCount * sizeof(VkPhysicalDevice));
  // VkPhysicalDevice *physDevs = new VkPhysicalDevice[devCount];
  // res                        = vkEnumeratePhysicalDevices(inst, &devCount, physDevs);
  // if (res != VK_SUCCESS) {
  //   fprintf(stderr, "Failed to enumerate physdevs, error %u\n", res);
  //   return 1;
  // }

  // PFN_vkGetPhysicalDeviceProperties2KHR pvkGetPhysicalDeviceProperties2KHR =
  //   (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(inst, "vkGetPhysicalDeviceProperties2KHR");
  // if (!pvkGetPhysicalDeviceProperties2KHR) {
  //   fprintf(stderr, "Enabled extension but no extension function?!\n");
  //   return 1;
  // }

  // VkPhysicalDeviceProperties2KHR         deviceProps;
  // VkPhysicalDeviceMultiviewPropertiesKHR multiviewProps;
  // multiviewProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHR;
  // multiviewProps.pNext = NULL;
  // deviceProps.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
  // deviceProps.pNext    = &multiviewProps;
  // pvkGetPhysicalDeviceProperties2KHR(physDevs[0], &deviceProps);

  // printf("Multiview properties:\n");
  // printf("\tmaxMultiviewViewCount = %u\n", multiviewProps.maxMultiviewViewCount);
  // printf("\tmaxMultiviewInstanceIndex = %u\n", multiviewProps.maxMultiviewInstanceIndex);
  // return 0;

  return 0;
}
