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

struct base_info {
	std::string name;
	int cpu_count,
		mem_size, // mem size: MB
		disk_size;

	base_info() = default;
	base_info(const char*name, int cpu_count = 0, int mem_size = 0, int disk_size = 0):
			name(name), cpu_count(cpu_count), mem_size(mem_size), disk_size(disk_size) {}
};

struct flavor_info : base_info {
	bool operator<(const flavor_info &b) const {
		return this->name < b.name;
	}
	bool operator==(const flavor_info &b) const {
		return this->name == b.name;
	}
	flavor_info operator*(int num) const {
		flavor_info tmp(*this);
		tmp.cpu_count *= num;
		tmp.mem_size *= num;
		return tmp;
	}
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
int get_interval_flavors_count(const string& vm_name, const Date& start_date, int interval);
string get_interval_popular_flavor(const Date& start_date, int interval); // return popular flavor's name
std::vector<int> get_per_flavor_count_by_interval(const std::string &vm_name, int interval);
std::vector<int> denoising(const std::string& vm_name) ;
std::vector<int> merge_cnt_day_by_interval(const std::vector<int> & by_day, int interval);

extern std::map<string, flavor_info> predict_flavors_info; // name -> info
extern std::map<string, std::vector<flavor>> flavors; // name -> flavors
extern std::pair<datetime, datetime> predict_interval;
extern datetime train_end_time;
