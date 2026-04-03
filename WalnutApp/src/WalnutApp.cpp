#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include "algoritmos.h"

struct Fraction {
	int num = 0;
	int den = 1;

	void Simplify() {
		if (den == 0) return;
		if (den < 0) { num = -num; den = -den; }
		int common = std::gcd(std::abs(num), std::abs(den));
		num /= common;
		den /= common;
	}

	std::string ToString() const {
		if (den == 1) return std::to_string(num);
		if (num == 0) return "0";
		return std::to_string(num) + "/" + std::to_string(den);
	}
};

class ExampleLayer : public Walnut::Layer
{
public:

	virtual void OnUIRender() override
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);

		PushWindowStyle();
		PushButtonStyle();

		ImGui::Begin("Matrix Operations: Multiplication, Determinant, Row Echelon, Inverse");

		if (ImGui::CollapsingHeader("Matrix 3x3", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static float matA[3][3] = { 0.0f };
			static float matB[3][3] = { 0.0f };
			static float matRes[3][3] = { 0.0f };

			ImGui::Columns(2, "Inputs", false);
			ImGui::Text("Matrix A");
			DrawMatrixInput("A", matA);

			ImGui::NextColumn();
			ImGui::Text("Matrix B");
			DrawMatrixInput("B", matB);

			ImGui::Columns(1);
			ImGui::Separator();

			if (ImGui::Button("Multiplicar", ImVec2(200, 30)))
			{
				for (int i = 0; i < 3; i++)
					for (int j = 0; j < 3; j++)
						matRes[i][j] = 0;

				for (int i = 0; i < 3; i++)
					for (int j = 0; j < 3; j++)
						for (int k = 0; k < 3; k++)
							matRes[i][j] += matA[i][k] * matB[k][j];
			}

			ImGui::Text("Matriz AxB:");
			DrawMatrixResult("matRes", matRes);

			ImGui::Separator();
			static float detA = 0.0f;
			if (ImGui::Button("Laplace", ImVec2(200, 30)))
			{
				detA = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
					- matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
					+ matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);
			}
			ImGui::Text("Determinante da Matriz A: %.2f", detA);

			ImGui::Separator();
			static float matEsc[3][3] = { 0.0f };
			if (ImGui::Button("Escalonamento", ImVec2(200, 30)))
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
							float factor = matEsc[k][i] / matEsc[i][i];
							for (int col = i; col < 3; col++) matEsc[k][col] -= factor * matEsc[i][col];
						}
					}
				}
			}
			ImGui::Text("Escalonamento da Matriz A:");
			DrawMatrixResult("matEsc", matEsc);

			ImGui::Separator();
			static Fraction matInv[3][3];
			static bool hasInverse = false;
			if (ImGui::Button("Inversa", ImVec2(200, 30)))
			{
				float detVal = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
					- matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
					+ matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);

				if (std::abs(detVal) > 0.0001f)
				{
					float cof[3][3];
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
							matInv[i][j].num = (int)std::round(cof[j][i] * 100.0f);
							matInv[i][j].den = (int)std::round(detVal * 100.0f);
							matInv[i][j].Simplify();
						}
					}
					hasInverse = true;
				}
				else hasInverse = false;
			}
			ImGui::Text("Inversa da Matriz A (Frações):");
			if (hasInverse) DrawMatrixResultFrac("matInv", matInv);
			else ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Matriz não inversível (Det = 0)");
		}


		// Matriz MxN 
		if (ImGui::CollapsingHeader("Matrix MxN"))
		{
			ImGui::Columns(2, "MxNInputs", false);

			static int rows = 3, cols = 3;
			static std::vector<std::vector<float>> matMN(rows, std::vector<float>(cols, 0.0f));
			static float detMN = 0.0f;
			static bool hasDetMN = true;
			static bool detComputed = false;

			static bool multiplicationValid = true;
			static std::vector<std::vector<float>> matMN3;

			int prevRows = rows, prevCols = cols;
			ImGui::SetNextItemWidth(80); ImGui::InputInt("Rows##mn", &rows);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80); ImGui::InputInt("Columns##mn", &cols);

			if (rows < 1) rows = 1; if (cols < 1) cols = 1;
			if (rows != prevRows || cols != prevCols) {
				matMN.assign(rows, std::vector<float>(cols, 0.0f));
				matMN3.clear();
				multiplicationValid = true;
				hasDetMN = true;
				detComputed = false;
			}
			
			DrawMatrixInput("MN", matMN, rows, cols);

			// Second matrix MxN

			ImGui::NextColumn();

			static int rows2 = 3, cols2 = 3;
			static std::vector<std::vector<float>> matMN2(rows2, std::vector<float>(cols2, 0.0f));

			int prevRows2 = rows2, prevCols2 = cols2;
			ImGui::SetNextItemWidth(80); ImGui::InputInt("Rows##mn2", &rows2);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80); ImGui::InputInt("Columns##mn2", &cols2);

			if (rows2 < 1) rows2 = 1; if (cols2 < 1) cols2 = 1;
			if (rows2 != prevRows2 || cols2 != prevCols2) {
				matMN2.assign(rows2, std::vector<float>(cols2, 0.0f));
				matMN3.clear();
				multiplicationValid = true;
			}

			DrawMatrixInput("MN2", matMN2, rows2, cols2);

			// Multiplication MxN

			ImGui::Columns(1);
			ImGui::Separator();

			if (ImGui::Button("Multiplicar Matrizes##mn", ImVec2(200, 30)))
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
				DrawMatrixResult("matMN3", matMN3);
			}
			else {
				ImGui::TextDisabled("Multiplicação inválida: Colunas da Matriz A devem ser iguais às Linhas da Matriz B.");
			}

			ImGui::Columns(1);
			ImGui::Separator();

			if (ImGui::Button("Determinante (Eliminação de Gauss)##mn", ImVec2(ImGui::CalcTextSize("Determinante (Eliminação de Gauss)").x + 16, 30)))
			{
				if (rows == cols)
				{
					int n = rows;
					std::vector<std::vector<float>> mat(n, std::vector<float>(n));
					for (int i = 0; i < n; i++)
						for (int j = 0; j < n; j++)
							mat[i][j] = matMN[i][j];

					try {
						detMN = (float)determinant(mat);
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
		}

		PopButtonStyle();
		ImGui::End();
		PopWindowStyle();
	}

private:
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

	void DrawMatrixInput(const char* id, float mat[3][3]) {
		static int nextFocusRow = -1, nextFocusCol = -1;
		static const char* activeId = nullptr;
		const float cellWidth = 60.0f;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120, 120, 120, 40));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120, 120, 120, 90));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				ImGui::PushID(i * 3 + j + (intptr_t)id);
				if (activeId == id && nextFocusRow == i && nextFocusCol == j) {
					ImGui::SetKeyboardFocusHere();
					nextFocusRow = -1; nextFocusCol = -1;
				}
				ImGui::SetNextItemWidth(cellWidth);
				ImGui::InputFloat("##cell", &mat[i][j], 0.0f, 0.0f, "%.2f");
				if (ImGui::IsItemFocused()) {
					activeId = id;
					if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && j > 0) { nextFocusRow = i; nextFocusCol = j - 1; }
					if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && j < 2) { nextFocusRow = i; nextFocusCol = j + 1; }
					if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = j; }
					if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < 2) { nextFocusRow = i + 1; nextFocusCol = j; }
				}
				ImGui::PopID();
				if (j < 2) ImGui::SameLine();
			}
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}

	void DrawMatrixInput(const char* id, std::vector<std::vector<float>>& mat, int rows, int cols) {
		static int nextFocusRow = -1, nextFocusCol = -1;
		static const char* activeId = nullptr;
		const float cellWidth = 60.0f;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120, 120, 120, 40));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120, 120, 120, 65));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120, 120, 120, 90));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				ImGui::PushID(i * cols + j + (intptr_t)id);
				if (activeId == id && nextFocusRow == i && nextFocusCol == j) {
					ImGui::SetKeyboardFocusHere();
					nextFocusRow = -1; nextFocusCol = -1;
				}
				ImGui::SetNextItemWidth(cellWidth);
				ImGui::InputFloat("##cell", &mat[i][j], 0.0f, 0.0f, "%.2f");
				if (ImGui::IsItemFocused()) {
					activeId = id;
					if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && j > 0) { nextFocusRow = i; nextFocusCol = j - 1; }
					if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && j < cols - 1) { nextFocusRow = i; nextFocusCol = j + 1; }
					if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = j; }
					if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = j; }
				}
				ImGui::PopID();
				if (j < cols - 1) ImGui::SameLine();
			}
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}

	void DrawMatrixResult(const char* id, float mat[3][3]) {
		const float cellWidth = 60.0f;
		const float cellHeight = ImGui::GetFrameHeight();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				ImVec2 pos = ImVec2(origin.x + j * 64, origin.y + i * (cellHeight + 4));
				dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
				std::string txt = std::to_string(mat[i][j]).substr(0, 4);
				dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
			}
		}
		ImGui::Dummy(ImVec2(190, 3 * (cellHeight + 4)));
	}

	void DrawMatrixResultFrac(const char* id, Fraction mat[3][3]) {
		const float cellWidth = 60.0f;
		const float cellHeight = ImGui::GetFrameHeight();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				ImVec2 pos = ImVec2(origin.x + j * 64, origin.y + i * (cellHeight + 4));
				dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 60), 4.0f);
				std::string txt = mat[i][j].ToString();
				dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
			}
		}
		ImGui::Dummy(ImVec2(190, 3 * (cellHeight + 4)));
	}

	void DrawMatrixResult(const char* id, const std::vector<std::vector<float>>& mat) {
		if (mat.empty()) return;
		const float cellWidth = 60.0f;
		const float cellHeight = ImGui::GetFrameHeight();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();
		for (int i = 0; i < mat.size(); i++) {
			for (int j = 0; j < mat[0].size(); j++) {
				ImVec2 pos = ImVec2(origin.x + j * 64, origin.y + i * (cellHeight + 4));
				dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
				std::string txt = std::to_string(mat[i][j]).substr(0, 4);
				dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
			}
		}
		ImGui::Dummy(ImVec2(mat[0].size() * 64, mat.size() * (cellHeight + 4)));
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Matrix GUI";
	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	return app;
}
