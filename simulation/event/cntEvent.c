#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include"pkg.h"
#include"node.h"
#include"oracle.h"
#include"contact.h"
#include"cntEvent.h"

void setup_cnt_events(struct Simulator *aSim, struct Hashtable *cntTable)
{
	unsigned long i;
	struct Item *aItem, *aCntItem;
	struct Pair *aPair; 
	struct Contact *aCnt;
	struct Event *aEvent;
	struct CntEvent *aCntEvent;

	if(cntTable == NULL || aSim == NULL)
		return;
	for(i=0;i<cntTable->size;i++) {
		aItem = cntTable->head[i];
		while(aItem!=NULL) {
			aPair = (struct Pair*)aItem->datap;
			/* setup events */
			if(aPair->contents.head != NULL) {
				aCntItem = aPair->contents.head;
				while(aCntItem) {
					aCnt = (struct Contact*)aCntItem->datap;
					if(aCnt->startAt >= aSim->exprStartAt && aCnt->startAt < aSim->exprEndAt) {
						aCntEvent = (struct CntEvent*)malloc(sizeof(struct CntEvent));
						strncpy(aCntEvent->name1, aPair->vName1, NAME_LENGTH);
						strncpy(aCntEvent->name2, aPair->vName2, NAME_LENGTH);
						aCntEvent->timestamp = aCnt->startAt;
						aCntEvent->duration = aCnt->endAt-aCnt->startAt;
						if(aSim->oracle->type == TYPE_ORACLE_MARKOV || aSim->oracle->type == TYPE_ORACLE_ZOOM) { 
							if(aCntItem == aPair->contents.head)
								aCntEvent->ict = -1;
							else
								aCntEvent->ict = (aCnt->startAt - ((struct Contact*)aCntItem->prev->datap)->endAt)/aSim->oracle->tGran;
						}
						aEvent = (struct Event*)malloc(sizeof(struct Event));
						event_init_func(aEvent, aCnt->startAt, aSim, aCntEvent, (int(*)(struct Simulator*, void*,void*))cnt_event_handler);
						add_event(aSim, aEvent);

					} else if(aCnt->startAt > aSim->exprEndAt )
						break;
					aCntItem = aCntItem->next;
				}
			}
			aItem = aItem->next;
		}
	}
}


int cnt_event_handler(void *nul, struct Simulator *aSim, struct CntEvent *aCntEvent)
{
	int rt;
	struct Node *aNode, *bNode;

	
	aNode = lookup_node(&aSim->vnodes, aCntEvent->name1);
	bNode = lookup_node(&aSim->vnodes, aCntEvent->name2);

	if(aSim->oracle->type == TYPE_ORACLE_MARKOV) 
		rt = process_markov_cnt_event(aSim, aNode, bNode, aCntEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_AVGDLY) 
		rt = process_avgdly_cnt_event(aSim, aNode, bNode, aCntEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_AVGPRB) 
		rt = process_avgprb_cnt_event(aSim, aNode, bNode, aCntEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_EPIDEMIC) 
		rt = process_epidemic_cnt_event(aSim, aNode, bNode);
	else if(aSim->oracle->type == TYPE_ORACLE_BUBBLE) 
		rt = process_bubble_cnt_event(aSim, aNode, bNode);
	else if(aSim->oracle->type == TYPE_ORACLE_SIMBET) 
		rt = process_simbet_cnt_event(aSim, aNode, bNode, aCntEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_ZOOM) 
		rt = process_zoom_cnt_event(aSim, aNode, bNode, aCntEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_GLOBAL_SOCIAL) 
		rt = process_global_social_cnt_event(aSim, aNode, bNode, aCntEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_FUTURE_CONTACTS) 
		rt = process_future_contacts_cnt_event(aSim, aNode, bNode, aCntEvent);
	else if(aSim->oracle->type == TYPE_ORACLE_NODE_MARKOV ||aSim->oracle->type == TYPE_ORACLE_NODE_RANDOM || aSim->oracle->type == TYPE_ORACLE_NODE_STATIC || aSim->oracle->type == TYPE_ORACLE_NODE_PRIORI) 
		rt = process_node_cnt_event(aSim, aNode, bNode, aCntEvent);

	return rt;
}

