#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <filesystem>

#include <Eigen/Dense>
#include "algoritmos.h"

// --- Vulkan Global Data ---
static VkAllocationCallbacks*   g_Allocator = nullptr;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int                      g_MinImageCount = 2;
static bool                     g_SwapChainRebuild = false;

// Helper to check Vulkan results
static void check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
}

void SetupVulkan(const char** extensions, uint32_t extensions_count) {
    VkResult err;

    // Create Instance
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = extensions_count;
    create_info.ppEnabledExtensionNames = extensions;
    err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
    check_vk_result(err);

    // Select GPU
    uint32_t gpu_count;
    vkEnumeratePhysicalDevices(g_Instance, &gpu_count, nullptr);
    std::vector<VkPhysicalDevice> gpus(gpu_count);
    vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus.data());
    g_PhysicalDevice = gpus[0];

    // Select Queue Family
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queues(count);
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues.data());
    for (uint32_t i = 0; i < count; i++)
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { g_QueueFamily = i; break; }

    // Create Logical Device
    float queue_priority[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = g_QueueFamily;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = queue_priority;

    const char* device_extensions[] = { "VK_KHR_swapchain" };
    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = 1;
    device_info.ppEnabledExtensionNames = device_extensions;
    err = vkCreateDevice(g_PhysicalDevice, &device_info, g_Allocator, &g_Device);
    check_vk_result(err);
    vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);

    // Create Descriptor Pool
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
    check_vk_result(err);
}

void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height) {
    wd->Surface = surface;
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);
    
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, present_modes, IM_ARRAYSIZE(present_modes));
    
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd) {
    if (g_SwapChainRebuild) return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount;
}

// --- UI Logic helpers ---
std::unordered_map<std::string, std::vector<std::vector<std::string>>> g_BufferMap;

void PushButtonStyle() {
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 80, 80, 80));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(120, 120, 120, 120));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(160, 160, 160, 150));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 220, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
}

void PopButtonStyle() { ImGui::PopStyleVar(); ImGui::PopStyleColor(4); }

void PushWindowStyle() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
}

void PopWindowStyle() { ImGui::PopStyleVar(2); ImGui::PopStyleColor(); }

