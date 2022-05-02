#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include <dlfcn.h>
#include"common.h"
#include"simulator.h"
#include"oracle.h"
#include"node.h"
#include"pkg.h"
#include"traffic.h"
#include"contact.h"
#include"files.h"
#include"traffic.h"

int main( int   argc,
          char *argv[] )
{
  time_t starttime = 0, endtime = 0;
  int traffictype = TYPE_TRAFFIC_GENERATOR_POISSON, poissonMean = 10, numPkgs = 100, randseed = 1111;
  struct Simulator *aSim;
  struct Hashtable traceTable;
  FILE *fsource, *ftraffic;

  struct Trace *aTrace;
  unsigned long i;
  
  struct Item *aItem, *bItem;
  struct Node *aNode;

  int K = 10;
  char *trffdumpfile = "default", *graphfile = "RRGnew.txt", *tempfile = "file.tmp";
  char buf[1024];
  int nEdges;
  struct Pkg *aPkg;
  struct Node *srcNode;
  struct Cell *aCell;
  int destId;
  struct RoutingPath* newPath;
  void *handle;
  int(*dlfunc)(char*, int, int, int, char*);
  struct RoutingEdge *newEdge;
  char *atoken, *btoken;


  if(argc < 3) {
	printf("Usage: %s [-t exprStartAt exprEndAt] [-poisson Mean(sec) numPkgs] [-kpaths K] [-i randseed] [-g graphfile] [-w dump.trff] .map [.bus .ogd .mgd ...]\n", argv[0]);
	exit(1);
  }
  while(argv[1]!=NULL && argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		starttime = strtot(argv[2]);
		endtime = strtot(argv[3]);
		argc-=3;
		argv+=3;
		break;
	
	case 'p':
		traffictype = TYPE_TRAFFIC_GENERATOR_POISSON;
		poissonMean = atoi(argv[2]);
		numPkgs = atoi(argv[3]);
		argc-=3;
		argv+=3;
		break;

	case 'g':
		graphfile = argv[2];
		argc-=2;
		argv+=2;
		break;

	case 'k':
		K = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'i':
		randseed = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'w':
		trffdumpfile = argv[2];
		argc-=2;
		argv+=2;
		break;
	default:
		printf("Usage: %s [-t exprStartAt exprEndAt] [-poisson Mean(sec) numPkgs] [-kpaths K] [-i randseed] [-g graphfile] [-w dump.trff] .map [.bus .ogd .mgd ...]\n", argv[0]);
		exit(1);
	}
  }

  srand(randseed);

  aSim = (struct Simulator*)malloc(sizeof(struct Simulator));
  simulator_init_func(aSim, starttime, endtime, 600);
  // setup experiment region
  if((fsource=fopen(argv[1], "rb"))!=NULL) {
	printf("Loading map ...\n");
  	aSim->region = region_load_func(fsource, NULL, -1);
	fclose(fsource);
  }

  // load bus & taxi traces
  hashtable_init(&traceTable, 4000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
  while(argc>2) {
	if((fsource=fopen(argv[2], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[2]);
		load_source_file(fsource, aSim->region, &traceTable, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, &aSim->routes, (void*(*)(FILE*, struct Region*, void *))load_route_with_hashtable, &starttime, &endtime, NULL);
		fclose(fsource);
	}
	argc--;
	argv++;
  }
  if(aSim->exprStartAt == 0) {
	aSim->exprStartAt = starttime;
	aSim->exprEndAt = endtime;
  }
  // setup vnodes
  for (i = 0; i<traceTable.size; i++) {
  	aItem = traceTable.head[i];
      	while(aItem != NULL ) {
		aTrace = (struct Trace*)aItem->datap;
		aNode=lookup_node(&aSim->vnodes, aTrace->vName);
		if(aNode == NULL) {
			aNode = (struct Node*)malloc(sizeof(struct Node));
			node_init_func(aNode, aTrace->vName, atoi(aTrace->onRoute), aSim->bufSize);
			hashtable_add(&aSim->vnodes, aNode->name, aNode);
		}
		aItem = aItem->next;
	}
  }
  printf("There %ld vehicular nodes.\n", aSim->vnodes.count);


  setup_cells_with_routes(aSim->region, &aSim->routes);

  // setup traffic generator
  if (traffictype == TYPE_TRAFFIC_GENERATOR_POISSON) {
	aSim->trafficGenerator = (struct TrafficGenerator*)malloc(sizeof(struct TrafficGenerator));
	trafficGenerator_init_func(aSim->trafficGenerator, aSim->exprStartAt, numPkgs, poissonMean, SELECT_RANDOM);
	generate_b2l_poisson_traffic(aSim);
  }
 
  handle = dlopen("/usr/local/lib/libkshortestpath.so", RTLD_LAZY);
  dlfunc = dlsym(handle, "kshortestpath");


  // setup routing paths
  if(aSim->pkgs.nItems) {
	  aItem = aSim->pkgs.head;
	  while(aItem) {
		aPkg = (struct Pkg*)aItem->datap;
		srcNode = lookup_node(&aSim->vnodes, aPkg->src);

		bItem = hashtable_find(&(aSim->destinations), aPkg->dst); 
		aCell = (struct Cell*)bItem->datap;
		destId = 1000000+aCell->xNumber*1000+aCell->yNumber;

		(*dlfunc)(graphfile, srcNode->onRoute, destId, K, tempfile);//generate routing indicator, put indicator list in tempfile;
		if((fsource=fopen(tempfile, "r"))!=NULL) {
			while (fgetc(fsource)!=EOF) {
				fseek(fsource, -1, SEEK_CUR);
				newPath = (struct RoutingPath*)malloc(sizeof(struct RoutingPath));
				routingPath_init_func(newPath);
				fgets(buf, 1024,fsource);//read a line from tempfile into buf
				sscanf(buf, "Cost: %lf Length: %d\n", &newPath->weight, &nEdges);
				for (i=0;i<(nEdges-2);i++) {
				      newEdge = (struct RoutingEdge*)malloc(sizeof(struct RoutingEdge));
				      memset(newEdge, 0, sizeof(struct RoutingEdge));
				      fgets(buf, 1024,fsource);//read a line from tempfile into buf
				      atoken=strtok(buf,",\n");// get the bus route (from) number before ","
				      btoken=strtok(NULL,",\n");// get the bus route (to) number after ","
				      strncpy(newEdge->head, atoken, strlen(atoken)+1);
				      strncpy(newEdge->tail, btoken, strlen(btoken)+1);
				      duallist_add_to_tail(&newPath->edges, newEdge);
				}
				newEdge = (struct RoutingEdge*)malloc(sizeof(struct RoutingEdge));
				memset(newEdge, 0, sizeof(struct RoutingEdge));
				fgets(buf, 1024,fsource);//read a line from tempfile into buf
				atoken=strtok(buf,",\n");// get the bus route (from) number before ","
				btoken=strtok(NULL,",\n");// get the bus route (to) number after ","
				strncpy(newEdge->head, atoken, strlen(atoken)+1);
				strncpy(newEdge->tail, atoken, strlen(atoken)+1);
				duallist_add_to_tail(&newPath->edges, newEdge);

				duallist_add_in_sequence_from_tail(&aPkg->routingPaths, newPath, (int(*)(void*,void*))routingPath_has_smaller_weight);
				aPkg->nEdges += newPath->edges.nItems;
			}
			fclose(fsource);
		} 

		(*dlfunc)(graphfile, -srcNode->onRoute, destId, K, tempfile);//generate routing indicator, put indicator list in tempfile;
		if((fsource=fopen(tempfile, "r"))!=NULL) {
			while (fgetc(fsource)!=EOF) {
				fseek(fsource, -1, SEEK_CUR);
				newPath = (struct RoutingPath*)malloc(sizeof(struct RoutingPath));
				routingPath_init_func(newPath);
				fgets(buf, 1024,fsource);//read a line from tempfile into buf
				sscanf(buf, "Cost: %lf Length: %d\n", &newPath->weight, &nEdges);
				for (i=0;i<(nEdges-2);i++) {
				      newEdge = (struct RoutingEdge*)malloc(sizeof(struct RoutingEdge));
				      memset(newEdge, 0, sizeof(struct RoutingEdge));
				      fgets(buf, 1024,fsource);//read a line from tempfile into buf
				      atoken=strtok(buf,",\n");// get the bus route (from) number before ","
				      btoken=strtok(NULL,",\n");// get the bus route (to) number after ","
				      strncpy(newEdge->head, atoken, strlen(atoken)+1);
				      strncpy(newEdge->tail, btoken, strlen(btoken)+1);
				      duallist_add_to_tail(&newPath->edges, newEdge);
				}
				newEdge = (struct RoutingEdge*)malloc(sizeof(struct RoutingEdge));
				memset(newEdge, 0, sizeof(struct RoutingEdge));
				fgets(buf, 1024,fsource);//read a line from tempfile into buf
				atoken=strtok(buf,",\n");// get the bus route (from) number before ","
				btoken=strtok(NULL,",\n");// get the bus route (to) number after ","
				strncpy(newEdge->head, atoken, strlen(atoken)+1);
				strncpy(newEdge->tail, atoken, strlen(atoken)+1);
				duallist_add_to_tail(&newPath->edges, newEdge);

				duallist_add_in_sequence_from_tail(&aPkg->routingPaths, newPath, (int(*)(void*,void*))routingPath_has_smaller_weight);
				aPkg->nEdges += newPath->edges.nItems;
			}
			fclose(fsource);
		}

		setup_routingGraph_hashtable(&aPkg->routingGraph, &aPkg->routingPaths);
		aItem = aItem->next;
	  }
  }
 
  remove(tempfile);
  /* dump network traffic */
  sprintf(buf, "%s.trff", trffdumpfile);
  ftraffic = fopen(buf, "w");
  if(ftraffic) {
	dump_traffic(ftraffic, &aSim->pkgs);

  	sprintf(buf, "%s.txt", trffdumpfile);
	FILE *fout = fopen(buf, "w");
	struct Item *aItem;
	aItem = aSim->pkgs.head;
	while(aItem) {
		pkg_print(fout, (struct Pkg*)aItem->datap);
		aItem = aItem->next;
	}
	fclose(fout);

	fclose(ftraffic);
  }


  hashtable_destroy(&traceTable, (void(*)(void*))trace_free_func);
  // destroy simulator
  simulator_free_func(aSim); 

  return 0;
}


