/* Notice :
*
* The set of Vanet programs take .ogd (original GPS data) files as input and may
* convert to .mgd (modified GPS data) files according to map matching
* algorithms. A specific data format convertor program should be provided to
* convert any other data format to .ogd before these data can be further
* processed in Vanet programs. 
*/

/* .ogd file format specification
*   -------------------------------
*   file_format_indicator
*   ogd_record (bus, taxi)
*   ogd_record (bus, taxi)
*   .....
*   
*
*   file_format_indicator
*   -------------------------------
*   FILE_ORIGINAL_GPS_TAXI  
*   FILE_ORIGINAL_GPS_BUS   
*
*   ogd_record taxi:
*   -------------------------------
*   taxiID, timestamp, longitude, lattitude, speed, heading, status
*
*   ogd_record bus:
*   busID, timestamp, longitude, lattitude, speed, heading, routeId, msyType, status, errorcode, routeLeng, gasVol 
*   
*/ 

/* .mgd file format specification
*   -------------------------------
*   file_format_indicator
*   mgd_record (bus, taxi)
*   mgd_record (bus, taxi)
*   .....
*   
*
*   file_format_indicator
*   -------------------------------
*   FILE_MODIFIED_GPS_TAXI  
*   FILE_MODIFIED_GPS_BUS   
*
*   mgd_record taxi:
*   -------------------------------
*   taxiID, timestamp, longitude, lattitude, speed, heading, status, onRoadID
*
*   mgd_record bus:
*   busID, timestamp, longitude, lattitude, speed, heading, routeId, msyType, status, errorcode, routeLeng, gasVol, onRoadID 
*   
*/ 

#ifndef TRACE_H
#define TRACE_H

#include <time.h>
#include "common.h"
#include "geometry.h"
#include "color.h"
#include <string>

#define TAXI_MAX_SPEED 120
#define BUS_MAX_SPEED 80

#define MIXED_TRACE_THRESHOLD 1000

#define TRACE_STATE_LIGHT 0

#define MAX_DISTANCE_ERROR 100
#define MAX_ANGLE_ERROR 90

#define HEADING_SPEED_THRESHOLD 10.8

#define INSERT_MODE_AVGSPEED 0
#define INSERT_MODE_TRAFFIC 1
#define OUTPUT_MODE_CELL 0
#define OUTPUT_MODE_INTERVAL 1 

#define CAR_LENGTH 3
#define SAFE_CAR_GAP 1.0		// the gap between vehicles when then stop
#define V_MAX 20
#define DET_TIME 0.5			// the simulation time of every step
/* the miminum time unit in seconds */
#define MINIMUM_TIME_UNIT 1

/* 
 * For a bus report, the MSB (bit[7] & 0x80) of state represents whether GPS is functioning (1 failure), 
 * 			    the b[6] & 0x40 represents whether a bus is parking (1 yes)
 * 			    the b[5] & 0x20 represents whether a bus is arriving at destination (1 yes)
 * 			    the b[4] & 0x10 represents which direction a bus is runnning in (1 downway)
 * 			    the b[3] & 0x08 represents whether a bus is openning its gate (1 yes)
 * 			    the b[2] & 0x04 represents whether a bus is arriving at a stop or leaving a stop (1 arriving)
 */
 
struct Report 
{
  //Function:  a struct to show the information of a vehicle

  time_t timestamp;
  struct Point gPoint;
  short speed;
  short angle;
  char state;

  /* for bus start->*/
  char msgType;
  double routeLeng;
  double gasVol;
  char errorInfo;
  /* <-end */

  char shown; 

  struct Duallist candRoads;
  struct Item *onRoad;
  int onRoadId;
  /* on the path determined by amend_algorithm */
  struct Path *onPath;

  struct Trace* fromTrace;
};


struct Trace
{
  //Function: a struct to store informarion of  traces
  char vName[NAME_LENGTH];
  char type;
  struct Duallist reports;
  struct Item *at;
  
  /*the duration of the trace*/
  int startCount;
  time_t startAt;
  time_t endAt;

  struct Box box; 

  /* for bus start-> */
  char onRoute[NAME_LENGTH];
  /* <-end */

  double maxSpeed;
  double countdown;
  char isHeadingValid;

  union Int_RGBO color;
  /* for multiple purposes */
  double var1;
  double var2;
  double var3;

  double oldx, oldy, oldangle;
  int finished;
};

struct neighbour_car
{
	int state; //0: now is not a neighbour, 1: now is a neighbour
	struct Item *carItem;
	int index;  
	int cell_x;
	int cell_y;
	char car_id[20];
	double v;
	double dis;
	double angle;
	// new variable
	int known;
	double TXweight;
	double RXweight;
	double SNR;
	double control_SNR;
	int packet_num;
	int data_rate;
	int beam_index;
	int scan_beam_index;
};


typedef struct simuInfo{
  
};



