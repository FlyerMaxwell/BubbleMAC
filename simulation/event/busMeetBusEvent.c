#include<stdlib.h>
#include<string.h>
#include"node.h"
#include"trace.h"
#include"geometry.h"
#include"contact.h"
#include"cntEvent.h"
#include"busMeetBusEvent.h"

int bus_meet_bus_event_handler(void* nul, struct Simulator *aSim, struct BusMeetBusEvent *aBusMeetBusEvent)
{
	struct Node *bNode1, *bNode2;
	int rt;

	bNode1 = lookup_node(&aSim->vnodes, strtok(aBusMeetBusEvent->bname1, "@"));
	if(strstr(strtok(NULL, "\0"), "upway"))
		bNode1->onRoute = ABS(bNode1->onRoute);
	else
		bNode1->onRoute = -ABS(bNode1->onRoute);

	bNode2 = lookup_node(&aSim->vnodes, strtok(aBusMeetBusEvent->bname2, "@"));
	if(strstr(strtok(NULL, "\0"), "upway"))
		bNode2->onRoute = ABS(bNode2->onRoute);
	else
		bNode2->onRoute = -ABS(bNode2->onRoute);

	if(!bNode1 || !bNode2) 
		return 1;

	if(aSim->oracle->type == TYPE_ORACLE_BUS_ONION) 
		rt = process_bus_onion_cnt_event(aSim, bNode1, bNode2, aBusMeetBusEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_BUS_EPIDEMIC) 
		rt = process_bus_epidemic_cnt_event(aSim, bNode1, bNode2, aBusMeetBusEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_BUS_SHAN) 
		rt = process_bus_shan_cnt_event(aSim, bNode1, bNode2, aBusMeetBusEvent);

	return rt;
}

int process_bus_onion_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct BusMeetBusEvent *aBusMeetBusEvent)
{
	return process_bus_shan_cnt_event(aSim, aNode, bNode, aBusMeetBusEvent);
}

int process_bus_epidemic_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct BusMeetBusEvent *aBusMeetBusEvent)
{
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		if( aPkg->value) {
			newPkg = pkg_copy_func(aPkg);
			newPkg->value = aSim->fwdMethod;
			newPkg->ttl = aPkg->ttl - 1;
			node_recv(aSim, bNode, newPkg);
			aPkg->value -= 1;
			if(!aPkg->value) {
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}
		}
		aItem = aItem->next;
	}

	aItem = bNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		if( aPkg->value) {
			newPkg = pkg_copy_func(aPkg);
			newPkg->value = aSim->fwdMethod;
			newPkg->ttl = aPkg->ttl - 1;
			node_recv(aSim, aNode, newPkg);
			aPkg->value -= 1;
			if(!aPkg->value) {
				temp = aItem->next;
				storage_remove_pkg(bNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}
		}
		aItem = aItem->next;
	}
	return 0;
}


int process_bus_shan_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct BusMeetBusEvent *aBusMeetBusEvent)
{
	struct Item *aItem, *bItem, *temp;
	struct Pkg *aPkg, *newPkg;
	char buf[128];

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		sprintf(buf, "%d,%d", aNode->onRoute, bNode->onRoute);
		if((bItem = hashtable_find(&aPkg->routingGraph, buf))!= NULL && ((struct RoutingEdge*)bItem->datap)->quota > 0) {
			newPkg = pkg_copy_func(aPkg);
			newPkg->value = aSim->fwdMethod;
			newPkg->ttl = aPkg->ttl - 1;
			node_recv(aSim, bNode, newPkg);
			aPkg->value -= 1;
			((struct RoutingEdge*)bItem->datap)->quota -= 1;
			if(!aPkg->value) {
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}
		}
		aItem = aItem->next;
	}

	aItem = bNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		sprintf(buf, "%d,%d", bNode->onRoute, aNode->onRoute);
		if((bItem = hashtable_find(&aPkg->routingGraph, buf))!= NULL && ((struct RoutingEdge*)bItem->datap)->quota > 0) {
			newPkg = pkg_copy_func(aPkg);
			newPkg->value = aSim->fwdMethod;
			newPkg->ttl = aPkg->ttl - 1;
			node_recv(aSim, aNode, newPkg);
			aPkg->value -= 1;
			((struct RoutingEdge*)bItem->datap)->quota -= 1;
			if(!aPkg->value) {
				temp = aItem->next;
				storage_remove_pkg(bNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}
		}
		aItem = aItem->next;
	}
	return 0;
}


