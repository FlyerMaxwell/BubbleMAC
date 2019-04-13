#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"common.h"
#include"simulator.h"
#include"oracle.h"
#include"node.h"
#include"pkg.h"
#include"cntEvent.h"
#include"traffic.h"
#include"contact.h"
#include"files.h"

void shrink_cntTable(struct Hashtable *cntTable, struct Hashtable *nodes);
void setup_painters(struct Simulator *aSim, int type, int nPainters);
void setup_pairwise_distance(struct Oracle *oracle);
struct Community
{
  int id;
  int size;
  int quota;
};
int community_has_id(int *id, struct Community *aCommunity)
{
  return *id == aCommunity->id;
}

int main( int   argc,
          char *argv[] )
{
  struct Simulator *aSim;
  time_t historyStart = 0, trainingStart=0, trainingEnd = 0, starttime = 0, endtime = 0, slotLength = 0;
  int oracle_type = -1; 

  int randseed = 1111;

  int magicNumber;
  time_t startAt = 0, endAt = 0;

  int mkvOrder = 1;
  double centralityGran = 2, maxCentrality, exBound;
  int useDefault = 1;

  double dumpInterval = 0, dumpClock, exDist = 0;
  char *coverdumpfile = NULL, *infdumpfile = NULL;
  char *centralityFile = NULL;
  char *similarityFile = NULL;
  char *painterFile = NULL;
  char *communityFile = NULL;
  FILE *fsource, *fcoverdump, *finfdump;
  int nPainters = 10;

  if(argc < 2) {
	printf("Usage: %s [-time historyStartAt trainingStartAt trainingEndAt exprStartAt exprEndAt slotLength(hours)] [-r randseed] [-v0 method(markov, random, static, priori)] [-v1 mkvOrder maxCentrality centrality_gran useDefault] [-v2 centralityFile] [-v3 communityFile] [-v4 nPainters] [-v5 painterFile] [-v6 similarityFile bound] [-w0 dump_interval(mins)] [-w1 coverDump] [-w2 influenceDump] [.cont ...]\n", argv[0]);
	exit(1);
  }
  while(argv[1]!=NULL && argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		historyStart = strtot(argv[2]);
		trainingStart = strtot(argv[3]);
		trainingEnd = strtot(argv[4]);
		starttime = strtot(argv[5]);
		endtime = strtot(argv[6]);
		slotLength = atoi(argv[7]);
		argc-=7;
		argv+=7;
		break;

	case 'r':
		randseed = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'v':
		if(argv[1][2] == '0') {
			if(!strncmp(argv[2], "markov", 32))
				oracle_type = TYPE_ORACLE_NODE_MARKOV;
			else if(!strncmp(argv[2], "static", 32))
				oracle_type = TYPE_ORACLE_NODE_STATIC;
			else if(!strncmp(argv[2], "random", 32))
				oracle_type = TYPE_ORACLE_NODE_RANDOM;
			else if(!strncmp(argv[2], "priori", 32))
				oracle_type = TYPE_ORACLE_NODE_PRIORI;
			argc-=2;
			argv+=2;
		} else if(argv[1][2] == '1') {
			mkvOrder = atoi(argv[2]);
			maxCentrality = atof(argv[3]);
			centralityGran = atof(argv[4]);
			useDefault = atoi(argv[5]);
			argc-=5;
			argv+=5;
		} else if(argv[1][2] == '2') {
			centralityFile = argv[2];
			argc-=2;
			argv+=2;
		} else if(argv[1][2] == '3') {
			communityFile = argv[2];
			argc-=2;
			argv+=2;
		} else if(argv[1][2] == '4') {
			nPainters = atoi(argv[2]);
			argc-=2;
			argv+=2;
		} else if(argv[1][2] == '5') {
			painterFile = argv[2];
			argc-=2;
			argv+=2;
		} else if(argv[1][2] == '6') {
			similarityFile = argv[2];
			exBound = atof(argv[3]);
			argc-=3;
			argv+=3;
		}
		break;

	case 'w':
		if(argv[1][2] == '0')
			dumpInterval = atof(argv[2])*60;
		else if(argv[1][2] == '1')
			coverdumpfile = argv[2];
		else if(argv[1][2] == '2')
			infdumpfile = argv[2];
		argc-=2;
		argv+=2;
		break;
	default:
		printf("Usage: %s [-time historyStartAt trainingStartAt trainingEndAt exprStartAt exprEndAt slotLength(hours)] [-r randseed] [-v0 method(markov, random, static, priori)] [-v1 mkvOrder maxCentrality centrality_gran useDefault] [-v2 centralityFile] [-v3 communityFile] [-v4 nPainters] [-v5 painterFile] [-v6 similarityFile bound] [-w0 dump_interval(mins)] [-w1 coverDump] [-w2 influenceDump] [.cont ...]\n", argv[0]);
	}
  }

  srand(randseed);

  aSim = (struct Simulator*)malloc(sizeof(struct Simulator));
  simulator_init_func(aSim, starttime, endtime, 10);

  /* setting up contacts */
  while(argc>1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fscanf(fsource, "%d\n", &magicNumber);
		printf("Loading %s file ...\n", argv[1]);
		if(magicNumber == FILE_CONTACT) {
			startAt = 0;
			endAt = 0;
			load_contacts_with_hashtable(fsource, NULL, &aSim->cntTable, PAIRWISE_TABLE, &startAt, &endAt);
		}
		fclose(fsource);
	}
	argc--;
	argv++;
  }
  /* setting up nodes */
  setup_vehicular_nodes_by_pairs(&aSim->cntTable, &aSim->vnodes, 0);
  printf("There are %ld vehicular nodes.\n", aSim->vnodes.count);

  /* setting up oracle */
  aSim->oracle = (struct Oracle*)malloc(sizeof(struct Oracle));
  oracle_init_func(aSim->oracle, oracle_type, aSim, trainingStart, trainingEnd);
  aSim->oracle->order = mkvOrder;
  aSim->oracle->maxCentrality = maxCentrality;
  aSim->oracle->centralityGran = centralityGran;
  aSim->oracle->useDefault = useDefault;
  aSim->oracle->centralityFile = centralityFile;
  aSim->oracle->communityFile = communityFile;
  aSim->oracle->similarityFile = similarityFile;
  aSim->oracle->exBound = exBound;
  aSim->oracle->painterFile = painterFile;
  aSim->oracle->historyStart = historyStart;
  aSim->oracle->slotLength = slotLength;

  if(aSim->oracle->setup_oracle)
	aSim->oracle->setup_oracle(aSim->oracle);


  setup_painters(aSim, oracle_type, nPainters);

  printf("Painting other cars...\n");
  /* dump coverage ratio */
  fcoverdump = fopen(coverdumpfile, "a");
  /* dump influence */
  finfdump = fopen(infdumpfile, "a");
  
  // setup events
  setup_cnt_events(aSim, &aSim->cntTable); 

  dumpClock = aSim->exprStartAt+dumpInterval;

  // process events
  while(consume_an_event(aSim)) {
	if(dumpInterval && aSim->clock >= dumpClock) {
		if(fcoverdump) {
		        fprintf(fcoverdump, "%.2lf ", aSim->paintedNodes*1.0/aSim->vnodes.count);
		}

		if(finfdump) {
		        fprintf(finfdump, "%.2lf ", aSim->nPaints*1.0/aSim->paintedNodes);
		}
		dumpClock += dumpInterval;
	}
  }

  if(fcoverdump) {
	fprintf(fcoverdump, "%.2lf ", aSim->paintedNodes*1.0/aSim->vnodes.count);
	fclose(fcoverdump);
  }

  if(finfdump) {
	fprintf(finfdump, "%.2lf ", aSim->nPaints*1.0/aSim->paintedNodes);
	fclose(finfdump);
  }

  // destroy simulator
  simulator_free_func(aSim);

  return 0;
}

