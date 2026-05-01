#pragma once

#include "imgui.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <Eigen/Dense>

// --- Shared buffer map ---
extern std::unordered_map<std::string, std::vector<std::vector<std::string>>> g_BufferMap;

// --- Algoritmos ---
double determinant(std::vector<std::vector<double>> mat);
std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B);
std::vector<std::vector<double>> escalonar(std::vector<std::vector<double>> mat);
double parseFraction(const std::string& s);
std::string valueToFraction(double value);

// --- Style helpers ---
void PushButtonStyle();
void PopButtonStyle();
void PushWindowStyle();
void PopWindowStyle();

// --- Matrix input widgets ---
void DrawMatrixInput3x3(const char* id, double mat[3][3]);
void DrawMatrixInputMxN(const char* id, std::vector<std::vector<double>>& mat, int rows, int cols);
void DrawSystemInput(const char* id, Eigen::MatrixXd& A, Eigen::VectorXd& b);

// --- Matrix result display widgets ---
void DrawMatrixResult3x3(const char* id, double mat[3][3]);
void DrawMatrixResultMxN(const char* id, const std::vector<std::vector<double>>& mat);
void DrawMatrixResultEigen(const char* id, const Eigen::MatrixXd& mat);
