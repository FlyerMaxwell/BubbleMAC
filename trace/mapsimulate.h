#include "trace.h"
#include "common.h"
#include "geometry.h"


void init_multilane(struct Region *region);

void generate_car(struct Region *region, int carRcdNum);

void load_car(struct Region *region);

void start_simulate(struct Region* region, struct Hashtable* Traces, int count);

void update_traffic_light(struct Region *region, int count);

double get_safe_velocity(Vehicle *fCar, Vehicle *lCar);

int is_change_possibile(struct Road *currentRoad, struct Item *laneItem, Vehicle *fCar, struct Item *fItem, struct Item *lItem, double vsafe, int direction);

void update_vehicle_single_lane(Vehicle *fCar, Vehicle *lCar);

void update_first_car(struct Hashtable *tempTraces, struct Road *currentRoad, struct Item *fItem);

struct Item* change_lane(struct Road *currentRoad, struct Item *laneItem, struct Item *fItem, struct Item *afterItem, struct Item *beforeItem, int direction);

double car_following_new_v(double g, double vf, double vl, double a, double b);

void add_to_crossLane(struct Road* currentRoad, struct Lane *currentLane, Vehicle *fCar);

void handle_crossLane(struct crossLane* currentLane);

struct Lane* find_available_lane(struct crossLane* currentLane, struct Road* nextRoad, Vehicle *fCar);

void trace_write(struct Hashtable *Traces, FILE *fOutput);
