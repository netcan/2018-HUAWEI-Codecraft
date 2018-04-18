/****************************************************************************
 > File Name: server.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:53
 ****************************************************************************/

#pragma once
#include "flavor.h"

struct base_info;
struct flavor_info;
int read_servers_info(char * info[MAX_INFO_NUM]);

struct server_info : base_info {

};
extern std::vector<server_info> servers_info; // name -> info

struct server {
	const server_info * info;
	int remain_cpu_count, remain_mem_size, remain_disk_size;
	std::map<string, int> flavors;

	server(const server_info * info = &servers_info[0]);
	void place_flavor(const flavor_info &flv, int flv_num = 1);

	friend bool operator<=(const flavor_info & f, const server& srv);
	server &operator-=(const flavor_info & f);
	server &operator/=(const flavor_info & f);
	int operator/(const flavor_info & f);
	double get_cpu_usage_ratio() const {
		if(! info) return 1.0;
		return 1.0 - remain_cpu_count * 1.0 / info->cpu_count;
	}
	double get_mem_usage_ratio() const {
		if(! info) return 1.0;
		return 1.0 - remain_mem_size * 1.0 / info->mem_size;
	}
	double get_ratio() const {
		return (get_mem_usage_ratio() + get_cpu_usage_ratio()) / 2.0;
	}
};

