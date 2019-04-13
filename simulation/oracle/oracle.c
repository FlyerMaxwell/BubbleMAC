#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"oracle.h"
#include"contact.h"

void pairwise_init_func(struct Pairwise *aPairwise, char *name1, char *name2, int k)
{
	if(aPairwise == NULL)
		return;
	strncpy(aPairwise->name1, name1, NAME_LENGTH);
	strncpy(aPairwise->name2, name2, NAME_LENGTH);

	aPairwise->total = 0;
	aPairwise->estimation = 0;
	aPairwise->similarity = 0;
	if(k>0) {
		aPairwise->preStates = (int*)malloc(sizeof(int)*k);
	} else
		aPairwise->preStates = NULL;
	hashtable_init(&aPairwise->transitions, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))transition_has_history);
	duallist_init(&aPairwise->futureContacts);
}


void pairwise_free_func(struct Pairwise *aPairwise)
{
	if(aPairwise == NULL)
		return;
	if(aPairwise->preStates != NULL)
		free(aPairwise->preStates);
	hashtable_destroy(&aPairwise->transitions, (void(*)(void*))transition_free_func);
	duallist_destroy(&aPairwise->futureContacts, free);
}



int pairwise_has_names(char *names, struct Pairwise *aPairwise)
{
	char buf[128];
	sprintf(buf, "%s,%s", aPairwise->name1, aPairwise->name2);
	return !strcmp(names, buf);
}



struct Pairwise* lookup_pairwise_in_oracle(struct Oracle *oracle, char *vname1, char*vname2) 
{
	struct Item *aItem;
	struct Pairwise *aPairwise;
	char key[64], *name1, *name2;

	if (0 > strcmp(vname1, vname2)) {
		name1 = vname1;
		name2 = vname2;
	} else {
		name1 = vname2;
		name2 = vname1;
	}
	sprintf(key, "%s,%s", name1, name2);
	aItem = hashtable_find(&oracle->pairwises, key);
	if(aItem != NULL) {
		aPairwise = (struct Pairwise*)aItem->datap;
	} else {
		aPairwise = NULL;
	} 
	return aPairwise;
}

void nodewise_init_func(struct Nodewise *aNodewise, char *name, int k)
{
	if(aNodewise == NULL)
		return;
	strncpy(aNodewise->name, name, NAME_LENGTH);

	aNodewise->total = 0;
	aNodewise->estimation = 0;
	aNodewise->community = -1;
	if(k>0) {
		aNodewise->preStates = (int*)malloc(sizeof(int)*k);
	} else
		aNodewise->preStates = NULL;
	hashtable_init(&aNodewise->transitions, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))transition_has_history);
	duallist_init(&aNodewise->centralities);
}


void nodewise_free_func(struct Nodewise *aNodewise)
{
	if(aNodewise == NULL)
		return;
	if(aNodewise->preStates != NULL)
		free(aNodewise->preStates);
	hashtable_destroy(&aNodewise->transitions, (void(*)(void*))transition_free_func);
	duallist_destroy(&aNodewise->centralities, free);
}

int nodewise_has_name(char *name, struct Nodewise *aNodewise)
{
	return !strcmp(name, aNodewise->name);
}

struct Nodewise* lookup_nodewise_in_oracle(struct Oracle *oracle, char *vname)
{
	struct Item *aItem;
	struct Nodewise *aNodewise;

	aItem = hashtable_find(&oracle->nodewises, vname);
	if(aItem != NULL) {
		aNodewise = (struct Nodewise*)aItem->datap;
	} else {
		aNodewise = NULL;
	} 
	return aNodewise;
}

double calculate_estimation(struct Transition *aTrans)
{
	struct Item *aItem;
	struct Prob *aProb;
	double rst = 0;

	if(aTrans == NULL)
		return -1;
	aItem = aTrans->probs.head;
	while(aItem != NULL) {
		aProb = (struct Prob*)aItem->datap;
		rst = rst + aProb->state * aProb->prob;
		aItem = aItem->next;
	}
	return rst;
}


int prob_has_state(int *state, struct Prob *aProb)
{
	return *state == aProb->state;
}


void transition_init_func(struct Transition *aTrans, char *history)
{
	if(aTrans == NULL)
		return;
	strncpy(aTrans->history, history, 64);
	duallist_init(&aTrans->probs);
}

void transition_free_func(struct Transition *aTrans)
{
	if(aTrans==NULL)
		return;
	duallist_destroy(&aTrans->probs, free);
}


int transition_has_history(char *history, struct Transition *aTrans)
{
	if(aTrans == NULL)
		return 0;
	return !strcmp(history, aTrans->history);
}


