/****************************************************************************
 > File Name: random.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-20 -- 11:07
 ****************************************************************************/

#pragma once
#include <random>


class Random {
private:
	std::random_device r;

public:
	std::default_random_engine generator;
	Random(): generator(r()) {}
	Random(unsigned seed): generator(seed) {}
	inline uint32_t Random_Int(uint32_t min, uint32_t max) { // [min, max]
		return std::uniform_int_distribution<uint32_t>{min, max}(generator);
	}
	inline double Random_Real(double min, double max) { // [min, max)
		return std::uniform_real_distribution<double>{min, max}(generator);
	}
	inline double Random_Norm(double mean = 0.0, double std = 1.0) {
		return std::normal_distribution<double>{mean, std}(generator);
	}
};

extern Random Rand;
