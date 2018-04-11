/****************************************************************************
 > File Name: server.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:53
 ****************************************************************************/

#pragma once
#include "flavor.h"


struct flavor_info;
struct server {
	static int cpu_count, mem_size, disk_size;
	int remain_cpu_count, remain_mem_size, remain_disk_size;

	server(int remain_cpu_count = cpu_count, int remain_mem_size = mem_size, int remain_disk_size = disk_size);
	friend bool operator<=(const flavor_info & f, const server& srv);
	server &operator-=(const flavor_info & f);
	server &operator/=(const flavor_info & f);
	int operator/(const flavor_info & f);
	double get_cpu_usage_ratio() const {
		return 1.0 - remain_cpu_count * 1.0 / cpu_count;
	}
	double get_mem_usage_ratio() const {
		return 1.0 - remain_mem_size * 1.0 / mem_size;
	}
	double get_usage_ratio(bool cpu = true) const {
		return cpu?get_cpu_usage_ratio():get_mem_usage_ratio();
	}
};

