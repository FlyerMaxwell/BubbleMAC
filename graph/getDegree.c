/* -*- mode: C -*-  */
/* 
   IGraph R package.
   Copyright (C) 2005-2012  Gabor Csardi <csardi.gabor@gmail.com>
   334 Harvard st, Cambridge MA, 02139 USA
   
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
   02110-1301 USA

*/

#include <igraph.h>
#include "common.h"
#include "graph.h"

void print_vector(igraph_vector_t *v, FILE *f) {
  long int i;
  for (i=0; i<igraph_vector_size(v); i++) {
    fprintf(f, "%li\n", (long int) VECTOR(*v)[i]);
  }
}

int main(int argc, char** argv) {

  igraph_t g;
  FILE *input, *output;
  igraph_vector_t v;
  char *outputfile = NULL;
  struct Duallist nodeList;
  struct Item *aItem;
  struct GraphNode *aGraphNode;
  unsigned long i;

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
  if (!input) { 
    return 1;
  }
  duallist_init(&nodeList);
  igraph_read_graph_ncol(&g, input, NULL, 1, IGRAPH_ADD_WEIGHTS_YES, IGRAPH_UNDIRECTED);
  load_node_and_edge_lists(input, &nodeList, NULL);
  fclose(input);

  igraph_vector_init(&v, 2000);
  output = fopen(outputfile, "w");
  igraph_degree(&g, &v, igraph_vss_all(), IGRAPH_OUT, IGRAPH_LOOPS);
  //print_vector(&v, output);

  aItem = nodeList.head;
  for (i=0; i<igraph_vector_size(&v); i++) {
	aGraphNode = (struct GraphNode*) aItem->datap;
	fprintf(output, "%s %li\n", aGraphNode->name, (unsigned long)VECTOR(v)[i]);
	aItem = aItem->next;
  }

//  igraph_write_graph_edgelist(&g, output);
  igraph_destroy(&g);
  igraph_vector_destroy(&v);

  return 0;
}
