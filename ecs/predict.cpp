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

void exponential_smoothing_predict(std::map<string, int>& solution_flavor, double noiseK = 3.0, double alpha = 0.91, double c = 1.0) {
//	exponential_smoothing ExpSmooth(cv_expontential_smoothing_predict());
	// 0.5, 1.8
	exponential_smoothing ExpSmooth(alpha);
//	exponential_smoothing ExpSmooth(0.99);
	for(const auto &f: predict_flavors_info) { // predict per vm
//		std::vector<int> by_day = get_per_flavor_count_by_interval(f.first, 1);
		// print per vm
		printf("%s\n", f.first.c_str());

		std::vector<int> by_day = denoising(get_per_flavor_count_by_interval(f.first, 1), noiseK); // 去噪
		std::vector<int> Y_count = denoising(merge_cnt_day_by_interval(by_day, during_days), noiseK);

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


// c: 预测*倍数
void exponential_smoothing_predict_by_day(std::map<string, int>& solution_flavor, double noiseK = 3.0, double alpha = 0.3, double c = 1.0) {
	// 0.3 best
	exponential_smoothing ExpSmooth(alpha);
	for(const auto &f: predict_flavors_info) { // predict per vm
		// best: k = 3.0
		std::vector<int> by_day = std::move(denoising(get_per_flavor_count_by_interval(f.first, 1), noiseK)); // 去噪

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

void lwlr_predict(std::map<string, int>& solution_flavor, double noiseK = 3.0, double k = 9.5, double c = 1.0) { // 去噪参数，权重参数k
	LWLR lwlr;
	for(const auto &f: predict_flavors_info) { // predict per vm
		std::vector<int> cnt_by_day = std::move(denoising(get_per_flavor_count_by_interval(f.first, 1), noiseK)); // 去噪, Y
		std::vector<int> days(cnt_by_day.size()); // X
		int day;
		for(day = 0; day < days.size(); ++day) days[day] = day;

		double cnt = 0;
		day += predict_interval.first.date - train_end_time.date - 1;


		for(int d = 0; d < during_days; ++d, ++day) {
//			printf("d=%d\n", day);
			cnt += lwlr.predict(days, cnt_by_day, day, k);
		}

		solution_flavor[f.first] = std::max(int(lround(cnt * c)), 0);
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
		bool GAP = (predict_interval.first.date - train_end_time.date >= 7);

		double avg = 0;
		for(int i = 0; i < days; ++i)
			avg += cnt_by_day[cnt_by_day.size() - 1 - i];
		avg /= days;

		double cnt = avg * during_days;
		double p = 1.0;

		if(!GAP) {
			p = 0.53;
		} else {
			p = 1.57;
		}


		cnt *= p;


//		cnt += Rand.Random_Int(0, 5);

		solution_flavor[f.first] = int(lround(cnt));
	}

}

std::vector<server> first_fit(const std::vector<std::pair<string, int>>& sfv, size_t *low_ratio_srv_loc = NULL) {
	// solution_server is empty
	std::vector<server> servers;
	int flavor_k_num = 0;
	double low_ratio = -1;
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

		if(low_ratio_srv_loc && (low_ratio < 0 || low_ratio > best_srv.get_ratio())) {
			low_ratio = best_srv.get_ratio();
			*low_ratio_srv_loc = servers.size();
		}

		servers.push_back(best_srv);

		if(i == best_k) flavor_k_num += cur_k_num;
		else flavor_k_num = cur_k_num;
		i = best_k;
	}

	return servers;
}

double get_flavors_mem_cpu_ratio(const std::map<string, int> & flavors_num) {
	int cpu = 0, mem = 0;
	for(const auto &fn: flavors_num) {
		const flavor_info& flv = predict_flavors_info[fn.first];
		cpu += flv.cpu_count * fn.second;
		mem += flv.mem_size * fn.second;
	}
	return mem * 1.0 / cpu;
}

bool check_remain_flavors_full(const std::map<string, int> & flavors_num) {
	bool full = false;
	for(const auto &fn: flavors_num)
		full = full || fn.second;
	return full;
}

void deploy_server_fit(const std::map<string, int> &solution_flavor, std::vector<server> &solution_server) {
	// General(0), High-Memory(1), High-Performance(2)
	auto remain_flavors = solution_flavor;
	bool empty = false;
	double mem_cpu_ratio = get_flavors_mem_cpu_ratio(remain_flavors);
	while(check_remain_flavors_full(remain_flavors)) {

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
	for(const auto & sf: solution_flavor) {
		sfv_reduced.push_back(std::make_pair(sf.first, sf.second));
		for (int i = 0; i < sf.second; ++i) sfv.push_back(std::make_pair(sf.first, 1));
	}

	size_t low_ratio_srv_loc = 0;
	std::vector<server> servers = std::move(first_fit(sfv_reduced)), best_servers;
	solution_server = servers;

	double last_deploy_ratio = get_deploy_ratio(solution_flavor, servers),
			cur_deploy_ratio = 0.0, best_deploy_ratio = -1;
	int accept = 0, reject = 0;

	printf("begin T: %lf\n", T);

	while(runing && T > Tmin) {
		for(int loop = 0; loop < inner_loop && runing; ++loop) {
			servers.clear(); // 记得清空
			sfv_reduced.clear();
			int i = Rand.Random_Int(0, sfv.size() - 1),
					j = Rand.Random_Int(0, sfv.size() - 1);
			while(sfv[i].first == sfv[j].first) i = Rand.Random_Int(0, sfv.size() - 1);
			std::swap(sfv[i], sfv[j]); // 随机交换两个flavor的位置

			sfv_reduced.emplace_back(sfv.begin()->first, 1);
			for(size_t sfv_i = 1; sfv_i < sfv.size(); ++sfv_i) {
				if (sfv[sfv_i] == sfv[sfv_i - 1]) ++sfv_reduced.back().second;
				else sfv_reduced.emplace_back(sfv[sfv_i].first, 1);
			}
			size_t cur_low_ratio_srv_loc = 0;

			servers = std::move(first_fit(sfv_reduced, &cur_low_ratio_srv_loc));
			cur_deploy_ratio = get_deploy_ratio(solution_flavor, servers);

			double dc = last_deploy_ratio - cur_deploy_ratio;
			if(std::min(1.0, exp(-dc / T)) > Rand.Random_Real(0, 1)) { // 接受
//				printf("%f\n", std::min(1.0, exp(-dc / T)));
				last_deploy_ratio = cur_deploy_ratio;
				++accept;
			} else {
				std::swap(sfv[i], sfv[j]); // 换回去
				++reject;
			}

			if(best_deploy_ratio < 0 || best_deploy_ratio < cur_deploy_ratio) {
				best_deploy_ratio = cur_deploy_ratio;
				best_servers = servers;
				low_ratio_srv_loc = cur_low_ratio_srv_loc;
			}

		}
		T *= delta;
	}
	printf("end T: %lf\n", T);
	printf("accept = %d reject = %d\n", accept, reject);

	solution_server = std::move(best_servers);

	// 对最小的srv进行填充、删除操作
	auto &low_srv = solution_server[low_ratio_srv_loc];

	if(solution_server.size() > 1 && low_srv.get_ratio() < 0.8) {// remove
		printf("remove: %lf\n", low_srv.get_ratio());
		for(const auto &flvs: low_srv.flavors)
			solution_flavor[flvs.first] -= flvs.second;
		solution_server.erase(solution_server.begin() + low_ratio_srv_loc);
	}
	else { // fill
		printf("fill: %lf\n", low_srv.get_ratio());
		flavor_info best_flv;
		double best_ratio = -1;
		for(const auto& sf: solution_flavor) {
			const auto &flv = predict_flavors_info[sf.first];
			if(flv <= low_srv) {
				server srv = low_srv;
				int flv_num = srv / flv;
				srv.place_flavor(flv, flv_num);
				double ratio = srv.get_ratio();
				if(best_ratio < ratio) {
					best_ratio = ratio;
					best_flv = flv;
				}
			}
		}
		if(best_ratio > 0) {
			int flv_num = low_srv / best_flv;
			low_srv.place_flavor(best_flv, flv_num);
			solution_flavor[best_flv.name] += flv_num;
		}
		printf("fill end: %lf\n", low_srv.get_ratio());
	}

	// 对其余的进行填充操作
	/*
	for(auto &srv: solution_server) {
		printf("fill: %lf\n", srv.get_ratio());
		flavor_info best_flv;
		double best_ratio = -1;
		for(const auto& sf: solution_flavor) {
			const auto &flv = predict_flavors_info[sf.first];
			if(flv <= srv) {
				server _srv = srv;
				int flv_num = _srv / flv;
				_srv.place_flavor(flv, flv_num);
				double ratio = _srv.get_ratio();
				if(best_ratio < ratio) {
					best_ratio = ratio;
					best_flv = flv;
				}
			}
		}
		if(best_ratio > 0) {
			int flv_num = srv / best_flv;
			srv.place_flavor(best_flv, flv_num);
			solution_flavor[best_flv.name] += flv_num;
		}
		printf("fill end: %lf\n", srv.get_ratio());
	}
	 */

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

//	deploy_server_SA(solution_flavor, solution_server, 1, 1e-2, 1e-5, 0.9995);
	deploy_server_fit(solution_flavor, solution_server);
	 */

	/*** 部署测试end ***/

	/*** 正赛begin ***/

	flavors = std::move(read_flavors(data, data_num));

//	interval_predict(solution_flavor);
//	linear_regression_predict(solution_flavor);
//	polynomial_regression_predict(solution_flavor);
//	exponential_smoothing_predict(solution_flavor);
//	exponential_smoothing_predict_by_day(solution_flavor); // 效果较好，70分
//	bp_predict(solution_flavor);
	avg_predict(solution_flavor);

	bool GAP = (predict_interval.first.date - train_end_time.date >= 7);

	/*
	if(! GAP) {
		lwlr_predict(solution_flavor, 3.0, 5.0); // 33.26
//		exponential_smoothing_predict_by_day(solution_flavor, 3.0, 0.3, 1.0); // 33.364
	} else {
		lwlr_predict(solution_flavor, 3.0, 9.5, 1.0); // 37.935
//		exponential_smoothing_predict_by_day(solution_flavor, 3.0, 0.22, 1.3); // 40.415
//		exponential_smoothing_predict(solution_flavor, 3.0, 0.91, 1.7); // 41.795
	}
	 */


	for(const auto & sf: solution_flavor)
		printf("%s: %d\n", sf.first.c_str(), sf.second);

	deploy_server_SA(solution_flavor, solution_server, 1, 1e-2, 1e-5, 0.9995);

//	deploy_server_SA_tradeoff(solution_flavor, solution_server, 1, 1.0, 0.001, 0.9999);
//	deploy_server_fit(solution_flavor, solution_server);

	// 利用率
//	assert(get_deploy_ratio(solution_flavor, solution_server) > 0.95);

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

	vector<vector<double>> tst = {
			{1,2,3},
			{4,5,4},
			{7,8,9},
	};
	vector<vector<double>> tst2 = {
			{4,5,4},
			{7,8,9},
			{1,2,3},
	};
	Matrix mat1(tst), mat2(tst2);
	(mat1 / 2).show();
	LWLR lwlr;
	vector<double> X, Y;
	for(int i = 0; i < 100; ++i) {
		X.push_back(i);
		Y.push_back(10 * sin(0.3 * i));
	}

	for(int i = 0; i < 100; ++i) {
		lwlr.predict(X, Y, i, 0.14);
	}
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
