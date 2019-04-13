#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pkg.h"

#define ADDRESS_LENGTH 32


void pkg_init_func(struct Pkg *aPkg, unsigned id, char* src, char* dst, time_t startAt, unsigned int size, int ttl, double value)
{
	if(aPkg == NULL) 
		return;
	aPkg->id = id;
	strncpy(aPkg->src, src, strlen(src)+1);
	strncpy(aPkg->dst, dst, strlen(dst)+1);
	aPkg->startAt = startAt;
	aPkg->endAt = 0;
	aPkg->size = size;
	aPkg->ttl = ttl;
	aPkg->value = value;
	duallist_init(&aPkg->routingRecord);
	duallist_init(&aPkg->routingPaths);
	aPkg->nEdges = 0;
	hashtable_init(&aPkg->routingGraph, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routingEdge_has_key);//add by cscs
}

void pkg_free_func(struct Pkg *aPkg)
{
	if(aPkg) {
		duallist_destroy(&aPkg->routingRecord, free);
		duallist_destroy(&aPkg->routingPaths, (void(*)(void*))routingPath_free_func);
		hashtable_destroy(&aPkg->routingGraph, NULL);
		free(aPkg);
	}
}


struct Pkg *pkg_copy_func(struct Pkg *aPkg)
{

	struct Pkg *nPkg;

	if(aPkg == NULL) 
		return NULL;
	nPkg = (struct Pkg*)malloc(sizeof(struct Pkg));
	nPkg->id = aPkg->id;
	strncpy(nPkg->src, aPkg->src, 2*NAME_LENGTH);
	strncpy(nPkg->dst, aPkg->dst, 2*NAME_LENGTH);
	nPkg->startAt = aPkg->startAt;
	nPkg->endAt = aPkg->endAt;
	nPkg->size = aPkg->size;
	nPkg->ttl = aPkg->ttl;
	nPkg->value = aPkg->value;
	duallist_copy(&nPkg->routingRecord, &aPkg->routingRecord,(void*(*)(void*))string_copy_func);
	duallist_copy(&nPkg->routingPaths, &aPkg->routingPaths,(void*(*)(void*))routingPath_copy_func);
	nPkg->nEdges = aPkg->nEdges;
	hashtable_init(&nPkg->routingGraph, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routingEdge_has_key);//add by cscs
	setup_routingGraph_hashtable(&nPkg->routingGraph, &nPkg->routingPaths);
	return nPkg;
}

void setup_routingGraph_hashtable(struct Hashtable *table, struct Duallist *routingPaths)
{
	struct Item *aPathItem, *aEdgeItem;
	struct RoutingPath *aPath;
	struct RoutingEdge *aEdge;
	char buf[128];

	if(routingPaths) {
		aPathItem = routingPaths->head;
		while(aPathItem) {
			aPath = (struct RoutingPath*)aPathItem->datap;
			aEdgeItem = aPath->edges.head;
			while(aEdgeItem) {
				aEdge = (struct RoutingEdge*)aEdgeItem->datap;
				sprintf(buf, "%s,%s", aEdge->head, aEdge->tail);
				hashtable_add_unique(table, buf, aEdge);	
				aEdgeItem = aEdgeItem->next;
			}	
			aPathItem = aPathItem->next;
		}
	}
}


struct RoutingEdge * routingEdge_copy_func(struct RoutingEdge *aEdge)
{
	struct RoutingEdge *rtEdge;

	if(aEdge) {
		rtEdge = (struct RoutingEdge*)malloc(sizeof(struct RoutingEdge));
		strncpy(rtEdge->head, aEdge->head, ADDRESS_LENGTH);
		strncpy(rtEdge->tail, aEdge->tail, ADDRESS_LENGTH);
		rtEdge->weight = aEdge->weight;
	}
	return rtEdge;
}


int pkg_has_id(int *id, struct Pkg *aPkg)
{
	return *id==aPkg->id; 
}

int pkg_has_earlier_startAt_than(struct Pkg *aPkg, struct Pkg *bPkg)
{
	return aPkg->startAt < bPkg->startAt;
}


