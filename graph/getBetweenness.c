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


int main(int argc, char**argv) {
  
  igraph_t g;
  igraph_vector_t bet, weights;
  char *outputfile = NULL;
  FILE *input, *output;
  struct Item *aItem;
  struct GraphNode *aGraphNode;
  struct Duallist nodeList;
  long int i;


  if(argc<2) {
	printf("usage:%s [-output outputfile] contactgraph\n", argv[0]);
	exit (1);
  }

  while(argc>1 && (argv[1][0])=='-' ) {
    switch ( argv[1][1]) {
      case 'o':
	outputfile = argv[2];
        argc-=2;
        argv+=2;
        break;

      default:
        printf("Bad option %s\n", argv[1]);
	printf("usage:%s [-output outputfile] contactgraph\n", argv[0]);
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

  igraph_vector_init(&weights,0);
  igraph_vector_init(&bet, 0);

  igraph_read_graph_ncol(&g, input, NULL, 0, IGRAPH_ADD_WEIGHTS_YES, IGRAPH_UNDIRECTED);
  load_vector(&weights, input, 3);

  duallist_init(&nodeList);
  load_node_and_edge_lists(input, &nodeList, NULL);

  igraph_betweenness(/*graph=*/ &g, /*res=*/ &bet, /*vids=*/ igraph_vss_all(), 
		     /*directed=*/0, /*weights=*/ &weights, /*nobigint=*/ 1);

  aItem = nodeList.head;
  for (i=0; i<igraph_vector_size(&bet); i++) {
	aGraphNode = (struct GraphNode*) aItem->datap;
	aGraphNode->betweenness = (unsigned long)VECTOR(bet)[i];
	fprintf(output, "%s %li\n", aGraphNode->name, aGraphNode->betweenness);
	aItem = aItem->next;
  }

  duallist_destroy(&nodeList, free); 
  igraph_destroy(&g);
  igraph_vector_destroy(&weights);
  igraph_vector_destroy(&bet);
  fclose(input);
  if(outputfile)
  	fclose(output);
  return 0;
}