void oracle_init_func(struct Oracle *oracle, int type, struct Simulator *aSim, time_t trainingStartAt, time_t trainingEndAt)
{
	if(oracle == NULL)
		return;
	oracle->type = type;
	oracle->aSim = aSim;
	oracle->trainingStartAt = trainingStartAt;
	oracle->trainingEndAt = trainingEndAt;
	oracle->size = 0;
	oracle->painterFile = NULL;
	oracle->communityFile = NULL;
	if(type == TYPE_ORACLE_MARKOV)
		oracle->setup_oracle = setup_oracle_mkv;
	else if (type == TYPE_ORACLE_AVGDLY)
		oracle->setup_oracle = setup_oracle_avg;
	else if (type == TYPE_ORACLE_AVGPRB)
		oracle->setup_oracle = setup_oracle_prob;
	else if (type == TYPE_ORACLE_BUBBLE)
		oracle->setup_oracle = setup_oracle_bubble;
	else if (type == TYPE_ORACLE_SIMBET)
		oracle->setup_oracle = setup_oracle_simbet;
	else if (type == TYPE_ORACLE_ZOOM)
		oracle->setup_oracle = setup_oracle_zoom;
	else if (type == TYPE_ORACLE_EPIDEMIC)
		oracle->setup_oracle = setup_oracle_epidemic;
	else if (type == TYPE_ORACLE_FUTURE_CONTACTS)
		oracle->setup_oracle = setup_oracle_future_contacts;
	else if (type == TYPE_ORACLE_GLOBAL_SOCIAL)
		oracle->setup_oracle = setup_oracle_global_social;
	else if (type == TYPE_ORACLE_NODE_MARKOV)
		oracle->setup_oracle = setup_oracle_node_mkv;
	else if (type == TYPE_ORACLE_NODE_STATIC)
		oracle->setup_oracle = setup_oracle_node_static;
	else if (type == TYPE_ORACLE_NODE_PRIORI)
		oracle->setup_oracle = setup_oracle_node_priori;
	else
		oracle->setup_oracle = NULL;

	hashtable_init(&oracle->pairwises, 10000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pairwise_has_names);
	hashtable_init(&oracle->nodewises, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))nodewise_has_name);
}

void oracle_free_func(struct Oracle *oracle)
{
	if(oracle) {
		hashtable_destroy(&oracle->pairwises, (void(*)(void*))pairwise_free_func);
		hashtable_destroy(&oracle->nodewises, (void(*)(void*))nodewise_free_func);
		free(oracle);
	}
}


void setup_oracle_mkv(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem, *bCntItem, *aTransItem, *aProbItem;
	struct Pair *aCntPair;
	int aIct;
	int k, valid, value, state;
	struct Pairwise *aPairwise;
	struct Transition *aTrans;
	struct Prob *aProb;
	char key[64], history[1024], strp[32]; 

	if(oracle == NULL || oracle->order == 0)
		return;
 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	setup_neighborhood(oracle);

	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the pairwise table */
			aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2 );
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, aCntPair->vName1, aCntPair->vName2, oracle->order);
				sprintf(key, "%s,%s", aCntPair->vName1, aCntPair->vName2);
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
			/* setup transitions */
			aPairwise->total = 0;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next) {
				if(((struct Contact*)aCntItem->next->datap)->startAt > oracle->trainingStartAt 
				 &&((struct Contact*)aCntItem->next->datap)->startAt < oracle->trainingEndAt) {
					aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/oracle->tGran;
					aPairwise->total += aIct * aIct;
					valid = 1;
					bCntItem = aCntItem;
					memset(history, 0, 1024);
					for(k=0;k<oracle->order;k++) {
						if(bCntItem && bCntItem->next) {
							aIct = (((struct Contact*)bCntItem->next->datap)->startAt - ((struct Contact*)bCntItem->datap)->endAt)/oracle->tGran;	
							if(aIct < oracle->T/oracle->tGran)
								value = aIct;
							else
								value = oracle->T/oracle->tGran;
							memset(strp, 0, 32);
							sprintf(strp, "%d,", value);
							strcat(history, strp);
						} else {
							valid = 0;
							break;
						}	
						bCntItem = bCntItem->next;
					}
					if(valid && bCntItem && bCntItem->next) {
						aIct = (((struct Contact*)bCntItem->next->datap)->startAt - ((struct Contact*)bCntItem->datap)->endAt)/oracle->tGran;	
						if(aIct < oracle->T/oracle->tGran)
							value = aIct;
						else
							value = oracle->T/oracle->tGran;
						state = value;
					} else {
						valid = 0;
					}
					if(valid) {
						aTransItem = hashtable_find(&aPairwise->transitions, history);
						if(aTransItem == NULL) {
							aTrans = (struct Transition*)malloc(sizeof(struct Transition));
							transition_init_func(aTrans, history);
							aTrans->count = 1;
							hashtable_add(&aPairwise->transitions, history, aTrans);
						} else {
							aTrans = (struct Transition*)aTransItem->datap;
							aTrans->count ++;
						}
						aProbItem = duallist_find(&aTrans->probs, &state, (int(*)(void*,void*))prob_has_state);
						if(aProbItem == NULL) {
							aProb = (struct Prob*) malloc(sizeof(struct Prob));
							aProb->state = state;
							aProb->prob = 1;
							duallist_add_to_tail(&aTrans->probs, aProb);
							/* statistic on how much memory used for this oracle */
							oracle->size ++;
						} else {
							aProb = (struct Prob*)aProbItem->datap;
							aProb->prob += 1;
						}
					}
				} 
				aCntItem = aCntItem->next;
			}

			if(aPairwise->total)
				aPairwise->defaultEstimate = aPairwise->total/(2*(oracle->trainingEndAt-oracle->trainingStartAt)/oracle->tGran);
			else	
				aPairwise->defaultEstimate = -1;

			/* setup previous states and current estimation */
			for(k=0;k<oracle->order;k++)
				aPairwise->preStates[k] = -1;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next && ((struct Contact*)aCntItem->next->datap)->startAt<oracle->aSim->exprStartAt) {
				aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/oracle->tGran;
				if(aIct < oracle->T/oracle->tGran)
					value = aIct;
				else
					value = oracle->T/oracle->tGran;
				update_previous_states(aPairwise->preStates, oracle->order, value);
				aCntItem = aCntItem->next;
			}
			aPairwise->estimation = estimate_next_delay(aPairwise, aPairwise->preStates, oracle->order, oracle->useDefault);
			aItem = aItem->next;
		}
	}
}

