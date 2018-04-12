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
#include <cmath>
#include <string>
#include <limits>
#include <cassert>

template <class T1, class T2>
double SSE(const std::vector<T1> &X, const std::vector<T2> &X_real) {
	assert(X.size() == X_real.size());
	double sse = 0.0;
	for(size_t i = 0; i < X.size(); ++i)
		sse += (X[i] - X_real[i]) * (X[i] - X_real[i]);
	return sse;
}

template <class T1, class T2>
double MSE(const std::vector<T1> &X, const std::vector<T2> &X_real) {
	assert(X.size() == X_real.size());
	return SSE(X, X_real) / X.size();
}

template <class T>
double mean(const std::vector<T> &X) {
	double m = 0;
	for(T x: X) m += x;
	m /= X.size();
	return m;
}

template <class T>
std::vector<double> normalize(const std::vector<T> &X, double &x_mean, double &x_diff) {
	x_mean = x_diff = 0;
	T x_min = X[0], x_max = X[0];
	for(size_t i = 0; i < X.size(); ++i) {
		x_mean += X[i];
		x_min = std::min(x_min, X[i]);
		x_max = std::max(x_max, X[i]);
	}
	x_mean /= X.size();
	x_diff = x_max - x_min;
	std::vector<double> X_normalize(X.size());
	for(size_t i = 0; i < X_normalize.size(); ++i)
		X_normalize[i] = (X[i] - x_mean) / x_diff;
	return X_normalize;
}

class linear_regression {
private:
	double a, b;

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
	double predict(T x) {
		return h(x);
	}

	void print_coefficient() {
#ifdef _DEBUG
		printf("a=%f b=%f\n", a, b);
#endif
	}
};

// 多项式回归，和多元回归一样，前面那个是一元回归
class polynomial_regression {
private:
	std::vector<double> w; // 系数
	std::vector<double> x_mean, x_diff;
//	double y_mean, y_diff;
	const double eps = 1e-6;

	template <class T>
	double norm(T x, int wj) {
		if(x_diff[wj] < eps) return 1;
		return (x - x_mean[wj]) / x_diff[wj];
	}

	template <class T>
	double h(T x) {
		T x_pow = 1;
		double ret = 0.0;
		for(size_t d = 0; d < w.size(); ++d) {
			double x_norm = norm(x_pow, d);
			ret += w[d] * x_norm;
			x_pow *= x;
		}
		return ret;
	}

public:
	polynomial_regression(int degree = 2):
			w(degree + 1), x_mean(degree + 1), x_diff(degree + 1) {}

	template <class T> // gradient decent
	void train(const std::vector<T> &X, const std::vector<T> &Y, double learn_rate = 0.01, int iteration_num = 2500) {
		assert(X.size() == Y.size());
		std::vector<double> temp_w(w.size());
//		std::fill(w.begin(), w.end(), 0); // 系数初始化

		// 对x进行归一化，求出x_mean, x_diff
		std::vector<std::vector<T>> X_pow(w.size(), std::vector<T>(X.size()));
		X_pow[0] = std::move(std::vector<T>(X.size(), 1));
		normalize(X_pow[0], x_mean[0], x_diff[0]);
		for(size_t j = 1; j < w.size(); ++j) {
			for(size_t i = 0; i < X.size(); ++i)
				X_pow[j][i] = X_pow[j-1][i] * X[i];
			normalize(X_pow[j], x_mean[j], x_diff[j]);
		}


		int cvg_step = -1;
		int step = 0;
		bool convergence = false;

		// begin iter {
		while((iteration_num == -1 || step < iteration_num) && (! convergence)) {
			++step;
//		while( (!convergence) || ++step < iteration_num) {

//		for(int step = 0; step < iteration_num; ++step) {
			for(size_t j = 0; j < w.size(); ++j) {
				double J_derived = 0.0;
				for(size_t i = 0; i < X.size(); ++i)
					J_derived += (h(X[i]) - Y[i]) * norm(X_pow[j][i], j);
				J_derived /= X.size();
				temp_w[j] = w[j] - learn_rate * J_derived;
			}
			convergence = true;
			for(size_t j = 0; convergence && j < w.size(); ++j)
				convergence = convergence && (fabs(temp_w[j] - w[j]) < eps);
			if(convergence && cvg_step == -1) cvg_step = step;
			std::copy(temp_w.begin(), temp_w.end(), w.begin());
		}
		// } end iter
#ifdef _DEBUG
		printf("step = %d\n", cvg_step);
#endif
	}
	template <class T>
	double predict(T x) {
		return h(x);
	}
	void print_coefficient() {
		for(size_t j = 0; j < w.size(); ++j)
			printf("w%ld = %lf ", j, w[j]);
		puts("");
	}

};

class exponential_smoothing {
private:
	double at, bt;
	const int max_weight = 100000;
	int alpha; // max_weight之1
	template <class T1, class T2>
	double weight_sum(int w, T1 a, T2 b) {
		return (w * a + (max_weight - w) * b) * 1.0 / max_weight;
	}
public:
	exponential_smoothing(double alpha): alpha(alpha * max_weight) {};
	template <class T>
	void train(const std::vector<T> &X) {
		std::vector<double> S1(X.size()), S2(X.size()), F(X.size());

		S1[0] = S2[0] = X[0];
		double tmp_at = 2 * S1[0] - S2[0],
				tmp_bt = alpha * (S1[0] - S2[0]) / (max_weight - alpha);
		F[0] = tmp_at;

		for(size_t t = 1; t < X.size(); ++t) {
			S1[t] = weight_sum(alpha, X[t], S1[t-1]);
			S2[t] = weight_sum(alpha, S1[t], S2[t-1]);
			F[t] = tmp_at + tmp_bt;
			tmp_at = 2 * S1[t] - S2[t];
			tmp_bt = alpha * (S1[t] - S2[t]) / (max_weight - alpha);
		}

		at = tmp_at, bt = tmp_bt;
#ifdef _DEBUG
		double sse = SSE(F, X);
//		printf("alpha = %lf sse=%lf\n", alpha * 1.0 / max_weight, sse);
#endif
	}
	template <class T>
	double predict(T t) {
		return at + t*bt;
	}

};



