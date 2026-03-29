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

		ImGui::Begin("Matrix Operations: Multiplication, Determinant by Laplace, Row Echelon Form");

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
					for (int k = 0; k < 3; k++) { // first loop when K finishes it, matRes pass to the next cell
						matRes[i][j] += matA[i][k] * matB[k][j];
						/* 
						First Row:

						matRes[0][0] = 0
						matRes[0][0] += matA[0][0] * matB[0][0]
						matRes[0][0] += matA[0][1] * matB[1][0]
						matRes[0][0] += matA[0][2] * matB[2][0]

						matRes[0][1] = 0
						matRes[0][1] += matA[0][0] * matB[0][1]
						matRes[0][1] += matA[0][1] * matB[1][1]
						matRes[0][1] += matA[0][2] * matB[2][1]

						matRes[0][2] = 0
						matRes[0][1] += matA[0][0] * matB[0][2]
						matRes[0][1] += matA[0][1] * matB[1][2]
						matRes[0][1] += matA[0][2] * matB[2][2]

						*/
					}
				}
			}
		}

		// Matrix Result
		ImGui::Text("Matrix A*B:");
		if (ImGui::BeginTable("ResultTable", 3))
		{
			for (int i = 0; i < 3; i++) {
				ImGui::TableNextRow();
				for (int j = 0; j < 3; j++) {
					ImGui::TableSetColumnIndex(j);
					ImGui::Text("%.2f", matRes[i][j]);
				}
			}
			ImGui::EndTable();
		}
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
			// Copia original
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					matEsc[i][j] = matA[i][j];

			// Permutação I - Verificar se o pivô [0][0] é zero e trocar
			if (matEsc[0][0] == 0)
			{
				if (matEsc[1][0] != 0) {
					for (int col = 0; col < 3; col++) std::swap(matEsc[0][col], matEsc[1][col]);
				}
				else if (matEsc[2][0] != 0) {
					for (int col = 0; col < 3; col++) std::swap(matEsc[0][col], matEsc[2][col]);
				}
			}

			// Escalonamento I - Zerar abaixo do pivô [0][0]
			if (matEsc[0][0] != 0)
			{
				float f1 = matEsc[1][0] / matEsc[0][0]; // fator = Elem / Pivo
				float f2 = matEsc[2][0] / matEsc[0][0]; 

				for (int col = 0; col < 3; col++)
				{
					matEsc[1][col] -= f1 * matEsc[0][col];
					matEsc[2][col] -= f2 * matEsc[0][col];
				}
			}


			// Permutação II - Pivô [1][1] (Só precisa olhar para a linha de baixo)
			if (matEsc[1][1] == 0)
			{
				if (matEsc[2][1] != 0) {
					for (int col = 0; col < 3; col++) std::swap(matEsc[1][col], matEsc[2][col]);
				}
			}

			// Escalonamento II - Zerar abaixo do pivô [1][1]
			if (matEsc[1][1] != 0)
			{
				float f3 = matEsc[2][1] / matEsc[1][1]; // Elem / Pivô

				for (int col = 0; col < 3; col++)
				{
					matEsc[2][col] -= f3 * matEsc[1][col];
				}
			}
		}

		ImGui::Text("Escalonamento da Matriz A:");
		if (ImGui::BeginTable("ResultTable2", 3))
		{
			for (int i = 0; i < 3; i++) {
				ImGui::TableNextRow();
				for (int j = 0; j < 3; j++) {
					ImGui::TableSetColumnIndex(j);
					ImGui::Text("%.2f", matEsc[i][j]);
				}
			}
			ImGui::EndTable();
		}



		// Inversa
		// Matriz de Cofatores Transposta * 1/Determinante = Matriz Inversa
		static float matInv[3][3] = { 0.0f };

		if (ImGui::Button("Inversa", ImVec2(200, 30)))
		{
			static float detA = matA[0][0] * (matA[1][1] * matA[2][2] - matA[1][2] * matA[2][1])
					- matA[0][1] * (matA[1][0] * matA[2][2] - matA[1][2] * matA[2][0])
					+ matA[0][2] * (matA[1][0] * matA[2][1] - matA[1][1] * matA[2][0]);

			if (detA != 0) // Só calcula se for inversível
			{
				float cof[3][3]; // Matriz de Cofatores temporária

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

				// 2. Transpor e Dividir ao mesmo tempo para preencher a matInv
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						// Transposta e divisão: cof[j][i] / detA
						matInv[i][j] = cof[j][i] / detA;
					}
				}
			}
		}

		ImGui::Text("Inversa da Matriz A:");
		if (ImGui::BeginTable("ResultTable3", 3))
		{
			for (int i = 0; i < 3; i++) {
				ImGui::TableNextRow();
				for (int j = 0; j < 3; j++) {
					ImGui::TableSetColumnIndex(j);
					ImGui::Text("%.2f", matInv[i][j]);
				}
			}
			ImGui::EndTable();
		}


		ImGui::End();
	}

private:
	// Create a 3x3 matrix input
	void DrawMatrixInput(const char* id, float mat[3][3])
	{
		ImGui::PushItemWidth(100.0f); // Size of the Cells
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				ImGui::PushID(i * 3 + j + (intptr_t)id); // Unique ID for each cell
				ImGui::InputFloat("##cell", &mat[i][j], 0.0f, 0.0f, "%.1f");
				if (j < 2) ImGui::SameLine();
				ImGui::PopID();
			}
		}
		ImGui::PopItemWidth();
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