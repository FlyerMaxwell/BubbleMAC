#ifndef ORACLE_H
#define ORACLE_H

#include"common.h"
#include"node.h"
#include"simulator.h"

#define TYPE_ORACLE_MARKOV 0
#define TYPE_ORACLE_AVGDLY 1
#define TYPE_ORACLE_AVGPRB 2
#define TYPE_ORACLE_EPIDEMIC 3
#define TYPE_ORACLE_BUBBLE 4 
#define TYPE_ORACLE_SIMBET 5
#define TYPE_ORACLE_ZOOM 6
#define TYPE_ORACLE_BUS_ONION 7
#define TYPE_ORACLE_BUS_EPIDEMIC 8
#define TYPE_ORACLE_BUS_SHAN 9
#define TYPE_ORACLE_FUTURE_CONTACTS 10
#define TYPE_ORACLE_GLOBAL_SOCIAL 11
#define TYPE_ORACLE_NODE_MARKOV 12
#define TYPE_ORACLE_NODE_STATIC 13 
#define TYPE_ORACLE_NODE_PRIORI 14 
#define TYPE_ORACLE_NODE_RANDOM 15 
/* Markov ICT predection */
struct Prob
{
  int state;
  unsigned prob;
};
int prob_has_state(int *state, struct Prob *aProb);

struct Transition
{
  char history[128];
  struct Duallist probs;
  unsigned count;
};
void transition_init_func(struct Transition *aTrans, char *history);
void transition_free_func(struct Transition *aTrans);
int transition_has_history(char *history, struct Transition *aTrans);

/* pairwise knowledge */
struct Pairwise
{
  char name1[NAME_LENGTH];
  char name2[NAME_LENGTH];
  int *preStates;
  double total;
  double estimation;
  double defaultEstimate;
  double similarity;
  struct Hashtable transitions;
  struct Duallist futureContacts;
};
void pairwise_init_func(struct Pairwise *aPairwise, char *name1, char *name2, int k);
void pairwise_free_func(struct Pairwise *aPairwise);
int pairwise_has_names(char *names, struct Pairwise *aPairwise);

/* nodewise knowledeg */
struct Nodewise
{
  char name[NAME_LENGTH];
  int *preStates;
  double total;
  double estimation;
  double defaultEstimate;
  int community;
  struct Hashtable transitions;
  struct Duallist centralities;
};
void nodewise_init_func(struct Nodewise *aNodewise, char *name, int k);
void nodewise_free_func(struct Nodewise *aNodewise);
int nodewise_has_name(char *name, struct Nodewise *aNodewise);
int nodewise_has_larger_centrality(struct Nodewise *aNodewise, struct Nodewise *bNodewise);
struct Oracle
{
  int type;
  /* global knowledge of all pairs */
  struct Hashtable pairwises;

  /* global knowledge of centrality of all nodes */
  struct Hashtable nodewises;

  struct Simulator *aSim;
  time_t trainingStartAt;
  time_t trainingEndAt;

  /* build up the knowledge oracle */
  void(*setup_oracle)(struct Oracle *oracle); 

  //for zoom oracle
  time_t lastT;
  time_t deltaT; 
  time_t checkWindowSize; 
  double socialRatio;
  double tuner;

  // for markov oracle 
  int order;
  time_t T;
  time_t tGran;
  int useDefault;
  // for node markov oracle
  double maxCentrality;
  double centralityGran;
  char* centralityFile;
  char* communityFile;
  char* similarityFile;
  char* painterFile;
  double exBound;
  time_t historyStart;
  time_t slotLength;

  // for SimBet
  double alfar;
  double beta;

  int neighborThreshold;

  // statistic on how much memory used for this oracle
  unsigned long size;
};
void oracle_init_func(struct Oracle *oracle, int type, struct Simulator *aSim, time_t trainingStartAt, time_t trainingEndAt);
void oracle_free_func(struct Oracle *oracle);

void setup_oracle_mkv(struct Oracle *oracle);
void setup_oracle_avg(struct Oracle *oracle);
void setup_oracle_prob(struct Oracle *oracle);
void setup_oracle_bubble(struct Oracle *oracle);
void setup_oracle_simbet(struct Oracle *oracle);
void setup_oracle_zoom(struct Oracle *oracle);
void setup_oracle_epidemic(struct Oracle *oracle);
void setup_oracle_future_contacts(struct Oracle *oracle);
void setup_oracle_global_social(struct Oracle *oracle);
void setup_oracle_node_mkv(struct Oracle *oracle);
void setup_oracle_node_priori(struct Oracle *oracle);
void setup_oracle_node_static(struct Oracle *oracle);

void setup_neighborhood(struct Oracle *oracle);

/* for ict-related oracle */
struct Pairwise* lookup_pairwise_in_oracle(struct Oracle *oracle, char *vname1, char*vname2);
/* for Markov oracle */
double calculate_estimation(struct Transition *aTrans);
void update_previous_states(int *preStates, int k, int value);
double estimate_next_delay(struct Pairwise *aPairwise, int *preStates, int k, int useDefault);
/* for SimBet and social oracles */
double calculate_betweenness(struct Node *aNode);
double calculate_similarity(struct Node *aNode, struct Node *dstNode);
void assess_bingoRatio_on_neighbor(struct NeighborNode *aNeighborNode, time_t deltaT);

/* for node centrality oracle */
struct Nodewise* lookup_nodewise_in_oracle(struct Oracle *oracle, char *vname);
double estimate_next_centrality(struct Nodewise *aNodewise, int *preStates, int k, int useDefault);

double check_similarity_with_oracle(struct Oracle *oracle, struct Node *aNode, struct Node *dstNode);
#endif
