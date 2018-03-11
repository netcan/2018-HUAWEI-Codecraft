#include "predict.h"

enum {
	CPU, MEM
} target;

std::pair<datetime, datetime> predict_interval;

//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	int line = 0;
	sscanf(info[line], "%d %d %d", &srv.cpu_count, &srv.mem_size, &srv.disk_size);
	line = read_flavors_info(info);

	info[line][3] = 0;
	target = strcmp(info[line], "CPU") == 0 ? CPU:MEM;
	line += 2;

	predict_interval.first = datetime(info[line++]);
	predict_interval.second = datetime(info[line]);


	// 需要输出的内容
	char * result_file = (char *)"17\n\n0 8 0 20";

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(result_file, filename);
}
