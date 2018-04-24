/****************************************************************************
 > File Name: matrix.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-04-23 -- 21:08
 ****************************************************************************/

#include "matrix.h"

const double Matrix::eps = 1e-30;

Matrix Matrix::T() {
	Matrix ret(mat[0].size(), mat.size());
	for(size_t i = 0; i < mat.size(); ++i)
		for(size_t j = 0; j < mat[i].size(); ++j)
			ret.mat[j][i] = mat[i][j];
	return ret;
}

Matrix operator*(Matrix lhs, const Matrix &rhs) {
	Matrix ret(lhs.row, rhs.col);
	assert(lhs.col == rhs.row);
	for(int i = 0; i < lhs.row; ++i)
		for(int j = 0; j < rhs.col; ++j)
			for(int k = 0; k < lhs.col; ++k)
				ret.mat[i][j] += lhs.mat[i][k] * rhs.mat[k][j];

	return ret;
}

Matrix operator*(Matrix lhs, double a) {
	for(size_t i = 0; i < lhs.mat.size(); ++i)
		for(size_t j = 0; j < lhs.mat[i].size(); ++j)
			lhs.mat[i][j] *= a;
	return lhs;
}

Matrix operator/(Matrix lhs, double a) {
	for(size_t i = 0; i < lhs.mat.size(); ++i)
		for(size_t j = 0; j < lhs.mat[i].size(); ++j)
			lhs.mat[i][j] /= a;
	return lhs;
}

Matrix operator-(Matrix lhs, const Matrix &rhs) {
	for(size_t i = 0; i < lhs.mat.size(); ++i)
		for(size_t j = 0; j < lhs.mat[i].size(); ++j)
			lhs.mat[i][j] -= rhs.mat[i][j];
	return lhs;
}


double det(Matrix t) {
	assert(t.col == t.row);
	double D = 0;
	if(t.row == 1) return t.mat[0][0];
	else if(t.row == 2) return t.mat[0][0] * t.mat[1][1] - t.mat[0][1] * t.mat[1][0];
	int sign = 1;

	for(size_t f = 0; f < t.row; ++f) {
		D += sign * t.mat[0][f] * det(cofactor(t, 0, f));
		sign = -sign;
	}

	return D;
}

Matrix cofactor(const Matrix &m, size_t r, size_t c) {
	assert(m.row == m.col);
	Matrix ret(m.row - 1, m.col - 1);
	size_t i = 0, j = 0;
	for(size_t row = 0; row < m.row; ++row) {
		for (size_t col = 0; col < m.col; ++col) {
			if (r != row && c != col) {
				ret.mat[i][j++] = m.mat[row][col];
				if (j == ret.col) {
					j = 0;
					++i;
				}
			}
		}
	}
	return ret;
}

Matrix adjoint(const Matrix &m) {
	Matrix ret(m.row, m.col);
	if(m.row == 1 && m.col == 1) {
		ret.mat[0][0] = 1;
		return ret;
	}

	for(size_t i = 0; i < ret.row; ++i)
		for (size_t j = 0; j < ret.col; ++j)
			ret.mat[i][j] = ((i + j)&1?-1:1) * det(cofactor(m, i, j));

	return ret;
}

Matrix Matrix::I() {
	double D = det(*this);
	Matrix ret(*this);
	assert(row == col);
	if(fabs(D) < eps) {
		printf("Singular matrix, can't find its inverse:\n");
		return ret;
	}

	Matrix adj = std::move(adjoint(*this));

	for(int i = 0; i < row; ++i)
		for(int j = 0; j < row; ++j)
			ret.mat[j][i] = adj.mat[i][j] / D;


	return ret;
}
