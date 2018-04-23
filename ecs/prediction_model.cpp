/****************************************************************************
 > File Name: linear_regression.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-22 -- 15:04
 ****************************************************************************/

#include "prediction_model.h"


BP_Network::BP_Network(std::initializer_list<int> sz): sizes(sz), num_layers(sizes.size()) {
	for(size_t i = 1; i < sizes.size(); ++i) {
		vector<double> bias(sizes[i]);
		for(size_t j = 0; j < sizes[i]; ++j)
			bias[j] = Rand.Random_Norm();
		biases.push_back(bias);
	}

	for(size_t i = 0; i < sizes.size() - 1; ++i) { // x
		size_t j = i + 1; // y
		vector<vector<double>> weight(sizes[j], vector<double>(sizes[i], 0));
		for (size_t y = 0; y < sizes[j]; ++y)
			for (size_t x = 0; x < sizes[i]; ++x)
				weight[y][x] = Rand.Random_Norm();
		weights.push_back(weight);
	}
}

vector<double> BP_Network::dot(const vector<vector<double>>& w, const vector<double>& a) const {
	assert(w[0].size() == a.size());
	vector<double> ret(w.size(), 0.0);
	for(size_t j = 0; j < w.size(); ++j)
		for(size_t i = 0; i < a.size(); ++i)
			ret[j] += w[j][i] * a[i];
	return ret;
}

vector<vector<double>> BP_Network::dot(const vector<double>& a, const vector<double>& b) const { // a * b^T，a,b列向量
	vector<vector<double>> ret(a.size(), vector<double>(b.size(), 0));
	for(size_t i = 0; i < a.size(); ++i)
		for(size_t j = 0; j < b.size(); ++j)
			ret[i][j] = a[i] * b[j];
	return ret;
}

vector<double> BP_Network::add(const vector<double>& _dot, const vector<double>& b) const {
	assert(_dot.size() == b.size());
	vector<double> ret(_dot.size(), 0.0);
	for(size_t i = 0; i < _dot.size(); ++i)
		ret[i] = _dot[i] + b[i];
	return ret;
}

vector<double> BP_Network::mul(const vector<double>& a, const vector<double>& b) const {
	assert(a.size() == b.size());
	vector<double> ret(a.size(), 0.0);
	for(size_t i = 0; i < a.size(); ++i)
		ret[i] = a[i] * b[i];
	return ret;
}

vector<vector<double>> BP_Network::transpose(const vector<vector<double>> &t) const {
	vector<vector<double>> ret(t[0].size(), vector<double>(t.size(), 0));
	for(size_t i = 0; i < t.size(); ++i)
		for(size_t j = 0; j < t[i].size(); ++j)
			ret[j][i] = t[i][j];
	return ret;
}

vector<double> BP_Network::sigmoid(const vector<double> &v) const {
	vector<double> ret(v.size(), 0.0);
	for(size_t i = 0; i < v.size(); ++i)
		ret[i] = sigmoid(v[i]);
	return ret;
}

vector<double> BP_Network::sigmoid_prime(const vector<double> &v) const {
	vector<double> ret(v.size(), 0.0);
	for(size_t i = 0; i < v.size(); ++i)
		ret[i] = sigmoid_prime(v[i]);
	return ret;
}

vector<double> BP_Network::feedforward(const vector<double>& a) const {
	assert(biases.size() == weights.size());
	vector<double> ret = a;
	for(size_t i = 0; i < biases.size() - 1; ++i)
		ret = sigmoid(add(dot(weights[i], ret), biases[i]));
	ret = add(dot(weights.back(), ret), biases.back());
	return ret;
}

