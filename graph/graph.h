#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "color.h"
#include "geometry.h"

struct GraphNode
{
  char name[32];
  struct Point point;
  int radius;
  struct RGBO color;
  unsigned long community;
  unsigned long betweenness;
  double similarity;
};

struct GraphComm
{
  int id;
  struct Box box;
  struct Duallist nodes;
};

struct GraphEdge
{
  struct GraphNode *aNode;
  struct GraphNode *bNode;
  int width;
  struct RGBO color;
};

int graphnode_has_name(char *name, struct GraphNode* aGraphNode);
int comm_has_larger_id(int *id, struct GraphComm* otherGraphComm);
int comm_has_id(int *id, struct GraphComm* otherGraphComm);
void comm_free_func(struct GraphComm *aGraphComm);
void load_node_and_edge_lists(FILE *f, struct Duallist *nodeList, struct Duallist *edgeList);
void load_and_setup_comm_list(FILE *fsource, struct Duallist *nodeList, struct Duallist *commList);

#endif

