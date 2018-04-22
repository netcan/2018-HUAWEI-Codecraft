/****************************************************************************
 > File Name: server.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:53
 ****************************************************************************/

#include "server.h"

server::server(const server_info * info) :
		info(info), remain_cpu_count(0),
		remain_mem_size(0),
		remain_disk_size(0) {
	if(info) {
		remain_cpu_count = info->cpu_count;
		remain_mem_size = info->mem_size;
		remain_disk_size = info->disk_size;
	}

}

bool operator<=(const flavor_info & f, const server& srv) {
	return f.cpu_count <= srv.remain_cpu_count &&
	       f.mem_size <= srv.remain_mem_size;
}

server &server::operator-=(const flavor_info & f) {
	remain_cpu_count -= f.cpu_count;
	remain_mem_size -= f.mem_size;
	return *this;
}
server &server::operator/=(const flavor_info & f) {
	int k = this->operator/(f);
	remain_cpu_count -= k * f.cpu_count;
	remain_mem_size -= k * f.mem_size;
	return *this;
}

int server::operator/(const flavor_info & f) {
	return std::min(remain_cpu_count / f.cpu_count, remain_mem_size / f.mem_size);
}

void server::place_flavor(const flavor_info &flv, int flv_num) {
	this->operator-=(flv * flv_num);
	if(flavors.find(flv.name) != flavors.end())
		flavors[flv.name] += flv_num;
	else flavors[flv.name] = flv_num;
}

std::vector<server_info> servers_info; // name -> info

int read_servers_info(char * info[MAX_INFO_NUM]) {
	int servers_info_num;
	char server_name[20];
	sscanf(info[0], "%d", &servers_info_num);
	int line;
	for(line = 1; servers_info_num--; ++line) {
		server_info srv_inf;
		sscanf(info[line], "%s %d %d %d", server_name, &srv_inf.cpu_count, &srv_inf.mem_size, &srv_inf.disk_size);
		srv_inf.mem_size *= 1024; // covert GB to MB
		srv_inf.name = server_name;
		servers_info.push_back(srv_inf);
	}

	// 排序

	std::sort(servers_info.begin(), servers_info.end(), [=](const server_info &lhs, const server_info &rhs) -> bool {
		if(lhs.cpu_count == rhs.cpu_count)
			return lhs.mem_size < rhs.mem_size;
		return lhs.cpu_count < rhs.cpu_count;
	});

	return ++line;
}

