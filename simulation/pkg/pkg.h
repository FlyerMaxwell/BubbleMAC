#ifndef PKG_H
#define PKG_H

#include<time.h>
#include"common.h"

struct RoutingEdge
{
  char head[NAME_LENGTH];
  char tail[NAME_LENGTH];
  int quota;
  double weight;
};//add by cscs
void routingEdge_dump_func(FILE *fdump, struct RoutingEdge *aRoutingEdge);
struct RoutingEdge * routingEdge_copy_func(struct RoutingEdge *aEdge);
int routingEdge_has_key(char *key, struct RoutingEdge *aEdge);
struct RoutingEdge* routingEdge_load_func(FILE *fload);

struct NeighborEdge
{ 
  struct RoutingEdge *neighbor;
  int leftHops;
};
int neighborEdge_has_larger_lefthops(struct NeighborEdge *thisNeighbor, struct NeighborEdge *otherNeighbor);

struct RoutingPath
{
  struct Duallist edges;
  double weight;
};
void routingPath_init_func(struct RoutingPath *aRoutingPath);
void routingPath_dump_func(FILE *fdump, struct RoutingPath *aRoutingPath);
struct RoutingPath * routingPath_copy_func(struct RoutingPath *aPath);
struct RoutingPath* routingPath_load_func(FILE *fload);
void routingPath_free_func(struct RoutingPath *aRoutingPath);
int routingPath_has_smaller_weight(struct RoutingPath *aPath, struct RoutingPath *bPath);

struct Pkg
{
  unsigned id;
  char src[2*NAME_LENGTH];
  char dst[2*NAME_LENGTH];
  time_t startAt;
  time_t endAt;
  unsigned size;
  // TTL
  int ttl;
  double value;
  // routing record
  struct Duallist routingRecord;
  // routing paths needed by Onion routing
  struct Duallist routingPaths;
  unsigned long nEdges;
  //routing graph by cscs
  struct Hashtable routingGraph;
};

void pkg_init_func(struct Pkg *aPkg, unsigned id, char* src, char* dst, time_t startAt, unsigned int size, int ttl, double value);
struct Pkg *pkg_copy_func(struct Pkg *aPkg);
void pkg_free_func(struct Pkg *aPkg);
int pkg_has_id(int *id, struct Pkg *aPkg);
int pkg_has_earlier_startAt_than(struct Pkg *aPkg, struct Pkg *bPkg);

void routingRecord_dump_func(FILE *fdump, char* aRoutingRecord);
char* routingRecord_load_func(FILE *fload);

void setup_routingGraph_hashtable(struct Hashtable *table, struct Duallist *routingPaths);
struct Pkg* pkg_delievered_load_func(FILE *fload);
struct Pkg* pkg_load_func(FILE *fload);
void pkg_print(FILE *fout, struct Pkg *aPkg);

void pkg_dump_func(FILE *fdump, struct Pkg *aPkg);

#endif
