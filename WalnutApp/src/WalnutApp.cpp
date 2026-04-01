#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

#include <GLFW/glfw3.h>

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

		ImGui::Begin("Matrix Operations: Multiplication, Determinant by Laplace, Row Echelon Form, Inverse");

		// Matrizes A e B ------------------------------------------------------------
		static float matA[3][3] = { 0.0f };
		static float matB[3][3] = { 0.0f };
		static float matRes[3][3] = { 0.0f };


		ImGui::Columns(2, "Inputs", false);

		ImGui::Text("Matrix A");
		DrawMatrixInput("A", matA);

		ImGui::NextColumn();

		ImGui::Text("Matrix B");
		DrawMatrixInput("B", matB);



		// Multiplication ------------------------------------------------------------
		ImGui::Columns(1);
		ImGui::Separator();

		if (ImGui::Button("Multiplicar", ImVec2(200, 30)))
		{
			// Reset result when the button is clicked again
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					matRes[i][j] = 0;

			// Matrix Multiplication
			for (int i = 0; i < 3; i++) { // final loop
				for (int j = 0; j < 3; j++) { // second loop
					for (int k = 0; k < 3; k++) { // first loop
						matRes[i][j] += matA[i][k] * matB[k][j];
					}
				}
			}
		}

		ImGui::Text("Matriz AxB:");
		DrawMatrixResult("matRes", matRes);
		// -----------------------------------------------


		// Laplace ---------------------------------------
		ImGui::Separator();
		ImGui::Columns(1);

		static float detA = 0.0f;

		if (ImGui::Button("Laplace", ImVec2(200, 30)))
		{
			detA = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
				- matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
				+ matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);
		}

		ImGui::Text("Determinante da Matriz A:");
		ImGui::Text("%.2f", detA);



		// Escalonamento ---------------------------------------
		ImGui::Separator();
		ImGui::Columns(1);

		static float matEsc[3][3] = { 0.0f };

		if (ImGui::Button("Escalonamento", ImVec2(200, 30)))
		{
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					matEsc[i][j] = matA[i][j];

			// Permutação I - Verificar se o pivô [0][0] é zero e trocar com outras linhas
			if (matEsc[0][0] == 0)
			{
				if (matEsc[1][0] != 0) {
					for (int col = 0; col < 3; col++) std::swap(matEsc[0][col], matEsc[1][col]);
				}
				else if (matEsc[2][0] != 0) {
					for (int col = 0; col < 3; col++) std::swap(matEsc[0][col], matEsc[2][col]);
				}
			}

			// Escalonamento I - Zerar elementos abaixo do pivô [0][0]
			if (matEsc[0][0] != 0)
			{
				float f1 = matEsc[1][0] / matEsc[0][0]; // fator = Elem / Pivô
				float f2 = matEsc[2][0] / matEsc[0][0]; 

				for (int col = 0; col < 3; col++)
				{
					matEsc[1][col] -= f1 * matEsc[0][col];
					matEsc[2][col] -= f2 * matEsc[0][col];
				}
			}


			// Permutação II - Verificar se o pivô [1][1] é zero e trocar com outras linhas
			if (matEsc[1][1] == 0)
			{
				if (matEsc[2][1] != 0) {
					for (int col = 0; col < 3; col++) std::swap(matEsc[1][col], matEsc[2][col]);
				}
			}

			// Escalonamento II - Zerar elementos abaixo do pivô [1][1]
			if (matEsc[1][1] != 0)
			{
				float f3 = matEsc[2][1] / matEsc[1][1]; 

				for (int col = 0; col < 3; col++)
				{
					matEsc[2][col] -= f3 * matEsc[1][col];
				}
			}
		}

		ImGui::Text("Escalonamento da Matriz A:");
		DrawMatrixResult("matEsc", matEsc);

		ImGui::Separator();

		// Matriz Inversa = Matriz de Cofatores Transposta * 1/Determinante
		static float matInv[3][3] = { 0.0f };

		if (ImGui::Button("Inversa", ImVec2(200, 30)))
		{
			// Determinante Laplace
			static float detA = 
					+ matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
					- matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
					+ matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);

			if (detA != 0) // Inversível: det != 0 e matriz quadrada [n][n]
			{
				// Matriz de Cofatores
				float cof[3][3];

				// Cofator = (-1)^(i+j) * Det(Submatriz)
				// Linha 0
				cof[0][0] = (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1]);
				cof[0][1] = -(matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0]);
				cof[0][2] = (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);

				// Linha 1
				cof[1][0] = -(matA[0][1] * matA[2][2] - matA[0][2] * matA[2][1]);
				cof[1][1] = (matA[0][0] * matA[2][2] - matA[0][2] * matA[2][0]);
				cof[1][2] = -(matA[0][0] * matA[2][1] - matA[0][1] * matA[2][0]);

				// Linha 2
				cof[2][0] = (matA[0][1] * matA[1][2] - matA[0][2] * matA[1][1]);
				cof[2][1] = -(matA[0][0] * matA[1][2] - matA[0][2] * matA[1][0]);
				cof[2][2] = (matA[0][0] * matA[1][1] - matA[0][1] * matA[1][0]);

				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						matInv[i][j] = cof[j][i] / detA; // cof(A)^t * 1/detA = A^-1
					}
				}
			}
		}

		ImGui::Text("Inversa da Matriz A:");
		DrawMatrixResult("matInv", matInv);

		ImGui::Separator();
	
		// Matriz MxN ------------------------------------------------------------


		PopButtonStyle();

		ImGui::End();

		PopWindowStyle();
	}