int process_node_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{
	if (aNode->isPainter) {
		if (bNode->painted == 0)
			aSim->paintedNodes ++;
		bNode->painted ++;
		aSim->nPaints ++;
	}

	if(bNode->isPainter) {
		if (aNode->painted == 0)
			aSim->paintedNodes ++;
		aNode->painted ++;
		aSim->nPaints ++;
	}
	return 0;
}


int process_markov_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{
	struct Pairwise *aPairwise;
	int value;

	/* update neighborhood */
	node_met_a_node(aNode, bNode, aSim->oracle->neighborThreshold);
	node_met_a_node(bNode, aNode, aSim->oracle->neighborThreshold);

	if(aCntEvent->ict == -1)
		return 1;
	/* update the oracle first */	
	aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aCntEvent->name1, aCntEvent->name2);
	if(aPairwise) {
		if(aCntEvent->ict < aSim->oracle->T/aSim->oracle->tGran)
			value = aCntEvent->ict;
		else
			value = aSim->oracle->T/aSim->oracle->tGran;
		update_previous_states(aPairwise->preStates, aSim->oracle->order, value);
		aPairwise->estimation = estimate_next_delay(aPairwise, aPairwise->preStates, aSim->oracle->order, aSim->oracle->useDefault);

		aPairwise->total += aCntEvent->ict * aCntEvent->ict;
		aPairwise->defaultEstimate = aPairwise->total/(2*(aSim->oracle->trainingEndAt-aSim->oracle->trainingStartAt)/aSim->oracle->tGran);
	}
	/* check whether to forward packets*/
	oracle_check_for_relaying_pkgs(aSim, aNode, bNode);
	oracle_check_for_relaying_pkgs(aSim, bNode, aNode);
	return 0;
}

int process_avgdly_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{
	struct Pairwise *aPairwise;

	/* update neighborhood */
	node_met_a_node(aNode, bNode, aSim->oracle->neighborThreshold);
	node_met_a_node(bNode, aNode, aSim->oracle->neighborThreshold);

	if(aCntEvent->ict == -1)
		return 1;
	/* update the oracle first */	
	aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aCntEvent->name1, aCntEvent->name2);
	if(aPairwise) {
		aPairwise->total += aCntEvent->ict * aCntEvent->ict ;
		aPairwise->estimation = aPairwise->total/(2*(aSim->oracle->trainingEndAt-aSim->oracle->trainingStartAt));
	}
	/* check whether to forward packets*/
	oracle_check_for_relaying_pkgs(aSim, aNode, bNode);
	oracle_check_for_relaying_pkgs(aSim, bNode, aNode);
	return 0;
}

int process_avgprb_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{
	struct Pairwise *aPairwise;

	/* update neighborhood */
	node_met_a_node(aNode, bNode, aSim->oracle->neighborThreshold);
	node_met_a_node(bNode, aNode, aSim->oracle->neighborThreshold);

	if(aCntEvent->ict == -1)
		return 1;
	/* update the oracle first */	
	aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aCntEvent->name1, aCntEvent->name2);
	if(aPairwise) {
		aPairwise->total += aCntEvent->ict ;
		aPairwise->estimation = aPairwise->total/(aSim->oracle->trainingEndAt-aSim->oracle->trainingStartAt);
	}

	/* check whether to forward packets*/
	oracle_check_for_relaying_pkgs(aSim, aNode, bNode);
	oracle_check_for_relaying_pkgs(aSim, bNode, aNode);
	return 0;
}

