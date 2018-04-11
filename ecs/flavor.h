/****************************************************************************
 > File Name: flavor.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:28
 ****************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "lib_io.h"
#include "datetime.h"


struct flavor_info {
	std::string vm_name;
	int cpu_count,
		mem_size; // mem_size: MB
	bool operator<(const flavor_info &b) const {
		return this->vm_name < b.vm_name;
	}
	bool operator==(const flavor_info &b) const {
		return this->vm_name == b.vm_name;
	}
	flavor_info operator*(int num) const {
		flavor_info tmp(*this);
		tmp.cpu_count *= num;
		tmp.mem_size *= num;
		return tmp;
	}
	flavor_info() = default;
	flavor_info(const char *vm_name, int cpu_count = 0, int mem_size = 0):
			vm_name(vm_name), cpu_count(cpu_count), mem_size(mem_size) {}
};

struct flavor {
	string vm_id;
	datetime create_at;
	const flavor_info * const info;


	flavor(const char *vm_id, const char *create_at, const flavor_info * const info = NULL):
			vm_id(vm_id), create_at(create_at), info(info) {}
	flavor(const datetime & date_time): create_at(date_time), info(NULL) {}

	bool operator<(const flavor &b) const {
		return this->create_at.date < b.create_at.date;
	}
	bool operator==(const flavor &b) const {
		return this->create_at.date == b.create_at.date;
	}

};

int read_flavors_info(char * info[MAX_INFO_NUM]);
std::map<string, std::vector<flavor>> read_flavors(char *data[MAX_DATA_NUM], int data_num);
std::map<string, int> read_deploy_test_cases(char *data[MAX_DATA_NUM], int data_num);
int get_interval_flavors_count(string vm_name, const Date start_date, int during_days);
string get_interval_popular_flavor(const Date start_date, int during_days); // return popular flavor's name
std::vector<int> get_per_flavor_per_interval_count(std::string vm_name);

extern std::map<string, flavor_info> predict_flavors_info; // vm_name -> info
extern std::map<string, std::vector<flavor>> flavors; // vm_name -> flavors
extern std::pair<datetime, datetime> predict_interval;
extern int during_days;