void setup_painters(struct Simulator *aSim, int oracle_type, int nPainters)
{
	struct Item *aItem,  *bItem;
	struct Node *aNode=NULL;	
	unsigned long i, k, j;
	struct Nodewise *aNodewise, *bNodewise;
	struct Pairwise *aPairwise;
	struct Duallist aDuallist, *communityList = NULL;
	double *aCentrality;
	FILE *fp;
	char buf[20000], *strp;
	struct Community *aCommunity;

	if(aSim->oracle->communityFile && aSim->oracle->nodewises.count) {
		fp = fopen(aSim->oracle->communityFile, "r");
		fgets(buf, 20000, fp);
		communityList = (struct Duallist*)malloc(sizeof(struct Duallist));
		duallist_init(communityList);
		while(fgets(buf, 20000, fp)) {
			aCommunity = (struct Community*)malloc(sizeof(struct Community));
			aCommunity->id = atoi(strtok(buf, " \n"));
			fgets(buf, 20000, fp);
			strp = strtok(buf, " \n");
			aNodewise = lookup_nodewise_in_oracle(aSim->oracle, strp);
			aNodewise->community = aCommunity->id;
			aCommunity->size = 1;
			while((strp = strtok(NULL, " \n"))!=NULL) {
				aNodewise = lookup_nodewise_in_oracle(aSim->oracle, strp);
				aNodewise->community = aCommunity->id;
				aCommunity->size += 1;
			}
			memset(buf, 0, 20000);
			aCommunity->quota = round(aCommunity->size*nPainters*1.0/aSim->vnodes.count);
			duallist_add_to_tail(communityList, aCommunity);
		}
		fclose(fp);
	}

	if(aSim->oracle->painterFile) {
		fp = fopen(aSim->oracle->painterFile, "r");
		while(fgets(buf, 20000, fp)) {
			/* set up the coresponding pair in the pairwise table */
			strp = strtok(buf, "\r\n");
			aNode = lookup_node(&aSim->vnodes, strp);
			if(aNode) {
				aNode->isPainter = 1;
				aNode->painted = 1;
				aSim->paintedNodes += 1;
				aSim->nPaints += 1;
			}
			
		}
		fclose(fp);
	} else if(oracle_type == TYPE_ORACLE_NODE_MARKOV) {

		duallist_init(&aDuallist);
		for(i=0;i<aSim->oracle->nodewises.size;i++) {
			aItem = aSim->oracle->nodewises.head[i];
			while(aItem!=NULL) {
				aNodewise = (struct Nodewise*)aItem->datap;
				// use Nodewise.total to store whether the node is a candidate 
				aNodewise->total = 1;
				duallist_add_in_sequence_from_tail(&aDuallist, aNodewise, (int(*)(void*, void*))nodewise_has_larger_centrality);
				aItem = aItem->next;
			}
		}

		if(communityList) {
			aItem = aDuallist.head;
			while(aItem) {
				aNodewise = (struct Nodewise*)aItem->datap;
				bItem = duallist_find(communityList, &aNodewise->community, (int(*)(void*,void*))community_has_id);
				if(bItem) {
					aCommunity = (struct Community*)bItem->datap;
					if(aCommunity->quota) {
						aNode = lookup_node(&aSim->vnodes, aNodewise->name);
						if(aNode) {
							aNode->isPainter = 1;
							aNode->painted = 1;
							aSim->paintedNodes += 1;
							aSim->nPaints += 1;
							aCommunity->quota --;
						}
					}
				}
				aItem = aItem->next;
			}
		} else {
			if(aSim->oracle->similarityFile) {
				setup_pairwise_distance(aSim->oracle);
				aItem = aDuallist.head;
				while(aItem) {
					aNodewise = (struct Nodewise*)aItem->datap;
					if (aNodewise->total) {
						bItem = aItem->next;
						while(bItem) {
							bNodewise = (struct Nodewise*)bItem->datap;
							if(strncmp(aNodewise->name, bNodewise->name, 32)>0){ 
								aPairwise = lookup_pairwise_in_oracle(aSim->oracle,  bNodewise->name, aNodewise->name);
							} else if(strncmp(aNodewise->name, bNodewise->name, 32)<0){ 
								aPairwise = lookup_pairwise_in_oracle(aSim->oracle,  aNodewise->name, bNodewise->name);
							} 
							if(aPairwise && aPairwise->estimation < aSim->oracle->exBound || aPairwise == NULL)
								bNodewise-> total = 0;	
							bItem = bItem->next;
						}
					}
					aItem = aItem->next;
				}

			}
			aItem = aDuallist.head;
			for(i=0;i<nPainters;) {
				aNodewise = (struct Nodewise*)aItem->datap;
				if (aNodewise->total) {
					aNode = lookup_node(&aSim->vnodes, aNodewise->name);
					if(aNode) {
						aNode->isPainter = 1;
						aNode->painted = 1;
						aSim->paintedNodes += 1;
						aSim->nPaints += 1;
						i++;
					}
				}
				aItem = aItem->next;
				if(aItem == NULL)
					break;
			}
		}
		duallist_destroy(&aDuallist, NULL);


	} else if(oracle_type == TYPE_ORACLE_NODE_RANDOM) {
		i = 0;
		while(i<nPainters) {
			aNode = randomly_pick_a_node(&aSim->vnodes, VEHICLE_TYPE_NULL);
			aNode->isPainter = 1;
			aNode->painted = 1;
			aSim->paintedNodes += 1;
			aSim->nPaints += 1;
			i++;
		}

	} else if(oracle_type == TYPE_ORACLE_NODE_STATIC) {
		duallist_init(&aDuallist);
		for(i=0;i<aSim->oracle->nodewises.size;i++) {
			aItem = aSim->oracle->nodewises.head[i];
			while(aItem!=NULL) {
				aNodewise = (struct Nodewise*)aItem->datap;
				// use Nodewise.total to store whether the node is a candidate 
				aNodewise->total = 1;
				duallist_add_in_sequence_from_tail(&aDuallist, aNodewise, (int(*)(void*, void*))nodewise_has_larger_centrality);
				aItem = aItem->next;
			}
		}

		if(communityList) {
			aItem = aDuallist.head;
			while(aItem) {
				aNodewise = (struct Nodewise*)aItem->datap;
				bItem = duallist_find(communityList, &aNodewise->community, (int(*)(void*,void*))community_has_id);
				if(bItem) {
					aCommunity = (struct Community*)bItem->datap;
					if(aCommunity->quota) {
						aNode = lookup_node(&aSim->vnodes, aNodewise->name);
						if(aNode) {
							aNode->isPainter = 1;
							aNode->painted = 1;
							aSim->paintedNodes += 1;
							aSim->nPaints += 1;
							aCommunity->quota --;
						}
					}
				}
				aItem = aItem->next;
			}
		} else {
			if(aSim->oracle->similarityFile) {
				setup_pairwise_distance(aSim->oracle);
				aItem = aDuallist.head;
				while(aItem) {
					aNodewise = (struct Nodewise*)aItem->datap;
					if (aNodewise->total) {
						bItem = aItem->next;
						while(bItem) {
							bNodewise = (struct Nodewise*)bItem->datap;
							if(strncmp(aNodewise->name, bNodewise->name, 32)>0){ 
								aPairwise = lookup_pairwise_in_oracle(aSim->oracle,  bNodewise->name, aNodewise->name);
							} else if(strncmp(aNodewise->name, bNodewise->name, 32)<0){ 
								aPairwise = lookup_pairwise_in_oracle(aSim->oracle,  aNodewise->name, bNodewise->name);
							} 
							if(aPairwise && aPairwise->estimation < aSim->oracle->exBound || aPairwise == NULL)
								bNodewise-> total = 0;	
							bItem = bItem->next;
						}
					}
					aItem = aItem->next;
				}
			}
			aItem = aDuallist.head;
			for(i=0;i<nPainters;) {
				aNodewise = (struct Nodewise*)aItem->datap;
				if (aNodewise->total) {
					aNode = lookup_node(&aSim->vnodes, aNodewise->name);
					if(aNode) {
						aNode->isPainter = 1;
						aNode->painted = 1;
						aSim->paintedNodes += 1;
						aSim->nPaints += 1;
						i++;
					}
				}
				aItem = aItem->next;
				if(aItem == NULL)
					break;
			}
		}
		duallist_destroy(&aDuallist, NULL);

	} else if(oracle_type == TYPE_ORACLE_NODE_PRIORI) {
		duallist_init(&aDuallist);
		for(i=0;i<aSim->oracle->nodewises.size;i++) {
			aItem = aSim->oracle->nodewises.head[i];
			while(aItem!=NULL) {
				aNodewise = (struct Nodewise*)aItem->datap;
				
				j = (aSim->oracle->trainingEndAt - aSim->oracle->historyStart)/(aSim->oracle->slotLength*3600) + 1;
				bItem = aNodewise->centralities.head;
				for (k = 0;k<j;k++) {
					bItem = bItem->next;
					aCentrality = (double*)bItem->datap;
				}
				aNodewise->estimation = *aCentrality;
				aNodewise->total = 1;
				duallist_add_in_sequence_from_tail(&aDuallist, aNodewise, (int(*)(void*, void*))nodewise_has_larger_centrality);
				aItem = aItem->next;
			}
		}
		
		/* use exclusive distance to filter candidates */
		/*
		printf("Filtering exclusive candidates...\n");
		setup_pairwise_distance(aSim->oracle);
		aItem = aDuallist.head;
		i = 0;
		while(aItem) {
			aNodewise = (struct Nodewise*)aItem->datap;
			if (aNodewise->total) {
				bItem = aItem->next;
				while(bItem) {
					bNodewise = (struct Nodewise*)bItem->datap;
					if(strncmp(aNodewise->name, bNodewise->name, 32)>0){ 
						aPairwise = lookup_pairwise_in_oracle(aSim->oracle,  bNodewise->name, aNodewise->name);
					} else if(strncmp(aNodewise->name, bNodewise->name, 32)<0){ 
						aPairwise = lookup_pairwise_in_oracle(aSim->oracle,  aNodewise->name, bNodewise->name);
					} 
					if(aPairwise && aPairwise->estimation < aSim->oracle->exDist || aPairwise == NULL)
						bNodewise-> total = 0;	
					bItem = bItem->next;
				}
			}
			aItem = aItem->next;
			i++;
		}
		*/
		if(communityList) {
			aItem = aDuallist.head;
			while(aItem) {
				aNodewise = (struct Nodewise*)aItem->datap;
				bItem = duallist_find(communityList, &aNodewise->community, (int(*)(void*,void*))community_has_id);
				if(bItem) {
					aCommunity = (struct Community*)bItem->datap;
					if(aCommunity->quota) {
						aNode = lookup_node(&aSim->vnodes, aNodewise->name);
						if(aNode) {
							aNode->isPainter = 1;
							aNode->painted = 1;
							aSim->paintedNodes += 1;
							aSim->nPaints += 1;
							aCommunity->quota --;
						}
					}
				}
				aItem = aItem->next;
			}
		} else {
			aItem = aDuallist.head;
			for(i=0;i<nPainters;) {
				aNodewise = (struct Nodewise*)aItem->datap;
				if (aNodewise->total) {
					aNode = lookup_node(&aSim->vnodes, aNodewise->name);
					if(aNode) {
						aNode->isPainter = 1;
						aNode->painted = 1;
						aSim->paintedNodes += 1;
						aSim->nPaints += 1;
						i++;
					}
				}
				aItem = aItem->next;
				if(aItem == NULL)
					break;
			}
		}
		duallist_destroy(&aDuallist, NULL);
	}
	if(communityList)
		duallist_destroy(communityList, free);
}