int nodewise_has_larger_centrality(struct Nodewise *aNodewise, struct Nodewise *bNodewise)
{
	return aNodewise->estimation > bNodewise->estimation;
}


void setup_oracle_node_static(struct Oracle *oracle)
{

	FILE *fp;
	struct Nodewise *aNodewise;
	char buf[4096], *strp1, *strp2; 

	if((fp=fopen(oracle->centralityFile, "r"))!= NULL) {
		while(fgets(buf, 4096, fp)) {
			strp1 = strtok(buf, " ");
			/* set up the coresponding nodewise in the nodewise table */
			aNodewise = lookup_nodewise_in_oracle(oracle, strp1);
			if(aNodewise == NULL) {
				aNodewise = (struct Nodewise*)malloc(sizeof(struct Nodewise));
				nodewise_init_func(aNodewise, strp1, oracle->order);
				strp2 = strtok(NULL, " \n");
				aNodewise->estimation = atoi(strp2);
				hashtable_add(&oracle->nodewises,strp1, aNodewise);
			}
			memset(buf, 0, 4096);
		}
		fclose(fp);
	}
}


void setup_oracle_node_priori(struct Oracle *oracle)
{
	struct Nodewise *aNodewise;

	char buf[4096], *strp1, *strp2; 
	FILE *fp;
	double *aCentrality;

	if(oracle == NULL || oracle->order == 0)
		return;

	if((fp=fopen(oracle->centralityFile, "r"))!= NULL) {
		while(fgets(buf, 4096, fp)) {
			strp1 = strtok(buf, " ");
			/* set up the coresponding nodewise in the nodewise table */
			aNodewise = lookup_nodewise_in_oracle(oracle, strp1);
			if(aNodewise == NULL) {
				aNodewise = (struct Nodewise*)malloc(sizeof(struct Nodewise));
				nodewise_init_func(aNodewise, strp1, oracle->order);
				while((strp2 = strtok(NULL, " \r\n"))!=NULL) {
					aCentrality = (double*)malloc(sizeof(double));
					*aCentrality = atof(strp2);
					duallist_add_to_tail(&aNodewise->centralities, aCentrality);
				}
				hashtable_add(&oracle->nodewises,strp1, aNodewise);
			}
			memset(buf, 0, 4096);
		}
		fclose(fp);
	}
}