int process_epidemic_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;

	/* update neighborhood */
	node_met_a_node(aNode, bNode, aSim->oracle->neighborThreshold);
	node_met_a_node(bNode, aNode, aSim->oracle->neighborThreshold);


	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		newPkg = pkg_copy_func(aPkg);
		if(strcmp(aPkg->dst, bNode->name)==0) {
			newPkg->endAt = aSim->clock;
			node_recv(aSim, bNode, newPkg);
			temp = aItem->next;
			storage_remove_pkg(aNode->storage, aPkg->id);
			aItem = temp;
			continue;
		} else { 
			node_recv(aSim, bNode, newPkg);
			aItem = aItem->next;
		}
	}

	aItem = bNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		newPkg = pkg_copy_func(aPkg);
		if(strcmp(aPkg->dst, aNode->name)==0) {
			newPkg->endAt = aSim->clock;
			node_recv(aSim, aNode, newPkg);
			temp = aItem->next;
			storage_remove_pkg(bNode->storage, aPkg->id);
			aItem = temp;
			continue;
		} else { 
			node_recv(aSim, aNode, newPkg);
			aItem = aItem->next;
		}
	}
	return 0;
}

int process_future_contacts_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{
	/* check whether to forward packets*/
	future_contacts_check_for_relaying_pkgs(aSim, aNode, bNode);
	future_contacts_check_for_relaying_pkgs(aSim, bNode, aNode);
	return 0;
}

int process_bubble_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	/* update neighborhood */
	node_met_a_node(aNode, bNode, aSim->oracle->neighborThreshold);
	node_met_a_node(bNode, aNode, aSim->oracle->neighborThreshold);

	/* check whether to forward packets*/
	bubble_check_for_relaying_pkgs(aSim, aNode, bNode);
	bubble_check_for_relaying_pkgs(aSim, bNode, aNode);
	return 0;
}

int process_global_social_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{

	/* check whether to forward packets*/
	global_social_check_for_relaying_pkgs(aSim, aNode, bNode);
	global_social_check_for_relaying_pkgs(aSim, bNode, aNode);

	return 0;
}

int process_simbet_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{
	/* update neighborhood */
	node_met_a_node(aNode, bNode, aSim->oracle->neighborThreshold);
	node_met_a_node(bNode, aNode, aSim->oracle->neighborThreshold);

	/* we should calculate betweenness of each node here,
 	* but considering the cost, we move the calculation in
 	* the training phase
 	*/

	/* check whether to forward packets*/
	simbet_check_for_relaying_pkgs(aSim, aNode, bNode);
	simbet_check_for_relaying_pkgs(aSim, bNode, aNode);

	return 0;
}


int process_zoom_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent)
{
	int value;
	int ictEstimation;
	struct Pairwise *aPairwise = NULL;
	struct NeighborNode *aNeighborNode, *bNeighborNode;
//	struct Estimation *aEstimation;

	if(aCntEvent->ict != -1) {
		/* update the Markov oracle first */	
		aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aCntEvent->name1, aCntEvent->name2);
		if(aPairwise) {
			if(aCntEvent->ict < aSim->oracle->T/aSim->oracle->tGran)
				value = aCntEvent->ict;
			else
				value = aSim->oracle->T/aSim->oracle->tGran;
			update_previous_states(aPairwise->preStates, aSim->oracle->order, value);
			aPairwise->estimation = estimate_next_delay(aPairwise, aPairwise->preStates, aSim->oracle->order, aSim->oracle->useDefault);

			aPairwise->total += aCntEvent->ict * aCntEvent->ict;
			aPairwise->defaultEstimate = aPairwise->total/(2*(aSim->oracle->trainingEndAt-aSim->oracle->trainingStartAt)/aSim->oracle->tGran);
		}
	}

	if(aPairwise && aPairwise->estimation!=-1)
		ictEstimation = (aPairwise->estimation + 1) * aSim->oracle->tGran;
	else
		ictEstimation = -1;
	/* update neighborhood */
	aNeighborNode = node_met_a_node(aNode, bNode, aSim->oracle->neighborThreshold);
	bNeighborNode = node_met_a_node(bNode, aNode, aSim->oracle->neighborThreshold);