void setup_pairwise_distance(struct Oracle *oracle)
{
	struct Pairwise *aPairwise;

	char buf[4096], *strp1, *strp2, key[32]; 
	FILE *fp;
	unsigned long nTotal = 0;
	double totalDist = 0;

	/* setup pairwise distance in the network from distFile*/
	if((fp=fopen(oracle->similarityFile, "r"))!= NULL) {
		while(fgets(buf, 4096, fp)) {
			/* set up the coresponding pair in the pairwise table */
			strp1 = strtok(buf, " ");
			strp2 = strtok(NULL, " ");
			if(strncmp(strp1, strp2, 32)>0){ 
				sprintf(key, "%s,%s", strp2, strp1);
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, strp2, strp1, oracle->order);
				aPairwise->estimation = atof(strtok(NULL, "\n"));
				totalDist += aPairwise->estimation;
				nTotal += 1;
			} else if(strncmp(strp1, strp2, 32)<0) {
				sprintf(key, "%s,%s", strp1, strp2);
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, strp1, strp2, oracle->order);
				aPairwise->estimation = atof(strtok(NULL, "\n"));
				totalDist += aPairwise->estimation;
				nTotal += 1;
			} else
				continue;	
			hashtable_add(&oracle->pairwises, key, aPairwise);

		}
		printf("Average distance between nodes: %.2lf\n", totalDist/nTotal);
		fclose(fp);
	}
}

