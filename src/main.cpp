#include "vulkan_setup.h"
#include "app_functions.h"
#include "Roboto-Regular.embed.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>
#include <filesystem>

#include <Eigen/Dense>

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Matrix GUI", nullptr, nullptr);

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

    // Load font
    /*
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
        std::cerr << "Warning: Could not load Roboto-Regular.ttf from any searched path. Using default font.\n";
        std::cerr << "Current working directory: " << std::filesystem::current_path() << "\n";
    } else {
        io.FontDefault = robotoFont;
    }
    */
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    
    ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF(
        const_cast<uint8_t*>(g_RobotoRegular), sizeof(g_RobotoRegular), 18.0f, &font_cfg
    );
    
    if (!robotoFont) {
        std::cerr << "Warning: Could not load embedded Roboto font. Using default.\n";
    } else {
        io.FontDefault = robotoFont;
    }

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance        = g_Instance;
    init_info.PhysicalDevice  = g_PhysicalDevice;
    init_info.Device          = g_Device;
    init_info.QueueFamily     = g_QueueFamily;
    init_info.Queue           = g_Queue;
    init_info.DescriptorPool  = g_DescriptorPool;
    init_info.MinImageCount   = (uint32_t)g_MinImageCount;
    init_info.ImageCount      = wd->ImageCount;
    init_info.Allocator       = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    // Upload fonts
    {
        VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers    = &command_buffer;
        vkEndCommandBuffer(command_buffer);
        vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);

        vkDeviceWaitIdle(g_Device);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    // --- Main loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0)
            continue;

        if (g_SwapChainRebuild) {
            ImGui_ImplVulkan_SetMinImageCount((uint32_t)g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device,
                &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, (uint32_t)g_MinImageCount);
            g_MainWindowData.FrameIndex = 0;
            g_SwapChainRebuild = false;
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

        ImGui::Begin("Matrix Operations: Multiplication, Determinant, Row Echelon, Inverse", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // ----------------------------------------------------------------
        // Matrix 3x3
        // ----------------------------------------------------------------
        if (ImGui::CollapsingHeader("Matrix 3x3", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static double matA[3][3]   = { 0.0 };
            static double matB[3][3]   = { 0.0 };
            static double matRes[3][3] = { 0.0 };

            ImGui::Columns(2, "Inputs", false);
            ImGui::Text("Matrix A");
            DrawMatrixInput3x3("A", matA);

            ImGui::NextColumn();
            ImGui::Text("Matrix B");
            DrawMatrixInput3x3("B", matB);
            ImGui::Columns(1);
            ImGui::Separator();

            if (ImGui::Button("Multiplicar", ImVec2(200, 30))) {
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
            if (ImGui::Button("Laplace", ImVec2(200, 30))) {
                detA = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
                     - matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
                     + matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);
            }
            ImGui::Text("Determinante: %.2f", detA);

            ImGui::Separator();
            static double matEsc[3][3] = { 0.0 };
            if (ImGui::Button("Escalonar", ImVec2(200, 30))) {
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
            if (ImGui::Button("Inversa", ImVec2(200, 30))) {
                double detVal = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
                              - matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
                              + matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);

                if (std::abs(detVal) > 0.0001) {
                    double cof[3][3];
                    cof[0][0] =  (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1]);
                    cof[0][1] = -(matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0]);
                    cof[0][2] =  (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);
                    cof[1][0] = -(matA[0][1] * matA[2][2] - matA[0][2] * matA[2][1]);
                    cof[1][1] =  (matA[0][0] * matA[2][2] - matA[0][2] * matA[2][0]);
                    cof[1][2] = -(matA[0][0] * matA[2][1] - matA[0][1] * matA[2][0]);
                    cof[2][0] =  (matA[0][1] * matA[1][2] - matA[0][2] * matA[1][1]);
                    cof[2][1] = -(matA[0][0] * matA[1][2] - matA[0][2] * matA[1][0]);
                    cof[2][2] =  (matA[0][0] * matA[1][1] - matA[0][1] * matA[1][0]);
                    for (int i = 0; i < 3; i++)
                        for (int j = 0; j < 3; j++)
                            matInv[i][j] = cof[j][i] / detVal;
                    hasInverse = true;
                } else {
                    hasInverse = false;
                }
            }
            ImGui::Text("Inversa:");
            if (hasInverse) DrawMatrixResult3x3("matInv", matInv);
            else ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Matriz não inversível (Det = 0)");
        }

        // ----------------------------------------------------------------
        // Matrix MxN
        // ----------------------------------------------------------------
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
            static Eigen::MatrixXd matMNInv;
            static bool hasInvMN = false;
            static bool invMNComputed = false;

            int prevRows = rows, prevCols = cols;
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Rows##mn", &rows);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Columns##mn", &cols);

            if (rows < 1) rows = 1;
            if (cols < 1) cols = 1;
            if (rows != prevRows || cols != prevCols) {
                matMN.resize(rows);
                for (int i = 0; i < rows; i++) matMN[i].resize(cols, 0.0);
                matMN3.clear(); matMN4.clear();
                multiplicationValid = true;
                hasDetMN = true;
                detComputed = false;
                invMNComputed = false;
            }
            DrawMatrixInputMxN("MN", matMN, rows, cols);

            ImGui::NextColumn();
            static int rows2 = 3, cols2 = 3;
            static std::vector<std::vector<double>> matMN2(rows2, std::vector<double>(cols2, 0.0));

            int prevRows2 = rows2, prevCols2 = cols2;
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Rows##mn2", &rows2);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80); ImGui::InputInt("Columns##mn2", &cols2);

            if (rows2 < 1) rows2 = 1;
            if (cols2 < 1) cols2 = 1;
            if (rows2 != prevRows2 || cols2 != prevCols2) {
                matMN2.resize(rows2);
                for (int i = 0; i < rows2; i++) matMN2[i].resize(cols2, 0.0);
                matMN3.clear();
                multiplicationValid = true;
            }
            DrawMatrixInputMxN("MN2", matMN2, rows2, cols2);

            ImGui::Columns(1);
            ImGui::Separator();

            if (ImGui::Button("Multiplicar##mn", ImVec2(200, 30))) {
                try {
                    matMN3 = multiplyMatrices(matMN, matMN2);
                    multiplicationValid = true;
                } catch (const std::exception&) {
                    multiplicationValid = false;
                }
            }
            if (multiplicationValid)
                DrawMatrixResultMxN("matMN3", matMN3);
            else
                ImGui::TextDisabled("Multiplicação inválida: Colunas da Matriz A devem ser iguais às Linhas da Matriz B.");

            ImGui::Separator();
            if (ImGui::Button("Escalonar##mn", ImVec2(200, 30)))
                matMN4 = escalonar(matMN);
            DrawMatrixResultMxN("matMN4", matMN4);

            ImGui::Separator();
            if (ImGui::Button("Determinante##mn", ImVec2(200, 30))) {
                if (rows == cols) {
                    try {
                        detMN = determinant(matMN);
                        hasDetMN = true;
                        detComputed = true;
                    } catch (const std::exception&) {
                        hasDetMN = false;
                        detComputed = false;
                    }
                } else {
                    hasDetMN = false;
                    detComputed = false;
                }
            }
            if (!hasDetMN)
                ImGui::TextDisabled("Determinante: Apenas para matrizes quadradas.");
            else if (detComputed)
                ImGui::Text("Determinante (Gauss): %.2f", detMN);

            ImGui::Separator();
            if (ImGui::Button("Inversa##mn", ImVec2(200, 30))) {
                if (rows == cols) {
                    Eigen::MatrixXd eigenMat(rows, cols);
                    for (int i = 0; i < rows; ++i)
                        for (int j = 0; j < cols; ++j)
                            eigenMat(i, j) = matMN[i][j];

                    if (std::abs(eigenMat.determinant()) > 1e-9) {
                        matMNInv = eigenMat.inverse();
                        hasInvMN = true;
                    } else {
                        hasInvMN = false;
                    }
                    invMNComputed = true;
                } else {
                    hasInvMN = false;
                    invMNComputed = false;
                }
            }
            if (rows != cols && invMNComputed)
                ImGui::TextDisabled("Inversa: Apenas para matrizes quadradas.");
            else if (invMNComputed) {
                if (hasInvMN) {
                    ImGui::Text("Inversa:");
                    DrawMatrixResultEigen("matMNInv", matMNInv);
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Matriz não inversível (Det = 0)");
                }
            }
        }

        // ----------------------------------------------------------------
        // System of Equations
        // ----------------------------------------------------------------
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

            if (rowsS < 1) rowsS = 1;
            if (colsS < 1) colsS = 1;
            if (rowsS != prevRows || colsS != prevCols) {
                matA.conservativeResize(rowsS, colsS);
                vecB.conservativeResize(rowsS);
                if (rowsS > prevRows) {
                    matA.bottomRows(rowsS - prevRows).setZero();
                    vecB.tail(rowsS - prevRows).setZero();
                }
                if (colsS > prevCols)
                    matA.rightCols(colsS - prevCols).setZero();
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
                    } else {
                        systemStatus = 2;
                        vecX = lu.solve(vecB);
                    }
                } else {
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
                } else if (systemStatus == 2) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "O sistema possui INFINITAS soluções (SPI).");
                    ImGui::Text("Grau de liberdade: %d", colsS - rankA);
                    ImGui::Separator();
                    ImGui::Text("Uma solução possível:");
                    for (int i = 0; i < vecX.size(); i++) {
                        std::string fractionStr = valueToFraction(vecX(i));
                        ImGui::Text("x%d = %s", i + 1, fractionStr.c_str());
                    }
                } else if (systemStatus == 3) {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "O sistema NÃO possui solução (SI).");
                }
            }
        }

        // ----------------------------------------------------------------
        // Vectors (placeholder)
        // ----------------------------------------------------------------
        if (ImGui::CollapsingHeader("Vectors", ImGuiTreeNodeFlags_DefaultOpen))
        {
        }

        PopButtonStyle();
        ImGui::End();
        PopWindowStyle();

        ImGui::Render();
        FrameRender(wd, ImGui::GetDrawData());
        FramePresent(wd);
    }

    // --- Cleanup ---
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
