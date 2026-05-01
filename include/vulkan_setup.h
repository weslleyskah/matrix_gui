#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

// --- Vulkan Global Data ---
extern VkAllocationCallbacks*   g_Allocator;
extern VkInstance               g_Instance;
extern VkPhysicalDevice         g_PhysicalDevice;
extern VkDevice                 g_Device;
extern uint32_t                 g_QueueFamily;
extern VkQueue                  g_Queue;
extern VkDescriptorPool         g_DescriptorPool;

extern ImGui_ImplVulkanH_Window g_MainWindowData;
extern int                      g_MinImageCount;
extern bool                     g_SwapChainRebuild;

void check_vk_result(VkResult err);
void SetupVulkan(const char** extensions, uint32_t extensions_count);
void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
void FramePresent(ImGui_ImplVulkanH_Window* wd);