/*
	if(aNeighborNode) {
		// add new estimation 
		aEstimation = (struct Estimation*)malloc(sizeof(struct Estimation));
		aEstimation->timestamp = aCntEvent->timestamp;
		aEstimation->estimatedTime = ictEstimation;
		duallist_add_to_head(&aNeighborNode->lastEstimations, aEstimation);
		assess_bingoRatio_on_neighbor(aNeighborNode, aSim->oracle->deltaT);
	}
					
	if(bNeighborNode) {
		aEstimation = (struct Estimation*)malloc(sizeof(struct Estimation));
		aEstimation->timestamp = aCntEvent->timestamp;
		aEstimation->estimatedTime = ictEstimation;
		duallist_add_to_head(&bNeighborNode->lastEstimations, aEstimation);
		assess_bingoRatio_on_neighbor(bNeighborNode, aSim->oracle->deltaT);
	}
*/
	/* check whether to forward packets*/
	zoom_check_for_relaying_pkgs(aSim, aNode, bNode);
	zoom_check_for_relaying_pkgs(aSim, bNode, aNode);

	return 0;
}

void update_network_bingoRatio(struct Simulator *aSim)
{
	int count;
	char changed;
	unsigned long i, j;
	struct Item *aItem, *bItem, *cItem, *temp;
	struct NeighborNode *aNeighborNode;
	struct Estimation *aEstimation;
	struct Node *cNode;

	/* remove all out-of-date estimations first*/
	for(i=0;i<aSim->vnodes.size;i++) {
		aItem = aSim->vnodes.head[i];
		while(aItem) {
			cNode = (struct Node*)aItem->datap;	
			for(j=0;j<cNode->neighbors.size;j++) {
				bItem = cNode->neighbors.head[j];
				while(bItem) {
					changed = 0;
					aNeighborNode = (struct NeighborNode*)bItem->datap;
					cItem = aNeighborNode->lastEstimations.head;
					while(cItem) {
						temp = cItem->next;
						aEstimation = (struct Estimation*)cItem->datap;
						if(aEstimation->timestamp < aSim->clock - aSim->oracle->lastT) {
							changed = 1;
							free(duallist_pick_item(&aNeighborNode->lastEstimations, cItem));
							cItem = temp;
						} else
							break;
					}

					if(changed) {
						count = 0;
						cItem = aNeighborNode->lastEstimations.head;
						while(cItem) {
							if(((struct Estimation*)cItem->datap)->bingo)
								count ++;
							cItem = cItem->next;
						}
						if(aNeighborNode->lastEstimations.nItems)
							aNeighborNode->bingoRatio = count*1.0/aNeighborNode->lastEstimations.nItems;
						else
							aNeighborNode->bingoRatio = 0;
					}

					bItem = bItem->next;
				}
			}
			aItem = aItem->next;
		}
	}
}

void future_contacts_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	struct Pairwise *aPairwise, *bPairwise;
	struct Item *aItem, *temp, *bItem, *cItem;
	struct Pkg *aPkg, *newPkg;
	char doFwd;
	time_t aDstTime, bDstTime;
	struct Contact *aContact;

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aNode->name, aPkg->dst);
		bPairwise = lookup_pairwise_in_oracle(aSim->oracle, bNode->name, aPkg->dst);

		aDstTime = -1, bDstTime = -1;
		if(aPairwise) {
			bItem = aPairwise->futureContacts.head;
			while(bItem) {
				aContact = (struct Contact*)bItem->datap;
				if(aContact->startAt<aSim->clock) {
					cItem = bItem->next;
					free(duallist_pick_item(&aPairwise->futureContacts, bItem));
					bItem = cItem;
					continue;
				} else {
					aDstTime = aContact->startAt - aSim->clock;
					break;
				}
				bItem = bItem->next;
			}
		}
		if(bPairwise) {
			bItem = bPairwise->futureContacts.head;
			while(bItem) {
				aContact = (struct Contact*)bItem->datap;
				if(aContact->startAt<aSim->clock) {
					cItem = bItem->next;
					free(duallist_pick_item(&bPairwise->futureContacts, bItem));
					bItem = cItem;
					continue;
				} else {
					bDstTime = aContact->startAt - aSim->clock;
					break;
				}
				bItem = bItem->next;
			}
		}
		
		doFwd = ( (aDstTime == -1 && bDstTime >= 0) 
			||(aDstTime >= 0 && bDstTime >= 0 && aDstTime > bDstTime) );
		if(strcmp(aPkg->dst, bNode->name)==0) {
			newPkg = pkg_copy_func(aPkg);
			newPkg->endAt = aSim->clock;
			node_recv(aSim, bNode, newPkg);
			temp = aItem->next;
			storage_remove_pkg(aNode->storage, aPkg->id);
			aItem = temp;
			continue;

		} else if(doFwd) {
			newPkg = pkg_copy_func(aPkg);
			node_recv(aSim, bNode, newPkg);
			temp = aItem->next;
			storage_remove_pkg(aNode->storage, aPkg->id);
			aItem = temp;
			continue;
		}
		aItem = aItem->next;
	}
}

