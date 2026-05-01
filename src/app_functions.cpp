#include "app_functions.h"
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <numeric>

// ============================================================
// Shared buffer map
// ============================================================

std::unordered_map<std::string, std::vector<std::vector<std::string>>> g_BufferMap;

// ============================================================
// Algoritmos
// ============================================================

double determinant(std::vector<std::vector<double>> mat) {
    int n = (int)mat.size();

    for (auto& row : mat)
        if ((int)row.size() != n)
            throw std::invalid_argument("Matriz não é quadrada!");

    double det = 1.0;
    int swaps = 0;

    for (int col = 0; col < n; col++) {
        int pivotRow = col;
        for (int row = col + 1; row < n; row++)
            if (std::abs(mat[row][col]) > std::abs(mat[pivotRow][col]))
                pivotRow = row;

        if (pivotRow != col) {
            std::swap(mat[col], mat[pivotRow]);
            swaps++;
        }

        if (mat[col][col] == 0)
            return 0.0;

        det *= mat[col][col];

        for (int row = col + 1; row < n; row++) {
            double factor = mat[row][col] / mat[col][col];
            for (int k = col; k < n; k++)
                mat[row][k] -= factor * mat[col][k];
        }
    }

    if (swaps % 2 != 0)
        det = -det;

    return det;
}

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B) {
    int m1 = (int)A.size();
    int n1 = (int)A[0].size();
    int m2 = (int)B.size();
    int n2 = (int)B[0].size();

    if (n1 != m2)
        throw std::invalid_argument("The number of columns of the first matrix must be equal to the number of lines of the second matrix.");

    std::vector<std::vector<double>> C(m1, std::vector<double>(n2, 0.0));

    for (int linA = 0; linA < m1; linA++)
        for (int colB = 0; colB < n2; colB++)
            for (int k = 0; k < n1; k++)
                C[linA][colB] += A[linA][k] * B[k][colB];

    return C;
}

std::vector<std::vector<double>> escalonar(std::vector<std::vector<double>> mat) {
    int linhas  = (int)mat.size();
    int colunas = (int)mat[0].size();
    int linhaPivo = 0;

    for (int col = 0; col < colunas && linhaPivo < linhas; col++) {
        int melhorLinha = linhaPivo;
        for (int linha = linhaPivo + 1; linha < linhas; linha++)
            if (std::abs(mat[linha][col]) > std::abs(mat[melhorLinha][col]))
                melhorLinha = linha;

        if (std::abs(mat[melhorLinha][col]) < 1e-10)
            continue;

        if (melhorLinha != linhaPivo)
            std::swap(mat[linhaPivo], mat[melhorLinha]);

        for (int linha = linhaPivo + 1; linha < linhas; linha++) {
            double fator = mat[linha][col] / mat[linhaPivo][col];
            for (int k = col; k < colunas; k++) {
                mat[linha][k] -= fator * mat[linhaPivo][k];
                if (std::abs(mat[linha][k]) < 1e-10)
                    mat[linha][k] = 0.0;
            }
        }
        linhaPivo++;
    }
    return mat;
}

double parseFraction(const std::string& s) {
    size_t slash = s.find('/');
    if (slash != std::string::npos) {
        double num = std::atof(s.substr(0, slash).c_str());
        double den = std::atof(s.substr(slash + 1).c_str());
        return (den != 0) ? num / den : 0.0;
    }
    return std::atof(s.c_str());
}

std::string valueToFraction(double value) {
    const double EPSILON = 1e-7;
    const int MAX_ITERATIONS = 8;

    if (std::abs(value) < 1e-10) return "0";
    if (std::isnan(value))       return "NaN";
    if (std::isinf(value))       return "Inf";

    bool negative = (value < 0);
    value = std::abs(value);

    double x = value;
    long long a = (long long)std::floor(x);

    long long num1 = 1, num2 = a;
    long long den1 = 0, den2 = 1;

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        if (std::abs(x - a) < EPSILON) break;

        x = 1.0 / (x - a);
        a = (long long)std::floor(x + EPSILON);

        long long num = a * num2 + num1;
        long long den = a * den2 + den1;

        if (num > 1000000 || den > 1000000) break;

        num1 = num2; num2 = num;
        den1 = den2; den2 = den;

        if (std::abs(value - (double)num2 / den2) < EPSILON) break;
    }

    long long final_num = num2;
    long long final_den = den2;

    std::string sign = negative ? "-" : "";

    if (final_den == 1)
        return sign + std::to_string(final_num);

    return sign + std::to_string(final_num) + "/" + std::to_string(final_den);
}

// ============================================================
// Style helpers
// ============================================================

void PushButtonStyle() {
    ImGui::PushStyleColor(ImGuiCol_Button,        IM_COL32(80,  80,  80,  80));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(120, 120, 120, 120));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  IM_COL32(160, 160, 160, 150));
    ImGui::PushStyleColor(ImGuiCol_Text,          IM_COL32(220, 220, 220, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
}

void PopButtonStyle() {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
}

void PushWindowStyle() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
}

