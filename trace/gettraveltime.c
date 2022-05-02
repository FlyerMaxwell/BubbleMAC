#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "common.h"
#include "trace.h"
#include "files.h"


struct TravelTime
{
  time_t upwayTime;
  time_t downwayTime;
};

struct RouteTravelTime
{
  char onRoute[32];
  struct Duallist travelTimes;
};

int routeTravelTime_has_name(char *key, struct RouteTravelTime *aRouteTravelTime)
{
	return(!strcmp(key, aRouteTravelTime->onRoute));
}

void routeTravelTime_init_func(struct RouteTravelTime *aRouteTravelTime)
{
	if(aRouteTravelTime) {
		memset(aRouteTravelTime->onRoute, 0, 32);
		duallist_init(&aRouteTravelTime->travelTimes);
	}
}
void routeTravelTime_free_func(struct RouteTravelTime *aRouteTravelTime)
{
	if(aRouteTravelTime) {
		duallist_destroy(&aRouteTravelTime->travelTimes, free);
		free(aRouteTravelTime);
	}
}

struct TravelTime * trace_travel_time(struct Trace *aTrace, time_t fromTime, time_t toTime)
{
	struct Item *aItem ;
	struct Report *aRep;
	struct TravelTime *rtTravelTime;
	char isUpway;
	time_t totalUpwayTime = 0;
	time_t totalDownwayTime = 0;
	time_t onewayStartTime;
	int nUpways = 0;
	int nDownways = 0;

	aItem = aTrace->reports.head;
	while(aItem!=NULL) {
		aRep = (struct Report*)aItem->datap;
		aItem = aItem->next;
	}
	
	//filter out pre-fromTime reports
	aItem = aTrace->reports.head;
	while(aItem!=NULL) {
		aRep = (struct Report*)aItem->datap;
		if(aRep->timestamp >= fromTime)
			break;
		aItem = aItem->next;
	}

	//find the first complete one-way
	if(aItem) {
		aRep = (struct Report*)aItem->datap;
		if(is_report_in_upway(aRep))
			isUpway = 1;
		else
			isUpway = 0;
	}
	while(aItem!=NULL) {
		aRep = (struct Report*)aItem->datap;
		if((is_report_in_upway(aRep) && isUpway) || (!is_report_in_upway(aRep) && !isUpway) || (aRep->state & 0x20)) {
			aItem = aItem->next;
		} else
			break;
	}

	//up and down
	while(aItem) {
		if(aItem) {
			aRep = (struct Report*)aItem->datap;
			if(aRep->timestamp > toTime)
				break;
			if(is_report_in_upway(aRep))
				isUpway = 1;
			else
				isUpway = 0;
			onewayStartTime = aRep->timestamp;
		}

		while(aItem && ( (is_report_in_upway((struct Report*)aItem->datap) && isUpway) || (!is_report_in_upway((struct Report*)aItem->datap) && !isUpway) || (aRep->state & 0x20)) ) {
			aItem = aItem->next;
		}
	
		if(aItem) {
			aRep = (struct Report*)aItem->datap;
			if(aRep->timestamp < toTime) {
				if (isUpway) {
					totalUpwayTime += aRep->timestamp - onewayStartTime;
					nUpways += 1;
				} else {
					totalDownwayTime += aRep->timestamp - onewayStartTime;
					nDownways += 1;
				}
			}
		}

	}

	if(nUpways || nDownways) {
		rtTravelTime = (struct TravelTime*)malloc(sizeof(struct TravelTime));
		if(nUpways)
			rtTravelTime->upwayTime = totalUpwayTime/nUpways;
		else
			rtTravelTime->upwayTime = 0;
		if(nDownways)
			rtTravelTime->downwayTime = totalDownwayTime/nDownways;
		else
			rtTravelTime->downwayTime = 0;
		return rtTravelTime;
	} else
		return NULL;
}

int main(int argc, char**argv)
{
  time_t sAt, eAt;
  time_t startAt = 0, endAt = 0;
  struct Hashtable traces;//
  FILE *fsource, *fdump;//
  struct Item *aItem, *bItem;
  struct Trace *aTrace;//
  struct Hashtable aRouteTable;//
  struct RouteTravelTime *aRouteTravelTime;//
  struct TravelTime *aTravelTime;
  time_t upway, downway;
  int nUpways, nDownways;
  unsigned long i;


  if(argc < 2) {
	printf("Usage: %s [-time fromTime toTime] (.lst .ogd .mgd ...)\n", argv[0]);
	exit(1);
  }

  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		startAt = strtot(argv[2]);
		endAt = strtot(argv[3]);
		argc-=3;
		argv+=3;
		break;
	default:
		printf("Usage: %s [-time fromTime toTime] (.lst .ogd .mgd ...)\n", argv[0]);
	}
  }

  hashtable_init(&traces, 2000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
  while(argc > 1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[1]);
		sAt = startAt;
		eAt = endAt;
		load_source_file(fsource, NULL, &traces, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL,NULL,NULL,NULL,NULL, &sAt, &eAt, NULL);
		fclose(fsource);
	}
	argc--;
	argv++;
  }



  hashtable_init(&aRouteTable, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routeTravelTime_has_name);
  for(i = 0; i<traces.size; i++) {
	  aItem = traces.head[i];
	  while(aItem != NULL) {
		  aTrace = (struct Trace*)aItem->datap;
		  if((bItem = hashtable_find(&aRouteTable, aTrace->onRoute))!=NULL) {
			aRouteTravelTime = (struct RouteTravelTime*)bItem->datap;
		  } else {
			aRouteTravelTime = (struct RouteTravelTime*)malloc(sizeof(struct RouteTravelTime));
			routeTravelTime_init_func(aRouteTravelTime);
			strncpy(aRouteTravelTime->onRoute, aTrace->onRoute, 32);
			hashtable_add(&aRouteTable, aTrace->onRoute, aRouteTravelTime);
		  }
		  if((aTravelTime = trace_travel_time(aTrace, startAt, endAt))!=NULL) 
			duallist_add_to_tail(&aRouteTravelTime->travelTimes, aTravelTime);
		  aItem = aItem->next;
	  }
  }

  for(i = 0; i<aRouteTable.size; i++) {
	  aItem = aRouteTable.head[i];
	  while(aItem != NULL) {
		aRouteTravelTime = (struct RouteTravelTime*)aItem->datap;
		upway = 0, downway = 0, nUpways = 0, nDownways = 0;
		bItem = aRouteTravelTime->travelTimes.head;
		while(bItem) {
			aTravelTime = (struct TravelTime*) bItem->datap;
			if(aTravelTime->upwayTime) {
				upway += aTravelTime->upwayTime;
				nUpways ++;
			}
			if(aTravelTime->downwayTime) {
				downway += aTravelTime->downwayTime;
				nDownways ++;
			}
			bItem = bItem->next;
		}
		printf("%s %ld %ld\n",aRouteTravelTime->onRoute, nUpways?upway/nUpways:0, nDownways?downway/nDownways:0);
		aItem = aItem->next;
	}
  }
  hashtable_destroy(&aRouteTable, (void(*)(void*))routeTravelTime_free_func);
  hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
  return 0;
} 
