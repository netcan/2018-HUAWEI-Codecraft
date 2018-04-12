/****************************************************************************
 > File Name: flavor.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 17:28
 ****************************************************************************/

#include "flavor.h"
#include "server.h"

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

std::map<string, std::vector<flavor>> read_flavors(char *data[MAX_DATA_NUM], int data_num) {
	char vm_id[32], vm_name[32], date_time[32];
	std::map<string, std::vector<flavor>> flavors; // vm_name -> flavors
	for(int i = 0; i < data_num; ++i) {
		sscanf(data[i], "%s %s %[^\n]", vm_id, vm_name, date_time);
		if(predict_flavors_info.find(vm_name) != predict_flavors_info.end()) {
			flavor f(vm_id, date_time, &predict_flavors_info[vm_name]);
			flavors[vm_name].push_back(f);
			train_end_time = std::max(train_end_time, datetime(date_time));
		}
	}
	return flavors;
}

std::map<string, int> read_deploy_test_cases(char *data[MAX_DATA_NUM], int data_num) {
	std::map<string, int> solution_flavor; // vm_name: count
	int line = 0;
	sscanf(data[line], "%d %d %d", &server::cpu_count, &server::mem_size, &server::disk_size);
	int flavor_num = 0;
	sscanf(data[line + 4], "%d", &flavor_num);
	server::mem_size *= 1024; // covert to MB
	line += 6;
	char flavor_name[20];
	int cnt;
	for(int i = 0; i < flavor_num; ++i) {
		sscanf(data[line+i], "%[^:]:%d", flavor_name, &cnt);
//		printf("%s %d\n", flavor_name, cnt);
		solution_flavor[flavor_name] = cnt;
	}
	return solution_flavor;
}

int get_interval_flavors_count(const string& vm_name, const Date& start_date, int interval) { // [start_date, start_date + interval)
	int ret = 0;
	if(start_date + interval <= flavors[vm_name].begin()->create_at.date)
		return -1;

	flavor flavor_end_date(datetime(start_date + interval));
	for(std::vector<flavor>::iterator f_it = std::lower_bound(
		flavors[vm_name].begin(), flavors[vm_name].end(), flavor(datetime(start_date))
	); f_it < flavors[vm_name].end() && *f_it < flavor_end_date; ++f_it, ++ret);

//	printf("%d-%d-%d -> %d-%d-%d: %d\n", start_date.year, start_date.month, start_date.day, t.year, t.month, t.day, ret);
	return ret;
}

string get_interval_popular_flavor(const Date& start_date, int interval) { // return popular flavor's name
	flavor flavor_end_date(datetime(start_date + interval));
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

std::vector<int> get_per_flavor_count_by_interval(const std::string &vm_name, int interval) {
	std::vector<int> Y_count;
	int cnt = 0;
	for(Date d = train_end_time.date + 1 + (-interval);
	    (cnt = get_interval_flavors_count(vm_name, d, interval))!= -1;
	    d += -interval) {
		Y_count.push_back(cnt);
	}
	std::reverse(Y_count.begin(), Y_count.end());
	return Y_count;
}


std::vector<int> denoising(const std::string& vm_name) {
	std::vector<int> by_day = get_per_flavor_count_by_interval(vm_name, 1),
			by_day_sort = by_day;
//	for(auto cnt: by_day)
//		if(cnt) by_day_sort.push_back(cnt);


	std::sort(by_day_sort.begin(), by_day_sort.end()); // 从小到大排序
	double Q1, Q2, Q3, IQR, max_outlier, min_outlier;
	int max_cnt = -1;
	int pos = 0, n = int(by_day_sort.size());
	// Q2
	if(n & 1)  // 奇数个
		Q2 = by_day_sort[n/2];
	else {
		pos = (n - 1) / 2;
		Q2 = (by_day_sort[pos] + by_day_sort[pos + 1]) / 2.0;
	}
	// Q1
	pos = int((n+1)/4.0-1);
	Q1 = 0.25 * by_day_sort[pos] + 0.75 * by_day_sort[pos + 1];
	// Q3
	pos = int((n+1)*3/4.0-1);
	Q3 = 0.75 * by_day_sort[pos] + 0.25 * by_day_sort[pos + 1];
	IQR = Q3 - Q1;
	max_outlier = Q3 + 1.5 * IQR;
	min_outlier = Q1 - 1.5 * IQR;

	for(auto cnt: by_day_sort)
		if(cnt <= max_outlier) max_cnt = std::max(max_cnt, cnt);

	for(auto &cnt: by_day)
		if(cnt > max_outlier) cnt = max_cnt; // 异常值用最大值来代替


//	printf("%s\n", vm_name.c_str());
//	printf("Q1=%lf Q2=%lf Q3=%lf IQR=%lf maxo = %lf mino = %lf\n", Q1, Q2, Q3, IQR, max_outlier, min_outlier);
//	for(auto cnt: by_day) {
//		printf("%d ", cnt);
//	}
//	puts("");

	return by_day;
}

std::vector<int> merge_cnt_day_by_interval(const std::vector<int> & by_day, int interval) {
	int cnt = 0;
	std::vector<int> Y_count;
	int intval = interval;
	for(int i = by_day.size() - 1; i >= 0; --i) {
		cnt += by_day[i];
		--intval;
		if(intval == 0) {
			Y_count.push_back(cnt);
			cnt = 0;
			if(i) intval = interval;
		}
	}
	if(intval > 0) Y_count.push_back(cnt);

	std::reverse(Y_count.begin(), Y_count.end());
	return Y_count;
}