void setup_oracle_node_mkv(struct Oracle *oracle)
{
	unsigned long i;
	int j, s, e;
	struct Item *aItem, *aCentralityItem, *bCentralityItem, *aTransItem, *aProbItem;
	struct Nodewise *aNodewise;
	int k, valid, value, state;

	struct Transition *aTrans;
	struct Prob *aProb;
	char history[1024], strp[32], buf[4096], *strp1, *strp2; 
	FILE *fp;
	double *aCentrality, *bCentrality;

	if(oracle == NULL || oracle->order == 0)
		return;

	if((fp=fopen(oracle->centralityFile, "r"))!= NULL) {
		while(fgets(buf, 4096, fp)) {
			strp1 = strtok(buf, " ");
			/* set up the coresponding nodewise in the nodewise table */
			aNodewise = lookup_nodewise_in_oracle(oracle, strp1);
			if(aNodewise == NULL) {
				aNodewise = (struct Nodewise*)malloc(sizeof(struct Nodewise));
				nodewise_init_func(aNodewise, strp1, oracle->order);
				while((strp2 = strtok(NULL, " \r\n"))!=NULL) {
					aCentrality = (double*)malloc(sizeof(double));
					*aCentrality = atof(strp2);
					duallist_add_to_tail(&aNodewise->centralities, aCentrality);
				}
				hashtable_add(&oracle->nodewises,strp1, aNodewise);
			}
			memset(buf, 0, 4096);
		}
		fclose(fp);
	}

	s = (oracle->trainingStartAt - oracle->historyStart)/(oracle->slotLength*3600);
	e = (oracle->trainingEndAt - oracle->historyStart)/(oracle->slotLength*3600);

	for(i=0;i<oracle->nodewises.size;i++) {
		aItem = oracle->nodewises.head[i];
		while(aItem!=NULL) {
			aNodewise = (struct Nodewise*)aItem->datap;
			/* setup transitions */
			aCentralityItem = aNodewise->centralities.head;
			j = 0;
			while(j<s){
				aCentralityItem = aCentralityItem->next;
				j ++;
			}
			aNodewise->total = 0;
			while(j<=e) {
				aCentrality = (double*)aCentralityItem->datap;
				if((e+1-j)%(24/oracle->slotLength)==0) {
					if(*aCentrality < oracle->maxCentrality)
						value = *aCentrality/oracle->centralityGran;
					else
						value = oracle->maxCentrality/oracle->centralityGran;
					aNodewise->total += value;
				}
				valid = 1;
				bCentralityItem = aCentralityItem;
				memset(history, 0, 1024);
				for(k=0;k<oracle->order;k++) {
					if(bCentralityItem) {
						bCentrality = (double*)bCentralityItem->datap;
						if(*bCentrality < oracle->maxCentrality)
							value = *bCentrality/oracle->centralityGran;
						else
							value = oracle->maxCentrality/oracle->centralityGran;
						memset(strp, 0, 32);
						sprintf(strp, "%d,", value);
						strcat(history, strp);
					} else {
						valid = 0;
						break;
					}	
					bCentralityItem = bCentralityItem->next;
				}
				if(valid && bCentralityItem) {
					bCentrality = (double*)bCentralityItem->datap;
					if(*bCentrality < oracle->maxCentrality)
						value = *bCentrality/oracle->centralityGran;
					else
						value = oracle->maxCentrality/oracle->centralityGran;
					state = value;
				} else {
					valid = 0;
				}

				if(valid) {
					aTransItem = hashtable_find(&aNodewise->transitions, history);
					if(aTransItem == NULL) {
						aTrans = (struct Transition*)malloc(sizeof(struct Transition));
						transition_init_func(aTrans, history);
						aTrans->count = 1;
						hashtable_add(&aNodewise->transitions, history, aTrans);
					} else {
						aTrans = (struct Transition*)aTransItem->datap;
						aTrans->count ++;
					}
					aProbItem = duallist_find(&aTrans->probs, &state, (int(*)(void*,void*))prob_has_state);
					if(aProbItem == NULL) {
						aProb = (struct Prob*) malloc(sizeof(struct Prob));
						aProb->state = state;
						aProb->prob = 1;
						duallist_add_to_tail(&aTrans->probs, aProb);
						/* statistic on how much memory used for this oracle */
						oracle->size ++;
					} else {
						aProb = (struct Prob*)aProbItem->datap;
						aProb->prob += 1;
					}
				}
				aCentralityItem = aCentralityItem->next;
				j++;
			}

			if(aNodewise->total)
				aNodewise->defaultEstimate = aNodewise->total*24/(oracle->slotLength*(e-s+1));
			else	
				aNodewise->defaultEstimate = -1;

			/* setup previous states and current estimation */
			for(k=0;k<oracle->order;k++)
				aNodewise->preStates[k] = -1;
			aCentralityItem = aNodewise->centralities.head;
			j = 0;
			while(aCentralityItem && j<=e) {
				aCentrality = (double*)aCentralityItem->datap;
				if(*aCentrality < oracle->maxCentrality)
					value = *aCentrality/oracle->centralityGran;
				else
					value = oracle->maxCentrality/oracle->centralityGran;
				update_previous_states(aNodewise->preStates, oracle->order, value);
				aCentralityItem = aCentralityItem->next;
			}
			aNodewise->estimation = estimate_next_centrality(aNodewise, aNodewise->preStates, oracle->order, oracle->useDefault);
			aItem = aItem->next;
		}
	}

}

