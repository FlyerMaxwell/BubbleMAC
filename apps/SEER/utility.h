#ifndef UTILITY_H
#define UTILITY_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>

/* one meter on the ground */
#define DELTA0 1e-5
#define DELTA1 1e-6
#define DELTA2 1e-10
#define DELTA4 1e-24
#define MAXVALUE 1000000
#define MAXERROR 200 
#define MAXINTERVAL 1500
#define MAXMAXINTERVAL 6000
#define ROAD_COUPLE_CHECKED 0x4 
#define ROAD_REDUNDANT_CHECKED 0x2
#define ROAD_REDUNDANT 0x8
#define REDUNDANT_SCALE 1.5
#define CHECK_PATH_SCALE 2.5 
#define NOTSET -12345
#define SMALL_TO_BIG 1
#define BIG_TO_SMALL 2
#define CROSS_SIZE 10 

#define MARK_LENGTH 12 
#define STRAIGHT_ANGLE 140

#define INTERPOLATED 100
/* data structure definition */

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

struct Stack
{
  void* data;
  struct Stack *next;
};

struct Set
{
  void *data;
  struct Set *next;
};

struct Queue
{
  void *data;
  struct Queue *next;
  struct Queue *prev;
};

// shape file stuff >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
typedef enum _ROADTYPE {ROAD_VIADUCT, ROAD_PRIMARY, ROAD_SECONDARY, ROAD_TRACK, ROAD_HIGHWAY, ROAD_FREEWAY, ROAD_UNKNOWN, ROAD_FIXED} RoadType;


struct Point
{
  double x;
  double y;
};

  
struct PolylineRcd
{
  double box[4];
  int nParts;
  int nPoints;
  int *parts;
  struct Point *points;
};

struct PolygonRcd
{
  double box[4];
  int nParts;
  int nPoints;
  int *parts;
  struct Point *points;
};

struct PolygonMRcd
{
  double box[4];
  int nParts;
  int nPoints;
  int *parts;
  struct Point *points;
  double MRange[2];
  double *measures;
};

struct Record_header
{
  int number;
  int length;
};

struct Polyline_record
{
  struct Record_header header;
  int type;
  struct PolylineRcd apolyline;
};

struct Polygon_record
{
  struct Record_header header;
  int type;
  struct PolygonRcd apolygon;
};

struct PolygonM_record
{
  struct Record_header header;
  int type;
  struct PolygonMRcd apolygonM;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct Window_size
{
  double cxmin;
  double cymin;
  double cxmax;
  double cymax;

  int sxmin;
  int symin;
  int sxmax;
  int symax;

  double scale;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// geometry structures >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

struct PointList
{
  double x;
  double y;
  struct PointList *prev;
  struct PointList *next;
};

struct PointListList
{
  struct PointList *aList;
  struct PointListList *next;
};

struct Segment
{
  double x1;
  double y1;
  double x2;
  double y2;
};

struct Box
{
  //two points with 4 variables can define a suqare area named box.
  double xmin;
  double ymin;
  double xmax;
  double ymax;
};

struct Color
{
  //Color contains three premary colours
  double red;
  double green;
  double blue;
};

struct Colormap
{
  int nColors;
  struct Color *colors;
};
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


//internal data structure >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct SampleList
{
  int speed;
  struct SampleList *next;
};

struct Slide
{
  struct SampleList *samples;
  double nSamples;
  double condition;
};

struct ScrapList
{
  struct PointList *points;
   
  struct Box box;

  double valueS2B;
  double valueB2S;

  struct Slide *s2b;
  struct Slide *b2s;
  struct ScrapList *prev;
  struct ScrapList *next;
};

struct RoadRefList;
struct Road
{
  int number;
  int type; 

  int nPoints;
  struct PointList *points;

  /*geographic box*/
  struct Box box;

  struct Color color;
  double width;
  double length;
  /* may use for multiple purposes */
  int value;
  /* average error of gps data on this road*/
  double error;
  int samples;

  /*hops from center road*/
  int hops;

  int smallEndAngle;
  int bigEndAngle;
  struct Cross *smallEnd;
  struct Cross *bigEnd;
  struct RoadRefList *refs;

