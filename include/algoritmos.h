#pragma once

#include <vector>
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <string>

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

// Escalonamento de Matriz MxN (Triangularização)

std::vector<std::vector<double>> escalonar(std::vector<std::vector<double>> mat) {
	int linhas = mat.size();
	int colunas = mat[0].size();

	int linhaPivo = 0; // Rastreia a próxima linha que receberá um pivô

	for (int col = 0; col < colunas && linhaPivo < linhas; col++) {

		// Pivotamento parcial: encontra a linha com maior valor absoluto na coluna atual
		int melhorLinha = linhaPivo;
		for (int linha = linhaPivo + 1; linha < linhas; linha++)
			if (std::abs(mat[linha][col]) > std::abs(mat[melhorLinha][col]))
				melhorLinha = linha;

		// Se o melhor pivô encontrado for zero, não há pivô nessa coluna, passa para a próxima
		if (std::abs(mat[melhorLinha][col]) < 1e-10)
			continue;

		// Troca a linha do pivô para a posição correta
		if (melhorLinha != linhaPivo)
			std::swap(mat[linhaPivo], mat[melhorLinha]);

		// Triangulação: zera todos os elementos abaixo do pivô
		for (int linha = linhaPivo + 1; linha < linhas; linha++) {
			double fator = mat[linha][col] / mat[linhaPivo][col];
			for (int k = col; k < colunas; k++) {
				mat[linha][k] -= fator * mat[linhaPivo][k];
				// Elimina erros de ponto flutuante próximos de zero
				if (std::abs(mat[linha][k]) < 1e-10)
					mat[linha][k] = 0.0;
			}
		}

		linhaPivo++; // Avança para a próxima linha somente quando um pivô foi encontrado
	}
	return mat;
}

// Fractions 

// Parses a string that may represent a fraction and returns its double value.
double parseFraction(const std::string& s) {
	size_t slash = s.find('/');
	if (slash != std::string::npos) {
		double num = std::atof(s.substr(0, slash).c_str());
		double den = std::atof(s.substr(slash + 1).c_str());
		return (den != 0) ? num / den : 0.0;
	}
	return std::atof(s.c_str());
}

// Continued Fraction: converts a decimal to a fraction string
// https://pi.math.cornell.edu/~gautam/ContinuedFractions.pdf
/*
Example:
Let x = 2.875
Its integral part is 2 and so the continued fraction starts as [2, ...].
2.875 − 2 = 0.875
Calculate 1/0.875 using a calculator to get 1.14285714285714. Its integral part is 1.
So we now have [2, 1, ...].
1.14285714285714−1 = 0.14285714285714. Calculate 1/0.14285714285714 to get 7.00000000000014
whose integral part is 7.
The continued fraction is now [2, 1, 7, ...].
7.00000000000014 − 7 = 0.00000000000014 which is “almost” 0.
So, we terminate the algorithm here to get [2, 1, 7].
2.875 = 2 + 1/(1 + 1/7) = 2 + 1/(8/7) = 2 + 7/8 = (16/8) + (7/8) = 23/8
*/
std::string valueToFraction(double value) {

	const double EPSILON = 1e-7;
	const int MAX_ITERATIONS = 8;

	if (std::abs(value) < 1e-10) return "0";
	if (std::isnan(value))         return "NaN";
	if (std::isinf(value))         return "Inf";

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