void update_previous_states(int *preStates, int k, int value)
{
	int i;
	if(k>0) {
		for(i=0;i<k-1;i++)
			preStates[i] = preStates[i+1];
		preStates[k-1] = value;
	}
}


double estimate_next_delay(struct Pairwise *aPairwise, int *preStates, int k, int useDefault)
{
	int i;
	char history[1024], strp[32];
	struct Transition *aTrans;
	struct Prob *aProb;
	struct Item *aTransItem, *aProbItem;
	double rt = 0;

	memset(history, 0, 1024);
	for(i=0;i<k;i++) {
		if(preStates[i] == -1) {
			return rt = -1;
		}
		memset(strp, 0, 32);
		sprintf(strp, "%d,", preStates[i]);
		strcat(history, strp);
	}
	
	aTransItem = hashtable_find(&aPairwise->transitions, history);
	if(aTransItem == NULL) {
		if(useDefault)
			rt = aPairwise->defaultEstimate;
		else
			rt = -1;
	} else {
		aTrans = (struct Transition*)aTransItem->datap;
		aProbItem = aTrans->probs.head;
		rt = 0;
		while(aProbItem != NULL) {
			aProb = (struct Prob*)aProbItem->datap;
			rt +=  aProb->state*aProb->prob*1.0/aTrans->count;
			aProbItem = aProbItem->next;
		}
	}
	return rt;
}

double estimate_next_centrality(struct Nodewise *aNodewise, int *preStates, int k, int useDefault)
{
	int i;
	char history[1024], strp[32];
	struct Transition *aTrans;
	struct Prob *aProb;
	struct Item *aTransItem, *aProbItem;
	double rt = 0;

	memset(history, 0, 1024);
	for(i=0;i<k;i++) {
		if(preStates[i] == -1) {
			return rt = -1;
		}
		memset(strp, 0, 32);
		sprintf(strp, "%d,", preStates[i]);
		strcat(history, strp);
	}
	
	aTransItem = hashtable_find(&aNodewise->transitions, history);
	if(aTransItem == NULL) {
		if(useDefault)
			rt = aNodewise->defaultEstimate;
		else
			rt = -1;
	} else {
		aTrans = (struct Transition*)aTransItem->datap;
		aProbItem = aTrans->probs.head;
		rt = 0;
		while(aProbItem != NULL) {
			aProb = (struct Prob*)aProbItem->datap;
			rt +=  aProb->state*aProb->prob*1.0/aTrans->count;
			aProbItem = aProbItem->next;
		}
	}
	return rt;
}

void setup_oracle_global_social(struct Oracle *oracle)
{
	struct Pairwise *aPairwise;
	char key[64], buf[32], *strp1, *strp2;
	FILE *fp;

	if(oracle == NULL)
		return;
	if((fp=fopen(oracle->aSim->similarityFile, "r"))!=NULL) {
		while(fgets(buf, 256, fp)) {
			strp1 = strtok(buf, " \n");
			strp2 = strtok(NULL," \n");
			/* set up the coresponding pair in the pairwise table */
			sprintf(key, "%s,%s", strp1, strp2);
			aPairwise = lookup_pairwise_in_oracle(oracle, strp1, strp2);
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, strp1, strp2, oracle->order);
				aPairwise->similarity = atof(strtok(NULL, " \n"));
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
		}
		fclose(fp);
	}
}


void setup_oracle_future_contacts(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem;
	struct Pair *aCntPair; 
	struct Pairwise *aPairwise;
	struct Contact *aContact, *newContact;
	char key[64];

	if(oracle == NULL)
		return;
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the pairwise table */
			aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2 );
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, aCntPair->vName1, aCntPair->vName2, oracle->order);
				sprintf(key, "%s,%s", aCntPair->vName1, aCntPair->vName2);
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
			/* setup future contacts*/
			aCntItem = aCntPair->contents.head;
			while(aCntItem) {
				aContact = (struct Contact*)aCntItem->datap;
				if(aContact->startAt > oracle->aSim->exprStartAt) {
					newContact = contact_copy_func(aContact);
					duallist_add_to_tail(&aPairwise->futureContacts, newContact);
				}
				if(aContact->startAt > oracle->aSim->exprEndAt) 
					break;
				aCntItem = aCntItem->next;
			}
			aItem = aItem->next;
		}
	}
}