void DrawMatrixInput3x3(const char* id, double mat[3][3]) {
    static int nextFocusRow = -1, nextFocusCol = -1;
    static const char* activeId = nullptr;
    const float cellWidth = 60.0f;

    auto& cellBuffers = g_BufferMap[std::string(id)];
    if ((int)cellBuffers.size() != 3) cellBuffers.assign(3, std::vector<std::string>(3, "0"));
    for (int i = 0; i < 3; i++)
        if ((int)cellBuffers[i].size() != 3) cellBuffers[i].assign(3, "0");

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120, 120, 120, 40));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120, 120, 120, 90));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

    ImGui::PushID(id);
    for (int i = 0; i < 3; i++) {
        ImGui::PushID(i);
        for (int j = 0; j < 3; j++) {
            ImGui::PushID(j);
            if (activeId == id && nextFocusRow == i && nextFocusCol == j) {
                ImGui::SetKeyboardFocusHere();
                nextFocusRow = -1; nextFocusCol = -1;
            }

            char buf[32];
            strncpy(buf, cellBuffers[i][j].c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';

            ImGui::SetNextItemWidth(cellWidth);
            if (ImGui::InputText("##cell", buf, sizeof(buf), ImGuiInputTextFlags_CharsNoBlank)) {
                cellBuffers[i][j] = buf;
                mat[i][j] = parseFraction(buf);
            }

            if (ImGui::IsItemFocused()) {
                activeId = id;
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && j > 0) { nextFocusRow = i;   nextFocusCol = j - 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && j < 2) { nextFocusRow = i;   nextFocusCol = j + 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = j; }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < 2) { nextFocusRow = i + 1; nextFocusCol = j; }
            }
            ImGui::PopID(); // j
            if (j < 2) ImGui::SameLine();
        }
        ImGui::PopID(); // i
    }
    ImGui::PopID(); // id
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void DrawMatrixInputMxN(const char* id, std::vector<std::vector<double>>& mat, int rows, int cols) {
    static int nextFocusRow = -1, nextFocusCol = -1;
    static const char* activeId = nullptr;
    const float cellWidth = 60.0f;

    auto& cellBuffers = g_BufferMap[std::string(id)];
    if ((int)cellBuffers.size() != rows) cellBuffers.resize(rows);
    for (int i = 0; i < rows; i++)
        if ((int)cellBuffers[i].size() != cols) cellBuffers[i].resize(cols, "0");

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120, 120, 120, 40));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120, 120, 120, 90));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

    ImGui::PushID(id);
    for (int i = 0; i < rows; i++) {
        ImGui::PushID(i);
        for (int j = 0; j < cols; j++) {
            ImGui::PushID(j);
            if (activeId == id && nextFocusRow == i && nextFocusCol == j) {
                ImGui::SetKeyboardFocusHere();
                nextFocusRow = -1; nextFocusCol = -1;
            }

            char buf[32];
            strncpy(buf, cellBuffers[i][j].c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';

            ImGui::SetNextItemWidth(cellWidth);
            if (ImGui::InputText("##cell", buf, sizeof(buf), ImGuiInputTextFlags_CharsNoBlank)) {
                cellBuffers[i][j] = buf;
                mat[i][j] = parseFraction(buf);
            }

            if (ImGui::IsItemFocused()) {
                activeId = id;
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && j > 0) { nextFocusRow = i;   nextFocusCol = j - 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && j < cols - 1) { nextFocusRow = i;   nextFocusCol = j + 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = j; }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = j; }
            }
            ImGui::PopID(); // j
            if (j < cols - 1) ImGui::SameLine();
        }
        ImGui::PopID(); // i
    }
    ImGui::PopID(); // id
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void DrawSystemInput(const char* id, Eigen::MatrixXd& A, Eigen::VectorXd& b) {
    int rows = (int)A.rows();
    int cols = (int)A.cols();
    static int nextFocusRow = -1, nextFocusCol = -1;
    static const char* activeId = nullptr;
    const float cellWidth = 60.0f;

    auto& cellBuffers = g_BufferMap[std::string(id)];
    if ((int)cellBuffers.size() != rows) cellBuffers.resize(rows);
    for (int i = 0; i < rows; i++) {
        if ((int)cellBuffers[i].size() != cols + 1) {
            cellBuffers[i].resize(cols + 1, "0");
        }
    }

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120, 120, 120, 40));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120, 120, 120, 90));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

    ImGui::PushID(id);
    for (int i = 0; i < rows; i++) {
        ImGui::PushID(i);
        for (int j = 0; j < cols; j++) {
            ImGui::PushID(j);
            if (activeId == id && nextFocusRow == i && nextFocusCol == j) {
                ImGui::SetKeyboardFocusHere();
                nextFocusRow = -1; nextFocusCol = -1;
            }

            char buf[32];
            strncpy(buf, cellBuffers[i][j].c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';

            ImGui::SetNextItemWidth(cellWidth);
            if (ImGui::InputText("##cellA", buf, sizeof(buf), ImGuiInputTextFlags_CharsNoBlank)) {
                cellBuffers[i][j] = buf;
                A(i, j) = parseFraction(buf);
            }

            if (ImGui::IsItemFocused()) {
                activeId = id;
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && j > 0) { nextFocusRow = i;   nextFocusCol = j - 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) { nextFocusRow = i;   nextFocusCol = j + 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = j; }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = j; }
            }
            ImGui::PopID(); // j
            ImGui::SameLine();
            ImGui::Text("x%d %s", j + 1, (j < cols - 1) ? "+ " : "= ");
            ImGui::SameLine();
        }

        // Handle vector b cell in the same row
        ImGui::PushID(cols); // Use 'cols' as index for the b column
        if (activeId == id && nextFocusRow == i && nextFocusCol == cols) {
            ImGui::SetKeyboardFocusHere();
            nextFocusRow = -1; nextFocusCol = -1;
        }

        char bufB[32];
        strncpy(bufB, cellBuffers[i][cols].c_str(), sizeof(bufB));
        bufB[sizeof(bufB) - 1] = '\0';

        ImGui::SetNextItemWidth(cellWidth);
        if (ImGui::InputText("##cellB", bufB, sizeof(bufB), ImGuiInputTextFlags_CharsNoBlank)) {
            cellBuffers[i][cols] = bufB;
            b(i) = parseFraction(bufB);
        }

        if (ImGui::IsItemFocused()) {
            activeId = id;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) { nextFocusRow = i;   nextFocusCol = cols - 1; }
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = cols; }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = cols; }
        }
        ImGui::PopID(); // b-column ID
        ImGui::PopID(); // i-row ID
    }
    ImGui::PopID(); // id-matrix ID
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void DrawMatrixResult3x3(const char* id, double mat[3][3]) {
    const float cellWidth = 100.0f;
    const float cellHeight = ImGui::GetFrameHeight();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ImVec2 pos = ImVec2(origin.x + j * 105, origin.y + i * (cellHeight + 4));
            dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
            std::string txt = valueToFraction(mat[i][j]);
            dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
        }
    }
    ImGui::Dummy(ImVec2(3 * 105, 3 * (cellHeight + 4)));
}

