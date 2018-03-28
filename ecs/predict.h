#pragma once
#include "lib_io.h"
#include "server.h"
#include "flavor.h"
#include "datetime.h"
#include "random.h"
#include "prediction_model.h"
#include <ctime>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstdio>
#include <cstring>

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);

void interval_predict(std::map<string, int>& solution_flavor);
void linear_regression_predict(std::map<string, int>& solution_flavor);
void polynomial_regression_predict(std::map<string, int>& solution_flavor);
void exponential_smoothing_predict(std::map<string, int>& solution_flavor);
double shell_coefficient(std::map<string, int>& predict_solution_flavor,  std::map<string, int>& real_solution_flavor);
double cv_expontential_smoothing_predict();

void deploy_server(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server);
char* get_result(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server);
void show_ratio(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server);
bool solution_flavor_cmp(std::pair<string, int>& a, std::pair<string, int>& b);

extern std::pair<datetime, datetime> predict_interval;
extern int during_days;

