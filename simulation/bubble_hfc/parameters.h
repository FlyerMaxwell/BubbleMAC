#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <stdio.h>

#define false 0
#define true 1

/***********************parameters**********************/
extern int slot;
extern double BRAKE_COE;
extern int UpLocSlot;
extern int SlotPerFrame;
extern int NOT_OCCUPIED;
extern int SLOT_COLLISION;
extern int HEAD;
extern int TAIL;

extern int Car_Number;
extern int BEACON_NUM;
extern int BEACON_INTERVAL;
extern int safe_dis; //safe distance between two cars
extern int com_dis; //communication distance
extern int sector_num;
extern int sim_time;
extern int DATA_TIME;
extern double neighbour_angle;
extern int learning_cycle_num;
extern int separate_num;
extern int select_waste;

extern int timestamp;
extern int bi_num;
extern int lane_num;
extern int simulation_speed;
extern int traffic_density;

extern int magic_number;
extern int log_data;
extern int PACKET_NUM;
extern int SELECT_TIME;
extern int SELECT_NUM;
extern FILE *carinfo;

extern double min_beam_width;
extern double min_scan_interval;
extern double scan_alpha;
extern double scan_beta;
extern double scan_theta;
extern double angle_omega;
extern double angle_theta;
extern double radio_gain;
extern double noise_power;
extern int MAX_RATE;
/***********************parameters**********************/

#endif
