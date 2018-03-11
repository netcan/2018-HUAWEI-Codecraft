/****************************************************************************
 > File Name: server.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:53
 ****************************************************************************/

#ifndef ECS_SERVER_H
#define ECS_SERVER_H


struct server {
	int cpu_count, mem_size, disk_size;
};
extern server srv;


#endif //ECS_SERVER_H
