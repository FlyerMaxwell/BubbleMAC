#ifndef SIMULATOR_H
#define SIMULATOR_H

#include"common.h"
#include"oracle.h"

#define NO_REPLICA_FWD 0
#define BETTER_ESTIMATE_FWD 1
#define EVERBEST_ESTIMATE_FWD 2
#define KEEP_REPLICA_FWD 3

struct Simulator
{
  // experiment region
  struct Region *region;


  // bus routes
  struct Hashtable routes;
  // vehicles and static storage nodes
  struct Hashtable vnodes;
  struct Hashtable snodes;
  // pairwise contacts
  struct Hashtable cntTable;
  // pairwise icts
  struct Hashtable ictTable;
  


  // oracle
  struct Oracle *oracle;


  // packets traversing and recieved in the simulation
  struct Duallist pkgs;
  unsigned long sentPkgs;




  
  // traffic generator
  struct TrafficGenerator *trafficGenerator;
  struct Hashtable destinations;

  // driven events
  struct Duallist *eventSlots;
  time_t slotSize;
  unsigned long eventNums;

  // replication control
  int fwdMethod;

  // expr time setting
  time_t exprStartAt;
  time_t exprEndAt;
  time_t clock;

  unsigned int bufSize;

  unsigned int pkgSize;
  int pkgTTL;
  int pkgRcdRoute;

  unsigned long trafficCount;

  // similarity filename
  char *similarityFile;

  unsigned long paintedNodes;
  unsigned long nPaints;
};

void simulator_init_func(struct Simulator *aSim, time_t exprStartAt, time_t exprEndAt, time_t slotSize);
void simulator_free_func(struct Simulator *aSim);

#endif
