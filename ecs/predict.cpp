#include "predict.h"

enum {
	CPU, MEM
} target;
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
		solution_flavor[f.first] = int(lround(ExpSmooth.predict(Y_count.back())));
	}

}

void deploy_server(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server) {
	std::vector<std::pair<string, int>> sfv;
	for(const auto & sf: solution_flavor) sfv.push_back(sf);

	// 排序
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

	std::vector<server> servers;
	solution_server.emplace_back();
	servers.emplace_back();

	// first fit
	for(const auto & sf: sfv) { // sfv是排过序了的版本
		const string& vm_name = sf.first;
		int vm_count = sf.second;
		const flavor_info& flv = predict_flavors_info[vm_name];
		for (int f_count = 0; f_count < vm_count; ++f_count) {
			for(size_t i = 0; i < servers.size(); ++i) { // first fit
				if (flv <= servers[i]) {
					servers[i] -= flv;
					if(solution_server[i].find(vm_name) != solution_server[i].end())
						++solution_server[i][vm_name];
					else solution_server[i][vm_name] = 1;
					break;
				} else if(i == servers.size() - 1) { // new server
					solution_server.emplace_back();
					servers.emplace_back();
				}
			}
		}
	}
	// sfv是排过序了的

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
		if(best_deploy_ratio < 0 || deploy_ratio > best_deploy_ratio) {
			best_deploy_ratio = deploy_ratio;
			servers = fill_servers;
			solution_flavor = fill_solution_flavor;
			solution_server = fill_solution_server;
		}
		break;
	}

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


double get_deploy_ratio(std::map<string, int> &solution_flavor, std::vector<std::map<string, int>> &solution_server) {
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

	flavors = std::move(read_flavors(data, data_num));

	interval_predict(solution_flavor);
//	xjb_predict(solution_flavor);
//	linear_regression_predict(solution_flavor);
//	polynomial_regression_predict(solution_flavor);
//	exponential_smoothing_predict(solution_flavor);

	deploy_server(solution_flavor, solution_server);
	get_deploy_ratio(solution_flavor, solution_server);


	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(get_result(solution_flavor, solution_server), filename);
}
