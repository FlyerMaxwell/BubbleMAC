#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"
#include"files.h"
#include"contact.h"
#include"busroute.h"
#include"trace.h"

struct SeenRoute
{
  char name[32];
  time_t aggICT;
  struct Duallist aggContacts;
}; 
void seenRoute_init_func(struct SeenRoute *aSeenRoute)
{
	memset(aSeenRoute->name, 0, 32);
	aSeenRoute->aggICT = 0;
	duallist_init(&aSeenRoute->aggContacts);
}
void seenRoute_free_func(struct SeenRoute *aSeenRoute)
{
	if(aSeenRoute) {
		duallist_destroy(&aSeenRoute->aggContacts, NULL);
		free(aSeenRoute);
	}
}
int seenRoute_has_name(char *name, struct SeenRoute *aSeenRoute)
{
	return !strcmp(aSeenRoute->name, name);
}


struct BusSeesRoutes
{
  char name[32];
  struct Duallist seenRoutes;
};
void busSeesRoutes_init_func(struct BusSeesRoutes *aBusSeesRoutes)
{
	memset(aBusSeesRoutes->name, 0, 32);
	duallist_init(&aBusSeesRoutes->seenRoutes);
}
void busSeesRoutes_free_func(struct BusSeesRoutes *aBusSeesRoutes)
{
	if(aBusSeesRoutes) {
		duallist_destroy(&aBusSeesRoutes->seenRoutes, (void(*)(void*))seenRoute_free_func);
		free(aBusSeesRoutes);
	}
}
int busSeesRoutes_has_name(char *name, struct BusSeesRoutes *aBusSeesRoutes)
{
	return !strcmp(aBusSeesRoutes->name, name);
}

struct RouteHasBuses
{
  char name[32];
  struct Duallist buses;
};
void routeHasBuses_init_func(struct RouteHasBuses *aRouteHasBuses)
{
	memset(aRouteHasBuses->name, 0, 32);
	duallist_init(&aRouteHasBuses->buses);
}
void routeHasBuses_free_func(struct RouteHasBuses *aRouteHasBuses)
{
	if(aRouteHasBuses) {
		duallist_destroy(&aRouteHasBuses->buses, (void(*)(void*))busSeesRoutes_free_func);
		free(aRouteHasBuses);
	}
}
int routeHasBuses_has_name(char *name, struct RouteHasBuses *aRouteHasBuses)
{
	return !strcmp(aRouteHasBuses->name, name);
}


void load_route_travel_time(FILE *fsource, struct Hashtable *busroutes)
{
	char buf[128], buf1[32], *strp, *strp1, *strp2;
	struct Item *aItem;
	struct Busroute *aRoute;

	if(fsource && busroutes) {
		while(fgets(buf, 128, fsource)) {
			strp = strtok(buf, " \n");
			strp1= strtok(NULL, " \n");
			strp2= strtok(NULL, " \n");
			sprintf(buf1, "%s_upway", strp);
			aItem = hashtable_find(busroutes, buf1);
			if (aItem) {			
				aRoute = (struct Busroute*)aItem->datap;
				aRoute->value = atof(strp1);
			}
			sprintf(buf1, "%s_downway", strp);
			aItem = hashtable_find(busroutes, buf1);
			if (aItem) {			
				aRoute = (struct Busroute*)aItem->datap;
				aRoute->value = atof(strp2);
			}
		}
	}
}

