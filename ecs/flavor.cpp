/****************************************************************************
 > File Name: flavor.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:28
 ****************************************************************************/

#include "flavor.h"

vector<flavor_info> flavors_info;

int read_flavors_info(char * info[MAX_INFO_NUM]) {
	int flavors_num;
	char vm_name[20];
	sscanf(info[2], "%d", &flavors_num);
	int line;
	for(line = 3; flavors_num--; ++line) {
		flavor_info f_inf;
		sscanf(info[line], "%s %d %d", vm_name, &f_inf.cpu_count, &f_inf.mem_size);
		f_inf.vm_name = string(vm_name);
		flavors_info.push_back(f_inf);
	}
	sort(flavors_info.begin(), flavors_info.end());
	return ++line;
}