void PopWindowStyle() {
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

// ============================================================
// Matrix input widgets
// ============================================================

void DrawMatrixInput3x3(const char* id, double mat[3][3]) {
    static int nextFocusRow = -1, nextFocusCol = -1;
    static const char* activeId = nullptr;
    const float cellWidth = 60.0f;

    auto& cellBuffers = g_BufferMap[std::string(id)];
    if ((int)cellBuffers.size() != 3) cellBuffers.assign(3, std::vector<std::string>(3, "0"));
    for (int i = 0; i < 3; i++)
        if ((int)cellBuffers[i].size() != 3) cellBuffers[i].assign(3, "0");

    ImGui::PushStyleColor(ImGuiCol_FrameBg,        IM_COL32(120, 120, 120, 40));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  IM_COL32(120, 120, 120, 90));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4, 4));

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
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)  && j > 0) { nextFocusRow = i;     nextFocusCol = j - 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && j < 2) { nextFocusRow = i;     nextFocusCol = j + 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)    && i > 0) { nextFocusRow = i - 1; nextFocusCol = j;     }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)  && i < 2) { nextFocusRow = i + 1; nextFocusCol = j;     }
            }
            ImGui::PopID();
            if (j < 2) ImGui::SameLine();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
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

    ImGui::PushStyleColor(ImGuiCol_FrameBg,        IM_COL32(120, 120, 120, 40));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  IM_COL32(120, 120, 120, 90));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4, 4));

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
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)  && j > 0)        { nextFocusRow = i;     nextFocusCol = j - 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && j < cols - 1) { nextFocusRow = i;     nextFocusCol = j + 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)    && i > 0)        { nextFocusRow = i - 1; nextFocusCol = j;     }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)  && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = j;     }
            }
            ImGui::PopID();
            if (j < cols - 1) ImGui::SameLine();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
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
    for (int i = 0; i < rows; i++)
        if ((int)cellBuffers[i].size() != cols + 1)
            cellBuffers[i].resize(cols + 1, "0");

    ImGui::PushStyleColor(ImGuiCol_FrameBg,        IM_COL32(120, 120, 120, 40));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  IM_COL32(120, 120, 120, 90));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4, 4));

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
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)  && j > 0)        { nextFocusRow = i;     nextFocusCol = j - 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))                  { nextFocusRow = i;     nextFocusCol = j + 1; }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)    && i > 0)        { nextFocusRow = i - 1; nextFocusCol = j;     }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)  && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = j;     }
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("x%d %s", j + 1, (j < cols - 1) ? "+ " : "= ");
            ImGui::SameLine();
        }

        ImGui::PushID(cols);
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
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))                   { nextFocusRow = i;     nextFocusCol = cols - 1; }
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)    && i > 0)        { nextFocusRow = i - 1; nextFocusCol = cols;     }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)  && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = cols;     }
        }
        ImGui::PopID();
        ImGui::PopID();
    }
    ImGui::PopID();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

// ============================================================
// Matrix result display widgets
// ============================================================

void DrawMatrixResult3x3(const char* id, double mat[3][3]) {
    const float cellWidth  = 100.0f;
    const float cellHeight = ImGui::GetFrameHeight();
    ImDrawList* dl         = ImGui::GetWindowDrawList();
    ImVec2 origin          = ImGui::GetCursorScreenPos();

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
    const float cellWidth  = 100.0f;
    const float cellHeight = ImGui::GetFrameHeight();
    ImDrawList* dl         = ImGui::GetWindowDrawList();
    ImVec2 origin          = ImGui::GetCursorScreenPos();

    for (size_t i = 0; i < mat.size(); i++) {
        for (size_t j = 0; j < mat[0].size(); j++) {
            ImVec2 pos = ImVec2(origin.x + j * 105, origin.y + i * (cellHeight + 4));
            dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
            std::string txt = valueToFraction(mat[i][j]);
            dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
        }
    }
    ImGui::Dummy(ImVec2((float)mat[0].size() * 105, (float)mat.size() * (cellHeight + 4)));
}

void DrawMatrixResultEigen(const char* id, const Eigen::MatrixXd& mat) {
    if (mat.size() == 0) return;
    const float cellWidth  = 100.0f;
    const float cellHeight = ImGui::GetFrameHeight();
    ImDrawList* dl         = ImGui::GetWindowDrawList();
    ImVec2 origin          = ImGui::GetCursorScreenPos();

    for (int i = 0; i < mat.rows(); i++) {
        for (int j = 0; j < mat.cols(); j++) {
            ImVec2 pos = ImVec2(origin.x + j * 105, origin.y + i * (cellHeight + 4));
            dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
            std::string txt = valueToFraction(mat(i, j));
            dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
        }
    }
    ImGui::Dummy(ImVec2((float)mat.cols() * 105, (float)mat.rows() * (cellHeight + 4)));
}
