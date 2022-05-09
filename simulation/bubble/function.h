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

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

//---bubble updated by YX----//

int init_simulation(struct Duallist *ALL_Vehicles);
void updateLocation(struct Duallist *ALL_Vehicles, int slot);
void handle_neighbours(struct Duallist *ALL_Vehicles);

double distance_between_vehicle(const struct vehicle* aCar, const struct vehicle* bCar);



// void clearPackets(struct Region* region);
// void handle_transmitter(struct Region* region, int slot);
// void handle_receiver(struct Region* region, int slot);

// struct packet * generate_packet(struct vehicle *aCar, int slot);
// struct collision* generate_collision(struct vehicle *aCar, struct vehicle *bCar,  int type, int slot);
// void log_collision(struct collision* coli);
// void log_packet(struct packet * aPkt, int slot);
// bool IsFront(struct vehicle *aCar, struct vehicle *tCar);
// void insertFrontRear(struct vehicle *aCar, struct packet *pkt);
// void bubble_mac_protocol(struct Region* aRegion);


// //---bubble updated by fc----//

// double safeDistance(const struct vehicle* v1, const struct vehicle* v2);
// double vehicleDistance(const struct vehicle* v1, const struct vehicle* v2);
// bool curInFront(const struct vehicle* cur, const struct vehicle* tar);
// int randSlot(int* occupied, int div);

// void degrade(struct vehicle* cur_vehicle);
// void applyForSlot(struct vehicle* cur_vehicle);

// //-----mmwave----//
// void update_trans_rate(struct Region *region);
// void update_vehicle_info(struct Region *region);
// void calculate_rate(struct Region *region);
// int show_graph_degree(struct Region *region);

// double calculate_dis(double x1, double y1, double x2, double y2);
// double calculate_angle_diff(double angle1, double angle2);
// int car_legal(struct Region *region, struct vehicle *aCar, struct Cell *aCell); 
#endif
