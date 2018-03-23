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
	double y_mean, y_diff;

	template <class T>
	double h(T x) {
		return a * x + b;
	}
public:
	template <class T>
	void train(const std::vector<T> &X, const std::vector<T> &Y) {
		assert(X.size() == Y.size());
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
	void train_gradient_decent(const std::vector<T> &X, const std::vector<T> &Y, double learn_rate = 0.01, int iteration_num = 2500) {
		assert(X.size() == Y.size());
		a = b = 0.0;
		y_mean = y_diff = 0.0;

		T y_min = Y[0], y_max = Y[0];
		for(size_t i = 0; i < Y.size(); ++i) {
			y_mean += Y[i];
			y_min = std::min(y_min, Y[i]);
			y_max = std::max(y_max, Y[i]);
		}
		y_mean /= Y.size();
		y_diff = y_max - y_min;
		std::vector<double> Y_normalize(Y.size());
		for(size_t i = 0; i < Y_normalize.size(); ++i)
			Y_normalize[i] = (Y[i] - y_mean) / y_diff;

		for(int i = 0; i < iteration_num; ++i) {
			double J_derived = 0.0;
			for(int j = 0; j < X.size(); ++j)
				J_derived += (h(X[j]) - Y_normalize[j]) * 1;
			J_derived /= X.size();
			double temp_b = b - learn_rate * J_derived;

			J_derived = 0.0;
			for(int j = 0; j < X.size(); ++j)
				J_derived += (h(X[j]) - Y_normalize[j]) * X[j];
			J_derived /= X.size();
			double temp_a = a - learn_rate * J_derived;

			a = temp_a;
			b = temp_b;
		}

	}

	template <class T>
	double predict(T x) {
		return h(x);
	}
	template <class T>
	double predict_gradient_decent(T x) {
		return h(x) * y_diff + y_mean;
	}

	void print_coefficient() {
		printf("a=%f b=%f\n", a, b);
	}
};

extern linear_regression LinearReg;