int main( int   argc, char *argv[] )
{
  char *cgdumpfile = "route.cg";
  char buf1[32], buf2[32], key[64];
  char *vName1, *rName1, *vName2, *rName2;
  FILE *fsource, *fdump;
  
  struct Hashtable pairTable;
  struct Item *aPairItem, *aItem, *bItem;
  struct Pair *aPair;

  struct Item *aContactItem;
  struct Contact *aContact, *bContact;

  struct Duallist routes;
  struct Item *aRouteItem, *bRouteItem;
  struct RouteHasBuses *aRouteHasBuses, *bRouteHasBuses;

  struct Item *aBusItem, *bBusItem;
  struct BusSeesRoutes *aBusSeesRoutes, *bBusSeesRoutes;

  struct Item *aSeenRouteItem, *bSeenRouteItem;
  struct SeenRoute *aSeenRoute, *bSeenRoute;

  int magicNumber;

  struct Hashtable pairIctTable;

  unsigned long pairTableSize = 10e6;
  unsigned long i, nICTs;
  time_t *ict, total;
  int id1, id2;

  struct Region *region = NULL;
  struct Hashtable busroutes;
  long routeTableSize = 500;
  char *mapfile = NULL, *routelstfile = NULL, *traveltimefile = NULL;
  struct Busroute *aRoute;
  struct Duallist *cellList;
  struct Cell *aCell;
  char *strp, *strp1;

  if(argc < 2) {
	printf("Usage: %s [-w route.cg] [-route map route.lst traveltimefile] .cont ...\n", argv[0]);
	exit(1);
  }
  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 'r':
		mapfile = argv[2];
		routelstfile = argv[3];
		traveltimefile = argv[4];
		argc-=4;
		argv+=4;
		break;
		
	case 'w':
		cgdumpfile = argv[2];
		argc-=2;
		argv+=2;
		break;
		
	default:
		printf("Usage: %s [-w route.cg] [-route map route.lst traveltimefile] .cont ...\n", argv[0]);
		exit(1);
	}
  }

  if((fdump = fopen(cgdumpfile, "w"))==NULL) {
	printf("Cannot write to file!\n");
	exit (2);
  }

  duallist_init(&routes);

  hashtable_init(&pairTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  while(argc>1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fscanf(fsource, "%d\n", &magicNumber);
		printf("Loading %s file ...\n", argv[1]);
		if(magicNumber == FILE_CONTACT) {
			load_contacts_with_hashtable(fsource, NULL, &pairTable, PAIRWISE_TABLE, NULL, NULL);
		} else {
			printf("Wrong file type! File %s has been ignored.\n", argv[1]);
		}
		fclose(fsource);
	}
	argc--;
	argv++;
  }

  for (i=0;i<pairTable.size;i++) {
	aPairItem = pairTable.head[i];
	while (aPairItem != NULL) {
		aPair = (struct Pair*)aPairItem->datap;

		strncpy(buf1, aPair->vName1, 32);
		vName1 = strtok(buf1, "@");
		rName1 = strtok(NULL, "\0");
		strncpy(buf2, aPair->vName2, 32);
		vName2 = strtok(buf2, "@");
		rName2 = strtok(NULL, "\0");

		aRouteItem = duallist_find(&routes, rName1, (int(*)(void*,void*))routeHasBuses_has_name);
		if(!aRouteItem) {
			aRouteHasBuses = (struct RouteHasBuses*)malloc(sizeof(struct RouteHasBuses));
			routeHasBuses_init_func(aRouteHasBuses);
			strncpy(aRouteHasBuses->name, rName1, 32);
			duallist_add_to_tail(&routes, aRouteHasBuses);
		} else
			aRouteHasBuses = (struct RouteHasBuses*)aRouteItem->datap;
		aBusItem = duallist_find(&aRouteHasBuses->buses, vName1, (int(*)(void*,void*))busSeesRoutes_has_name);
		if(!aBusItem) {
			aBusSeesRoutes = (struct BusSeesRoutes*)malloc(sizeof(struct BusSeesRoutes));
			busSeesRoutes_init_func(aBusSeesRoutes);
			strncpy(aBusSeesRoutes->name, vName1, 32);
			duallist_add_to_tail(&aRouteHasBuses->buses, aBusSeesRoutes);
		} else
			aBusSeesRoutes = (struct BusSeesRoutes*)aBusItem->datap;

		aSeenRouteItem = duallist_find(&aBusSeesRoutes->seenRoutes, rName2, (int(*)(void*,void*))seenRoute_has_name); 
		if(!aSeenRouteItem) {
			aSeenRoute = (struct SeenRoute*)malloc(sizeof(struct SeenRoute));
			seenRoute_init_func(aSeenRoute);
			strncpy(aSeenRoute->name, rName2, 32);
			duallist_add_to_tail(&aBusSeesRoutes->seenRoutes, aSeenRoute);
		} else
			aSeenRoute = (struct SeenRoute*)aSeenRouteItem->datap;


		bRouteItem = duallist_find(&routes, rName2, (int(*)(void*,void*))routeHasBuses_has_name);
		if(!bRouteItem) {
			bRouteHasBuses = (struct RouteHasBuses*)malloc(sizeof(struct RouteHasBuses));
			routeHasBuses_init_func(bRouteHasBuses);
			strncpy(bRouteHasBuses->name, rName2, 32);
			duallist_add_to_tail(&routes, bRouteHasBuses);
		} else
			bRouteHasBuses = (struct RouteHasBuses*)bRouteItem->datap;
		bBusItem = duallist_find(&bRouteHasBuses->buses, vName2, (int(*)(void*,void*))busSeesRoutes_has_name);
		if(!bBusItem) {
			bBusSeesRoutes = (struct BusSeesRoutes*)malloc(sizeof(struct BusSeesRoutes));
			busSeesRoutes_init_func(bBusSeesRoutes);
			strncpy(bBusSeesRoutes->name, vName2, 32);
			duallist_add_to_tail(&bRouteHasBuses->buses, bBusSeesRoutes);
		} else
			bBusSeesRoutes = (struct BusSeesRoutes*)bBusItem->datap;

		bSeenRouteItem = duallist_find(&bBusSeesRoutes->seenRoutes, rName1, (int(*)(void*,void*))seenRoute_has_name); 
		if(!bSeenRouteItem) {
			bSeenRoute = (struct SeenRoute*)malloc(sizeof(struct SeenRoute));
			seenRoute_init_func(bSeenRoute);
			strncpy(bSeenRoute->name, rName1,32);
			duallist_add_to_tail(&bBusSeesRoutes->seenRoutes, bSeenRoute);
		} else
			bSeenRoute = (struct SeenRoute*)bSeenRouteItem->datap;

		aContactItem = aPair->contents.head;
		while(aContactItem) {
			aContact = (struct Contact*)aContactItem->datap;
			duallist_add_in_sequence_from_tail(&aSeenRoute->aggContacts, aContact, (int(*)(void*,void*))is_earlier_than_contact);
			duallist_add_in_sequence_from_tail(&bSeenRoute->aggContacts, aContact, (int(*)(void*,void*))is_earlier_than_contact);
			aContactItem = aContactItem->next;
		}
		aPairItem = aPairItem->next;
	}
  }

  hashtable_init(&pairIctTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  aRouteItem = routes.head;
  while(aRouteItem) {
	aRouteHasBuses = (struct RouteHasBuses*)aRouteItem->datap;
	aBusItem = aRouteHasBuses->buses.head;
	while(aBusItem) {
		aBusSeesRoutes = (struct BusSeesRoutes*)aBusItem->datap;
		aSeenRouteItem = aBusSeesRoutes->seenRoutes.head;
		while(aSeenRouteItem) {
			aSeenRoute = (struct SeenRoute*)aSeenRouteItem->datap;
			nICTs = 0;
			aContactItem = aSeenRoute->aggContacts.head;
			while(aContactItem!=NULL && aContactItem->next!=NULL) {
				aContact = (struct Contact*)aContactItem->datap;
				bContact = (struct Contact*)aContactItem->next->datap;
				if(bContact->startAt-aContact->endAt > 0) {
					aSeenRoute->aggICT += bContact->startAt-aContact->endAt;
					nICTs ++;
				}
				aContactItem = aContactItem->next;
			}
			if(nICTs) {
				sprintf(key, "%s,%s", aRouteHasBuses->name, aSeenRoute->name);
				aPairItem = hashtable_find(&pairIctTable, key);
				if(!aPairItem) {
					aPair = (struct Pair*)malloc(sizeof(struct Pair));
					pair_init_func(aPair);
					strncpy(aPair->vName1, aRouteHasBuses->name, 32);
					strncpy(aPair->vName2, aSeenRoute->name, 32);
					hashtable_add(&pairIctTable, key, aPair);
				} else 
					aPair = (struct Pair*)aPairItem->datap;

				aSeenRoute->aggICT /= nICTs;
				ict = (time_t *)malloc(sizeof(time_t));
				*ict = aSeenRoute->aggICT;
				duallist_add_to_tail(&aPair->contents, ict);
			}
			aSeenRouteItem = aSeenRouteItem->next;
		}
		aBusItem = aBusItem->next;
	}
	aRouteItem = aRouteItem->next;
  }

  
  fprintf(fdump, "%ld\n\n", routes.nItems);
  for (i=0;i<pairIctTable.size;i++) {
	aPairItem = pairIctTable.head[i];
	while (aPairItem != NULL) {
		aPair = (struct Pair*)aPairItem->datap;
  		total = 0;
		aItem = aPair->contents.head;
		while(aItem) {
			total += *((time_t*)aItem->datap);
			aItem = aItem->next;
		}
		convert_routeid_char_to_int(aPair->vName1, &id1);
		convert_routeid_char_to_int(aPair->vName2, &id2);
		fprintf(fdump, "%d %d %ld\n", id1, id2, total/aPair->contents.nItems);
		aPairItem = aPairItem->next;
	} 
  }

  // setup experiment region
  if((fsource=fopen(mapfile, "rb"))!=NULL) {
	printf("Loading map ...\n");
  	region = region_load_func(fsource, NULL, -1);
	fclose(fsource);
  }

  hashtable_init(&busroutes, routeTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);
  if((fsource=fopen(routelstfile, "r"))!=NULL) {
	printf("Loading bus routes ...\n");
	load_source_file(fsource, region, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, &busroutes, (void*(*)(FILE*, struct Region*, void *))load_route_with_hashtable, NULL, NULL, NULL);
	fclose(fsource);
  }

  if((fsource=fopen(traveltimefile, "rb"))!=NULL) {
	printf("Loading route travel time information ...\n");
	load_route_travel_time(fsource, &busroutes);
	fclose(fsource);
  }
  if(region && busroutes.count) {
	for (i = 0; i<busroutes.size; i++) {
		aItem = busroutes.head[i];
		while(aItem) {
			aRoute = (struct Busroute*)aItem->datap;
			if(aRoute->value) {
				strncpy(buf1, aRoute->name, 32);
				strp = strtok(buf1, "_");
				strp1 = strtok(NULL, "\0");
				cellList = get_route_coverage(region, aRoute);
				if(cellList) {
					bItem = cellList->head;
					while(bItem) {
						aCell = (struct Cell*)bItem->datap;
						fprintf(fdump, "%d %d %.0lf\n", strstr(strp1, "upway")?ABS(atoi(strp)):-ABS(atoi(strp)), 1000000+aCell->xNumber*1000+aCell->yNumber, aRoute->value/2);
						bItem = bItem->next;
					}
					duallist_destroy(cellList, NULL);
				}
			}
			aItem = aItem->next;
		}
	}
	hashtable_destroy(&busroutes, (void(*)(void*))route_free_func);
	region_free_func(region);
  }


  duallist_destroy(&routes, (void(*)(void*))routeHasBuses_free_func);
  hashtable_destroy(&pairIctTable, (void(*)(void*))pair_free_func);
  hashtable_destroy(&pairTable, (void(*)(void*))pair_free_func);
  fclose(fdump);
  return 0;
}