void pkg_dump_func(FILE *fdump, struct Pkg *aPkg)
{
	fwrite(&aPkg->id, sizeof(unsigned), 1, fdump);
	fwrite(&aPkg->src,sizeof(char), NAME_LENGTH, fdump);
	fwrite(&aPkg->dst,sizeof(char), NAME_LENGTH, fdump);
	fwrite(&aPkg->startAt, sizeof(time_t), 1, fdump);
	fwrite(&aPkg->endAt, sizeof(time_t), 1, fdump);
	fwrite(&aPkg->size, sizeof(unsigned), 1, fdump);
	fwrite(&aPkg->ttl, sizeof(int), 1, fdump);
	fwrite(&aPkg->value, sizeof(double), 1, fdump);
	duallist_dump(fdump, &aPkg->routingRecord, (void(*)(FILE*,void*))routingRecord_dump_func);
	duallist_dump(fdump, &aPkg->routingPaths, (void(*)(FILE*, void*))routingPath_dump_func);
	fwrite(&aPkg->nEdges, sizeof(unsigned long), 1, fdump);
}

struct Pkg* pkg_load_func(FILE *fload)
{
	struct Pkg *newPkg;

	newPkg = (struct Pkg*)malloc(sizeof(struct Pkg));
	duallist_init(&newPkg->routingRecord);
	hashtable_init(&newPkg->routingGraph, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routingEdge_has_key);

	fread(&newPkg->id, sizeof(unsigned), 1, fload);
	fread(&newPkg->src,sizeof(char), NAME_LENGTH, fload);
	fread(&newPkg->dst,sizeof(char), NAME_LENGTH, fload);
	fread(&newPkg->startAt, sizeof(time_t), 1, fload);
	fread(&newPkg->endAt, sizeof(time_t), 1, fload);
	fread(&newPkg->size, sizeof(unsigned), 1, fload);
	fread(&newPkg->ttl, sizeof(int), 1, fload);
	fread(&newPkg->value, sizeof(double), 1, fload);
	duallist_load(fload, &newPkg->routingRecord, (void*(*)(FILE*))routingRecord_load_func);
	duallist_load(fload, &newPkg->routingPaths, (void*(*)(FILE*))routingPath_load_func);
	fread(&newPkg->nEdges, sizeof(unsigned long), 1, fload);
	hashtable_init(&newPkg->routingGraph, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))routingEdge_has_key);
	setup_routingGraph_hashtable(&newPkg->routingGraph, &newPkg->routingPaths);
	return newPkg;
}

struct Pkg* pkg_delievered_load_func(FILE *fload)
{
	struct Pkg *aPkg;
	
	aPkg = pkg_load_func(fload);
	if(aPkg && aPkg->endAt == 0) {
		free(aPkg);
		return NULL;
	} else
		return aPkg;
}

void pkg_print(FILE *fout, struct Pkg *aPkg)
{
	struct RoutingPath *aPath;
	struct RoutingEdge *aEdge;
	struct Item *aPathItem, *aEdgeItem;
	int i;

	if(fout) {
		fprintf(fout, "#%d, src:%s, dst:%s, #routingEdges:%ld, #distinct edges:%ld\n", aPkg->id, aPkg->src, aPkg->dst, aPkg->nEdges, aPkg->routingGraph.count);
		i = 0;
		aPathItem = aPkg->routingPaths.head;
		while(aPathItem) {
			aPath = (struct RoutingPath*)aPathItem->datap;
			fprintf(fout, "path #%d weight:%lf ", i, aPath->weight);
			aEdgeItem = aPath->edges.head;
			while (aEdgeItem) {
				aEdge = (struct RoutingEdge*)aEdgeItem->datap;
				fprintf(fout, "%s -> %s ", aEdge->head, aEdge->tail);
				aEdgeItem = aEdgeItem->next;
			}
			fprintf(fout, "\n");
			i ++;
			aPathItem = aPathItem->next;
		} 
	}
}

