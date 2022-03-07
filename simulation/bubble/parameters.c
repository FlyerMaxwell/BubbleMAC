#include "parameters.h"

//Buble MAC

int slot = 0;
double BRAKE_COE = 1;
int UpLocSlot = 10;//每个slot单位是0.5ms，更新位置的时间间隔为5ms，故为10个slot.      slot/UpLocSlot即为第几个5ms，或第几个位置
int SlotPerFrame = 200;//每个slot单位是0.5ms，一个frame是0.1秒，故为200个slot         slot/SlotPerFrame即为第几帧
int NOT_OCCUPIED = -1;
int SLOT_COLLISION = -2;
int HEAD_SLOT = 1;
int TAIL_SLOT = 199;




//time granularity: 10us as 1
int Car_Number = 0;
int BEACON_NUM = 4;
int BEACON_INTERVAL = 500;   // 5ms/10us = 500
int safe_dis = 5; //safe distance between two cars
int com_dis = 100; //communication distance
int sector_num = 38;  // 24*16us/10us=58
int sim_time = 500;

//30fps: 480p(30MB): 3110400   720p(78MB): 8294400   1080p(178MB): 22118400   (10Byte)
int DATA_TIME = 8294400*2*25/78; 

double neighbour_angle = 10;
int learning_cycle_num = 2;
int separate_num = 1;
int select_waste = 3; //time consumption for one selection

int timestamp =0;
int bi_num=0;
int lane_num=2;
int simulation_speed =1;
int traffic_density=40;

int magic_number =7;
int log_data = 0;
int PACKET_NUM = 3;           
int SELECT_TIME = 1;
int SELECT_NUM = 1;
FILE *carinfo;

double min_beam_width = 10.0;
double min_scan_interval = 5.0;
double scan_alpha = 30.0;
double scan_beta = 12.0;
double scan_theta = 15.0;
double radio_gain = 35.0; //dBm
double noise_power = -80.655; //dBm
int MAX_RATE = 907; // 6.76Gbps -> 907x10Bytes/10us