  struct ScrapList *scraps;
  double nSamplesS2B;
  double nSamplesB2S;
};


struct RoadList
{
  struct Road *aRoad;
  int direction;
  double density;
  struct RoadList *prev;
  struct RoadList *next;
};

struct Roads
{
  struct RoadList *head;
  int nRoads;
};

struct DistrictList
{
  struct PointListList *rings;
  int nRings;
  struct Color color;
  struct DistrictList *next;
};

struct Districts
{
  struct DistrictList *head;
  int nDistricts;
};

struct RiverList
{
  struct PointListList *rings;
  int nRings;
  struct Color color;
  struct RiverList *next;
};

struct Rivers
{
  struct RiverList *head;
  int nRivers;
};

struct PathList
{
  struct RoadList *roads;
  double length;
  int turns;
  struct PathList *next;
  struct PathList *prev;
};

struct RoadRefList
{
  struct Roads *byWho;
  struct RoadList *aRef;
  struct RoadRefList *prev;
  struct RoadRefList *next;
};



struct CrossRefList;
struct Cross
{
  int number;
  struct Point gPoint;
  struct Box box;
  struct Roads roads;
  struct CrossRefList *refs;
};

struct CrossList
{
  struct Cross *aCross;
  struct CrossList *prev;
  struct CrossList *next;
};

struct Crosses
{
  struct CrossList *head;
  int nCrosses;
};

struct CrossRefList
{
  struct Crosses *byWho;
  struct CrossList *aRef;
  struct CrossRefList *prev;
  struct CrossRefList *next;
};


struct RoadToCross
{
  struct Road *fromRoad;
  struct Cross *toCross;
  struct RoadList *roads;
  struct CrossList *crosses;
  double distance;
  int turns;
  struct RoadToCross *next;
  struct RoadToCross *prev;
};


struct Cell
{
  int xNumber;
  int yNumber;
  struct Box box;
  struct Roads roads; 
  struct Crosses crosses;
  /* for the use of computing contacts*/
  struct LogList **slides;
  struct LogList *alls;
};

struct CellList
{
  struct Cell *aCell;
  struct CellList *next;
};

struct Region
{
  struct Box box;
  struct Roads roads;
  struct Crosses crosses;	
  struct Districts districts;
  struct Rivers rivers;

  int height;
  int width;

  time_t starttime;
  time_t endtime;
  int interval;
  int scrap;

  double cellSize;
  struct Cell *mesh;
  char *map;

  double commRange;

