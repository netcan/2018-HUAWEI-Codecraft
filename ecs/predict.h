#ifndef ECS_PREDICT_H
#define ECS_PREDICT_H

#include "lib_io.h"
#include "server.h"
#include "flavor.h"
#include "datetime.h"
#include <stdio.h>
#include <string.h>

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);

extern std::pair<datetime, datetime> predict_interval;

#endif
