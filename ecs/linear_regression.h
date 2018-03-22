/****************************************************************************
 > File Name: linear_regression.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-22 -- 15:04
 ****************************************************************************/

#pragma once

#include <vector>
#include <cstdio>
#include <cassert>

class linear_regression {
private:
	double a, b;
public:
	template <class T>
	void train(const std::vector<T> &X, const std::vector<T> &Y) {
		double x_mean = 0.0, y_mean = 0.0, x2_mean = 0.0,
				xy_mean = 0.0;
		for(int i = 0; i < X.size(); ++i) {
			T x = X[i], y = Y[i];
			x_mean += x;
			x2_mean += x * x;
			y_mean += y;
			xy_mean += x * y;
		}
		x2_mean /= X.size();
		x_mean /= X.size();
		y_mean /= X.size();
		xy_mean /= X.size();
		a = (x_mean * y_mean - xy_mean) / (x_mean * x_mean - x2_mean);
		b = y_mean - a * x_mean;
	}
	template <class T>
	double predict(T x) {
		assert(a>=0);
		return a * x + b;
	}
	void print_coefficient() {
		printf("a=%f b=%f\n", a, b);
	}
};

extern linear_regression LinearReg;

