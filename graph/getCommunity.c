/* -*- mode: C -*-  */
/* 
   IGraph library.
   Copyright (C) 2006-2012  Gabor Csardi <csardi.gabor@gmail.com>
   334 Harvard street, Cambridge, MA 02139 USA
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA 
   02110-1301 USA

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <igraph.h>
#include "common.h"
#include "graph.h"

void print_vector(igraph_vector_t *v, FILE *f)
{
  long int i;
  for (i=0; i<igraph_vector_size(v); i++) {
    fprintf(f, " %.2lf", (double)VECTOR(*v)[i]);
  }
  fprintf(f, "\n");
}

void load_vector(igraph_vector_t *v, FILE *f, int field)
{
  struct Duallist aList;
  char buf[1024];
  char *strp;
  double *newd;
  struct Item *aItem;
  unsigned long i;

  duallist_init(&aList);
  if(f) {
	fseek(f, 0, SEEK_SET);
	while(fgets(buf, 1024, f)) {
		strp=strtok(buf," \n");
		for(i=0;i<field-1;i++)
			strp = strtok(NULL, " \n");
		newd = (double*)malloc(sizeof(double));
		*newd = atof(strp);
		duallist_add_to_tail(&aList, newd);
	}
	igraph_vector_resize(v, aList.nItems);
	aItem = aList.head;
	for(i=0;i<aList.nItems;i++) {
		igraph_vector_set(v, i, *((double*)aItem->datap));
		aItem = aItem->next;
	}
	duallist_destroy(&aList, free);
  }

}
void show_all_commu_results(igraph_t *g, igraph_vector_t *mod, igraph_matrix_t *merges, FILE* f) {
  long int i, j;
  igraph_vector_t membership;

  for(i=0;i<igraph_vector_size(mod);i++) {
	  igraph_vector_init(&membership, 0);

	  fprintf(f, "Modularity:  %f\n", VECTOR(*mod)[i]);
	  igraph_community_to_membership(merges, igraph_vcount(g), i, &membership, 0);
	  fprintf(f, "Membership: ");
	  for (j=0; j<igraph_vector_size(&membership); j++) {
	    fprintf(f, " %li", (unsigned long)VECTOR(membership)[j]);
	  }
	  fprintf(f, "\n");
	  igraph_vector_destroy(&membership);
  }
}

void show_single_commu_results(double mod, igraph_vector_t *membership, FILE* f) {
  long int j;

  fprintf(f, "Modularity:  %f\n", mod);
  fprintf(f, "Membership: ");
  for (j=0; j<igraph_vector_size(membership); j++) {
    fprintf(f, " %li", (unsigned long)VECTOR(*membership)[j]);
  }
  fprintf(f, "\n");
}


void show_max_commu_results(igraph_t *g, igraph_vector_t *mod, igraph_matrix_t *merges, FILE* f) {
  long int i;
  igraph_vector_t membership;

  igraph_vector_init(&membership, 0);

  i=igraph_vector_which_max(mod);
  fprintf(f, "Modularity:  %f\n", VECTOR(*mod)[i]);
  igraph_community_to_membership(merges, igraph_vcount(g), i, &membership, 0);
  fprintf(f, "Membership: ");
  for (i=0; i<igraph_vector_size(&membership); i++) {
    fprintf(f, "%li ", (long int)VECTOR(membership)[i]);
  }
  fprintf(f, "\n");

  igraph_vector_destroy(&membership);
}


int main(int argc, char**argv) {
  
  igraph_t g;
  igraph_vector_t bet, weights, modularity;
  igraph_matrix_t merges;
  char *outputfile = NULL;
  char method = 0;
  FILE *input, *output;
  double maxmod;
  igraph_vector_t membership;
  struct Duallist nodeList, commList;
  struct Item *aItem, *bItem;
  struct GraphComm *aGraphComm;
  struct GraphNode *aGraphNode;
  long int i;


  if(argc<2) {
	printf("usage:%s [-method fast|optimal| ] [-output outputfile] contactgraph\n", argv[0]);
	exit (1);
  }

  while(argc>1 && (argv[1][0])=='-' ) {
    switch ( argv[1][1]) {
      case 'm':
	if(strcmp(argv[2], "fast")==0)
		method = 0;
	else if(strcmp(argv[2], "optimal")==0)
		method = 1;
        argc-=2;
        argv+=2;
        break;

      case 'o':
	outputfile = argv[2];
        argc-=2;
        argv+=2;
        break;

      default:
        printf("Bad option %s\n", argv[1]);
	printf("usage:%s [-method fast|optimal| ] [-output all|max outputfile] contactgraph\n", argv[0]);
        exit(1);
      }
  }

  input=fopen(argv[1], "r");
  if(outputfile)
	output = fopen(outputfile, "w");
  else
	output = stdout;

  if (!input || !output) { 
    return 1;
  }

  igraph_vector_init(&modularity,0);
  igraph_matrix_init(&merges,0,0);
  igraph_vector_init(&weights,0);
  igraph_vector_init(&membership, 0);

  igraph_read_graph_ncol(&g, input, NULL, 0, IGRAPH_ADD_WEIGHTS_YES, IGRAPH_UNDIRECTED);
  load_vector(&weights, input, 3);

  if(method == 0) {
	igraph_community_fastgreedy(&g, 0, &merges, &modularity, 0);
	i=igraph_vector_which_max(&modularity);
  	maxmod = VECTOR(modularity)[i];
  	igraph_community_to_membership(&merges, igraph_vcount(&g), i, &membership, 0);
	
  } else if (method == 1) {
	igraph_community_optimal_modularity(&g, &maxmod, &membership, 1);
	show_single_commu_results(maxmod, &membership, output);
  }

  duallist_init(&nodeList);
  load_node_and_edge_lists(input, &nodeList, NULL);

  igraph_vector_init(&bet, 0);

  igraph_betweenness(/*graph=*/ &g, /*res=*/ &bet, /*vids=*/ igraph_vss_all(), 
		     /*directed=*/0, /*weights=*/ &weights, /*nobigint=*/ 1);

  igraph_vector_destroy(&bet);

  duallist_init(&commList);
  aItem = nodeList.head;
  for (i=0; i<igraph_vector_size(&membership); i++) {
	aGraphNode = (struct GraphNode*) aItem->datap;
	aGraphNode->community = (unsigned long)VECTOR(membership)[i];
	if((bItem = duallist_find(&commList, &aGraphNode->community, (int(*)(void*,void*))comm_has_id))==NULL) {
		aGraphComm = (struct GraphComm*)malloc(sizeof(struct GraphComm));
		aGraphComm->id = aGraphNode->community;
		duallist_init(&aGraphComm->nodes);
		duallist_add_to_tail(&aGraphComm->nodes, aGraphNode);
		duallist_add_in_sequence_from_head(&commList, aGraphComm, (int(*)(void*,void*))comm_has_larger_id);
	} else {
		aGraphComm = (struct GraphComm*)bItem->datap;
		duallist_add_to_tail(&aGraphComm->nodes, aGraphNode);
	}	
	aItem = aItem->next;
  }

  fprintf(output, "Modularity:%lf\n", maxmod);
  aItem = commList.head;
  while(aItem) {
	aGraphComm = (struct GraphComm*)aItem->datap;
	fprintf(output, "%d\n", aGraphComm->id);
	bItem = aGraphComm->nodes.head;
	while(bItem) {
		aGraphNode = (struct GraphNode*)bItem->datap;
		fprintf(output, "%s ", aGraphNode->name);
		bItem = bItem->next;
	}
	fprintf(output, "\n");
	aItem = aItem->next;
  }

  duallist_destroy(&nodeList, free); 
  duallist_destroy(&commList, (void(*)(void*))comm_free_func); 
  igraph_destroy(&g);
  igraph_vector_destroy(&modularity);
  igraph_vector_destroy(&weights);
  igraph_vector_destroy(&membership);
  igraph_matrix_destroy(&merges);
  fclose(input);
  if(outputfile)
  	fclose(output);
  return 0;
}