typedef struct vehicle
{
  int handled;                  //  to indicate whether the car has been updated during this time
  
  //basic info
  char id[20];// id of the car
  char type[10]; //model of the car

  //dynamic info
  double x;
  double y;
  double angle;
  double speed;
  double pos;
  char lane[10];
  double slope;
  double flow;
  double speed2;
  
  //slot info
  int slot_condition;
  int isQueueing;
  int slot_occupied;
  int* slot_oneHop;
  int* slot_twoHop;

  //Commrange
  double commRadius;
  
  //Packets Received 
  struct Duallist packets;
  //Neighbors to acc
  struct Duallist neighbours;
  //info about frontV and rearV
  struct Duallist frontV, rearV;

  struct Duallist history_neigh;
  
}Vehicle;

// typedef struct vehicle
// {
//   char id[20];// id of the car
//   int handled;                  //  to indicate whether the car has been updated during this time
//   double position, positionNew;	// distance from the head of road before update  
//   double v, vnext;		// the velocity of the vehicle
//   double vmax;		// the max velocity of the vehicle
//   double a;		// the accelration of the vehicle, regard as constant
//   double b;		// the decelration of the vehicle, regard as constant
//   double x;
//   double y;
//   struct Lane *belongLane;
//   struct Path *pathInfo;
//   struct Item *currentRoadItem;

//   //varaibles fpr bubble MAC
//   int belongLaneID;
//   int slot_occupied;  //
//   int slot_condition; //0 for no slot occupied, 1 for accessing slots and 2 for occupied already.
//   int isExpansion;
//   int car_role;
//   int radius_flag;
//   int isQueueing;
//   double commRadius;
//   double dir_x, dir_y; //车辆的方向矢量(dir_x, dir_y)。可调用IsFront(struct vehicle *aCar, struct vehicle *tCar)判断车辆间位置
  
//   int* slot_oneHop;
//   int* slot_twoHop;
//   struct Duallist packets;
//   struct Duallist neighbours;
//   struct Duallist frontV, rearV; //store the front neighbors and rear neighbors according to the received packets.目前只保存距离最近的前一个车和后一个车，可以扩展到n个。


//   //variables for mmwave simulation
//   double x1;
//   double y1;
//   struct Cell *belongCell;
//   // struct Duallist neighbours;
//   int choice_num;
//   int match_id;
//   int role; //-1 init, 0 listen, 1 scan, 2 communicating
//   int communicate_time;
  
//   struct Duallist history_neigh;
//   struct Duallist known_neigh;
//   struct Duallist choose_neigh;   //chosen by who
//   struct Duallist real_neigh;
//   int choose_car_id;
//   struct Item *choose_car_Item;  //temporary link
//   struct Item *choice_car;  //choose which car in one round

//   int weight;
  
//   int scan_time;
//   int communicated_num;
// }Vehicle;

typedef struct packet{
  int slot;
  int condition;////1:tx & tx   2:access_collison 3. merge_collison
  struct vehicle* srcVehicle, *dstVehicle;
  int isQueueing;
  // int *slot_used_by_queue;//不要了
  int *slot_resource_oneHop_snapShot;
}Packet;

typedef struct collision{//发生碰撞的时候记录：碰撞类型，slot，两个车及其onehop twohop的使用情况
  int type; //1:tx & tx   2:access_collison 3. merge_collison
  int slot;
  struct vehicle *src, *dst;
  int *src_oneHop;
  int *dst_oneHop;
  int *src_twoHop;
  int *dst_twoHop;
  // int *two_oneHop;
}Collision;

void report_init_func(struct Report *aRep);
void report_free_func(struct Report *aRep);
int report_has_earlier_timestamp_than(struct Report* aRep, struct Report *bRep);
int report_has_later_timestamp_than(struct Report* aRep, struct Report *bRep);
int are_two_reports_duplicated(struct Report *aRep, struct Report *bRep);
void dump_report(FILE *fp, struct Report* aRep, int type);
int remove_dull_reports(struct Trace *aTrace);
int is_report_in_upway(struct Report *aRep);

void trace_init_func(struct Trace *aTrace);
void trace_free_func(struct Trace *aTrace);
int trace_has_name(char *name, struct Trace* aTrace);
int is_trace_mixed(struct Trace *aTrace);
void set_trace_table_time(struct Hashtable *traces, time_t atClock);
void set_selected_traces_time(struct Duallist *selectedTraces, time_t atClock);
void trace_dump_func(FILE *fp, struct Trace *aTrace);
struct Trace* load_trace_with_hashtable(int magicNumber, FILE *ftrace, struct Region *aRegion, struct Hashtable *traces, time_t *startAt, time_t *endAt, struct Box *box);

struct Trace* insert_reports(struct Region *aRegion, struct Trace *aTrace, int insertMode, int outputMode, int interval);
struct Duallist *insert_reports_in_path(struct Path *aPath, struct Report *aRep, struct Report* nextRep, int insertMode);

struct CandRoad
{
  struct Road *aRoad;
  double distance;
  double angle;
  double weight;
  struct Point gPoint;
  struct Segment onSegment;
};

int candroad_has_road(struct Road *aRoad, struct CandRoad *aCandRoad);
void add_candidate_roads(struct Cell *aCell, struct Report *aRep, int angleValid);
int candroad_has_smaller_weight_than(struct CandRoad *aCandRoad, struct CandRoad* bCandRoad);

struct Path *get_path_between_two_reports(struct Region *aRegion, struct Report *aRep, struct Report *bRep, double lengthreshold);

#endif
