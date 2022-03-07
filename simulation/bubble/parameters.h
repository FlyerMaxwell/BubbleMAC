#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <stdio.h>
#include<unordered_map>
#define false 0
#define true 1
#define ROLE_SINGLE 0
#define ROLE_HEAD 1
#define ROLE_TAIL 2
#define ROLE_MID 3
using namespace std;
/***********************bubble parameters**********************/
extern int slot;
extern double BRAKE_COE;
extern int UpLocSlot;
extern int SlotPerFrame;
extern int NOT_OCCUPIED;
extern int SLOT_COLLISION;
extern int HEAD_SLOT;
extern int TAIL_SLOT;
extern unordered_map<int, struct vehicle*> allCars;
// extern int ROLE_SINGLE;
// extern int ROLE_HEAD;
// extern int ROLE_TAIL;
// extern int ROLE_MID;



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
