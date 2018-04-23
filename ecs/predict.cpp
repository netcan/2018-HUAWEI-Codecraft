#include "predict.h"

int during_days = 0;

std::pair<datetime, datetime> predict_interval;
datetime train_end_time;
bool runing = true;
char result[20 * 20 * 10000];

typedef void (sigFunc)(int);

// 定时器begin
sigFunc *
Signal(int signo, sigFunc *func) {
	struct sigaction	act, oact;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
// 定时器end
void timeOutHandler(int signo) {
	runing = false;
	return;
}


void interval_predict(std::map<string, int>& solution_flavor) {
	for(const auto &f: predict_flavors_info) { // predict per vm
		int	s = get_interval_flavors_count(
			f.first, train_end_time.date + 1 + (-during_days), during_days
		);
		solution_flavor[f.first] = s;
	}
}


void linear_regression_predict(std::map<string, int>& solution_flavor) {
	linear_regression LinearReg;
	for(const auto &f: predict_flavors_info) { // predict per vm

		std::vector<int> by_day = std::move(denoising(get_per_flavor_count_by_interval(f.first, 1))); // 去噪
		std::vector<int> Y_count = merge_cnt_day_by_interval(by_day, during_days);

//		std::vector<int> Y_count = get_per_flavor_count_by_interval(f.first, during_days);

		std::vector<int> X;
		int x = 1;
		for(size_t i = 0; i < Y_count.size(); ++i) {
//			Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);
			X.push_back(x++);
		}

		LinearReg.train(X, Y_count);

		solution_flavor[f.first] = std::max(0, int(lround(LinearReg.predict(x))));


//		solution_flavor[f.first] = int(lround(LinearReg.predict(x) -
//		                                      LinearReg.predict(x - 1)));

#ifdef _DEBUG
		printf("%s predict_x=%d\n", f.first.c_str(), x);
		for(size_t i = 0; i < X.size(); ++i)
			printf("(%d, %d) ", X[i], Y_count[i] - (i > 0 ? Y_count[i-1]: 0));
		puts("");
		LinearReg.print_coefficient();
		for(size_t i = 0; i < X.size(); ++i)
			printf("(%d, %ld) ", X[i], lround(LinearReg.predict(X[i])) -
			                           lround(i > 0 ? LinearReg.predict(X[i-1]) : 0));
		puts("\n-----------------");
#endif
	}

}
void polynomial_regression_predict(std::map<string, int>& solution_flavor) {
	polynomial_regression PolyReg(2);
	for(const auto &f: predict_flavors_info) { // predict per vm
		std::vector<int> by_day = std::move(denoising(get_per_flavor_count_by_interval(f.first, 1), 3.0)); // 去噪
		std::vector<int> Y_count = denoising(merge_cnt_day_by_interval(by_day, during_days), 3.0);

//		std::vector<int> Y_count = get_per_flavor_count_by_interval(f.first, during_days);

		std::vector<int> X;
		int x = 1;
		for(size_t i = 0; i < Y_count.size(); ++i) {
//			Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);
			X.push_back(x++);
		}

		double t = (predict_interval.second.date - train_end_time.date - 1)*1.0 / during_days;

		PolyReg.train(X, Y_count, 1e-1, -1);
//		solution_flavor[f.first] = std::max(int(lround(PolyReg.predict(x) - PolyReg.predict(x - 1))), 0);

		solution_flavor[f.first] = std::max(int(lround(PolyReg.predict(x - 1.0 + t))), 0);

#ifdef _DEBUG
		printf("%s predict_x=%d\n", f.first.c_str(), x);
		for(size_t i = 0; i < X.size(); ++i)
			printf("(%d, %d) ", X[i], Y_count[i]);
		puts("");
		PolyReg.print_coefficient();
		for(size_t i = 0; i < X.size(); ++i)
			printf("(%d, %ld) ", X[i], lround(PolyReg.predict(X[i])));
		puts("\n-----------------");
#endif
	}

}

double shell_coefficient(const std::map<string, int>& predict_solution_flavor,
                         const std::map<string, int>& real_solution_flavor) {
	assert(predict_solution_flavor.size() == real_solution_flavor.size());
	double rmse = 0, rsy = 0, rsY = 0;
	for(const auto &vm: predict_solution_flavor) {
		const string & vm_name = vm.first;
		int yi = const_cast<std::map<string, int> &>(predict_solution_flavor)[vm_name],
			Yi = const_cast<std::map<string, int> &>(real_solution_flavor)[vm_name];
		rmse += (yi - Yi) * (yi - Yi);
		rsy += yi * yi;
		rsY += Yi * Yi;
	}
	rmse /= predict_solution_flavor.size();
	rsy /= predict_solution_flavor.size();
	rsY /= predict_solution_flavor.size();
	return sqrt(rmse) / (sqrt(rsy) + sqrt(rsY));
}

double cv_expontential_smoothing_predict() {
	std::map<string, int> predict_solution_flavor, real_solution_flavor;
	int best_alpha = 0;
	double best_accuracy = -1;
	const int max_alpha = 10000;

	double t = 1.0;
	for(int alpha = 0; alpha <= max_alpha; ++alpha) {
		exponential_smoothing ExpSmooth(alpha * 1.0 / max_alpha);
		for (const auto &f: predict_flavors_info) { // predict per vm
			std::vector<int> Y_count = get_per_flavor_count_by_interval(f.first, during_days);
			real_solution_flavor[f.first] = Y_count.back(); Y_count.pop_back();
//			for(size_t i = 0; i < Y_count.size(); ++i)
//				Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);

			ExpSmooth.train(Y_count);
//			predict_solution_flavor[f.first] = int(lround(ExpSmooth.predict(Y_count.back()))) - Y_count.back();

			predict_solution_flavor[f.first] = std::max(0, int(lround(ExpSmooth.predict(t))));
		}
		double sc = shell_coefficient(predict_solution_flavor,
		                              real_solution_flavor);
		if(best_accuracy < 0 || best_accuracy > sc) {
			best_accuracy = sc;
			best_alpha = alpha;
		}
#ifdef _DEBUG
		printf("alpha = %lf, accuracy = %lf\n",
		       alpha * 1.0 / max_alpha, sc);
#endif
	}
	printf("best_alpha = %lf best_accuracy = %f\n", best_alpha * 1.0/ max_alpha, best_accuracy);
	return best_alpha * 1.0 / max_alpha;
}

void exponential_smoothing_predict(std::map<string, int>& solution_flavor) {
//	exponential_smoothing ExpSmooth(cv_expontential_smoothing_predict());
	// 0.5, 1.8
	exponential_smoothing ExpSmooth(0.91);
//	exponential_smoothing ExpSmooth(0.99);
	double c = 1.7; // 系数！
	for(const auto &f: predict_flavors_info) { // predict per vm
//		std::vector<int> by_day = get_per_flavor_count_by_interval(f.first, 1);
		// print per vm
		printf("%s\n", f.first.c_str());

		std::vector<int> by_day = denoising(get_per_flavor_count_by_interval(f.first, 1), 3.0); // 去噪
		std::vector<int> Y_count = denoising(merge_cnt_day_by_interval(by_day, during_days), 3.0);

//		std::vector<int> Y_count = get_per_flavor_count_by_interval(f.first, during_days);
//		std::vector<int> Y2_count = merge_cnt_day_by_interval(by_day, during_days);
//		assert(Y_count == Y2_count);

//		for(size_t i = 0; i < Y_count.size(); ++i)
//			Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);


		ExpSmooth.train(Y_count);
//		solution_flavor[f.first] = std::max(int(lround(ExpSmooth.predict(Y_count.back()))) - Y_count.back(), 0);

		double t = (predict_interval.second.date - train_end_time.date - 1)*1.0 / during_days;
		solution_flavor[f.first] = lround(std::max(0, int(lround(ExpSmooth.predict(t)))) * c);
	}

}


void exponential_smoothing_predict_by_day(std::map<string, int>& solution_flavor) {
	// 0.3 best
	exponential_smoothing ExpSmooth(0.30);
	double c = 1.0;
	for(const auto &f: predict_flavors_info) { // predict per vm
		// best: k = 3.0
		std::vector<int> by_day = std::move(denoising(get_per_flavor_count_by_interval(f.first, 1), 3.0)); // 去噪
		ExpSmooth.train(by_day);
		// 参数
		int cnt = 0;
		for(int t = predict_interval.first.date - train_end_time.date, d = 0; d < during_days; ++t, ++d)
			cnt += std::max(0, int(lround(ExpSmooth.predict(t))));
		solution_flavor[f.first] = int(lround(cnt * c));
	}
}

void bp_predict(std::map<string, int>& solution_flavor) {
	for(const auto &f: predict_flavors_info) { // predict per vm
		std::vector<int> by_day = std::move(denoising(get_per_flavor_count_by_interval(f.first, 1), 3.0)); // 去噪
		BP_Network bp({1, 10, 1});
		vector<pair<vector<double>, vector<double>>> train_data;
		int day = 0;
		for(day = 0; day < by_day.size(); ++day)
			train_data.emplace_back(vector<double>(1, day * 1.0), vector<double>(1, by_day[day] * 1.0));

		bp.SGD(train_data, 50, during_days, 3.0);
		double cnt = 0;
		day += predict_interval.first.date - train_end_time.date - 1;
		for(int d = 0; d < during_days; ++d, ++day)
			cnt += bp.feedforward({day * 1.0})[0];

		solution_flavor[f.first] = std::max(int(lround(cnt)), 0);

	}

}

void avg_predict(std::map<string, int>& solution_flavor) {
	for(const auto &f: predict_flavors_info) { // predict per vm
		std::vector<int> cnt_by_day = get_per_flavor_count_by_interval(f.first, 1); // 获取当天数据
		int days = 10;

		double m = mean(cnt_by_day), sd = SD(cnt_by_day);

		 // 滤3.5标准差
//		for(auto &cnt: cnt_by_day)
//			if(cnt > 3.5 * sd) cnt = int(lround(m));

		// 去最后10天数据的平均值
		double avg = 0;
		for(int i = 0; i < days; ++i)
			avg += cnt_by_day[cnt_by_day.size() - 1 - i];
		avg /= days;

		double cnt = avg * during_days;
		if(during_days >= 8) cnt *= 1.60;
		else cnt *= 0.60;

		cnt += Rand.Random_Int(0, 5);

		solution_flavor[f.first] = int(lround(cnt));
	}

}

std::vector<server> first_fit(const std::vector<std::pair<string, int>>& sfv) {
	// solution_server is empty
	std::vector<server> servers;
	int flavor_k_num = 0;
	for(size_t i = 0; i < sfv.size(); ) {
		server best_srv;
		double max_usage_ratio = -1;
		int best_k = 0, cur_k_num = 0;
		for(const auto & srv_info: servers_info) {
			server srv(&srv_info);
			size_t k, k_num = 0;
			// 开始填充
			for(k = i; k < sfv.size(); ++k) {
				const string& vm_name = sfv[k].first;
				int vm_count = sfv[k].second;
				if(k == i) vm_count -= flavor_k_num;

				const flavor_info& flv = predict_flavors_info[vm_name];
				if(flv <= srv) { // 能放下
					int vnum = std::min(srv / flv, vm_count);
					srv.place_flavor(flv, vnum);
					if(vnum < vm_count) { // 有剩余
						k_num = vnum;
						break;
					}
				} else {
					k_num = 0;
					break;
				}
			}
			double usage_ratio = srv.get_ratio();
			if(max_usage_ratio < 0 || max_usage_ratio < usage_ratio) {
				max_usage_ratio = usage_ratio;
				best_srv = std::move(srv);
				best_k = k;
				cur_k_num = k_num;
			}
		}
		servers.push_back(best_srv);
		if(i == best_k) flavor_k_num += cur_k_num;
		else flavor_k_num = cur_k_num;
		i = best_k;
	}

	return servers;
}

void deploy_server_fit(const std::map<string, int> &solution_flavor, std::vector<server> &solution_server) {
	// General(0), High-Memory(1), High-Performance(2)
	for(size_t i = 0; i < servers_info.size(); ++i)
		solution_server.emplace_back(&servers_info[i]);

	for(const auto& sf: solution_flavor) {
		// find the best location
		const string & vm_name = sf.first;
		int vm_count = sf.second;

		const flavor_info& flv = predict_flavors_info[vm_name];
		double vm_ratio = flv.mem_size * 1.0 / flv.cpu_count;
		for (int f_count = 0; f_count < vm_count; ) {
			int srv_loc = -1;
			double min_diff = -1;
			for (int loc = 0; loc < solution_server.size(); ++loc) {
				if (flv <= solution_server[loc]) { // 能装下
					double srv_ratio = solution_server[loc].remain_mem_size * 1.0 / solution_server[loc].remain_cpu_count;
					double diff = fabs(srv_ratio - vm_ratio);
					if (min_diff < 0 || min_diff > diff) {
						min_diff = diff;
						srv_loc = loc;
					}
				}
			}

			if(srv_loc != -1) { // 找到合适的位置
				int vnum = solution_server[srv_loc] / flv;
				if(vnum + f_count > vm_count) vnum = vm_count - f_count;
				f_count += vnum;
				solution_server[srv_loc].place_flavor(flv, vnum);
			} else { // 开辟新空间
				min_diff = -1;
				for(int loc = 0; loc < servers_info.size(); ++loc) {
					double srv_ratio = servers_info[loc].mem_size * 1.0 / servers_info[loc].cpu_count;
					double diff = fabs(srv_ratio - vm_ratio);
					if (min_diff < 0 || min_diff > diff) {
						min_diff = diff;
						srv_loc = loc;
					}
				}
				solution_server.emplace_back(&servers_info[srv_loc]);
			}
		}
	}

}

void deploy_server_SA_tradeoff(std::map<string, int> &solution_flavor,
                               std::vector<server> &solution_server,
                               int inner_loop, double T, double Tmin, double delta) {

	// 退火
	std::vector<std::pair<string, int>> sfv;

//	for(const auto & sf: solution_flavor) sfv.push_back(sf);

	for(const auto & sf: solution_flavor)
		for(int i = 0; i < sf.second; ++i) sfv.push_back(std::make_pair(sf.first, 1));

	std::vector<server> servers = std::move(first_fit(sfv)),
			best_servers = servers;
	int servers_num = servers.size();

	solution_server = servers;

	double last_deploy_ratio = get_deploy_ratio(solution_flavor, servers),
			cur_deploy_ratio = 0.0, best_deploy_ratio = -1;
	bool action = 0; // 1交换位置，0加flavor

	while(runing && T > Tmin) {
		for(int loop = 0; loop < inner_loop && runing; ++loop) {
			int i, j, k;
			action = Rand.Random_Int(1, 100) > 60; // 比率
//			action = Rand.Random_Int(1, 100) > 0; // 比率

			if(action) { // 交换
				i = Rand.Random_Int(0, sfv.size() - 1), j = Rand.Random_Int(0, sfv.size() - 1);
				while(i == j) i = Rand.Random_Int(0, sfv.size() - 1);
				std::swap(sfv[i], sfv[j]); // 随机交换两个flavor的位置
			} else {
				k = Rand.Random_Int(0, sfv.size() - 1); // 选一个flavor加或者减
//				++sfv[k].second;
				sfv.push_back(std::make_pair(sfv[k].first, 1));
			}

			servers = std::move(first_fit(sfv));
			cur_deploy_ratio = get_deploy_ratio(sfv, servers);

			double dc = last_deploy_ratio - cur_deploy_ratio;
			if(std::min(1.0, exp(-dc / T)) > Rand.Random_Real(0, 1) && servers_num >= servers.size()) { // 接受
//				printf("%f\n", std::min(1.0, exp(-dc / T)));
				servers_num = servers.size(); // 压缩
				last_deploy_ratio = cur_deploy_ratio;
			} else { // reject
				if(action) std::swap(sfv[i], sfv[j]); // 换回去
				else {
//					--sfv[k].second;
					sfv.pop_back();
				}
			}

			if(best_deploy_ratio < 0 || best_deploy_ratio < cur_deploy_ratio) {
				best_deploy_ratio = cur_deploy_ratio;
				best_servers = servers;
			}

		}
		T *= delta;
	}

	for(auto& sf: solution_flavor) sf.second = 0;

	for(const auto& sf: sfv) {
//		solution_flavor[sf.first] = sf.second;
		++solution_flavor[sf.first];
	}
	solution_server = best_servers;

}

double get_servers_avg_usage_ratio(const std::vector<server> &solution_server) {
	double r = 0;
	for(const auto &srv: solution_server)
		r += srv.get_ratio();
	return r / solution_server.size();
}

void deploy_server_SA(std::map<string, int> &solution_flavor,
                      std::vector<server> &solution_server,
                      int inner_loop, double T, double Tmin, double delta) {
	// 退火
	std::vector<std::pair<string, int>> sfv, sfv_reduced;
	for(const auto & sf: solution_flavor)
		for(int i = 0; i < sf.second; ++i) sfv.push_back(std::make_pair(sf.first, 1));

	std::vector<server> servers = std::move(first_fit(sfv)), best_servers;
	solution_server = servers;

	double last_deploy_ratio = get_deploy_ratio(solution_flavor, servers),
			cur_deploy_ratio = 0.0, best_deploy_ratio = -1;

	while(runing && T > Tmin) {
		for(int loop = 0; loop < inner_loop && runing; ++loop) {
			servers.clear(); // 记得清空
			sfv_reduced.clear();
			int i = Rand.Random_Int(0, sfv.size() - 1),
				j = Rand.Random_Int(0, sfv.size() - 1);
			while(sfv[i].first == sfv[j].first) i = Rand.Random_Int(0, sfv.size() - 1);
			std::swap(sfv[i], sfv[j]); // 随机交换两个flavor的位置

			sfv_reduced.emplace_back(sfv.begin()->first, 1);
			for(size_t sfv_i = 1; sfv_i < sfv.size(); ++sfv_i)
				if(sfv[sfv_i] == sfv[sfv_i - 1])  ++sfv_reduced.back().second;
				else sfv_reduced.emplace_back(sfv[sfv_i].first, 1);

			servers = std::move(first_fit(sfv_reduced));
			cur_deploy_ratio = get_deploy_ratio(solution_flavor, servers);

			double dc = last_deploy_ratio - cur_deploy_ratio;
			if(std::min(1.0, exp(-dc / T)) > Rand.Random_Real(0, 1)) { // 接受
//				printf("%f\n", std::min(1.0, exp(-dc / T)));
				last_deploy_ratio = cur_deploy_ratio;
			} else std::swap(sfv[i], sfv[j]); // 换回去

			if(best_deploy_ratio < 0 || best_deploy_ratio < cur_deploy_ratio) {
				best_deploy_ratio = cur_deploy_ratio;
				best_servers = servers;
			}

		}
		T *= delta;
	}
	solution_server = best_servers;
}


char* get_result(std::map<string, int>& solution_flavor, std::vector<server>& solution_server) {
	char buffer[20 * 20];
	char *pr = result, *pb = buffer;
	int flavor_count = 0;
	for(const auto & flv: solution_flavor)
		flavor_count += flv.second;

	// flavors num
	snprintf(buffer, sizeof(buffer), "%d\n", flavor_count);

	pb = buffer; while(*pb && (*pr++ = *pb++));
	for(const auto & flv: solution_flavor) {
		snprintf(buffer, sizeof(buffer), "%s %d\n", flv.first.c_str(), flv.second);
		pb = buffer; while(*pb && (*pr++ = *pb++));
	}

	std::map<std::string, std::vector<int>> server_result;
	for(size_t i = 0; i < solution_server.size(); ++i) {
		server_result[solution_server[i].info->name].push_back(i);
	}

	for(const auto& sr: server_result) {
		// specify server num
		snprintf(buffer, sizeof(buffer), "\n%s %ld", sr.first.c_str(), sr.second.size());
		pb = buffer; while(*pb && (*pr++ = *pb++));
		for(size_t i = 0; i < sr.second.size(); ++i) {
			snprintf(buffer, sizeof(buffer), "\n%s-%ld", sr.first.c_str(), i + 1);
			pb = buffer; while(*pb && (*pr++ = *pb++));
			for(const auto& flv: solution_server[sr.second[i]].flavors) {
				snprintf(buffer, sizeof(buffer), " %s %d", flv.first.c_str(), flv.second);
				pb = buffer; while(*pb && (*pr++ = *pb++));
			}
		}
		snprintf(buffer, sizeof(buffer), "\n");
		pb = buffer; while(*pb && (*pr++ = *pb++));
	}

	return result;
}




//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{

	Signal(SIGALRM, timeOutHandler);
	// 定时器
	alarm(88);

	int line = 0;
	std::map<string, int> solution_flavor; // name: count
	std::vector<server> solution_server;
	line += read_servers_info(info);

	line += read_flavors_info(info + line);

	predict_interval.first = datetime(info[line++]);
	predict_interval.second = datetime(info[line]);
	if(predict_interval.second.time.hour >= 23) {
		predict_interval.second.date += 1;
		predict_interval.second.time = Time(0, 0, 0);
	}
	during_days = predict_interval.second.date - predict_interval.first.date;


	/*** 部署测试begin ***/
	/*
	solution_flavor = std::move(read_deploy_test_cases(data, data_num));
	for(const auto & sf: solution_flavor)
		printf("%s: %d\n", sf.first.c_str(), sf.second);

//	std::vector<std::pair<string, int>> sfv;
//	for(const auto & sf: solution_flavor)
//		for(size_t i = 0; i < sf.second; ++i)
//		sfv.emplace_back(sf.first, 1);
//	solution_server = std::move(first_fit(sfv));

	deploy_server_SA(solution_flavor, solution_server, 1, 1e-2, 1e-5, 0.9999);
	*/

	/*** 部署测试end ***/

	/*** 正赛begin ***/
	flavors = std::move(read_flavors(data, data_num));

//	interval_predict(solution_flavor);
//	linear_regression_predict(solution_flavor);
//	polynomial_regression_predict(solution_flavor);
	exponential_smoothing_predict(solution_flavor);
//	exponential_smoothing_predict_by_day(solution_flavor);
//	bp_predict(solution_flavor);
//	avg_predict(solution_flavor);

	for(const auto & sf: solution_flavor)
		printf("%s: %d\n", sf.first.c_str(), sf.second);

	deploy_server_SA(solution_flavor, solution_server, 1, 1e-2, 1e-5, 0.9999);
//	deploy_server_SA_tradeoff(solution_flavor, solution_server, 1, 1.0, 0.001, 0.9999);
//	deploy_server_fit(solution_flavor, solution_server);

	// 利用率
//	assert(get_deploy_ratio(solution_flavor, solution_server) > 0.90);

	/*** 正赛end ***/

	/** 测试begin **/
	/*
	BP_Network bp_network({1, 4, 1});
	vector<pair<vector<double>, vector<double>>> mini_batch({
		make_pair<vector<double>, vector<double>>({3}, {1}),
		make_pair<vector<double>, vector<double>>({2}, {2}),
		make_pair<vector<double>, vector<double>>({1}, {3})
	});
	bp_network.SGD(mini_batch, 30, 3, 3.0);
	for(auto y: bp_network.feedforward({7})) {
		printf("%lf ", y);
	}
	puts("");
	 */

	/** 测试end **/


	for(const auto &srv: solution_server) {
		printf("%18s %5.3lf%%(%6.3lf%% %6.3lf%%)\n",
		       srv.info->name.c_str(),
		       srv.get_ratio() * 100.0, srv.get_cpu_usage_ratio() * 100, srv.get_mem_usage_ratio() * 100
		);
	}
	puts("");
	printf("%f(avg = %f)\n", get_deploy_ratio(solution_flavor, solution_server), get_servers_avg_usage_ratio(solution_server));



	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(get_result(solution_flavor, solution_server), filename);
}
