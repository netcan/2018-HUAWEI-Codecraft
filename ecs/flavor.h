/****************************************************************************
 > File Name: flavor.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:28
 ****************************************************************************/

#ifndef ECS_FLAVOR_H
#define ECS_FLAVOR_H
#include <string>
#include <vector>
#include <algorithm>
#include "predict.h"
#include "datetime.h"

using std::string;
using std::vector;

struct flavor_info {
	string vm_name;
	int cpu_count, mem_size;
	bool operator<(const flavor_info &b) const {
		return this->vm_name < b.vm_name;
	}
	bool operator==(const flavor_info &b) const {
		return this->vm_name == b.vm_name;
	}

};

struct flavor {
	string vm_id;
	datetime create_at;
	flavor_info *info;
};

int read_flavors_info(char * info[MAX_INFO_NUM]);

extern vector<flavor_info> flavors_info;

#endif //ECS_FLAVOR_H
