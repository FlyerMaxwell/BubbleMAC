#ifndef WEIGHTED_BLOSSOM_H
#define WEIGHTED_BLOSSOM_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "trace.h"

typedef long long ll;

struct Edge{
  int u,v,w;
};

void update_slack(int u,int x);
void set_slack(int x);
void q_push(int x);
void set_st(int x,int b);
int get_pr(int b,int xr);
void set_match(int u,int v);
void augment(int u,int v);
int get_lca(int u,int v);
void add_blossom(int u,int lca,int v);
void expand_blossom(int b);
bool on_found_Edge(const Edge &e);
bool matching();
//pair<ll,int> weight_blossom();
int weighted_blossom(struct Region *region, int car_num);

#endif

