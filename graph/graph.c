#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"

int graphnode_has_name(char *name, struct GraphNode* aGraphNode)
{
	return !strcmp(name, aGraphNode->name);
}

int comm_has_larger_id(int *id, struct GraphComm* otherGraphComm)
{
	return *id > otherGraphComm->id;
}
int comm_has_id(int *id, struct GraphComm* otherGraphComm)
{
	return *id == otherGraphComm->id;
}
void comm_free_func(struct GraphComm *aGraphComm)
{
	duallist_destroy(&aGraphComm->nodes, NULL);
	free(aGraphComm);
}

void load_node_and_edge_lists(FILE *f, struct Duallist *nodeList, struct Duallist *edgeList)
{
	struct Hashtable nodes;
	char buf[1024];
	char *strp;
	struct GraphNode *aNode, *bNode;
	struct GraphEdge *aEdge;
	struct Item *aItem, *bItem;
	
	hashtable_init(&nodes, 2000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))graphnode_has_name);
	if(f) {
		fseek(f, 0, SEEK_SET);
		while(fgets(buf, 1024, f)) {
			strp=strtok(buf," \n");
			if((aItem = hashtable_find(&nodes, strp))==NULL) {
				aNode = (struct GraphNode*)malloc(sizeof(struct GraphNode));
				strncpy(aNode->name, strp, 32);
				aNode->community = 0;
				hashtable_add(&nodes, strp, aNode);
				duallist_add_to_tail(nodeList, aNode);
			} else {
				aNode = (struct GraphNode*)aItem->datap;
			}

			strp=strtok(NULL," \n");
			if((bItem = hashtable_find(&nodes, strp))==NULL) {
				bNode = (struct GraphNode*)malloc(sizeof(struct GraphNode));
				strncpy(bNode->name, strp, 32);
				bNode->community = 0;
				hashtable_add(&nodes, strp, bNode);
				duallist_add_to_tail(nodeList, bNode);
			} else {
				bNode = (struct GraphNode*)bItem->datap;
			}
			
			if(edgeList) {
				aEdge = (struct GraphEdge*)malloc(sizeof(struct GraphEdge));
				aEdge->aNode = aNode;
				aEdge->bNode = bNode;
				duallist_add_to_tail(edgeList, aEdge);
			}
		}
		hashtable_destroy(&nodes, NULL);
	}
}

void load_and_setup_comm_list(FILE *fsource, struct Duallist *nodeList, struct Duallist *commList)
{
	struct GraphComm *newComm;
	char buf[128];
	struct Item *aItem;
	struct GraphNode *aNode;
	int i, c;

	if(fsource) {
		fseek(fsource, 0, SEEK_SET);
		fgets(buf, 128, fsource);
		while(fgets(buf, 1024, fsource)) {
			newComm = (struct GraphComm*)malloc(sizeof(struct GraphComm));
			newComm->id = atoi(buf);
			duallist_init(&newComm->nodes);

			i = 0;
			memset(buf, 0, 128);
			do {
				c = fgetc(fsource);
				if(c==' ') {
					aItem = duallist_find(nodeList, buf, (int(*)(void*,void*))graphnode_has_name);	
					aNode = (struct GraphNode*)aItem->datap;
					duallist_add_to_tail(&newComm->nodes, aNode);	

					i = 0;
					memset(buf, 0, 128);
				} else {
					buf[i] = c;
					i++;
				}
				
			} while(c !='\n');
			duallist_add_to_tail(commList, newComm);
		}
	}
}
