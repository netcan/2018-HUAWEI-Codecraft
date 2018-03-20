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
	std::default_random_engine generator;

public:
	Random(): generator(r()) {}
	inline uint32_t Random_Int(uint32_t min, uint32_t max) { // [min, max]
		return std::uniform_int_distribution<uint32_t>{min, max}(generator);
	}
};

extern Random Rand;