void DrawMatrixResultMxN(const char* id, const std::vector<std::vector<double>>& mat) {
    if (mat.empty()) return;
    const float cellWidth = 100.0f;
    const float cellHeight = ImGui::GetFrameHeight();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    for (size_t i = 0; i < mat.size(); i++) {
        for (size_t j = 0; j < mat[0].size(); j++) {
            ImVec2 pos = ImVec2(origin.x + j * 105, origin.y + i * (cellHeight + 4));
            dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
            std::string txt = valueToFraction(mat[i][j]);
            dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
        }
    }
    ImGui::Dummy(ImVec2(mat[0].size() * 105, mat.size() * (cellHeight + 4)));
}

void DrawMatrixResultEigen(const char* id, const Eigen::MatrixXd& mat) {
    if (mat.size() == 0) return;
    const float cellWidth = 100.0f;
    const float cellHeight = ImGui::GetFrameHeight();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    for (int i = 0; i < mat.rows(); i++) {
        for (int j = 0; j < mat.cols(); j++) {
            ImVec2 pos = ImVec2(origin.x + j * 105, origin.y + i * (cellHeight + 4));
            dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
            std::string txt = valueToFraction(mat(i, j));
            dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
        }
    }
    ImGui::Dummy(ImVec2(mat.cols() * 105, mat.rows() * (cellHeight + 4)));
}

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Matrix Operations", nullptr, nullptr);

    uint32_t extensions_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    SetupVulkan(extensions, extensions_count);

    VkSurfaceKHR surface;
    check_vk_result(glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface));

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Load Font from file with multiple path attempts
    const char* font_filename = "Roboto-Regular.ttf";
    std::vector<std::string> font_paths = {
        "dependencies/font/Roboto/static/Roboto-Regular.ttf",
        "../dependencies/font/Roboto/static/Roboto-Regular.ttf",
        "../../dependencies/font/Roboto/static/Roboto-Regular.ttf",
        "matrix_gui/dependencies/font/Roboto/static/Roboto-Regular.ttf"
    };

    ImFont* robotoFont = nullptr;
    for (const auto& path : font_paths) {
        if (std::filesystem::exists(path)) {
            robotoFont = io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f);
            if (robotoFont) break;
        }
    }

    if (!robotoFont) {
        std::cerr << "Warning: Could not load Roboto-Regular.ttf from any searched path. Using default font." << std::endl;
        std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
    }

    if (robotoFont)
        io.FontDefault = robotoFont;

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.MinImageCount = (uint32_t)g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.Allocator = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    // Upload Fonts
    {
        VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        vkEndCommandBuffer(command_buffer);
        vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);

        vkDeviceWaitIdle(g_Device);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (g_SwapChainRebuild) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (width > 0 && height > 0) {
                ImGui_ImplVulkan_SetMinImageCount((uint32_t)g_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, (uint32_t)g_MinImageCount);
                g_MainWindowData.FrameIndex = 0;
                g_SwapChainRebuild = false;
            }
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::GetIO().IniFilename = nullptr;

        PushWindowStyle();
        PushButtonStyle();

        ImGui::Begin("Matrix Operations: Multiplication, Determinant, Row Echelon, Inverse", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        if (ImGui::CollapsingHeader("Matrix 3x3", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static double matA[3][3] = { 0.0 };
            static double matB[3][3] = { 0.0 };
            static double matRes[3][3] = { 0.0 };

            ImGui::Columns(2, "Inputs", false);
            ImGui::Text("Matrix A");
            DrawMatrixInput3x3("A", matA);

            ImGui::NextColumn();
            ImGui::Text("Matrix B");
            DrawMatrixInput3x3("B", matB);

            ImGui::Columns(1);
            ImGui::Separator();

            if (ImGui::Button("Multiplicar", ImVec2(200, 30)))
            {
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        matRes[i][j] = 0.0;

                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        for (int k = 0; k < 3; k++)
                            matRes[i][j] += matA[i][k] * matB[k][j];
            }

            ImGui::Text("Matriz AxB:");
            DrawMatrixResult3x3("matRes", matRes);

            ImGui::Separator();
            static double detA = 0.0;
            if (ImGui::Button("Laplace", ImVec2(200, 30)))
            {
                detA = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
                    - matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
                    + matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);
            }
            ImGui::Text("Determinante: %.2f", detA);

            ImGui::Separator();
            static double matEsc[3][3] = { 0.0 };
            if (ImGui::Button("Escalonar", ImVec2(200, 30)))
            {
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        matEsc[i][j] = matA[i][j];

                for (int i = 0; i < 2; i++) {
                    if (matEsc[i][i] == 0) {
                        for (int k = i + 1; k < 3; k++) {
                            if (matEsc[k][i] != 0) {
                                for (int col = 0; col < 3; col++) std::swap(matEsc[i][col], matEsc[k][col]);
                                break;
                            }
                        }
                    }
                    if (matEsc[i][i] != 0) {
                        for (int k = i + 1; k < 3; k++) {
                            double factor = matEsc[k][i] / matEsc[i][i];
                            for (int col = i; col < 3; col++) matEsc[k][col] -= factor * matEsc[i][col];
                        }
                    }
                }
            }
            ImGui::Text("Escalonamento:");
            DrawMatrixResult3x3("matEsc", matEsc);

            ImGui::Separator();
            static double matInv[3][3] = { 0.0 };
            static bool hasInverse = false;
            if (ImGui::Button("Inversa", ImVec2(200, 30)))
            {
                double detVal = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
                    - matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
                    + matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);

                if (std::abs(detVal) > 0.0001)
                {
                    double cof[3][3];
                    cof[0][0] = (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1]);
                    cof[0][1] = -(matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0]);
                    cof[0][2] = (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);
                    cof[1][0] = -(matA[0][1] * matA[2][2] - matA[0][2] * matA[2][1]);
                    cof[1][1] = (matA[0][0] * matA[2][2] - matA[0][2] * matA[2][0]);
                    cof[1][2] = -(matA[0][0] * matA[2][1] - matA[0][1] * matA[2][0]);
                    cof[2][0] = (matA[0][1] * matA[1][2] - matA[0][2] * matA[1][1]);
                    cof[2][1] = -(matA[0][0] * matA[1][2] - matA[0][2] * matA[1][0]);
                    cof[2][2] = (matA[0][0] * matA[1][1] - matA[0][1] * matA[1][0]);

                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            matInv[i][j] = cof[j][i] / detVal;
                        }
                    }
                    hasInverse = true;
                }
                else hasInverse = false;
            }
            ImGui::Text("Inversa:");
            if (hasInverse) DrawMatrixResult3x3("matInv", matInv);
            else ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Matriz não inversível (Det = 0)");
        }

        if (ImGui::CollapsingHeader("Matrix MxN"))
        {
            ImGui::Columns(2, "MxNInputs", false);

            static int rows = 3, cols = 3;
            static std::vector<std::vector<double>> matMN(rows, std::vector<double>(cols, 0.0));
            static double detMN = 0.0;
            static bool hasDetMN = true;
            static bool detComputed = false;

            static bool multiplicationValid = true;
            static std::vector<std::vector<double>> matMN3;
            static std::vector<std::vector<double>> matMN4;

            int prevRows = rows, prevCols = cols;
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Rows##mn", &rows);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Columns##mn", &cols);

            if (rows < 1) rows = 1; if (cols < 1) cols = 1;
            if (rows != prevRows || cols != prevCols) {
                matMN.assign(rows, std::vector<double>(cols, 0.0));
                g_BufferMap["MN"].clear();
                matMN3.clear();
                multiplicationValid = true;
                hasDetMN = true;
                detComputed = false;
            }
            
            DrawMatrixInputMxN("MN", matMN, rows, cols);

            ImGui::NextColumn();
            static int rows2 = 3, cols2 = 3;
            static std::vector<std::vector<double>> matMN2(rows2, std::vector<double>(cols2, 0.0));

            int prevRows2 = rows2, prevCols2 = cols2;
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Rows##mn2", &rows2);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Columns##mn2", &cols2);

            if (rows2 < 1) rows2 = 1; if (cols2 < 1) cols2 = 1;
            if (rows2 != prevRows2 || cols2 != prevCols2) {
                matMN2.assign(rows2, std::vector<double>(cols2, 0.0));
                matMN3.clear();
                multiplicationValid = true;
            }

            DrawMatrixInputMxN("MN2", matMN2, rows2, cols2);

            ImGui::Columns(1);
            ImGui::Separator();

            if (ImGui::Button("Multiplicar##mn", ImVec2(200, 30)))
            {
                try {
                    matMN3 = multiplyMatrices(matMN, matMN2);
                    multiplicationValid = true;
                }
                catch (const std::exception& e) {
                    multiplicationValid = false;
                }
            }
            if (multiplicationValid)
            {
                DrawMatrixResultMxN("matMN3", matMN3);
            }
            else {
                ImGui::TextDisabled("Multiplicação inválida: Colunas da Matriz A devem ser iguais às Linhas da Matriz B.");
            }

            ImGui::Separator();
            if (ImGui::Button("Escalonar##mn", ImVec2(200, 30)))
            {
                matMN4 = escalonar(matMN);
            }
            DrawMatrixResultMxN("matMN4", matMN4);

            ImGui::Separator();
            if (ImGui::Button("Determinante##mn", ImVec2(200, 30)))
            {
                if (rows == cols)
                {
                    try {
                        detMN = determinant(matMN);
                        hasDetMN = true;
                        detComputed = true;
                    }
                    catch (const std::exception& e) {
                        hasDetMN = false;
                        detComputed = false;
                    }
                }
                else
                {
                    hasDetMN = false;
                    detComputed = false;
                }
            }

            if (!hasDetMN) {
                ImGui::TextDisabled("Determinante: Apenas para matrizes quadradas.");
            }
            else if (detComputed)
            {
                ImGui::Text("Determinante (Gauss): %.2f", detMN);
            }

            ImGui::Separator();
            static Eigen::MatrixXd matMNInv;
            static bool hasInvMN = false;
            static bool invMNComputed = false;

            if (ImGui::Button("Inversa##mn", ImVec2(200, 30) ))
            {
                if (rows == cols)
                {
                    Eigen::MatrixXd eigenMat(rows, cols);
                    for (int i = 0; i < rows; ++i)
                        for (int j = 0; j < cols; ++j)
                            eigenMat(i, j) = matMN[i][j];

                    if (std::abs(eigenMat.determinant()) > 1e-9)
                    {
                        matMNInv = eigenMat.inverse();
                        hasInvMN = true;
                        invMNComputed = true;
                    }
                    else
                    {
                        hasInvMN = false;
                        invMNComputed = true;
                    }
                }
                else
                {
                    hasInvMN = false;
                    invMNComputed = false;
                }
            }

            if (rows != cols && invMNComputed) {
                ImGui::TextDisabled("Inversa: Apenas para matrizes quadradas.");
            }
            else if (invMNComputed)
            {
                if (hasInvMN)
                {
                    ImGui::Text("Inversa:");
                    DrawMatrixResultEigen("matMNInv", matMNInv);
                }
                else
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Matriz não inversível (Det = 0)");
            }
        }

        if (ImGui::CollapsingHeader("System of Equations", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static int rowsS = 3, colsS = 3;
            static Eigen::MatrixXd matA(3, 3);
            static Eigen::VectorXd vecB(3);
            static Eigen::VectorXd vecX;
            static bool solved = false;
            static int rankA = 0;
            static int rankAug = 0;
            static int systemStatus = 0; 

            static bool initialized = false;
            if (!initialized) {
                matA.setZero();
                vecB.setZero();
                initialized = true;
            }

            ImGui::Text("Dimensões do Sistema:");
            int prevRows = rowsS, prevCols = colsS;
            ImGui::SetNextItemWidth(120); ImGui::InputInt("Equações (m)##sys", &rowsS);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120); ImGui::InputInt("Variáveis (n)##sys", &colsS);

            if (rowsS < 1) rowsS = 1; if (colsS < 1) colsS = 1;
            if (rowsS != prevRows || colsS != prevCols) {
                matA.conservativeResize(rowsS, colsS);
                vecB.conservativeResize(rowsS);
                if (rowsS > prevRows) {
                    matA.bottomRows(rowsS - prevRows).setZero();
                    vecB.tail(rowsS - prevRows).setZero();
                }
                if (colsS > prevCols) {
                    matA.rightCols(colsS - prevCols).setZero();
                }
                solved = false;
                systemStatus = 0;
            }

            ImGui::Separator();
            ImGui::Text("Sistema de Equações (Ax = b):");
            DrawSystemInput("SysInput", matA, vecB);

            ImGui::Separator();
            if (ImGui::Button("Resolver Sistema", ImVec2(200, 30))) {
                Eigen::FullPivLU<Eigen::MatrixXd> lu(matA);
                rankA = (int)lu.rank();
                
                Eigen::MatrixXd matAug(rowsS, colsS + 1);
                matAug << matA, vecB;

                Eigen::FullPivLU<Eigen::MatrixXd> luAug(matAug);
                rankAug = (int)luAug.rank();

                if (rankA == rankAug) {
                    if (rankA == colsS) {
                        systemStatus = 1; 
                        vecX = lu.solve(vecB);
                    }
                    else {  
                        systemStatus = 2; 
                        vecX = lu.solve(vecB); 
                    }
                }
                else {
                    systemStatus = 3;
                }
                solved = true;
            }

            if (solved) {
                ImGui::Text("Posto da Matriz de Coeficientes (rank A): %d", rankA);
                ImGui::Text("Posto da Matriz Ampliada (rank [A|b]): %d", rankAug);
                ImGui::Text("Número de Variáveis (n): %d", colsS);

                if (systemStatus == 1) {
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "O sistema possui uma ÚNICA solução (SPD).");
                    ImGui::Separator();
                    ImGui::Text("Solução encontrada:");
                    for (int i = 0; i < vecX.size(); i++) {
                        std::string fractionStr = valueToFraction(vecX(i));
                        ImGui::Text("x%d = %s", i + 1, fractionStr.c_str());
                    }
                }
                else if (systemStatus == 2) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "O sistema possui INFINITAS soluções (SPI).");
                    ImGui::Text("Grau de liberdade: %d", colsS - rankA);
                    ImGui::Separator();
                    ImGui::Text("Uma solução possível:");
                    for (int i = 0; i < vecX.size(); i++) {
                        std::string fractionStr = valueToFraction(vecX(i));
                        ImGui::Text("x%d = %s", i + 1, fractionStr.c_str());
                    }
                }
                else if (systemStatus == 3) {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "O sistema NÃO possui solução (SI).");
                }
            }
        }

        PopButtonStyle();
        ImGui::End();
        PopWindowStyle();

        ImGui::Render();
        FrameRender(wd, ImGui::GetDrawData());
        FramePresent(wd);
    }

    vkDeviceWaitIdle(g_Device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, wd, g_Allocator);
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);
    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
