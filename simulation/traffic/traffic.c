#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"oracle.h"
#include"traffic.h"
#include"node.h"
#include"event.h"


void trafficGenerator_init_func(struct TrafficGenerator *trafficGenerator, time_t trafficStartAt, int numPkgs, time_t mean, int selectPolicy)
{
	if(trafficGenerator) {
		trafficGenerator->numPkgs = numPkgs;
		trafficGenerator->value = mean;
		trafficGenerator->startAt = trafficStartAt;
		trafficGenerator->selectPolicy = selectPolicy;
	}
}


void trafficGenerator_free_func(struct TrafficGenerator *trafficGenerator)
{
	if(trafficGenerator) {
		free(trafficGenerator);
	}
}

/* generates an exponential random variable given the mean in seconds*/
time_t ExpRnd(time_t mean)
{
  double unif_rnd,exp_rnd;

  unif_rnd=rand()*1.0/RAND_MAX;
  exp_rnd=-mean*log(1-unif_rnd);
  return(exp_rnd);
}


void generate_v2v_poisson_traffic(struct Simulator *aSim)
{
	int index;
	unsigned long i, j;
	time_t clock;
	struct Pkg *aPkg;
	struct TrafficGenerator *trafficGenerator;
	struct Node *srcNode=NULL, *dstNode=NULL;	
	struct Item *aItem;
	struct NeighborNode *aNeighborNode;


	if(aSim == NULL)
		return;
	trafficGenerator = aSim->trafficGenerator;

	clock = trafficGenerator->startAt;
	for(i=0;i<trafficGenerator->numPkgs;i++) {
		clock = clock + ExpRnd(trafficGenerator->value);
		if(trafficGenerator->selectPolicy == SELECT_RANDOM) { 
			srcNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL);
			while(srcNode == (dstNode=randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL))) {}
		} else if(trafficGenerator->selectPolicy == SELECT_FRIENDS) {
			srcNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL);
			while(srcNode->neighbors.count == 0) 
				srcNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL);
			index = rand()%srcNode->neighbors.count+1;
			for(j=0;j<srcNode->neighbors.size;j++) {
				aItem = srcNode->neighbors.head[j];
				while(aItem != NULL) {
					aNeighborNode = (struct NeighborNode*)aItem->datap;
					index --;
					if(index == 0) {
						dstNode = aNeighborNode->node;
						break;
					}
					aItem = aItem->next;
				}
			}
		} else if(trafficGenerator->selectPolicy == SELECT_STRANGERS) {
			srcNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL);
			dstNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL);
			while(srcNode == dstNode || hashtable_find(&srcNode->neighbors, dstNode->name)) {
				dstNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL);
			}
		}
		aPkg = (struct Pkg*)malloc(sizeof(struct Pkg));	
		pkg_init_func(aPkg, i, srcNode->name, dstNode->name, clock, aSim->pkgSize, -1, 0);
		duallist_add_to_tail(&aSim->pkgs, aPkg);
	}
}

// bus -> location traffic
void generate_b2l_poisson_traffic(struct Simulator *aSim)
{
	int i;
	time_t clock;
	struct Pkg *aPkg;
	struct TrafficGenerator *trafficGenerator;
	struct Node *srcNode=NULL;	
	struct Cell *aCell;
	struct Item *aItem;
	char buf[32];
	int cellid;

	if(aSim == NULL)
		return;
	trafficGenerator = aSim->trafficGenerator;

	clock = aSim->clock;
	for(i=0;i<trafficGenerator->numPkgs;i++) {
		clock = clock + ExpRnd(trafficGenerator->value);
		srcNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_BUS);
		aCell = randomly_pick_a_bus_covered_cell(aSim->region);
		cellid = 1000000+aCell->xNumber*1000+aCell->yNumber;
		sprintf(buf, "%d", cellid);
		aItem = hashtable_find(&aSim->destinations, buf);
		if(!aItem)
			hashtable_add(&aSim->destinations, buf, aCell);
		aPkg = (struct Pkg*)malloc(sizeof(struct Pkg));	
		pkg_init_func(aPkg, i, srcNode->name, buf, clock, aSim->pkgSize, -1, 0);
		duallist_add_to_tail(&aSim->pkgs, aPkg);
	}
}

void dump_traffic(FILE *fdump, struct Duallist *pkgs)
{
	duallist_dump(fdump, pkgs, (void(*)(FILE*, void*))pkg_dump_func);
}

void load_traffic(FILE *fload, struct Duallist *pkgs, int aim)
{
	if(aim==DELIEVERED_PKGS)
		duallist_load(fload, pkgs, (void*(*)(FILE*))pkg_delievered_load_func);
	else
		duallist_load(fload, pkgs, (void*(*)(FILE*))pkg_load_func);
}

