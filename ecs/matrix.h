/****************************************************************************
 > File Name: matrix.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-04-23 -- 21:08
 ****************************************************************************/

#pragma once

#include <vector>
#include <cassert>
#include <cmath>
#include <cstdio>

using std::vector;

class Matrix {
private:
	size_t row, col;
	vector<vector<double>> mat;
	const static double eps;

public:

	Matrix(size_t row, size_t col): row(row), col(col), mat(row, vector<double>(col, 0.0)) {}
	Matrix(const vector<vector<double>> m): row(m.size()), col(m[0].size()), mat(m) {}


	friend Matrix cofactor(const Matrix &m, size_t r, size_t c);
	friend Matrix adjoint(const Matrix &m);
	friend class LWLR;

	friend double det(Matrix t);

	void show() const {
		for(int i = 0; i < row; ++i) {
			for(int j = 0; j < col; ++j)
				printf("%6.3f ", mat[i][j]);
			puts("");
		}
	}

	Matrix T();
	Matrix I();
	friend Matrix operator*(Matrix lhs, const Matrix &rhs);
	friend Matrix operator-(Matrix lhs, const Matrix &rhs);
	friend Matrix operator*(Matrix lhs, double a);
	friend Matrix operator/(Matrix lhs, double a);

};


