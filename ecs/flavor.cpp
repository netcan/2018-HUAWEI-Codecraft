/****************************************************************************
 > File Name: flavor.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:28
 ****************************************************************************/

#include "flavor.h"

using std::string;
std::map<string, flavor_info> flavors_info;
std::map<string, std::vector<flavor>> flavors;

int read_flavors_info(char * info[MAX_INFO_NUM]) {
	int flavors_num;
	char vm_name[20];
	sscanf(info[2], "%d", &flavors_num);
	int line;
	for(line = 3; flavors_num--; ++line) {
		flavor_info f_inf;
		sscanf(info[line], "%s %d %d", vm_name, &f_inf.cpu_count, &f_inf.mem_size);
		f_inf.vm_name = vm_name;
		flavors_info[vm_name] = f_inf;
	}
	return ++line;
}

void read_flavors(char *data[MAX_DATA_NUM], int data_num) {
	char vm_id[32], vm_name[32], date_time[32];
	for(int i = 0; i < data_num; ++i) {
		sscanf(data[i], "%s %s %[^\n]", vm_id, vm_name, date_time);
		if(flavors_info.find(vm_name) != flavors_info.end()) {
			flavor f(vm_id, date_time, &flavors_info[vm_name]);
			flavors[vm_name].push_back(f);
		}
	}
}

int get_interval_flavors_count(string vm_name, const Date start_date, int during_days) {
	int ret = 0;
	flavor flavor_end(datetime(start_date + during_days));
	for(std::vector<flavor>::iterator f_it = std::lower_bound(
		flavors[vm_name].begin(), flavors[vm_name].end(), flavor(datetime(start_date))
	); f_it < flavors[vm_name].end() && *f_it < flavor_end; ++f_it, ++ret);
	return ret;
}

