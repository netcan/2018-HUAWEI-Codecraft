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
std::vector<int> merge_cnt_day_by_interval(const std::vector<int> & by_day, int interval);

extern std::map<string, flavor_info> predict_flavors_info; // name -> info
extern std::map<string, std::vector<flavor>> flavors; // name -> flavors
extern std::pair<datetime, datetime> predict_interval;
extern datetime train_end_time;

template <class T>
std::vector<T> denoising(const std::vector<T>& data, double k = 1.5) {
	std::vector<int> dta = data,
			data_sort = dta;

//	printf("%s(%ld)\n", vm_name.c_str(), by_day.size());
//	for(auto cnt: by_day) {
//		printf("%3d", cnt);
//	}
//	puts("");
//	for(auto cnt: by_day)
//		if(cnt) data_sort.push_back(cnt);


	std::sort(data_sort.begin(), data_sort.end()); // 从小到大排序
	double Q1, Q2, Q3, IQR, max_outlier, min_outlier;
	int max_cnt = -1;
	int pos = 0, n = int(data_sort.size());
	// Q2
	printf("n = %d\n", n);
	if(n & 1)  // 奇数个
		Q2 = data_sort[n/2];
	else {
		pos = (n - 1) / 2;
		Q2 = (data_sort[pos] + data_sort[pos + 1]) / 2.0;
	}
	// Q1
	pos = int((n+1)/4.0-1);
	Q1 = 0.25 * data_sort[pos] + 0.75 * data_sort[pos + 1];
	// Q3
	pos = int((n+1)*3/4.0-1);
	Q3 = 0.75 * data_sort[pos] + 0.25 * data_sort[pos + 1];
	IQR = Q3 - Q1;
	max_outlier = Q3 + k * IQR;
	min_outlier = Q1 - k * IQR;

	for(auto cnt: data_sort)
		if(cnt <= max_outlier) max_cnt = std::max(max_cnt, cnt);

	for(auto &cnt: dta)
		if(cnt > max_outlier) cnt = max_cnt; // 异常值用最大值来代替


	printf("Q1=%lf Q2=%lf Q3=%lf IQR=%lf maxo = %lf mino = %lf\n", Q1, Q2, Q3, IQR, max_outlier, min_outlier);
	printf("Before\n");
	for(auto cnt: data) {
		printf("%3d", cnt);
	}
	printf("\nAfter\n");
	for(auto cnt: dta) {
		printf("%3d", cnt);
	}
	puts("");

	return dta;
}
