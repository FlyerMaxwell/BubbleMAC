#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "trace.h"
#include "parameters.h"
#include "log_result.h"

int init_simulation(struct Region *region);
int generate_car(struct Region *region);
int handle_neighbour(struct Region *region);
void update_trans_rate(struct Region *region);
void update_vehicle_info(struct Region *region);
void calculate_rate(struct Region *region);
int show_graph_degree(struct Region *region);

double calculate_dis(double x1, double y1, double x2, double y2);
double calculate_angle_diff(double angle1, double angle2);
int car_legal(struct Region *region, struct vehicle *aCar, struct Cell *aCell); 

#endif