void setup_bus_meet_bus_events(struct Simulator *aSim, struct Hashtable *cntTable)
{
	unsigned long i;
	struct Item *aItem, *aBusMeetBusItem;
	struct Pair *aPair; 
	struct Contact *aBusMeetBus;
	struct Event *aEvent;
	struct BusMeetBusEvent *aBusMeetBusEvent;
	char vname1[32], vname2[32];
	
	if(cntTable == NULL || aSim == NULL)
		return;
	for(i=0;i<cntTable->size;i++) {
		aItem = cntTable->head[i];
		while(aItem!=NULL) {
			aPair = (struct Pair*)aItem->datap;
			/* setup events */
			if(aPair->vName1[0]=='b' && aPair->vName2[0]=='b' && aPair->contents.head != NULL) {
				strncpy(vname1, aPair->vName1, 32);
				strncpy(vname2, aPair->vName2, 32);
				if(hashtable_find(&aSim->vnodes, strtok(vname1, "@")) && hashtable_find(&aSim->vnodes, strtok(vname2, "@"))) {
					aBusMeetBusItem = aPair->contents.head;
					while(aBusMeetBusItem) {
						aBusMeetBus = (struct Contact*)aBusMeetBusItem->datap;
						if(aBusMeetBus->startAt >= aSim->exprStartAt && aBusMeetBus->startAt <= aSim->exprEndAt) {
							aBusMeetBusEvent = (struct BusMeetBusEvent*)malloc(sizeof(struct BusMeetBusEvent));
							strncpy(aBusMeetBusEvent->bname1, aPair->vName1, 2*NAME_LENGTH);
							strncpy(aBusMeetBusEvent->bname2, aPair->vName2, 2*NAME_LENGTH);
							aBusMeetBusEvent->gPoint.x = aBusMeetBus->gPoint.x;
							aBusMeetBusEvent->gPoint.y = aBusMeetBus->gPoint.y;
							aBusMeetBusEvent->timestamp = aBusMeetBus->startAt;

							aEvent = (struct Event*)malloc(sizeof(struct Event));
							event_init_func(aEvent, aBusMeetBus->startAt, aSim, aBusMeetBusEvent, (int(*)(struct Simulator*, void*,void*))bus_meet_bus_event_handler);
							add_event(aSim, aEvent);
						}

						if(aBusMeetBus->endAt >= aSim->exprStartAt && aBusMeetBus->endAt <= aSim->exprEndAt) {
							aBusMeetBusEvent = (struct BusMeetBusEvent*)malloc(sizeof(struct BusMeetBusEvent));
							strncpy(aBusMeetBusEvent->bname1, aPair->vName1, 2*NAME_LENGTH);
							strncpy(aBusMeetBusEvent->bname2, aPair->vName2, 2*NAME_LENGTH);
							aBusMeetBusEvent->gPoint.x = aBusMeetBus->gPoint.x;
							aBusMeetBusEvent->gPoint.y = aBusMeetBus->gPoint.y;
							aBusMeetBusEvent->timestamp = aBusMeetBus->endAt;

							aEvent = (struct Event*)malloc(sizeof(struct Event));
							event_init_func(aEvent, aBusMeetBus->endAt, aSim, aBusMeetBusEvent, (int(*)(struct Simulator*, void*,void*))bus_meet_bus_event_handler);
							add_event(aSim, aEvent);

						} else if(aBusMeetBus->startAt > aSim->exprEndAt )
							break;
						aBusMeetBusItem = aBusMeetBusItem->next;
					}
				}
			}
			aItem = aItem->next;
		}
	}
}
