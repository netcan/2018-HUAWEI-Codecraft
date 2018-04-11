#include "predict.h"

int during_days = 0;

std::string init_f;

std::pair<datetime, datetime> predict_interval;
char result[20 * 20 * 10000];

void interval_predict(std::map<string, int>& solution_flavor) {
	for(const auto &f: predict_flavors_info) { // predict per vm
		int	s = get_interval_flavors_count(
			f.first, predict_interval.first.date + (-during_days), during_days
		);
		solution_flavor[f.first] = s;
	}
}

void xjb_predict(std::map<string, int>& solution_flavor) {
	char *d[MAX_DATA_NUM]; int dln = read_file(d, MAX_DATA_NUM, init_f.c_str());
	std::map<string, std::vector<flavor>> flvs = read_flavors(d, dln);
	for(const auto &f: predict_flavors_info) { // predict per vm
		string vm_name = f.first;
		int s = 0;
		flavor flavor_end_date(datetime(predict_interval.first.date + during_days + 1));
		for(std::vector<flavor>::iterator f_it = std::lower_bound(
				flvs[vm_name].begin(), flvs[vm_name].end(), flavor(datetime(predict_interval.first.date))
		); f_it < flvs[vm_name].end() && *f_it < flavor_end_date; ++f_it, ++s);
		solution_flavor[f.first] = s;
	}
	release_buff(d, dln);
}