void BP_Network::update_mini_batch(vector<pair<vector<double>, vector<double>>>& mini_batch, double eta) {
	vector<vector<double>> nabla_b;
	vector<vector<vector<double>>> nabla_w;

	for(size_t i = 0; i < biases.size(); ++i) {
		vector<double> b(biases[i].size(), 0.0);
		nabla_b.push_back(b);
	}
	for(size_t i = 0; i < weights.size(); ++i) {
		vector<vector<double>> weight(weights[i].size(), vector<double>(weights[i][0].size(), 0.0));
		nabla_w.push_back(weight);
	}

	for(const auto & xy: mini_batch) {
		const vector<double> &x = xy.first,
							&y = xy.second;
		auto delta_bw = backprop(x, y);
		const vector<vector<double>> & delta_nabla_b = delta_bw.first;
		const vector<vector<vector<double>>> & delta_nabla_w = delta_bw.second;

		assert(nabla_b.size() == delta_nabla_b.size());
		assert(nabla_b[0].size() == delta_nabla_b[0].size());

		for(size_t i = 0; i < nabla_b.size(); ++i)
			for(size_t j = 0; j < nabla_b[i].size(); ++j)
				nabla_b[i][j] += delta_nabla_b[i][j];

		assert(nabla_w.size() == delta_nabla_w.size());
		assert(nabla_w[0].size() == delta_nabla_w[0].size());
		assert(nabla_w[0][0].size() == delta_nabla_w[0][0].size());

		for(size_t i = 0; i < nabla_w.size(); ++i)
			for(size_t j = 0; j < nabla_w[i].size(); ++j)
				for(size_t k = 0; k < nabla_w[i][j].size(); ++k)
					nabla_w[i][j][k] += delta_nabla_w[i][j][k];
	}
	// 梯度下降，更新偏导值
	assert(nabla_b.size() == biases.size());
	assert(nabla_b[0].size() == biases[0].size());
	for(size_t i = 0; i < nabla_b.size(); ++i)
		for(size_t j = 0; j < nabla_b[i].size(); ++j)
			biases[i][j] -= eta * nabla_b[i][j] / mini_batch.size();

	assert(nabla_w.size() == weights.size());
	assert(nabla_w[0].size() == weights[0].size());
	assert(nabla_w[0][0].size() == weights[0][0].size());
	for(size_t i = 0; i < nabla_w.size(); ++i)
		for(size_t j = 0; j < nabla_w[i].size(); ++j)
			for(size_t k = 0; k < nabla_w[i][j].size(); ++k)
				weights[i][j][k] -= eta * nabla_w[i][j][k] / mini_batch.size();


}

pair<vector<vector<double>>, vector<vector<vector<double>>>> BP_Network::backprop(const vector<double> &x, const vector<double> &y) {
	vector<vector<double>> nabla_b;
	vector<vector<vector<double>>> nabla_w;

	for(size_t i = 0; i < biases.size(); ++i) {
		vector<double> b(biases[i].size(), 0.0);
		nabla_b.push_back(b);
	}
	for(size_t i = 0; i < weights.size(); ++i) {
		vector<vector<double>> weight(weights[i].size(), vector<double>(weights[i][0].size(), 0.0));
		nabla_w.push_back(weight);
	}

	vector<double> activation = x;
	vector<vector<double>> activations, zs;
	activations.push_back(x);

	assert(biases.size() == weights.size());
	for(size_t i = 0; i < biases.size(); ++i) {
		vector<double> z = add(dot(weights[i], activation), biases[i]);
		zs.push_back(z);
		activation = sigmoid(z);
		activations.push_back(activation);
	}
	vector<double> delta = mul(cost_derivative(activations.back(), y),
	                           sigmoid_prime(zs.back()));
	nabla_b.back() = delta;
	nabla_w.back() = dot(delta, activations[activations.size() - 2]);
	for(size_t l = 2; l < num_layers; ++l) {
		const vector<double> &z = zs[zs.size() - l];
		vector<double> sp = sigmoid_prime(z);
		delta = mul(dot(transpose(weights[weights.size() - l + 1]), delta), sp);
		nabla_b[nabla_b.size() - l] = delta;
		nabla_w[nabla_w.size() - l] = dot(delta, activations[activations.size() -l - 1]);
	}
	return std::make_pair(nabla_b, nabla_w);
}


vector<double> BP_Network::cost_derivative(const vector<double> &output_activations, const vector<double> &y) const {
	assert(output_activations.size() == y.size());
	vector<double> ret(y.size(), 0);
	for(size_t i = 0; i < y.size(); ++i)
		ret[i] = output_activations[i] - y[i];
	return ret;
}

void BP_Network::SGD(vector<pair<vector<double>, vector<double>>>& train_data, int epochs, int mini_batch_size, double eta) {
	mini_batch_size = std::min<int>(mini_batch_size, train_data.size());
	for(int epoch = 0; epoch < epochs; ++epoch) {
		// 打乱数据
		if(mini_batch_size < train_data.size())
			std::shuffle(train_data.begin(), train_data.end(), Rand.generator);

		for(size_t bs = 0; bs  < train_data.size(); bs += mini_batch_size) {
			vector<pair<vector<double>, vector<double>>> mini_batchs;
			for(size_t i = 0; i < mini_batch_size && i + bs < train_data.size(); ++i)
				mini_batchs.push_back(train_data[i + bs]);
			update_mini_batch(mini_batchs, eta);
		}
	}

}