void setup_oracle_avg(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem;
	struct Pair *aCntPair; 
	struct Pairwise *aPairwise;
	int aIct;
	char key[64];

	if(oracle == NULL)
		return;
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the pairwise table */
			aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2 );
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, aCntPair->vName1, aCntPair->vName2, oracle->order);
				sprintf(key, "%s,%s", aCntPair->vName1, aCntPair->vName2);
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
			/* setup average estimation */
			aPairwise->total = 0;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next) {
				aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/3600;	
				if(((struct Contact*)aCntItem->next->datap)->startAt > oracle->trainingStartAt 
				&& ((struct Contact*)aCntItem->next->datap)->startAt < oracle->trainingEndAt) {
					aPairwise->total += aIct * aIct ;
				}
				aCntItem = aCntItem->next;
			}
			if(aPairwise->total)
				aPairwise->estimation = aPairwise->total/(2*(oracle->trainingEndAt-oracle->trainingStartAt)/3600);
			else	
				aPairwise->estimation = -1;
			aItem = aItem->next;
		}
	}
}


void setup_oracle_prob(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem;
	struct Pair *aCntPair; 
	struct Pairwise *aPairwise;
	int aIct;
	char key[64];

	if(oracle == NULL)
		return;
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the pairwise table */
			aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2 );
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, aCntPair->vName1, aCntPair->vName2, oracle->order);
				sprintf(key, "%s,%s", aCntPair->vName1, aCntPair->vName2);
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
			/* setup average link-break probability estimation */
			aPairwise->total = 0;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next) {
				aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/3600;	
				if(((struct Contact*)aCntItem->next->datap)->startAt > oracle->trainingStartAt 
				&& ((struct Contact*)aCntItem->next->datap)->startAt < oracle->trainingEndAt) {
					aPairwise->total += aIct ;
				}
				aCntItem = aCntItem->next;
			}
			if(aPairwise->total)
				aPairwise->estimation = aPairwise->total/((oracle->trainingEndAt-oracle->trainingStartAt)/3600);
			else	
				aPairwise->estimation = -1;

			aItem = aItem->next;
		}
	}
}

void setup_oracle_epidemic(struct Oracle *oracle)
{
 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	setup_neighborhood(oracle);
}
void setup_oracle_bubble(struct Oracle *oracle)
{
 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	setup_neighborhood(oracle);
}

void setup_oracle_simbet(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem;
	struct Node *aNode;

 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	setup_neighborhood(oracle);

	/* calculate egocentric betweenness */
	for(i=0;i<oracle->aSim->vnodes.size;i++) {
		aItem = oracle->aSim->vnodes.head[i];
		while(aItem!=NULL) {
			aNode = (struct Node*)aItem->datap;
			if(oracle->alfar) {
				aNode->betweenness = calculate_betweenness(aNode);
			}
			aItem = aItem->next;
		}
	}
}