void routingRecord_dump_func(FILE *fdump, char* aRoutingRecord)
{	
	int length;
	length = strlen(aRoutingRecord);
	fwrite(&length, sizeof(int), 1, fdump);
	fwrite(aRoutingRecord, sizeof(char), length, fdump);
}

char* routingRecord_load_func(FILE *fload)
{
	char *newp;
	int length;

	fread(&length, sizeof(int), 1, fload);
	newp = (char*)malloc(sizeof(char)*(length+1));
	fread(newp, sizeof(char), length, fload);
	newp[length] = '\0';
	return newp;
}

void routingEdge_dump_func(FILE *fdump, struct RoutingEdge *aRoutingEdge)
{
	int length;
	length = strlen(aRoutingEdge->head);
	fwrite(&length, sizeof(int), 1, fdump);
	fwrite(aRoutingEdge->head, sizeof(char), length, fdump);
	length = strlen(aRoutingEdge->tail);
	fwrite(&length, sizeof(int), 1, fdump);
	fwrite(aRoutingEdge->tail, sizeof(char), length, fdump);
	fwrite(&aRoutingEdge->weight, sizeof(char), length, fdump);
}

struct RoutingEdge* routingEdge_load_func(FILE *fload)
{
	struct RoutingEdge *newp;
	int length;

	newp = (struct RoutingEdge*)malloc(sizeof(struct RoutingEdge));
	memset(newp, 0, sizeof(struct RoutingEdge));
	fread(&length, sizeof(int), 1, fload);
	fread(newp->head, sizeof(char), length, fload);
	fread(&length, sizeof(int), 1, fload);
	fread(newp->tail, sizeof(char), length, fload);
	fread(&newp->weight, sizeof(char), length, fload);
	return newp;
}

int routingEdge_has_key(char *key, struct RoutingEdge *aEdge)
{
	char buf[128];

	memset(buf, 0, 128);
	sprintf(buf, "%s,%s", aEdge->head, aEdge->tail);
	if (strcmp(key, buf)==0)
		return 1;
	else return 0;
}

int neighborEdge_has_larger_lefthops(struct NeighborEdge *thisNeighbor, struct NeighborEdge *otherNeighbor)
{
	return thisNeighbor->leftHops > otherNeighbor->leftHops;
}


int routingPath_has_smaller_weight(struct RoutingPath *aPath, struct RoutingPath *bPath)
{
	return aPath->weight < bPath->weight;
}


void routingPath_init_func(struct RoutingPath *aRoutingPath)
{
	duallist_init(&aRoutingPath->edges);
	aRoutingPath->weight = 0;
}

void routingPath_free_func(struct RoutingPath *aRoutingPath)
{
	if(aRoutingPath) {
		duallist_destroy(&aRoutingPath->edges, free);
	}
	free(aRoutingPath);
}


void routingPath_dump_func(FILE *fdump, struct RoutingPath *aRoutingPath)
{
	duallist_dump(fdump, &aRoutingPath->edges, (void(*)(FILE*,void*))routingEdge_dump_func);
	fwrite(&aRoutingPath->weight, sizeof(double), 1, fdump);
}

struct RoutingPath* routingPath_load_func(FILE *fload)
{
	struct RoutingPath *newPath;

	newPath = (struct RoutingPath*)malloc(sizeof(struct RoutingPath));
	routingPath_init_func(newPath);
	duallist_load(fload, &newPath->edges, (void*(*)(FILE*))routingEdge_load_func);
	fread(&newPath->weight, sizeof(double), 1, fload);
	return newPath;
}

struct RoutingPath * routingPath_copy_func(struct RoutingPath *aPath)
{
	struct RoutingPath *newPath;

	newPath = (struct RoutingPath*)malloc(sizeof(struct RoutingPath));
	routingPath_init_func(newPath);
	duallist_copy(&newPath->edges, &aPath->edges, (void*(*)(void*))routingEdge_copy_func);
	newPath->weight = aPath->weight;
	return newPath;
}