void oracle_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	struct Pairwise *aPairwise, *bPairwise;
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;
	char doFwd;

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aNode->name, aPkg->dst);
		bPairwise = lookup_pairwise_in_oracle(aSim->oracle, bNode->name, aPkg->dst);

		doFwd =  (((aPairwise && aPairwise->estimation==-1)||aPairwise==NULL) && bPairwise && bPairwise->estimation!=-1)
			|| ( aPairwise && aPairwise->estimation!=-1 && bPairwise && bPairwise->estimation!=-1 && aPairwise->estimation>bPairwise->estimation); 
		if(aSim->fwdMethod == NO_REPLICA_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;

			 } else if( doFwd) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}


		} else if(aSim->fwdMethod == BETTER_ESTIMATE_FWD) {
			if ( strcmp(aPkg->dst, bNode->name)==0 ) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else if( doFwd) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
			}

		} else if(aSim->fwdMethod == EVERBEST_ESTIMATE_FWD) {
			if ( strcmp(aPkg->dst, bNode->name)==0 ) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else  if( (aPkg->value==-1 && doFwd)
				||  (aPkg->value!=-1 && bPairwise && bPairwise->estimation!=-1 && aPkg->value>bPairwise->estimation)) {
				newPkg = pkg_copy_func(aPkg);
				aPkg->value = bPairwise->estimation;
				newPkg->value = bPairwise->estimation;
				node_recv(aSim, bNode, newPkg);
			}

		}
		aItem = aItem->next;
	}
}

void bubble_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		if(aSim->fwdMethod == NO_REPLICA_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			 } else if( aNode->neighbors.count < bNode->neighbors.count ) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}


		} else if(aSim->fwdMethod == BETTER_ESTIMATE_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else if ( aNode->neighbors.count < bNode->neighbors.count ) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
			}

		} else if(aSim->fwdMethod == EVERBEST_ESTIMATE_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else  if ((aPkg->value==-1 && aNode->neighbors.count < bNode->neighbors.count) 
				||  (aPkg->value!=-1 && bNode->neighbors.count > aPkg->value)) {
				newPkg = pkg_copy_func(aPkg);
				aPkg->value = bNode->neighbors.count;
				newPkg->value = bNode->neighbors.count;
				node_recv(aSim, bNode, newPkg);
			}
		}
		aItem = aItem->next;
	}
}

