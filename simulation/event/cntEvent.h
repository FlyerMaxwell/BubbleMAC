#ifndef CNTEVENT_H
#define CNTEVENT_H

#include<time.h>
#include"common.h"
#include"event.h"
#include"node.h"

struct CntEvent
{ 
  char name1[NAME_LENGTH];
  char name2[NAME_LENGTH];
  time_t timestamp;
  time_t duration;
  /* ict between this contact and the previous contact */
  time_t ict;
};

void setup_cnt_events(struct Simulator *aSim, struct Hashtable *cntTable);
int cnt_event_handler(void * nul, struct Simulator *aSim, struct CntEvent *aCntEvent);

int process_markov_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);
int process_avgdly_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);
int process_avgprb_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);
int process_epidemic_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);
int process_bubble_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);
int process_simbet_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);
int process_zoom_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);
int process_global_social_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);
int process_future_contacts_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);
int process_node_cnt_event(struct Simulator *aSim, struct Node *aNode, struct Node *bNode, struct CntEvent *aCntEvent);

void zoom_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);
void oracle_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);
void bubble_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);
void simbet_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);
void future_contacts_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);
void global_social_check_for_relaying_pkgs(struct Simulator *aSim, struct Node *aNode, struct Node *bNode);

void update_network_bingoRatio(struct Simulator *aSim);
int node_recv(struct Simulator *aSim, struct Node *aNode, struct Pkg *aPkg);
#endif