void setup_oracle_zoom(struct Oracle *oracle)
{
	unsigned long i,j;
	struct Item *aItem, *aCntItem, *bCntItem;
	struct Pair *aCntPair; 
	struct Contact *aCnt;
	struct Node *aNode, *bNode;
	int value;
	struct Pairwise *aPairwise;
	int ict, ictEstimation;
	int *preStates,k;
	struct NeighborNode *aNeighborNode, *bNeighborNode;
	struct Estimation *aEstimation;

	int *counts;
	unsigned long nItems, zeros, total;
	double strength;

	/* setup Markov oracle during the training phase */
	setup_oracle_mkv(oracle);

	preStates = (int*)malloc(sizeof(int)*oracle->order);

 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	nItems = ceil((oracle->trainingEndAt - oracle->trainingStartAt)*1.0/oracle->checkWindowSize);
	counts = (int*)malloc(sizeof(int)*nItems);
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			aNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName1);
			bNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName2);
			if(aNode && bNode) {

				memset(counts, 0, sizeof(int)*nItems);
				aCntItem = aCntPair->contents.head;
				while(aCntItem != NULL) {
					aCnt = (struct Contact*)aCntItem->datap;
					if(aCnt->endAt > oracle->trainingStartAt && aCnt->endAt < oracle->trainingEndAt) {

						counts[(aCnt->startAt - oracle->trainingStartAt)/oracle->checkWindowSize] ++;

						ictEstimation = -1;
						aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2);
						if(aPairwise) {
							/* setup previous states and current estimation */
							for(k=0;k<oracle->order;k++)
								preStates[k] = -1;
							k = 0;
							bCntItem = aCntItem;
							while(bCntItem!=aCntPair->contents.head && k<oracle->order) {
								ict = (((struct Contact*)bCntItem->datap)->startAt - ((struct Contact*)bCntItem->prev->datap)->endAt)/oracle->tGran;
								if(ict < oracle->T/oracle->tGran)
									value = ict;
								else
									value = oracle->T/oracle->tGran;
								preStates[oracle->order-k-1]=value;
								k++;
								bCntItem = bCntItem->prev;
							}
							ictEstimation = estimate_next_delay(aPairwise, preStates, oracle->order, oracle->useDefault);
					
						}	
						if(ictEstimation !=-1) {
							ictEstimation = (ictEstimation+1) * oracle->tGran;
						}

						aNeighborNode = node_met_a_node(aNode, bNode, oracle->neighborThreshold);
						bNeighborNode = node_met_a_node(bNode, aNode, oracle->neighborThreshold);

						if(aNeighborNode) {
							/* add new estimation */
							aEstimation = (struct Estimation*)malloc(sizeof(struct Estimation));
							aEstimation->timestamp = aCnt->startAt;
							aEstimation->estimatedTime = ictEstimation;
							duallist_add_to_head(&aNeighborNode->lastEstimations, aEstimation);
						}
					
						if(bNeighborNode) {
							aEstimation = (struct Estimation*)malloc(sizeof(struct Estimation));
							aEstimation->timestamp = aCnt->startAt;
							aEstimation->estimatedTime = ictEstimation;
							duallist_add_to_head(&bNeighborNode->lastEstimations, aEstimation);
						}

					}
					aCntItem = aCntItem->next;
				}

				zeros = 0;
				total = 0;
				for(j=0;j<nItems;j++) {
					if (counts[j] == 0) zeros ++;
					total += counts[j];
				}
				if(1 - zeros*1.0/nItems >= oracle->socialRatio) 
					strength = total*1.0/nItems;
				else
					strength = 0;

				if(aNeighborNode) {
					aNeighborNode->strength = strength;
					assess_bingoRatio_on_neighbor(aNeighborNode, oracle->deltaT);
				}
				if(bNeighborNode) {
					bNeighborNode->strength = strength;
					assess_bingoRatio_on_neighbor(bNeighborNode, oracle->deltaT);
				}
				
			}
			aItem = aItem->next;
		}
	}
	free(counts);
	free(preStates);

	/* calculate egocentric betweenness */
	/* we should calculate betweenness of each node int contact event handler,
	* but considering the cost, we move the calculation in the training phase */
	for(i=0;i<oracle->aSim->vnodes.size;i++) {
		aItem = oracle->aSim->vnodes.head[i];
		while(aItem!=NULL) {
			aNode = (struct Node*)aItem->datap;
			aNode->betweenness = calculate_betweenness(aNode);
			aItem = aItem->next;
		}
	}
}



void setup_neighborhood(struct Oracle *oracle)
{
	unsigned long i,j;
	struct Item *aItem, *aCntItem;
	struct Pair *aCntPair; 
	struct Contact *aCnt;
	struct Node *aNode, *bNode;
	struct NeighborNode *aNeighborNode, *bNeighborNode;

	int *counts;
	unsigned long nItems, zeros, total;
	double strength;


	if(oracle == NULL)
		return;

	nItems = ceil((oracle->trainingEndAt - oracle->trainingStartAt)*1.0/oracle->checkWindowSize);
	counts = (int*)malloc(sizeof(int)*nItems);
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			aNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName1);
			bNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName2);
			if(aNode && bNode) {
				memset(counts, 0, sizeof(int)*nItems);
				aCntItem = aCntPair->contents.head;
				while(aCntItem != NULL) {
					aCnt = (struct Contact*)aCntItem->datap;
					if(aCnt->endAt > oracle->trainingStartAt && aCnt->endAt < oracle->trainingEndAt) {
						counts[(aCnt->startAt - oracle->trainingStartAt)/oracle->checkWindowSize] ++;
						aNeighborNode = node_met_a_node(aNode, bNode, oracle->neighborThreshold);
						bNeighborNode = node_met_a_node(bNode, aNode, oracle->neighborThreshold);
					}
					aCntItem = aCntItem->next;
				}
				zeros = 0;
				total = 0;
				for(j=0;j<nItems;j++) {
					if (counts[j] == 0) zeros ++;
					total += counts[j];
				}
				if(1 - zeros*1.0/nItems >= oracle->socialRatio) 
					strength = total*1.0/nItems;
				else
					strength = 0;

				if(aNeighborNode) {
					aNeighborNode->strength = strength;
				}
				if(bNeighborNode) {
					bNeighborNode->strength = strength;
				}
			}
			aItem = aItem->next;
		}
	}
	free(counts);
}

