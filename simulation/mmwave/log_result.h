#ifndef LOG_RESULT_H
#define LOG_RESULT_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "trace.h"
#include "parameters.h"
void log_ND(struct Region *region, int pro, double ratio);
void log_rate(struct Region *region);
void log_neighbour(struct Region *region);
void write_CDF_result(struct Region *region);
void write_total_link_result(int total_link[], int num, int pro, int total_edge_num);
void write_sector_num_result(int avg_time[], int num);
void log_metric1(struct Region *region, int pro);
void log_metric2(struct Region *region, int pro);
void log_metric3(struct Region *region, int pro);
void log_metric4(struct Region *region, int pro);
void log_rate(struct Region *region);
void log_throughput(struct Region *region, int pro);
#endif
