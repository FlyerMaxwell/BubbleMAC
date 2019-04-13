#include<stdlib.h>
#include<string.h>
#include"simulator.h"
#include"contact.h"
#include"traffic.h"
#include"busroute.h"
#include"node.h"

void simulator_init_func(struct Simulator *aSim, time_t exprStartAt, time_t exprEndAt, time_t slotSize)
{
	unsigned long i; 

	if(aSim == NULL)
		return;

	aSim->region = NULL;//所谓的初始化，就是将值全部赋为0，指针赋为NULL
	aSim->oracle = NULL;
	aSim->trafficGenerator = NULL;
	aSim->exprStartAt = exprStartAt;
	aSim->exprEndAt = exprEndAt;
	aSim->slotSize=slotSize;
	aSim->eventSlots = (struct Duallist*)malloc(sizeof(struct Duallist)*(1+(exprEndAt-exprStartAt)/slotSize));
	for(i=0;i<(exprEndAt-exprStartAt)/slotSize;i++)
		duallist_init(aSim->eventSlots+i);
	aSim->eventNums = 0;
	aSim->clock = exprStartAt;
	aSim->fwdMethod = NO_REPLICA_FWD;
	aSim->bufSize = -1;
	aSim->pkgSize = 0;
	aSim->pkgTTL = -1;
	aSim->pkgRcdRoute = 0;
	aSim->trafficCount = 0;
	aSim->similarityFile = NULL;
	aSim->sentPkgs = 0;
	aSim->paintedNodes = 0;
	aSim->nPaints = 0;

	duallist_init(&aSim->pkgs);//并且将里面的双链表和hashtable初始化
	hashtable_init(&aSim->routes, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);
	hashtable_init(&aSim->vnodes, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))node_has_name);
	hashtable_init(&aSim->snodes, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))node_has_name);
  	hashtable_init(&aSim->cntTable, 100000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  	hashtable_init(&aSim->ictTable, 100000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
	hashtable_init(&aSim->destinations, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))cell_has_int_nos);
}

void simulator_free_func(struct Simulator *aSim)
{
	int i;

	if(aSim == NULL)
		return;
	if(aSim->region)
		region_free_func(aSim->region);
	if(aSim->oracle)
	  	oracle_free_func(aSim->oracle); 
	if(aSim->trafficGenerator)
		trafficGenerator_free_func(aSim->trafficGenerator);
	for(i=0;i<(aSim->exprEndAt-aSim->exprStartAt)/aSim->slotSize;i++)
		duallist_destroy(aSim->eventSlots+i, free);
	free(aSim->eventSlots);
	duallist_destroy(&aSim->pkgs,(void(*)(void*))pkg_free_func);
	hashtable_destroy(&aSim->routes, (void(*)(void*))route_free_func);
	hashtable_destroy(&aSim->vnodes, (void(*)(void*))node_free_func);
	hashtable_destroy(&aSim->snodes, (void(*)(void*))node_free_func);
	hashtable_destroy(&aSim->cntTable, (void(*)(void*))pair_free_func);
  	hashtable_destroy(&aSim->ictTable, (void(*)(void*))pair_free_func);
	hashtable_destroy(&aSim->destinations, NULL);
	free(aSim);
}

