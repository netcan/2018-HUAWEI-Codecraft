/****************************************************************************
 > File Name: server.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:53
 ****************************************************************************/

#include "server.h"

int server::cpu_count = 0;
int server::mem_size = 0; // covert to MB
int server::disk_size = 0;

server::server(int remain_cpu_count, int remain_mem_size, int remain_disk_size) :
		remain_cpu_count(remain_cpu_count),
		remain_mem_size(remain_mem_size),
		remain_disk_size(remain_disk_size) {}

bool operator<=(const flavor_info & f, const server& srv) {
	return f.cpu_count <= srv.remain_cpu_count &&
	       f.mem_size <= srv.remain_mem_size;
}

server &server::operator-=(const flavor_info & f) {
	remain_cpu_count -= f.cpu_count;
	remain_mem_size -= f.mem_size;
	return *this;
}
