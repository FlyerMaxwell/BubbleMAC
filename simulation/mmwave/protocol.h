#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "trace.h"
//#include "blossom.h"
#include "weighted_blossom.h"
#include "parameters.h"

int final_bi_protocol_random(struct Region *region);
int IEEE_AD(struct Region *region);
int COMNET_V2(struct Region *region);
int JSAC_17(struct Region *region);
int final_bi_optimal(struct Region *region);

int COMNET_ND(struct Region *region, int circ_num);
int Random_Neighbour_Discovery(struct Region *region, int circ_num);
int IEEE_Neighbour_Discovery(struct Region *region, int circ_num);

int random_Match(struct Region *region, int circ_num);
int COMNET_Match(struct Region *region, int circ_num);
int simple_weighted_Match(struct Region *region, int circ_num);

int compare_rate(struct neighbour_car *aNeigh, struct neighbour_car *bNeigh);
int compare_weight(struct neighbour_car *aNeigh, struct neighbour_car *bNeigh);
void remove_car_from_NeighList(struct Duallist *duallist, struct vehicle *aCar);
int data_exchange_phase(struct Region *region, int data_change_time);
int AD_exchange_data(struct Region *region, int data_change_time);
int init_all_car(struct Region *region);
#endif