double calculate_betweenness(struct Node *aNode)
{
	double A[aNode->neighbors.count][aNode->neighbors.count];
	double A2[aNode->neighbors.count][aNode->neighbors.count];
	int i,j,l;
	unsigned long k;
	struct Item *aItem, *bItem, *cItem;
	struct NeighborNode *aNeighborNode, *bNeighborNode;
	double rt;

	i=0;
	for(k=0;k<aNode->neighbors.size;k++) {
		aItem = aNode->neighbors.head[k];
		while(aItem) {
			i++;
			j = i;
			aNeighborNode = (struct NeighborNode*)aItem->datap;
			A[0][i] = aNeighborNode->strength;
			for(l=0;l<((int)aNode->neighbors.count)-(i+1);l++) {
				j++;
				bItem = hashtable_next_item(&aNode->neighbors, aItem);
				bNeighborNode = (struct NeighborNode*)bItem->datap;
				cItem = hashtable_find(&aNeighborNode->node->neighbors, bNeighborNode->node->name);
				if(cItem)
					A[i][j] = ((struct NeighborNode*)cItem->datap)->strength;
				else
					A[i][j] = 0;
			}
			aItem = aItem->next;
		}
	}

	for(i=0;i<aNode->neighbors.count;i++)
		for(j=0;j<i+1;j++) {
			if(i==j)
				A[i][j] = 0;
			else
				A[i][j] = A[j][i];
		}
	for(i=0;i<aNode->neighbors.count;i++)
		for(j=i+1;j<aNode->neighbors.count;j++) {
			A2[i][j] = 0;
			for(k=0;k<aNode->neighbors.count;k++)
				A2[i][j] += A[i][k]*A[k][j];
		}
	rt = 0;
	for(i=0;i<aNode->neighbors.count;i++)
		for(j=i+1;j<aNode->neighbors.count;j++) 
			if(A[i][j]==0 && A2[i][j])
				rt += 1/A2[i][j];

	return rt;
}

double check_similarity_with_oracle(struct Oracle *oracle, struct Node *aNode, struct Node *dstNode)
{
	struct Pairwise *aPairwise;

	aPairwise = lookup_pairwise_in_oracle(oracle, aNode->name, dstNode->name);
	if(aPairwise == NULL) 
		return 0;
	else
		return aPairwise->similarity;
}

double calculate_similarity(struct Node *aNode, struct Node *dstNode)
{
	struct Duallist commons;
	struct Item *aItem, *bItem, *cItem;
	unsigned long i;
	struct NeighborNode *aNeighborNode;
	double rt;

	duallist_init(&commons);
	/* direct common neighbors */
	aItem = hashtable_find(&aNode->neighbors, dstNode->name);
	if(aItem) {
		for(i=0;i<dstNode->neighbors.size;i++) {
			bItem = dstNode->neighbors.head[i];
			while(bItem) {
				aNeighborNode = (struct NeighborNode*)bItem->datap;
				cItem = hashtable_find(&aNode->neighbors, aNeighborNode->node->name);
				if(cItem) {
					duallist_add_to_tail(&commons, aNeighborNode);
				}
				bItem = bItem->next;
			}
		}
	}  

	/* indirect common neighbors */
	for(i=0;i<aNode->neighbors.size;i++) {
		aItem = aNode->neighbors.head[i];
		while(aItem) {
			aNeighborNode = (struct NeighborNode*)aItem->datap;
			if(aNeighborNode->node!=dstNode) {
				bItem = hashtable_find(&aNeighborNode->node->neighbors, dstNode->name);
				if(bItem) {
					cItem = duallist_find(&commons, aNeighborNode->node->name, (int(*)(void*,void*))neighborNode_has_name);
					if(!cItem)
						duallist_add_to_tail(&commons, aNeighborNode);
				}
			}
			aItem = aItem->next;
		}
	}
	rt = commons.nItems;
	duallist_destroy(&commons, NULL);
	return rt;	
}

void assess_bingoRatio_on_neighbor(struct NeighborNode *aNeighborNode, time_t deltaT)
{
	struct Item *aItem;
	time_t error;
	struct Estimation *aEstimation, *bEstimation;
	int count;

	if(aNeighborNode) {
		aItem = aNeighborNode->lastEstimations.head;
		while(aItem && aItem->next) {
			aEstimation = (struct Estimation*)aItem->datap;
			bEstimation = (struct Estimation*)aItem->next->datap;
			if(aEstimation->estimatedTime != -1) {
				error = (aEstimation->estimatedTime + aEstimation->timestamp) > bEstimation->timestamp? (aEstimation->timestamp + aEstimation->estimatedTime - bEstimation->timestamp) : (bEstimation->timestamp - aEstimation->timestamp - aEstimation->estimatedTime);
			} else
				error = -1;
			if(error!= -1 && error < deltaT) {
				aEstimation->bingo = 1; 
			} else
				aEstimation->bingo = 0;

			aItem = aItem->next;
		}
		count = 0;
		aItem = aNeighborNode->lastEstimations.head;
		while(aItem) {
			if(((struct Estimation*)aItem->datap)->bingo)
				count ++;
			aItem = aItem->next;
		}
		aNeighborNode->bingoRatio = count*1.0/aNeighborNode->lastEstimations.nItems;
	}
}