  struct Colormap cmap;
  /*min and max value of certain metric*/
  double lower;
  double upper;
};


struct CandRoadList
{
  struct Road *aRoad;
  double distance;
  double weight;
  double interAngl;
  struct Point gPoint;
  struct Segment onSegment;
  struct Cross *toCross;
  struct CandRoadList *next;
  struct CandRoadList *prev;
};

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


struct Polygon
{
  struct Box box;
  struct PointList *points;
  int nPoints;
  struct PointList *currentPoint;
  struct PointList *mouseAt;
  double scale;
  int state;
};


// trace data >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct Log
{
  int vId;
  time_t timestamp;
  struct Point gPoint;
  int speed;
  int angle;
  char state;
  char shown; 
  double maxdist;

  struct RGBO *refColor;

  struct CellList *surroundings;

  /* in increasing order of distance */
  struct CandRoadList *candRoads;
  struct CandRoadList *onRoad;
};

struct LogList
{
  struct Log *aLog;
  struct LogList *next;
  struct LogList *prev;
};

struct RGBO
{
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char opacity;
};

union Int_RGBO
{
  unsigned integer;
  struct RGBO values;
};

struct TraceList
{
  char *name;
  union Int_RGBO color;
  struct LogList *logs;
  struct LogList *at;
  double maxSpeed;
  double countdown;
  struct TraceList *next;
};


struct ActiveTraceList
{
  struct TraceList *activeTrace;
  struct ActiveTraceList *next;
};


struct TraceData
{
  struct TraceList *head;
  int nTraces;
  struct TraceList *focus;
  struct ActiveTraceList *showWhich;
};

union Int_byte
{
  unsigned long integer;
  char byte[4];
};

struct Int2
{
  unsigned long int1;
  unsigned long int2;
};

union Int2_byte
{
  struct Int2 a2Int;
  char byte[8];
};

struct ContactList
{
  double x;
  double y;
  time_t timestamp;
  struct ContactList *next;
};

struct ContactPairList
{
  struct Int2 v1v2;
  struct ContactList *contacts;
  struct ContactPairList *next;
};

struct Hash_table
{
  struct ContactPairList **tableHead;
  /* the size of the table */
  unsigned long size;
  /*how many entries in the tabel in total */
  unsigned long count;
  /*how many table entries have been filled */
  unsigned long full;
};
 
/*
*
* General tool functions
*
*******************************************************************/
int big2little(int);

int equald(double a, double b, double delta);
int greaterd(double a, double b, double delta);
int smallerd(double a, double b, double delta);
double max(double a, double b);
double min(double a, double b);

time_t strtot(const char *timestr);
void ttostr(time_t timestamp, char *time);
double distance_meter(double x1, double y1, double x2, double y2);
double angle_between(double x1, double y1, double x2, double y2);
double interangle(double angle1, double angle2);
double interangle_sharp(double angle1, double angle2);

void calculate_road_hops(int centerroad);
void dump_road_hops();

void stack_push(struct Stack** stackp, void* data);
void* stack_pop(struct Stack ** stackp);

void queue_add(struct Queue**queuep, void*data);
void* queue_pick(struct Queue **queuep);

void set_add(struct Set **setp, void *data);
void* set_scan(struct Set *setp, int no);
void* set_pick(struct Set **setp);
int is_set_empty(struct Set *setp);

unsigned long hash_func(unsigned long vId1, unsigned long vId2);
void insert_contact(struct Hash_table *table, unsigned long vId1, unsigned long vId2, struct ContactList *aContact);
void inittable(struct Hash_table *table, int size);
struct ContactPairList* lookup_contact_pair(struct Hash_table *table, unsigned long vId1, unsigned long vId2);
void removetable(struct Hash_table *table);

void find_contacts(struct Hash_table *table);


/*
*
* Mouce-on-screen operations
*
********************************************************************/
int is_legal(struct Polygon *chosen_polygon, struct Point *aPoint);
int is_polygon(struct Polygon *chosen_polygon, struct Point *aPoint);
void build_polygon(struct Polygon ** chosen_polygon, struct Point *aPoint);
void close_polygon(struct Polygon *chosen_polygon);
void free_polygon(struct Polygon * chosen_polygon);

/*
*
******************************************************************/
void load_colormap(char *filename, struct Colormap *cmap);
void free_colormap(struct Colormap *cmap);
void get_color(struct Colormap *cmap, struct Color *rtColor, double value, double lower, double upper);

void init_road_info(time_t timestamp);
int update_road_info(time_t timestamp);

/*
*
* Internal data structure operations
*
********************************************************************/
int is_point_in_box(struct Point *aPoint, struct Box *aBox);
int are_boxes_intersected(struct Box *aBox, struct Box *bBox);
int is_point_on_segment(struct Point *aPoint, struct Segment* aSegment);
int is_direct_point_on_segment(struct Point *aPoint, double angl, struct Segment *aSegment);
int are_segments_intersected(struct Segment *aSegment, struct Segment *bSegment);
int is_point_in_polygon(struct Polygon *chosen_polygon, struct Point *aPoint);
int is_cross_in_crosslist(struct Cross *aCross, struct CrossList *crosslist);
void create_box(struct Point *aPoint, double size, struct Box *aBox);
struct PointList* cut_segment_by_distance(struct Segment *aSegment, double dist);

double dist_point_to_segment(struct Point *aPoint, struct Segment*aSegment, struct Point *bPoint);
double dist_point_to_road(struct Point *aPoint, struct Road *aRoad, struct Point *bPoint, struct Segment* onSegment);
double dist_between_on_roads(struct Point *aPoint, struct Point *bPoint, struct RoadList *roads);
double dist_on_point_list(struct PointList *head);

struct PointList* point_list_between(struct Point *aPoint, struct Point *bPoint, struct RoadList *roads);

void clip_polyline_by_polygon(struct Polyline_record *rcd, struct Polygon *chosen_polygon, struct Region *region);
void add_polyline_in_polygon(struct Polyline_record *rcd, struct Polygon *chosen_polygon, struct Region *region);

void add_polyline(struct Region *region, struct PointList *startp, struct PointList *endp, int number, int type);
void insert_intersection_point(struct Point *basePoint, struct Point *aPoint, struct PointList **potentials);

void build_region(struct Polygon *chosen_polygon, double cellSize, struct Region **region);
void segment_cut_segment(struct Segment *aSegment, struct Segment *bSegment, struct Point *aPoint, struct Point *bPoint);

struct Cell* create_mesh(struct Point *aPoint, struct Point *bPoint, double cellsize);
void destroy_mesh(struct Cell *mesh);

struct CellList * surroundings_from_point(struct Region *region, struct Point *aPoint);
void gps_to_canvas(struct Window_size* awnd, double x, double y, double *rx, double *ry);
/*return the precision lost*/
double canvas_to_gps(struct Window_size* awnd, double x, double y, double *rx, double *ry);

void add_road_ref(struct Road *newRoad, struct Roads *roadList);
void add_cross_ref(struct Cross *newCross, struct Crosses *crossList);

struct Road* find_road_by_id(struct RoadList *head, int id);
void setup_road (struct Road *newRoad, struct Region *region);
void setup_cell_slides();
void setup_roads_slides(char *roadlistfile);
void setup_single_road_slides(struct Road *aRoad, int nSlides);
void install_log_to_cell(struct Log *aLog);
void install_log_to_cell_slide(struct Log *aLog);
void install_log_to_road_slide(struct Log *aLog);
struct PointList * box_on_road(struct Box *box, struct Road *aRoad);

void find_two_road_crosses(struct Region *region, struct Set**twoRoadCrossSet);
void find_redundant_roads(struct Region *region, struct Set**redundantRoadSet);
struct PathList* find_n_path(struct Cross *from, struct Road *fromRoad, struct Road *toRoad, struct Cross *to, int turns, double distBetween, int n);

void deal_cross_on_road(struct PointList *aPointEntry, struct Road *aRoad, struct Cross *aCross, struct Cell *aCell, struct Region *region);
void deal_two_road_crosses(struct Cross *twoRoadCross, struct Region *region);

struct PointList *point_list_on_roadlist(struct RoadList *path);
struct PointList * copy_point_to_list(double x, double y, struct PointList**head);
void copy_pointlist (struct PointList *pointList, struct PointList **head);
void copy_roadlist(struct RoadList *origRoadList, struct RoadList **newRoadList);
void copy_crosslist(struct CrossList *origCrossList, struct CrossList **newCrossList);
void add_road_to_roadlist_front(struct RoadList *aRoadEntry, struct RoadList **roadlist);
void add_road_to_roadlist_back(struct RoadList *aRoadEntry, struct RoadList **roadlist);
void add_cross_to_crosslist_front(struct Cross *aCross, struct CrossList **crosslist);
void add_cross_to_crosslist_back(struct Cross *aCross, struct CrossList **crosslist);
void add_district_to_region(struct Polygon_record *pgrcd,struct Region *region);
void add_river_to_region(struct PolygonM_record *pgmrcd, struct Region *region);
int count_turns_on_roadlist(struct RoadList *path);
struct RoadList * connect_roadlists(struct RoadList *aRoadList, struct RoadList *bRoadList);

struct RoadList * sort_road_by_density(struct RoadList *aList);
void calculate_road_condition(double *lower, double *upper);
void load_road_condition(char *roadinfo);
void dump_road_condition();
void dump_every_sample_on_road(int rId);
void dump_road_slides(struct RoadList *aRoad, FILE *fdump);

void free_pointlist(struct PointList *pointList);
void remove_road (struct Road *aRoad);
void remove_cross (struct Cross *aCross);
void remove_road_from_refs(struct Road *aRoad);
void remove_cross_from_refs(struct Cross *aCross);

void free_roadlist(struct RoadList *roads);
void free_crosslist(struct CrossList *crosses);
void free_path(struct PathList *path);
void free_region(struct Region *region);
void free_cell(struct Cell *aCell);
#endif
