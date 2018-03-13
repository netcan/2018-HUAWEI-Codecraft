#include "predict.h"

enum {
	CPU, MEM
} target;

std::pair<datetime, datetime> predict_interval;
char result[20 * 20 * 10000];

void interval_predict(std::map<string, int>& solution_flavor) {
	int during_days = predict_interval.second.date - predict_interval.first.date;
	for(const auto &f: flavors_info) { // predict per vm
		int	s = get_interval_flavors_count(
				f.first, predict_interval.first.date + (-during_days), during_days
		);
		solution_flavor[f.first] = s;
	}
}


void deploy_server(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server) {
	std::vector<std::pair<string, int>> sfv;
	for(const auto & sf: solution_flavor) sfv.push_back(sf);

	std::sort(sfv.begin(), sfv.end(), [=](const std::pair<string, int> &lhs, const std::pair<string, int> &rhs) -> bool {
		const auto & flv_inf_lhs = flavors_info[lhs.first],
					flv_inf_rhs = flavors_info[rhs.first];
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

	for(const auto & sf: sfv) { // sfv是排过序了的版本
		string vm_name = sf.first;
		int vm_count = sf.second;
		const flavor_info& flv = flavors_info[vm_name];
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

	string fill_vm_name = sfv[rand() % sfv.size()].first;
	const auto & flv = flavors_info[fill_vm_name];

	for (size_t i = 0; i < servers.size(); ++i) {
		while (flv <= servers[i]) {
			servers[i] -= flv;
			++solution_flavor[fill_vm_name];
			if (solution_server[i].find(fill_vm_name) != solution_server[i].end())
				++solution_server[i][fill_vm_name];
			else solution_server[i][fill_vm_name] = 1;
		}
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

void show_ratio(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server) {
#ifdef _DEBUG
	int r = 0, R;
	for(const auto& sf: solution_flavor)
		r += (target == CPU?
		      flavors_info[sf.first].cpu_count:
		      flavors_info[sf.first].mem_size) * sf.second;
	R = int(solution_server.size()) * (target == CPU?
	                              server::cpu_count:
	                              server::mem_size);
	printf("%d/%d = %.2f\n", r, R, r * 1.0/R);
#endif
}


//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	srand(time(0));

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

	read_flavors(data, data_num);
	interval_predict(solution_flavor);
	deploy_server(solution_flavor, solution_server);
	show_ratio(solution_flavor, solution_server);


	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(get_result(solution_flavor, solution_server), filename);
}
