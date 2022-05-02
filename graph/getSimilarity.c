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


int main(int argc, char**argv) {
  
  igraph_t g;
  char *outputfile = NULL;
  FILE *input, *output;
  struct Item *aItem, *bItem;
  struct GraphNode *aGraphNode, *bGraphNode;
  struct Duallist nodeList;
  igraph_vector_t pairs, res;
  long int i, j, k, n;
  char *name1, *name2;


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

  igraph_read_graph_ncol(&g, input, NULL, 0, IGRAPH_ADD_WEIGHTS_YES, IGRAPH_UNDIRECTED);
  duallist_init(&nodeList);
  load_node_and_edge_lists(input, &nodeList, NULL);

  igraph_vector_init(&res, 0);
  n = igraph_vcount(&g);
  igraph_vector_init(&pairs, 0);
  for (i = 0; i < n; i++) {
    for (j = i+1; j < n; j++) {
      igraph_vector_push_back(&pairs, i);
      igraph_vector_push_back(&pairs, j);
    }
  }
  igraph_similarity_jaccard_pairs(&g, &res, &pairs, IGRAPH_ALL, 0);
  aItem = nodeList.head;
  for (i = 0, k = 0; i < n; i++) {
    aGraphNode = (struct GraphNode*) aItem->datap;
    bItem = aItem->next;
    for (j = i+1; j < n; j++, k++) {
	bGraphNode = (struct GraphNode*) bItem->datap;
	if (0 > strcmp(aGraphNode->name, bGraphNode->name)) {
		name1 = aGraphNode->name;
		name2 = bGraphNode->name;
	} else {
		name1 = bGraphNode->name;
		name2 = aGraphNode->name;
	}
        if(VECTOR(res)[k])
		fprintf(output, "%s %s %.2f\n", name1, name2, VECTOR(res)[k]);
	bItem = bItem->next;
    }
    aItem = aItem->next;
  }

  igraph_vector_destroy(&pairs);
  igraph_vector_destroy(&res);
  duallist_destroy(&nodeList, free); 
  igraph_destroy(&g);
  fclose(input);
  if(outputfile)
  	fclose(output);
  return 0;
}
