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

enum {
	CPU, MEM
} target;
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);

void interval_predict(std::map<string, int>& solution_flavor);
void xjb_predict(std::map<string, int>& solution_flavor);
void linear_regression_predict(std::map<string, int>& solution_flavor);
void polynomial_regression_predict(std::map<string, int>& solution_flavor);
void exponential_smoothing_predict(std::map<string, int>& solution_flavor);
double shell_coefficient(const std::map<string, int>& predict_solution_flavor,const std::map<string, int>& real_solution_flavor);
double cv_expontential_smoothing_predict();

std::vector<server> first_fit(const std::vector<std::pair<string, int>>& sfv, std::vector<std::map<string, int>>& solution_server);
void fill_deploy_server(std::map<string, int> &solution_flavor, std::vector<std::map<string, int>> &solution_server);
void deploy_server_SA(std::map<string, int> &solution_flavor, std::vector<std::map<string, int>>& solution_server, int inner_loop = 10, double T = 20.0, double delta = 0.99999);
void deploy_server_SA_fill(std::map<string, int> &solution_flavor, std::vector<std::map<string, int>>& solution_server, int inner_loop = 10, double T = 20.0, double delta = 0.99999);
char* get_result(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server);
bool solution_flavor_cmp(std::pair<string, int>& a, std::pair<string, int>& b);

extern std::pair<datetime, datetime> predict_interval;
extern int during_days;
extern std::string init_f;

template <class SF>
double get_deploy_ratio(const SF &solution_flavor, const std::vector<std::map<string, int>> &solution_server) {
	int r = 0, R;
	for(const auto& sf: solution_flavor)
		r += (target == CPU?
		      predict_flavors_info[sf.first].cpu_count:
		      predict_flavors_info[sf.first].mem_size) * sf.second;
	R = int(solution_server.size()) * (target == CPU?
	                                   server::cpu_count:
	                                   server::mem_size);
#ifdef _DEBUG
	printf("%d/%d = %.2f\n", r, R, r * 1.0/R);
#endif
	return r * 1.0 / R;
}
