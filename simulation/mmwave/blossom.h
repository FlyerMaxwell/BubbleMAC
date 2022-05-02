#ifndef BLOSSOM_H
#define BLOSSOM_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "trace.h"

void addEdge(int x,int y);
int Find(int x);
int lca(int x, int y);
void blossom(int x, int y, int k);
int bfs(int s);
int blossom_algorithm(struct Region *region, int car_num);

#endif


