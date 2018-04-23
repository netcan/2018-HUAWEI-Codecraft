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

void interval_predict(std::map<string, int>& solution_flavor);
void xjb_predict(std::map<string, int>& solution_flavor);
void linear_regression_predict(std::map<string, int>& solution_flavor);
void polynomial_regression_predict(std::map<string, int>& solution_flavor);
void exponential_smoothing_predict(std::map<string, int>& solution_flavor);
void exponential_smoothing_predict_by_day(std::map<string, int>& solution_flavor);
void bp_predict(std::map<string, int>& solution_flavor);
void avg_predict(std::map<string, int>& solution_flavor);
double shell_coefficient(const std::map<string, int>& predict_solution_flavor,const std::map<string, int>& real_solution_flavor);
double cv_expontential_smoothing_predict();

void deploy_server_SA(std::map<string, int> &solution_flavor, std::vector<server> &solution_server, int inner_loop = 10, double T = 20.0, double Tmin=0.1, double delta = 0.99999);
void deploy_server_SA_tradeoff(std::map<string, int> &solution_flavor, std::vector<server> &solution_server, int inner_loop = 10, double T = 20.0, double Tmin = 0.1, double delta = 0.99999);
void deploy_server_fit(const std::map<string, int> &solution_flavor, std::vector<server> &solution_server);
char* get_result(std::map<string, int>& solution_flavor, std::vector<server>& solution_server);
bool solution_flavor_cmp(std::pair<string, int>& a, std::pair<string, int>& b);

extern std::pair<datetime, datetime> predict_interval;
extern int during_days;

double get_servers_avg_usage_ratio(const std::vector<server> &solution_server);

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
