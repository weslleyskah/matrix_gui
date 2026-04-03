#pragma once

#include <vector>
#include <cmath>
#include <stdexcept>

/* Eliminação de Gauss
Para matrizes quadradas, o determinante é o produto da diagonal principal 
após o escalonamento triangular superior. 
Cada troca de linha inverte o sinal do determinante.
*/
double determinant(std::vector<std::vector<double>> mat) {
	int n = mat.size();

	// Valida se é quadrada
	for (auto& row : mat)
		if ((int)row.size() != n)
			throw std::invalid_argument("Matriz não é quadrada!");

	double det = 1.0;
	int swaps = 0;

	for (int col = 0; col < n; col++) {
		// Pivotamento parcial: acha a linha com maior valor absoluto na coluna
		int pivotRow = col;
		for (int row = col + 1; row < n; row++)
			if (std::abs(mat[row][col]) > std::abs(mat[pivotRow][col]))
				pivotRow = row;

		// Troca de linhas
		if (pivotRow != col) {
			std::swap(mat[col], mat[pivotRow]);
			swaps++;
		}

		// Pivot zero = determinante é 0
		if (mat[col][col] == 0)
			return 0.0;

		det *= mat[col][col];

		// Eliminação
		for (int row = col + 1; row < n; row++) {
			double factor = mat[row][col] / mat[col][col];
			for (int k = col; k < n; k++)
				mat[row][k] -= factor * mat[col][k];
		}
	}

	// Cada troca de linha inverte o sinal
	if (swaps % 2 != 0)
		det = -det;

	return det;
}

// Mutiplication MxN Matrices

std::vector <std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B) {
	bool valid = true;

	int m1 = A.size();    // lines
	int n1 = A[0].size(); // columns

	int m2 = B.size();
	int n2 = B[0].size();

	if (n1 != m2) {
		valid = false;
		throw std::invalid_argument("The number of columns of the first matrix must be equal to the number of lines of the second matrix.");
	}

	std::vector <std::vector<double>> C(m1, std::vector<double>(n2, 0.0));

	for (int linA = 0; linA < m1; linA++) {
		for (int colB = 0; colB < n2; colB++) {
			for (int k = 0; k < n1; k++) {       // k == colA or linB
				C[linA][colB] += A[linA][k] * B[k][colB];
			}
		}
	}

	return C;

}

// Escalonamento de Matriz (Triangularização)

std::vector<std::vector<double>> escalonar(std::vector<std::vector<double>> mat) {

	int n = mat.size();
	int cols = mat[0].size();
	// A última coluna é o vetor de termos independentes

	for (int col = 0; col < cols-1; col++) {

		// Pivotamento parcial: acha a linha com maior valor absoluto na coluna
		int pivotRow = col;
		for (int row = col + 1; row < n; row++)
			if (std::abs(mat[row][col]) > std::abs(mat[pivotRow][col]))
				pivotRow = row;

		// Troca de linhas
		if (pivotRow != col) {
			std::swap(mat[col], mat[pivotRow]);
		}

		// Se o pivot encontrado for zero, a coluna já está escalonada com zero abaixo, então continua para a próxima coluna
		if (mat[col][col] == 0 || std::abs(mat[col][col]) < 1e-10)
			continue;

		// Triangulação: Transformar em zero abaixo do pivot
		for (int row = col + 1; row < n; row++) {
			double factor = mat[row][col] / mat[col][col];
			for (int k = col; k < cols; k++) {
				mat[row][k] -= factor * mat[col][k]; // line2 = line2 - factor * line1
				if (std::abs(mat[row][k]) < 1e-6) {
					mat[row][k] = 0.0;
				}
			}
		}

	}
	return mat;
}
