#pragma once

#include "lib_io.h"
#include "flavor.h"
#include "server.h"
#include "datetime.h"
#include "random.h"
#include "prediction_model.h"
#include <signal.h>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstdio>
#include <cstring>

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);


extern std::pair<datetime, datetime> predict_interval;
extern int during_days;


template <class SF>
double get_deploy_ratio(const SF &solution_flavor, const std::vector<server> & servers) {
	int rCpu = 0, rMem = 0, RCpu = 0, RMem = 0;
	for(const auto& sf: solution_flavor) {
		rCpu += predict_flavors_info[sf.first].cpu_count * sf.second;
		rMem += predict_flavors_info[sf.first].mem_size * sf.second;
	}
	for(const auto& srv: servers) {
		RCpu += srv.info->cpu_count;
		RMem += srv.info->mem_size;
	}

#ifdef _DEBUG
//	printf("(%d/%d + %d/%d)/2= %.2f\n", rCpu, RCpu, rMem, RMem, (rMem * 1.0 / RMem + rCpu * 1.0 / RCpu) / 2.0);
#endif
//	printf("(%d/%d + %d/%d)/2= %.2f\n", rCpu, RCpu, rMem, RMem, (rMem * 1.0 / RMem + rCpu * 1.0 / RCpu) / 2.0);
	return (rMem * 1.0 / RMem + rCpu * 1.0 / RCpu) / 2.0;
}