private:
	// Buttons
	void PushButtonStyle()
	{
	    ImGui::PushStyleColor(ImGuiCol_Button,        IM_COL32(80, 80, 80, 80));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(120, 120, 120, 120));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  IM_COL32(160, 160, 160, 150));
	    ImGui::PushStyleColor(ImGuiCol_Text,          IM_COL32(220, 220, 220, 255));
	    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
	    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	}
	
	void PopButtonStyle()
	{
	    ImGui::PopStyleVar(2);
	    ImGui::PopStyleColor(4);
	}

	// Window
	void PushWindowStyle()
	{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 220)); // dark background
		ImGui::PushStyleColor(ImGuiCol_TitleBg, IM_COL32(40, 40, 40, 255));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(55, 55, 55, 255));
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, IM_COL32(30, 30, 30, 200));
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(80, 80, 80, 60));
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, IM_COL32(120, 120, 120, 60));
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, IM_COL32(150, 150, 150, 120));
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, IM_COL32(180, 180, 180, 180));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
	}

	void PopWindowStyle()
	{
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(8);
	}

	// Create Matrix 
	void DrawMatrixInput(const char* id, float mat[3][3])
	{
		const float cellWidth = 60.0f;
		const float cellSpacingX = 4.0f;
		const float cellSpacingY = 4.0f;
		const float cellHeight = ImGui::GetFrameHeight();
		const float rounding = 4.0f;
		const ImU32 cellBg = IM_COL32(120, 120, 120, 40);
		const ImU32 cellBgHover = IM_COL32(120, 120, 120, 65);
		const ImU32 cellBgActive = IM_COL32(120, 120, 120, 90);

		float gridW = 3 * cellWidth + 2 * cellSpacingX;
		float gridH = 3 * cellHeight + 2 * cellSpacingY;

		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2      origin = ImGui::GetCursorScreenPos();

		ImGui::Dummy(ImVec2(gridW, gridH));
		ImGui::SetCursorScreenPos(origin);

		// Override ImGui's own input background colors
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);

		ImGui::PushItemWidth(cellWidth);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				float cx = origin.x + j * (cellWidth + cellSpacingX);
				float cy = origin.y + i * (cellHeight + cellSpacingY);

				// Determine which bg shade to use based on hover/active state
				ImVec2 cellMin(cx, cy);
				ImVec2 cellMax(cx + cellWidth, cy + cellHeight);
				ImVec2 mouse = ImGui::GetMousePos();
				bool hovered = mouse.x >= cellMin.x && mouse.x <= cellMax.x &&
					mouse.y >= cellMin.y && mouse.y <= cellMax.y;
				bool active = hovered && ImGui::IsMouseDown(0);

				dl->AddRectFilled(cellMin, cellMax,
					active ? cellBgActive : hovered ? cellBgHover : cellBg,
					rounding);

				ImGui::SetCursorScreenPos(cellMin);
				ImGui::PushID(i * 3 + j + (intptr_t)id);
				ImGui::InputFloat("##cell", &mat[i][j], 0.0f, 0.0f, "%.2f");
				ImGui::PopID();
			}
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);

		ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + gridH));
		ImGui::Dummy(ImVec2(gridW, 0.0f));
	}

	// Matrix Result
	void DrawMatrixResult(const char* id, float mat[3][3])
	{
		const float cellWidth = 60.0f;
		const float cellSpacingX = 4.0f;
		const float cellSpacingY = 4.0f;
		const float cellHeight = ImGui::GetFrameHeight();
		const float rounding = 4.0f;
		const ImU32 cellBg = IM_COL32(120, 120, 120, 25); // slightly dimmer than input

		float gridW = 3 * cellWidth + 2 * cellSpacingX;
		float gridH = 3 * cellHeight + 2 * cellSpacingY;

		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2      origin = ImGui::GetCursorScreenPos();

		ImGui::Dummy(ImVec2(gridW, gridH));
		ImGui::SetCursorScreenPos(origin);

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				float cx = origin.x + j * (cellWidth + cellSpacingX);
				float cy = origin.y + i * (cellHeight + cellSpacingY);

				ImVec2 cellMin(cx, cy);
				ImVec2 cellMax(cx + cellWidth, cy + cellHeight);

				dl->AddRectFilled(cellMin, cellMax, cellBg, rounding);

				// Center the text inside the cell
				char buf[32];
				snprintf(buf, sizeof(buf), "%.2f", mat[i][j]);
				ImVec2 textSize = ImGui::CalcTextSize(buf);
				ImVec2 textPos(
					cx + (cellWidth - textSize.x) * 0.5f,
					cy + (cellHeight - textSize.y) * 0.5f
				);

				dl->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), buf);
			}
		}

		ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + gridH));
		ImGui::Dummy(ImVec2(gridW, 0.0f));
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Matrizes";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}