void global_social_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;
	struct Node *dstNode;
	double sim_aNode_dst, sim_bNode_dst, SimUtil_aNode, SimUtil_bNode, BetUtil_aNode, BetUtil_bNode, SimBetUtil_aNode, SimBetUtil_bNode;

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		dstNode = lookup_node(&aSim->vnodes, aPkg->dst);
		/* calculate the similarity between aNode and the dest*/
		sim_aNode_dst = check_similarity_with_oracle(aSim->oracle, aNode, dstNode);
		/* calculate the similarity between bNode and the dest*/
		sim_bNode_dst = check_similarity_with_oracle(aSim->oracle, bNode, dstNode);
		if(sim_aNode_dst+sim_bNode_dst == 0) {
			SimUtil_aNode = 0;
			SimUtil_bNode = 0;
		} else {
			SimUtil_aNode = sim_aNode_dst/(sim_aNode_dst+sim_bNode_dst);
			SimUtil_bNode = sim_bNode_dst/(sim_aNode_dst+sim_bNode_dst);
		}

		if(aNode->betweenness+bNode->betweenness == 0) {
			BetUtil_aNode = 0;
			BetUtil_bNode = 0;
		} else {
			BetUtil_aNode = aNode->betweenness/(aNode->betweenness+bNode->betweenness);
			BetUtil_bNode = bNode->betweenness/(aNode->betweenness+bNode->betweenness);
		}
		SimBetUtil_aNode = aSim->oracle->alfar*SimUtil_aNode + aSim->oracle->beta*BetUtil_aNode;
		SimBetUtil_bNode = aSim->oracle->alfar*SimUtil_bNode + aSim->oracle->beta*BetUtil_bNode;

		if(aSim->fwdMethod == NO_REPLICA_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			 } else if( SimBetUtil_aNode < SimBetUtil_bNode ) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			 }


		} else if(aSim->fwdMethod == BETTER_ESTIMATE_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			 } else if( SimBetUtil_aNode < SimBetUtil_bNode ) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
			 }


		} else if(aSim->fwdMethod == EVERBEST_ESTIMATE_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else  if ((aPkg->value==-1 && SimBetUtil_aNode < SimBetUtil_bNode) 
				||  (aPkg->value!=-1 && SimBetUtil_bNode > aPkg->value)) {
				newPkg = pkg_copy_func(aPkg);
				aPkg->value = SimBetUtil_bNode;
				newPkg->value = SimBetUtil_bNode;
				node_recv(aSim, bNode, newPkg);
			}
		}
		aItem = aItem->next;
	}
}

void simbet_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;
	struct Node *dstNode;
	double sim_aNode_dst, sim_bNode_dst, SimUtil_aNode, SimUtil_bNode, BetUtil_aNode, BetUtil_bNode, SimBetUtil_aNode, SimBetUtil_bNode;

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		dstNode = lookup_node(&aSim->vnodes, aPkg->dst);
		/* calculate the similarity between aNode and the dest*/
		sim_aNode_dst = calculate_similarity(aNode, dstNode);
		/* calculate the similarity between bNode and the dest*/
		sim_bNode_dst = calculate_similarity(bNode, dstNode);
		if(sim_aNode_dst+sim_bNode_dst == 0) {
			SimUtil_aNode = 0;
			SimUtil_bNode = 0;
		} else {
			SimUtil_aNode = sim_aNode_dst/(sim_aNode_dst+sim_bNode_dst);
			SimUtil_bNode = sim_bNode_dst/(sim_aNode_dst+sim_bNode_dst);
		}

		if(aNode->betweenness+bNode->betweenness == 0) {
			BetUtil_aNode = 0;
			BetUtil_bNode = 0;
		} else {
			BetUtil_aNode = aNode->betweenness/(aNode->betweenness+bNode->betweenness);
			BetUtil_bNode = bNode->betweenness/(aNode->betweenness+bNode->betweenness);
		}
		SimBetUtil_aNode = aSim->oracle->alfar*SimUtil_aNode + aSim->oracle->beta*BetUtil_aNode;
		SimBetUtil_bNode = aSim->oracle->alfar*SimUtil_bNode + aSim->oracle->beta*BetUtil_bNode;

		if(aSim->fwdMethod == NO_REPLICA_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			 } else if( SimBetUtil_aNode < SimBetUtil_bNode ) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			 }


		} else if(aSim->fwdMethod == BETTER_ESTIMATE_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			 } else if( SimBetUtil_aNode < SimBetUtil_bNode ) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
			 }


		} else if(aSim->fwdMethod == EVERBEST_ESTIMATE_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else  if ((aPkg->value==-1 && SimBetUtil_aNode < SimBetUtil_bNode) 
				||  (aPkg->value!=-1 && SimBetUtil_bNode > aPkg->value)) {
				newPkg = pkg_copy_func(aPkg);
				aPkg->value = SimBetUtil_bNode;
				newPkg->value = SimBetUtil_bNode;
				node_recv(aSim, bNode, newPkg);
			}
		}
		aItem = aItem->next;
	}
}

