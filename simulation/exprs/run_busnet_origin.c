#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include"common.h"
#include"simulator.h"
#include"oracle.h"
#include"node.h"
#include"pkg.h"
#include"traffic.h"
#include"contact.h"
#include"files.h"
#include"traffic.h"
#include"busArriveCellEvent.h"
#include"busMeetBusEvent.h"
#include"cntEvent.h"

void setup_b2l_traffic_events(struct Simulator *aSim);

int main( int   argc,
          char *argv[] )
{
  time_t starttime = 0, endtime = 0;
  time_t st = 0, et = 0;
  struct Simulator *aSim;
  struct Hashtable cntTable, traceTable;
  FILE *fsource, *ftraffic, *fpkgdump, *fdlvdump, *ftrffdump, *fdebug;
  int oracle_type = TYPE_ORACLE_BUS_SHAN, fwdTimes = 1;// unicast by default 

  struct Trace *aTrace;
  unsigned long i, k,j;
  
  struct Item *aItem, *bItem, *cItem, *tempItem;
  struct Node *aNode;
  struct Pkg *aPkg;

  int recordRoute = 0, nPkgs = 0, pkgTTL = -1, randseed;
  char *pkgdumpfile = NULL, *dlvdumpfile = NULL, *trffdumpfile = NULL;

  int pkgHead, pkgHeadSize = 1;
  char buf[128], *strp;
  struct Hashtable aRouteTable;
  struct RoutingEdge *aEdge;
  struct RoutingPath *aPath;
  double dumpInterval = 0, dumpClock;

  if(argc < 6) {
	printf("Usage: %s [-time exprStartAt exprEndAt] [-oracle \"onion\"|\"shan\"|\"epidemic\"] [-forward nTimes] [-headsize pkgheadsize] [-TTL hops] [-packets nPkgs randseed] [-record] [-w0 dumpinterval(hour)] [-w1 delaydump][-w2 deliverydump][-w3 volumedump] .map .trff [.cont .mgd ...]\n", argv[0]);
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

	case 'o':
		if(strcmp("onion",argv[2])==0) {
			oracle_type = TYPE_ORACLE_BUS_ONION;
		} else if (strcmp("shan", argv[2])==0) {
			oracle_type = TYPE_ORACLE_BUS_SHAN;
		} else if (strcmp("epidemic", argv[2])==0) {
			oracle_type = TYPE_ORACLE_BUS_EPIDEMIC;
		}
		argc-=2;
		argv+=2;
		break;

	case 'T':
		pkgTTL = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'f':
		fwdTimes = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'p':
		nPkgs = atoi(argv[2]);
		randseed = atoi(argv[3]);
		argc-=3;
		argv+=3;
		break;

	case 'h':
		pkgHeadSize = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'r':
		recordRoute = 1;
		argc-=1;
		argv+=1;
		break;

	case 'w':
		if(argv[1][2] == '0')
			dumpInterval = atof(argv[2])*3600;
		if(argv[1][2] == '1')
			pkgdumpfile = argv[2];
		if(argv[1][2] == '2')
			dlvdumpfile = argv[2];
		if(argv[1][2] == '3')
			trffdumpfile = argv[2];
		argc-=2;
		argv+=2;
		break;
	default:
		printf("Usage: %s [-time exprStartAt exprEndAt] [-oracle \"onion\"|\"shan\"|\"epidemic\"] [-forward nTimes] [-headsize pkgheadsize] [-TTL hops] [-packets nPkgs randseed] [-record] [-w0 dumpinterval(hour)] [-w1 delaydump][-w2 deliverydump][-w3 volumedump] .map .trff [.cont .mgd ...]\n", argv[0]);
		exit(1);
	}
  }


  aSim = (struct Simulator*)malloc(sizeof(struct Simulator));
  simulator_init_func(aSim, starttime, endtime, 1);
  aSim->fwdMethod = fwdTimes;
  aSim->pkgTTL = pkgTTL;
  aSim->pkgRcdRoute = recordRoute;

  /* setting up oracle */
  aSim->oracle = (struct Oracle*)malloc(sizeof(struct Oracle));
  oracle_init_func(aSim->oracle, oracle_type, aSim, 0, 0);


  // setup experiment region
  if((fsource=fopen(argv[1], "rb"))!=NULL) {
	printf("Loading map ...\n");
  	aSim->region = region_load_func(fsource, NULL, -1);
	fclose(fsource);
  }

  // setup traffic 
  if((ftraffic=fopen(argv[2], "r"))!=NULL) {
	printf("Loading packets to be forwarded ...\n");
	load_traffic(ftraffic, &aSim->pkgs, ALL_PKGS);
	if(nPkgs) {
		struct Duallist pkgs;
		int index;
		duallist_init(&pkgs);
		srand(randseed);
		while(nPkgs) {
			index = rand()%aSim->pkgs.nItems;
			aItem = aSim->pkgs.head;
			for(i=0;i<index;i++)
				aItem = aItem->next;
			if(((struct Pkg*)aItem->datap)->routingPaths.nItems) {
				duallist_add_to_tail(&pkgs, duallist_pick_item(&aSim->pkgs, aItem));
				nPkgs --;
			}
		}
		duallist_destroy(&aSim->pkgs,(void(*)(void*))pkg_free_func);
		duallist_copy_by_reference(&aSim->pkgs, &pkgs);
		duallist_destroy(&pkgs, NULL);
	}
	fclose(ftraffic);
  }


  hashtable_init(&aRouteTable, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))string_has_name);

  aItem = aSim->pkgs.head;
  while(aItem) {
	  aPkg = (struct Pkg*)aItem->datap;
	  aPkg->value = aSim->fwdMethod;
	  if (oracle_type == TYPE_ORACLE_BUS_ONION || oracle_type == TYPE_ORACLE_BUS_SHAN) {
		  /* how many routing paths can be contained using Onion routing */
		  pkgHead = 0;
		  bItem = aPkg->routingPaths.head;
		  while(bItem) {
			  aPath = (struct RoutingPath*)bItem->datap;
			  if(pkgHead + aPath->edges.nItems <= pkgHeadSize) {
				  pkgHead += aPath->edges.nItems;
				  bItem = bItem->next;
			  } else
				  break; 
		  }
		  if (oracle_type == TYPE_ORACLE_BUS_SHAN) {
			  while(bItem) {
				  tempItem = bItem->next;
				  bItem->next = NULL;
				  hashtable_destroy(&aPkg->routingGraph, NULL);
				  hashtable_init(&aPkg->routingGraph, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routingEdge_has_key);
				  setup_routingGraph_hashtable(&aPkg->routingGraph, &aPkg->routingPaths);
				  if(aPkg->routingGraph.count <= pkgHeadSize) {
					  bItem->next = tempItem;
					  bItem = bItem->next;
				  } else {
					  bItem->next = tempItem;
					  break;
				  }
			  }
		  }
		  while(bItem) {
			  tempItem = bItem->next;
			  routingPath_free_func(duallist_pick_item(&aPkg->routingPaths, bItem));
			  bItem = tempItem;
		  }
		  hashtable_destroy(&aPkg->routingGraph, NULL);
		  hashtable_init(&aPkg->routingGraph, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routingEdge_has_key);
		  setup_routingGraph_hashtable(&aPkg->routingGraph, &aPkg->routingPaths);

  		  //setup a route hashtable involved in routingGraph
		  for(i=0;i<aPkg->routingGraph.size;i++) {
			  bItem = aPkg->routingGraph.head[i];
			  while (bItem){
				  aEdge = (struct RoutingEdge*)bItem->datap;
				  sprintf(buf, "%d", ABS(atoi(aEdge->head)));
				  if(!hashtable_find(&aRouteTable, buf)) {
					  strp = (char*)malloc(sizeof(char)*32);
					  memset(strp, 0, 32);
					  strncpy(strp, buf, 32);
					  hashtable_add(&aRouteTable, strp, strp);
				  }
				  sprintf(buf, "%d", ABS(atoi(aEdge->tail)));
				  if(!hashtable_find(&aRouteTable, buf)) {
					  strp = (char*)malloc(sizeof(char)*32);
					  memset(strp, 0, 32);
					  strncpy(strp, buf, 32);
					  hashtable_add(&aRouteTable, strp, strp);
				  }
			  
				  bItem = bItem->next;
			  }
		  }	
	  } else {
		  /* epidemic routing uses no routing instructions */
		  duallist_destroy(&aPkg->routingPaths, (void(*)(void*))routingPath_free_func);
		  duallist_init(&aPkg->routingPaths);
		  hashtable_destroy(&aPkg->routingGraph, NULL);
		  hashtable_init(&aPkg->routingGraph, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routingEdge_has_key);
	  }
	  aItem = aItem->next;
  }

  

  // load bus traces, contacts between buses 
  hashtable_init(&traceTable, 4000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
  hashtable_init(&cntTable, 1000000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  while(argc>3) {
	if((fsource=fopen(argv[3], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[3]);
		st = starttime, et = endtime;
		load_source_file(fsource, aSim->region, &traceTable, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, &cntTable, PAIRWISE_TABLE, (void*(*)(FILE*, struct Region*, void *, int, time_t *, time_t *))load_contacts_with_hashtable, NULL, NULL, NULL, NULL, NULL, NULL, &st, &et, NULL);
		fclose(fsource);
	}
	argc--;
	argv++;
  }

  // setup vnodes
  for (i = 0; i<traceTable.size; i++) {
  	aItem = traceTable.head[i];
      	while(aItem != NULL ) {
		aTrace = (struct Trace*)aItem->datap;
		aNode=lookup_node(&aSim->vnodes, aTrace->vName);
		if(aNode == NULL) {
			if(aTrace->vName[0]=='b') {
  				if((oracle_type != TYPE_ORACLE_BUS_EPIDEMIC && hashtable_find(&aRouteTable, aTrace->onRoute) ) || (oracle_type == TYPE_ORACLE_BUS_EPIDEMIC) ) {
					aNode = (struct Node*)malloc(sizeof(struct Node));
					node_init_func(aNode, aTrace->vName, atoi(aTrace->onRoute), aSim->bufSize);
					hashtable_add(&aSim->vnodes, aNode->name, aNode);
				}
			} 
		}
		aItem = aItem->next;
	}
  }
  hashtable_destroy(&aRouteTable, free);
  
  // setup events
  setup_b2l_traffic_events(aSim);
  setup_bus_meet_bus_events(aSim, &cntTable);
  hashtable_destroy(&cntTable, (void(*)(void*))pair_free_func);

  setup_bus_arrive_cell_events(aSim, &traceTable);
  hashtable_destroy(&traceTable, (void(*)(void*))trace_free_func);

  /* dump pkgs*/ 
  fpkgdump = fopen(pkgdumpfile, "w");
  /* dump delivery ratio */
  fdlvdump = fopen(dlvdumpfile, "w");
  /* dump network traffic */
  ftrffdump = fopen(trffdumpfile, "w");
  // process events
  printf("Start to process %ld events ...\n", aSim->eventNums);
  dumpClock = aSim->exprStartAt+dumpInterval;
  while(consume_an_event(aSim)) {
	if(dumpInterval && aSim->clock >= dumpClock) {
		i = 0, j = 0;
		aItem = aSim->pkgs.head;
		while(aItem != NULL) {
		      aPkg = (struct Pkg*)aItem->datap;
		      if(aPkg->startAt < aSim->clock ) {
			      j ++;
		      }
		      if(aPkg->endAt != 0) {
				if(fpkgdump) 
				      fprintf(fpkgdump, "%ld ", aPkg->endAt-aPkg->startAt);
			     	i ++;
		      } else {
				if(fpkgdump) 
				      fprintf(fpkgdump, "-1 ");
		      }
		      aItem = aItem->next;
		}
		if(fpkgdump) 
			fprintf(fpkgdump, "\n");

		if(fdlvdump) {
		        fprintf(fdlvdump, "%.2lf ", i*1.0/aSim->pkgs.nItems);
		}

		if(ftrffdump) {
		        fprintf(ftrffdump, "%.2lf ", j==0?0:(aSim->trafficCount-j)*1.0/j);
		}
		
		dumpClock += dumpInterval;
	}

  }


  printf("Dumping results.\n");
  // collect resutls
  if(fpkgdump ) {
	  i = 0;
	  aItem = aSim->pkgs.head;
	  while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		if(aPkg->endAt != 0) {
			if(fpkgdump)
				fprintf(fpkgdump, "%ld ", aPkg->endAt-aPkg->startAt);
			i ++;
		} else {
			if(fpkgdump)
				fprintf(fpkgdump, "-1 ");
		}
		aItem = aItem->next;
	  }
	  fprintf(fpkgdump, "\n");
	  fclose(fpkgdump);
  }
  /* dump delivery ratio */
  if(fdlvdump) {
	fprintf(fdlvdump, "%.2lf ", i*1.0/aSim->pkgs.nItems);
	fclose(fdlvdump);
  }

  /* dump network traffic */
  if(ftrffdump) {
	fprintf(ftrffdump, "%.2lf ", (aSim->trafficCount-aSim->pkgs.nItems)*1.0/aSim->pkgs.nItems);
	fclose(ftrffdump);
  }
  // destroy simulator
  simulator_free_func(aSim); 

  return 0;
}


void setup_b2l_traffic_events(struct Simulator *aSim)
{
	struct Item *aItem;
	struct Event *aEvent;
	struct Pkg *aPkg, *newPkg;
	struct Node *srcNode=NULL;	

	if(aSim->pkgs.nItems) {
		aItem = aSim->pkgs.head;
		while(aItem) {
			aPkg = (struct Pkg*)aItem->datap;
			aPkg->ttl = aSim->pkgTTL;
			newPkg = pkg_copy_func(aPkg);
			srcNode = lookup_node(&aSim->vnodes, aPkg->src);
			aEvent = (struct Event*)malloc(sizeof(struct Event));
			event_init_func(aEvent, newPkg->startAt, srcNode, newPkg, (int(*)(struct Simulator*, void*,void*))node_recv);
			add_event(aSim, aEvent);
			aItem = aItem->next;
		}
	}
}
