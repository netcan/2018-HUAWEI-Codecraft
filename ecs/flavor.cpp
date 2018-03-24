/****************************************************************************
 > File Name: flavor.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:28
 ****************************************************************************/

#include "flavor.h"

using std::string;
std::map<string, flavor_info> predict_flavors_info;
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
		predict_flavors_info[vm_name] = f_inf;
	}
	return ++line;
}

void read_flavors(char *data[MAX_DATA_NUM], int data_num) {
	char vm_id[32], vm_name[32], date_time[32];
	for(int i = 0; i < data_num; ++i) {
		sscanf(data[i], "%s %s %[^\n]", vm_id, vm_name, date_time);
		if(predict_flavors_info.find(vm_name) != predict_flavors_info.end()) {
			flavor f(vm_id, date_time, &predict_flavors_info[vm_name]);
			flavors[vm_name].push_back(f);
		}
	}
}

int get_interval_flavors_count(string vm_name, const Date start_date, int during_days) {
	int ret = 0;
	if(start_date + during_days <= flavors[vm_name].begin()->create_at.date)
		return -1;

	flavor flavor_end_date(datetime(start_date + during_days));
	for(std::vector<flavor>::iterator f_it = std::lower_bound(
		flavors[vm_name].begin(), flavors[vm_name].end(), flavor(datetime(start_date))
	); f_it < flavors[vm_name].end() && *f_it < flavor_end_date; ++f_it, ++ret);

//	printf("%d-%d-%d -> %d-%d-%d: %d\n", start_date.year, start_date.month, start_date.day, t.year, t.month, t.day, ret);
	return ret;
}

string get_interval_popular_flavor(const Date start_date, int during_days) { // return popular flavor's name
	flavor flavor_end_date(datetime(start_date + during_days));
	int popular_count = 0, count = 0;
	string popular_vm_name;
	for(const auto& flv: predict_flavors_info) {
		string vm_name = flv.first;
		for (std::vector<flavor>::iterator f_it = std::lower_bound(
				flavors[vm_name].begin(), flavors[vm_name].end(), flavor(datetime(start_date))
		); f_it < flavors[vm_name].end() && *f_it < flavor_end_date; ++f_it, ++count);
		if (count > popular_count) {
			popular_count = count;
			popular_vm_name = vm_name;
		} else if(count == popular_count) {
			if(flv.second.mem_size < predict_flavors_info[popular_vm_name].mem_size ||
			   flv.second.cpu_count < predict_flavors_info[popular_vm_name].cpu_count)
				popular_vm_name = vm_name;
		}
	}
	return popular_vm_name;
}

std::vector<int> get_per_flavor_per_interval_count(std::string vm_name) {
	std::vector<int> Y_count;
	int cnt = 0;
	for(Date d = predict_interval.first.date + (-during_days);
	    (cnt = get_interval_flavors_count(vm_name, d, during_days))!= -1;
	    d += -during_days) {
		Y_count.push_back(cnt);
	}
	std::reverse(Y_count.begin(), Y_count.end());
	return Y_count;
}