void zoom_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode)
{
	struct Pairwise *aPairwise, *bPairwise;
	struct Item *aItem, *temp;
	//struct Item *bItem, *cItem;
	struct Pkg *aPkg, *newPkg;
	//struct NeighborNode *aDstNeighbor, *bDstNeighbor;
	char cond1, cond2, cond3, doFwd;

	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aNode->name, aPkg->dst);
		bPairwise = lookup_pairwise_in_oracle(aSim->oracle, bNode->name, aPkg->dst);

		cond1 = ((aPairwise && aPairwise->estimation==-1)||aPairwise==NULL) && bPairwise && bPairwise->estimation!=-1;
		cond2 = aPairwise && aPairwise->estimation!=-1 && bPairwise && bPairwise->estimation!=-1 && aPairwise->estimation>bPairwise->estimation;
		cond3 = ((aPairwise && aPairwise->estimation==-1)||aPairwise==NULL) && ((bPairwise && bPairwise->estimation==-1)||bPairwise==NULL) && (aNode->betweenness<bNode->betweenness);
		doFwd = cond1 || cond2 || cond3;


/*
		cond1 = ((aPairwise && aPairwise->estimation==-1)||aPairwise==NULL) && bPairwise && bPairwise->estimation!=-1;
		cond2 = aPairwise && aPairwise->estimation!=-1 && bPairwise && bPairwise->estimation!=-1 && aPairwise->estimation>bPairwise->estimation;

		doFwd = 0;
		bItem = hashtable_find(&aNode->neighbors, aPkg->dst);
		cItem = hashtable_find(&bNode->neighbors, aPkg->dst);
		if(bItem && cItem) {
			aDstNeighbor = (struct NeighborNode*)bItem->datap;
			bDstNeighbor = (struct NeighborNode*)cItem->datap;
			if(aDstNeighbor->bingoRatio >= aSim->oracle->tuner && bDstNeighbor->bingoRatio>=aSim->oracle->tuner)
				doFwd = cond1 || cond2;
			else if ( bDstNeighbor->bingoRatio>=aSim->oracle->tuner )
				doFwd = bPairwise && bPairwise->estimation!=-1;
			else
				doFwd = aNode->betweenness<bNode->betweenness;
		} else if (cItem) {
			bDstNeighbor = (struct NeighborNode*)cItem->datap;
			if ( bDstNeighbor->bingoRatio>=aSim->oracle->tuner )
				doFwd = bPairwise && bPairwise->estimation!=-1;
			else
				doFwd = aNode->betweenness<bNode->betweenness;
		}
*/
		if(aSim->fwdMethod == NO_REPLICA_FWD) {
			 if(strcmp(aPkg->dst, bNode->name)==0) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;

			 } else if(doFwd) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}


		} else if(aSim->fwdMethod == BETTER_ESTIMATE_FWD) {
			if ( strcmp(aPkg->dst, bNode->name)==0 ) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else if( doFwd) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
			}

		} else if(aSim->fwdMethod == EVERBEST_ESTIMATE_FWD) {
			if ( strcmp(aPkg->dst, bNode->name)==0 ) {
				newPkg = pkg_copy_func(aPkg);
				newPkg->endAt = aSim->clock;
				node_recv(aSim, bNode, newPkg);
				temp = aItem->next;
				storage_remove_pkg(aNode->storage, aPkg->id);
				aItem = temp;
				continue;
			} else  if( (aPkg->value==-1 && doFwd)
				||  (aPkg->value!=-1 && bPairwise && bPairwise->estimation!=-1 && aPkg->value>bPairwise->estimation)) {
				newPkg = pkg_copy_func(aPkg);
				aPkg->value = bPairwise->estimation;
				newPkg->value = bPairwise->estimation;
				node_recv(aSim, bNode, newPkg);
			}

		}
		aItem = aItem->next;
	}


}



