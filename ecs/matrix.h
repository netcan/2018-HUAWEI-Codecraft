/****************************************************************************
 > File Name: matrix.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-04-23 -- 21:08
 ****************************************************************************/

#pragma once

#include <vector>

using std::vector;

class Matrix {
private:
	size_t row, col;
	vector<vector<double>> mat;
	const static double eps;

public:
	class Row {
		friend Matrix;
	private:
		Matrix &parent;
		size_t row;
		Row(Matrix &parent, int row): parent(parent), row(row) {}
	public:
		double &operator[](size_t col) {
			return parent.mat[row][col];
		}
	};

	Matrix(size_t row, size_t col): row(row), col(col), mat(row, vector<double>(col, 0.0)) {}
	Matrix(const vector<vector<double>> m): row(m.size()), col(m[0].size()), mat(m) {}

	Row operator[](size_t row)  {
		return Row(*this, row);
	}

	friend Matrix cofactor(const Matrix &m, size_t r, size_t c);
	friend Matrix adjoint(const Matrix &m);

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