void linear_regression_predict(std::map<string, int>& solution_flavor) {
	linear_regression LinearReg;
	for(const auto &f: predict_flavors_info) { // predict per vm
		std::vector<int> Y_count = get_per_flavor_per_interval_count(f.first);
		std::vector<int> X;
		int x = 1;
		for(size_t i = 0; i < Y_count.size(); ++i) {
			Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);
			X.push_back(x++);
		}

		LinearReg.train(X, Y_count);

		solution_flavor[f.first] = int(lround(LinearReg.predict(x) -
		                                      LinearReg.predict(x - 1)));

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
		std::vector<int> Y_count = get_per_flavor_per_interval_count(f.first);
		std::vector<int> X;
		int x = 1;
		for(size_t i = 0; i < Y_count.size(); ++i) {
//			Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);
			X.push_back(x++);
		}

		PolyReg.train(X, Y_count, 1e-1, -1);
//		solution_flavor[f.first] = std::max(int(lround(PolyReg.predict(x) - PolyReg.predict(x - 1))), 0);
		solution_flavor[f.first] = std::max(int(lround(PolyReg.predict(x))), 0);

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
	for(int alpha = 0; alpha <= max_alpha; ++alpha) {
		exponential_smoothing ExpSmooth(alpha * 1.0 / max_alpha);
		for (const auto &f: predict_flavors_info) { // predict per vm
			std::vector<int> Y_count = get_per_flavor_per_interval_count(f.first);
			real_solution_flavor[f.first] = Y_count.back(); Y_count.pop_back();
//			for(size_t i = 0; i < Y_count.size(); ++i)
//				Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);

			ExpSmooth.train(Y_count);
//			predict_solution_flavor[f.first] = int(lround(ExpSmooth.predict(Y_count.back()))) - Y_count.back();
			predict_solution_flavor[f.first] = int(lround(ExpSmooth.predict(Y_count.back())));
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
	exponential_smoothing ExpSmooth(0.60);
//	exponential_smoothing ExpSmooth(0.99);
	for(const auto &f: predict_flavors_info) { // predict per vm
		std::vector<int> Y_count = get_per_flavor_per_interval_count(f.first);
//		for(size_t i = 0; i < Y_count.size(); ++i)
//			Y_count[i] = Y_count[i] + (i > 0 ? Y_count[i-1] : 0);
		ExpSmooth.train(Y_count);
//		solution_flavor[f.first] = std::max(int(lround(ExpSmooth.predict(Y_count.back()))) - Y_count.back(), 0);
		solution_flavor[f.first] = std::max(0, int(lround(ExpSmooth.predict(Y_count.back()))));
	}

}

std::vector<server> first_fit(const std::vector<std::pair<string, int>>& sfv,
                              std::vector<std::map<string, int>>& solution_server) {
	// solution_server is empty
	std::vector<server> servers;
	solution_server.emplace_back();
	servers.emplace_back();
	// first fit
	for(const auto & sf: sfv) { // sfv是排过序了的版本
		const string& vm_name = sf.first;
		int vm_count = sf.second;
		const flavor_info& flv = predict_flavors_info[vm_name];
		for (int f_count = 0; f_count < vm_count; ) {
			for(size_t i = 0; i < servers.size(); ++i) { // first fit
				if (flv <= servers[i]) {
					int vnum = servers[i] / flv;
					if(vnum + f_count > vm_count) vnum = vm_count - f_count;
					f_count += vnum;
					servers[i] -= flv * vnum;

					if(solution_server[i].find(vm_name) != solution_server[i].end())
						solution_server[i][vm_name] += vnum;
					else solution_server[i][vm_name] = vnum;
					break;
				} else if(i == servers.size() - 1) { // new server
					solution_server.emplace_back();
					servers.emplace_back();
				}
			}
		}
	}
	return servers;
}

void deploy_server_SA_fill(std::map<string, int> &solution_flavor,
                      std::vector<std::map<string, int>>& solution_server,
                      int inner_loop, double T, double delta) {

	// 退火
	std::vector<std::pair<string, int>> sfv;

//	for(const auto & sf: solution_flavor) sfv.push_back(sf);

	for(const auto & sf: solution_flavor)
		for(int i = 0; i < sf.second; ++i) sfv.push_back(std::make_pair(sf.first, 1));

	std::vector<std::map<string, int>> SA_solution_server;
	std::vector<server> servers = std::move(first_fit(sfv, SA_solution_server));
	int servers_num = servers.size();

	solution_server = SA_solution_server;

	double last_deploy_ratio = get_deploy_ratio(solution_flavor, SA_solution_server),
			cur_deploy_ratio = 0.0, best_deploy_ratio = -1;
	bool action = 0; // 1交换位置，0加flavor

	while(T > 0.1) {
		for(int loop = 0; loop < inner_loop; ++loop) {
			SA_solution_server.clear(); // 记得清空
			int i, j, k;
			action = Rand.Random_Int(1, 100) > 20; // 比率

			if(action) {
				i = Rand.Random_Int(0, sfv.size() - 1), j = Rand.Random_Int(0, sfv.size() - 1);
				while(i == j) i = Rand.Random_Int(0, sfv.size() - 1);
				std::swap(sfv[i], sfv[j]); // 随机交换两个flavor的位置
			} else {
				k = Rand.Random_Int(0, sfv.size() - 1); // 选一个flavor加或者减
//				++sfv[k].second;
				sfv.push_back(std::make_pair(sfv[k].first, 1));
			}

			servers = std::move(first_fit(sfv, SA_solution_server));
			cur_deploy_ratio = get_deploy_ratio(sfv, SA_solution_server);

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
				solution_server = std::move(SA_solution_server);
			}

		}
		T *= delta;
	}

	for(auto& sf: solution_flavor) sf.second = 0;

	for(const auto& sf: sfv) {
//		solution_flavor[sf.first] = sf.second;
		++solution_flavor[sf.first];
	}

}

void deploy_server_SA(std::map<string, int> &solution_flavor,
                      std::vector<std::map<string, int>>& solution_server,
                      int inner_loop, double T, double delta) {
	// 退火
	std::vector<std::pair<string, int>> sfv;
	for(const auto & sf: solution_flavor)
		for(int i = 0; i < sf.second; ++i) sfv.push_back(std::make_pair(sf.first, 1));

	std::vector<std::map<string, int>> SA_solution_server;
	std::vector<server> servers = std::move(first_fit(sfv, SA_solution_server)), best_servers;
	solution_server = SA_solution_server;

	double last_server_num = servers.size() - 1.0 + (target == CPU ? servers.back().get_cpu_usage_ratio(): servers.back().get_mem_usage_ratio()),
			cur_server_num = 0.0, best_server_num = -1;

	while(T > 0.1) {
		for(int loop = 0; loop < inner_loop; ++loop) {
			SA_solution_server.clear(); // 记得清空
			int i = Rand.Random_Int(0, sfv.size() - 1),
				j = Rand.Random_Int(0, sfv.size() - 1);
			while(i == j) i = Rand.Random_Int(0, sfv.size() - 1);
			std::swap(sfv[i], sfv[j]); // 随机交换两个flavor的位置

			servers = std::move(first_fit(sfv, SA_solution_server));
			cur_server_num = servers.size() - 1.0 + servers.back().get_usage_ratio(target == CPU);

			double dc = cur_server_num - last_server_num;
			if(std::min(1.0, exp(-dc / T)) > Rand.Random_Real(0, 1)) { // 接受
//				printf("%f\n", std::min(1.0, exp(-dc / T)));
				last_server_num = cur_server_num;
			} else std::swap(sfv[i], sfv[j]); // 换回去

			if(best_server_num < 0 || best_server_num > cur_server_num) {
				best_server_num = cur_server_num;
				solution_server = std::move(SA_solution_server);
				best_servers = std::move(servers);
			}

		}
		T *= delta;
	}
	printf("best_server_num = %ld\n", best_servers.size());
	for(const auto& srv: best_servers)
		printf("%10.4f%%", srv.get_usage_ratio(target == CPU) * 100);
	puts("");

	// 填充
	/****
	 * CPU:
	 * flavor MEM/CPU < server MEM/CPU => flavor.MEM * server.CPU < flavor.CPU * server.MEM
	 * MEM:
	 * flavor MEM/CPU > server MEM/CPU => flavor.MEM * server.CPU > flavor.CPU * server.MEM
	 ****/

	/*
	std::vector<string> fill_flavors_name(best_servers.size());

	for (size_t j = 0; j < best_servers.size(); ++j) {
		int fill_vm_idx = 0, product = -1;
		for (size_t i = 0; i < sfv.size(); ++i) {
			const flavor_info &fifo = predict_flavors_info[sfv[i].first];
			int tmp_product_left = fifo.mem_size * best_servers[j].remain_cpu_count,
				tmp_product_right = fifo.cpu_count * best_servers[j].remain_mem_size;
			switch (target) {
				case CPU:
					if (tmp_product_left < tmp_product_right && (product == -1 || tmp_product_left > product)) {
						product = tmp_product_left;
						fill_vm_idx = i;
					}
					break;
				case MEM:
					if (tmp_product_left > tmp_product_right && (product == -1 || tmp_product_left < product)) {
						product = tmp_product_left;
						fill_vm_idx = i;
					}
					break;
			}
		}
		fill_flavors_name[j] = sfv[fill_vm_idx].first;
	}

//	printf("flavor mem/cpu = %f, server mem/cpu = %f\n", flv.mem_size * 1.0 / flv.cpu_count, server::mem_size * 1.0 / server::cpu_count);

	for (size_t i = 0; i < best_servers.size(); ++i) {
		const string &fill_vm_name = fill_flavors_name[i];
		const auto &flv = predict_flavors_info[fill_vm_name];
		printf("flavor mem/cpu = %f, server mem/cpu = %f\n", flv.mem_size * 1.0 / flv.cpu_count,
		       best_servers[i].remain_mem_size * 1.0 / best_servers[i].remain_cpu_count);

		while (flv <= best_servers[i]) {
			best_servers[i] -= flv;
			++solution_flavor[fill_vm_name];
			if (solution_server[i].find(fill_vm_name) != solution_server[i].end())
				++solution_server[i][fill_vm_name];
			else solution_server[i][fill_vm_name] = 1;
		}
	}
	 */


}



void fill_deploy_server(std::map<string, int> &solution_flavor, std::vector<std::map<string, int>> &solution_server) {
	std::vector<std::pair<string, int>> sfv;
	for(const auto & sf: solution_flavor) sfv.push_back(sf);

	// sfv排序
	std::sort(sfv.begin(), sfv.end(), [=](const std::pair<string, int> &lhs, const std::pair<string, int> &rhs) -> bool {
		const auto & flv_inf_lhs = predict_flavors_info[lhs.first],
					flv_inf_rhs = predict_flavors_info[rhs.first];
		if(target == CPU) {
			if(flv_inf_lhs.cpu_count == flv_inf_rhs.cpu_count)
				return flv_inf_lhs.mem_size < flv_inf_rhs.mem_size;
			return flv_inf_lhs.cpu_count < flv_inf_rhs.cpu_count;
		}
		else {
			if(flv_inf_lhs.mem_size == flv_inf_rhs.mem_size)
				return flv_inf_lhs.cpu_count < flv_inf_rhs.cpu_count;
			return flv_inf_lhs.mem_size < flv_inf_rhs.mem_size;
		}
	});


	std::vector<server> servers = std::move(first_fit(sfv, solution_server));

	// +C，提高希尔系数（精度）

//	for(const auto &sf: sfv) {
	for(int i = 0; i < sfv.size(); ++i) {
		string vm_name = sfv[i].first;
		int c = 5;
		const auto & flv = predict_flavors_info[vm_name];
		for(int i = 0; i < servers.size(); ++i) {
			while(flv <= servers[i] && c) {
				servers[i] -= flv;
				++solution_flavor[vm_name];
				++solution_server[i][vm_name];
				--c;
			}
		}
	}

	std::map<std::string, int> ff_solution_flavor = solution_flavor;
	std::vector<std::map<string, int>> ff_solution_server = solution_server;
	std::vector<server> ff_servers = servers;

	// 填充操作
//	string fill_vm_name = sfv[Rand.Random_Int(0, sfv.size() - 1)].first;
	double best_deploy_ratio = -1;
	for(auto fv: sfv) {
		string fill_vm_name = fv.first;
		const auto &flv = predict_flavors_info[fill_vm_name];
		std::map<std::string, int> fill_solution_flavor = ff_solution_flavor;
		std::vector<std::map<string, int>> fill_solution_server = ff_solution_server;
		std::vector<server> fill_servers = ff_servers;

		for (size_t i = 0; i < servers.size(); ++i) {
			while (flv <= fill_servers[i]) {
				fill_servers[i] -= flv;
				++fill_solution_flavor[fill_vm_name];
				if (fill_solution_server[i].find(fill_vm_name) != fill_solution_server[i].end())
					++fill_solution_server[i][fill_vm_name];
				else fill_solution_server[i][fill_vm_name] = 1;
			}
		}

		double deploy_ratio = get_deploy_ratio(fill_solution_flavor, fill_solution_server);
		if(deploy_ratio > best_deploy_ratio) {
			best_deploy_ratio = deploy_ratio;
			servers = fill_servers;
			solution_flavor = fill_solution_flavor;
			solution_server = fill_solution_server;
		}
		break;
	}
	printf("%lf\n", best_deploy_ratio);

}


char* get_result(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server) {
	char buffer[20 * 20];
	char *pr = result, *pb = buffer;
	int flavor_count = 0;
	for(const auto & flv: solution_flavor)
		flavor_count += flv.second;

	snprintf(buffer, sizeof(buffer), "%d\n", flavor_count);

	pb = buffer; while(*pb && (*pr++ = *pb++));
	for(const auto & flv: solution_flavor) {
		snprintf(buffer, sizeof(buffer), "%s %d\n", flv.first.c_str(), flv.second);
		pb = buffer; while(*pb && (*pr++ = *pb++));
	}

	snprintf(buffer, sizeof(buffer), "\n%ld\n", solution_server.size());
	pb = buffer; while(*pb && (*pr++ = *pb++));

	for(size_t i = 0; i < solution_server.size(); ++i) {
		snprintf(buffer, sizeof(buffer), "%ld", i + 1);
		pb = buffer; while(*pb && (*pr++ = *pb++));
		for(const auto & flv: solution_server[i]) {
			snprintf(buffer, sizeof(buffer), " %s %d", flv.first.c_str(), flv.second);
			pb = buffer; while(*pb && (*pr++ = *pb++));
		}
		*pr++ = '\n';
	}

	return result;
}




//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	int line = 0;
	std::map<string, int> solution_flavor; // vm_name: count
	std::vector<std::map<string, int>> solution_server;

	sscanf(info[line], "%d %d %d", &server::cpu_count, &server::mem_size, &server::disk_size);
	server::mem_size *= 1024; // covert to MB
	line = read_flavors_info(info);

	info[line][3] = 0;
	target = strcmp(info[line], "CPU") == 0 ? CPU:MEM;
	line += 2;

	predict_interval.first = datetime(info[line++]);
	predict_interval.second = datetime(info[line]);
	during_days = predict_interval.second.date - predict_interval.first.date;


	/*** 部署测试begin ***/

	/*
	data[2][3] = 0;
	target = strcmp(data[2], "CPU") == 0 ? CPU:MEM;
	solution_flavor = std::move(read_deploy_test_cases(data, data_num));
	deploy_server_SA(solution_flavor, solution_server, 1, 100.0, 0.9999);
	get_deploy_ratio(solution_flavor, solution_server);
	 */

	/*** 部署测试end ***/

	/*** 正赛begin ***/

	flavors = std::move(read_flavors(data, data_num));

	interval_predict(solution_flavor);
//	xjb_predict(solution_flavor);
//	linear_regression_predict(solution_flavor);
//	polynomial_regression_predict(solution_flavor);
//	exponential_smoothing_predict(solution_flavor);

//	fill_deploy_server(solution_flavor, solution_server);
//	deploy_server_SA(solution_flavor, solution_server, 1, 100.0, 0.9999);
	deploy_server_SA_fill(solution_flavor, solution_server, 1, 100.0, 0.9999);

	get_deploy_ratio(solution_flavor, solution_server);

	/*** 正赛end ***/


	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(get_result(solution_flavor, solution_server), filename);
}