int node_recv(struct Simulator *aSim, struct Node *aNode, struct Pkg *aPkg)
{
	struct Item *aItem, *aPathItem, *aEdgeItem;
	struct Pkg *bPkg;
	char buf[32], buf1[32];
	struct RoutingPath *aRoutingPath;
	struct RoutingEdge *aRoutingEdge;
	int hops, find, i;
	struct NeighborEdge *newp;
	struct Duallist nEdges;

	aSim->trafficCount ++;
	if(strcmp(aNode->name, aPkg->dst)==0) {
		aItem = duallist_find(&aSim->pkgs, &aPkg->id, (int(*)(void*,void*))pkg_has_id);
		bPkg = (struct Pkg*)aItem->datap;
		if( bPkg->endAt == 0 
		|| (bPkg->endAt && aPkg->endAt<bPkg->endAt) 
		|| (bPkg->endAt && aPkg->endAt==bPkg->endAt && aPkg->routingRecord.nItems < bPkg->routingRecord.nItems)) {
			aSim->sentPkgs ++;
			bPkg->endAt = aPkg->endAt;
			duallist_destroy(&bPkg->routingRecord, free);
			duallist_copy(&bPkg->routingRecord, &aPkg->routingRecord,(void*(*)(void*))string_copy_func);
		} 
		pkg_free_func(aPkg);
		return 0;
	} else {
		if(aPkg->ttl == 0) {
			pkg_free_func(aPkg);
			return 0;
		}
		if(storage_lookup_pkg(aNode->storage, aPkg->id)==NULL) {
			if(aSim->pkgRcdRoute) {
				if(aNode->onRoute)
					sprintf(buf, "%d", aNode->onRoute);
				else
					sprintf(buf, "%s", aNode->name);
				duallist_add_to_tail(&aPkg->routingRecord, string_copy_func(buf));
			} 

			// divide forwarding quota among edges
			duallist_init(&nEdges);
			aPathItem = aPkg->routingPaths.head;
			while(aPathItem) {
				aRoutingPath = (struct RoutingPath*)aPathItem->datap;
				hops = 0, find = 0;
				aEdgeItem = aRoutingPath->edges.head;
				while(aEdgeItem) {
					aRoutingEdge = (struct RoutingEdge*)aEdgeItem->datap;
					sprintf(buf, "%d", aNode->onRoute);
					sprintf(buf1, "%d", -aNode->onRoute);

					if(strcmp(aRoutingEdge->head,buf) == 0 || strcmp(aRoutingEdge->head, buf1) == 0) {
						find = 1;
						break;
					}
					hops ++;
					aEdgeItem = aEdgeItem->next;
				}					
				if(find) {
					newp = (struct NeighborEdge*)malloc(sizeof(struct NeighborEdge));
					newp->neighbor = aRoutingEdge;
					newp->leftHops = aRoutingPath->edges.nItems - hops;
					duallist_add_in_sequence_from_head(&nEdges, newp, (int(*)(void*,void*))neighborEdge_has_larger_lefthops);
				}
				aPathItem = aPathItem->next;
			}
			i = 0;
			aItem = nEdges.head;
			while(aItem) {
				if(i < ((int)aPkg->value)%nEdges.nItems) 
					((struct NeighborEdge*)aItem->datap)->neighbor->quota = ((int)aPkg->value)/nEdges.nItems + 1;
				else
					((struct NeighborEdge*)aItem->datap)->neighbor->quota = ((int)aPkg->value)/nEdges.nItems;
				i++;
				aItem = aItem->next;
			}
			duallist_destroy(&nEdges, free);

			return storage_add_pkg(aNode->storage, aPkg);
		} else {
			pkg_free_func(aPkg);
			return 0;
		}
	}
}

