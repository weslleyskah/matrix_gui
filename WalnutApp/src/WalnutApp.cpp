#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <numeric>
#include <cmath>
#include <stdexcept>


#include <Eigen>
#include "algoritmos.h"

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
			static double matA[3][3] = { 0.0 };
			static double matB[3][3] = { 0.0 };
			static double matRes[3][3] = { 0.0 };

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
						matRes[i][j] = 0.0;

				for (int i = 0; i < 3; i++)
					for (int j = 0; j < 3; j++)
						for (int k = 0; k < 3; k++)
							matRes[i][j] += matA[i][k] * matB[k][j];
			}

			ImGui::Text("Matriz AxB:");
			DrawMatrixResult("matRes", matRes);

			ImGui::Separator();
			static double detA = 0.0;
			if (ImGui::Button("Laplace", ImVec2(200, 30)))
			{
				detA = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
					- matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
					+ matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);
			}
			ImGui::Text("Determinante da Matriz A: %.2f", detA);

			ImGui::Separator();
			static double matEsc[3][3] = { 0.0 };
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
							double factor = matEsc[k][i] / matEsc[i][i];
							for (int col = i; col < 3; col++) matEsc[k][col] -= factor * matEsc[i][col];
						}
					}
				}
			}
			ImGui::Text("Escalonamento da Matriz A:");
			DrawMatrixResult("matEsc", matEsc);

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
			ImGui::Text("Inversa da Matriz A (Frações):");
			if (hasInverse) DrawMatrixResult("matInv", matInv);
			else ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Matriz não inversível (Det = 0)");
		}


		// Matriz MxN 
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
				matMN3.clear();
				multiplicationValid = true;
				hasDetMN = true;
				detComputed = false;
			}
			
			DrawMatrixInput("MN", matMN, rows, cols);

			// Second Matrix MxN

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

			// Escalonamento MxN
			ImGui::Columns(1);
			ImGui::Separator();

			if (ImGui::Button("Escalonar##mn", ImVec2(ImGui::CalcTextSize("Escalonar").x + 16, 30)))
			{
				matMN4 = escalonar(matMN);
			}
			DrawMatrixResult("matMN4", matMN4);

			// Determinante MxN
			ImGui::Columns(1);
			ImGui::Separator();

			if (ImGui::Button("Determinante (Eliminação de Gauss)##mn", ImVec2(ImGui::CalcTextSize("Determinante (Eliminação de Gauss)").x + 16, 30)))
			{
				if (rows == cols)
				{
					int n = rows;
					std::vector<std::vector<double>> mat(n, std::vector<double>(n));
					for (int i = 0; i < n; i++)
						for (int j = 0; j < n; j++)
							mat[i][j] = matMN[i][j];

					try {
						detMN = determinant(mat);
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


			// Inversa MxN (Eigen)
			ImGui::Columns(1);
			ImGui::Separator();

			static Eigen::MatrixXd matMNInv;
			static bool hasInvMN = false;
			static bool invMNComputed = false;

			if (ImGui::Button("Inversa (Eigen)##mn", ImVec2(ImGui::CalcTextSize("Inversa (Eigen)").x + 16, 30)))
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
					ImGui::Text("Inversa (Eigen):");
					DrawMatrixResult("matMNInv", matMNInv);
				}
				else
					ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Matriz não inversível (Det = 0)");
			}
		}



		// Sistemas Lineares
		if (ImGui::CollapsingHeader("System of Equations", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static int rowsS = 3, colsS = 3;
			static Eigen::MatrixXd matA(3, 3);
			static Eigen::VectorXd vecB(3);
			static Eigen::VectorXd vecX;
			static bool solved = false;
			static int rankA = 0;
			static int rankAug = 0;
			static int systemStatus = 0; // 0: None, 1: Única, 2: Infinitas, 3: Nenhuma

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

				// Os ranks são calculados depois de escalonar a matriz A e a matriz ampliada
				// por causa da função FullPivLU que é baseada em pivoteamento completo e pode 
				// alterar a ordem das linhas, o que afeta o cálculo do rank.

				// Matriz de coeficientes A
				Eigen::FullPivLU<Eigen::MatrixXd> lu(matA);
				rankA = (int)lu.rank();
				
				// Matriz ampliada [A|b]
				Eigen::MatrixXd matAug(rowsS, colsS + 1);
				matAug << matA, vecB;

				// Resultado da decomposição LU para a matriz ampliada
				Eigen::FullPivLU<Eigen::MatrixXd> luAug(matAug);
				rankAug = (int)luAug.rank();

				if (rankA == rankAug) {

					// rankA = rankAug = n (vars)  -> Única solução
					if (rankA == colsS) {
						systemStatus = 1; 
						vecX = lu.solve(vecB);
					}
					else {  
						// rankA = rankAug < n (vars)  -> Infinitas soluções
						systemStatus = 2; 
						vecX = lu.solve(vecB); // Uma das soluções possíveis
					}
				}
				else {
					// rankA < rankAug  -> Sem solução
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
						ImGui::Text("x%d = %.2f", i + 1, vecX(i));
					}
				}
				else if (systemStatus == 2) {
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "O sistema possui INFINITAS soluções (SPI).");
					ImGui::Text("Grau de liberdade: %d", colsS - rankA);
					ImGui::Separator();
					ImGui::Text("Uma solução possível:");
					for (int i = 0; i < vecX.size(); i++) {
						ImGui::Text("x%d = %.2f", i + 1, vecX(i));
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

	void DrawMatrixInput(const char* id, double mat[3][3]) {
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
				ImGui::InputDouble("##cell", &mat[i][j], 0.0, 0.0, "%.2f");
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

	void DrawMatrixInput(const char* id, std::vector<std::vector<double>>& mat, int rows, int cols) {
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
				ImGui::InputDouble("##cell", &mat[i][j], 0.0, 0.0, "%.2f");
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

	void DrawSystemInput(const char* id, Eigen::MatrixXd& A, Eigen::VectorXd& b) {
		int rows = (int)A.rows();
		int cols = (int)A.cols();
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
				ImGui::PushID(i * (cols + 1) + j + (intptr_t)id);
				if (activeId == id && nextFocusRow == i && nextFocusCol == j) {
					ImGui::SetKeyboardFocusHere();
					nextFocusRow = -1; nextFocusCol = -1;
				}
				
				// Usamos buffer de texto para aceitar "1/2"
				char buf[32];
				if (std::abs(A(i, j) - std::round(A(i, j))) < 1e-9)
					sprintf(buf, "%d", (int)std::round(A(i, j)));
				else
					sprintf(buf, "%.2f", A(i, j));

				ImGui::SetNextItemWidth(cellWidth);
				if (ImGui::InputText("##cellA", buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank)) {
					A(i, j) = parseFraction(buf);
				}

				if (ImGui::IsItemFocused()) {
					activeId = id;
					if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && j > 0) { nextFocusRow = i; nextFocusCol = j - 1; }
					if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) { nextFocusRow = i; nextFocusCol = j + 1; }
					if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = j; }
					if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = j; }
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::Text("x%d %s", j + 1, (j < cols - 1) ? "+ " : "= ");
				ImGui::SameLine();
			}

			// Vetor b
			ImGui::PushID(i * (cols + 1) + cols + (intptr_t)id);
			if (activeId == id && nextFocusRow == i && nextFocusCol == cols) {
				ImGui::SetKeyboardFocusHere();
				nextFocusRow = -1; nextFocusCol = -1;
			}
			
			char bufB[32];
			if (std::abs(b(i) - std::round(b(i))) < 1e-9)
				sprintf(bufB, "%d", (int)std::round(b(i)));
			else
				sprintf(bufB, "%.2f", b(i));

			ImGui::SetNextItemWidth(cellWidth);
			if (ImGui::InputText("##cellB", bufB, sizeof(bufB), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank)) {
				b(i) = parseFraction(bufB);
			}

			if (ImGui::IsItemFocused()) {
				activeId = id;
				if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) { nextFocusRow = i; nextFocusCol = cols - 1; }
				if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) { nextFocusRow = i - 1; nextFocusCol = cols; }
				if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i < rows - 1) { nextFocusRow = i + 1; nextFocusCol = cols; }
			}
			ImGui::PopID();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}


	void DrawMatrixResult(const char* id, double mat[3][3]) {
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

	void DrawMatrixResult(const char* id, const std::vector<std::vector<double>>& mat) {
		if (mat.empty()) return;
		const float cellWidth = 100.0f;
		const float cellHeight = ImGui::GetFrameHeight();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();
		for (int i = 0; i < mat.size(); i++) {
			for (int j = 0; j < mat[0].size(); j++) {
				ImVec2 pos = ImVec2(origin.x + j * 105, origin.y + i * (cellHeight + 4));
				dl->AddRectFilled(pos, ImVec2(pos.x + cellWidth, pos.y + cellHeight), IM_COL32(100, 100, 100, 40), 4.0f);
				std::string txt = valueToFraction(mat[i][j]);
				dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
			}
		}
		ImGui::Dummy(ImVec2(mat[0].size() * 105, mat.size() * (cellHeight + 4)));
	}

	void DrawMatrixResult(const char* id, const Eigen::MatrixXd& mat) {
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
				// std::string txt = std::to_string(mat(i, j)).substr(0, 4);
				dl->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(255, 255, 255, 255), txt.c_str());
			}
		}
		ImGui::Dummy(ImVec2(mat.cols() * 105, mat.rows() * (cellHeight + 4)));
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
