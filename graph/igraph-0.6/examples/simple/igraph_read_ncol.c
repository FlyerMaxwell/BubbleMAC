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

void print_vector(igraph_vector_t *v, FILE *f) {
  long int i;
  for (i=0; i<igraph_vector_size(v); i++) {
    fprintf(f, "%li ", (long int) VECTOR(*v)[i]);
  }
  fprintf(f, "\n");
}

int main(int argc, char** argv) {

  igraph_t g;
  FILE *input, *output;
  igraph_vector_t v;

  input=fopen(argv[1], "r");
  if (!input) { 
    return 1;
  }
  igraph_read_graph_ncol(&g, input, NULL, 1, IGRAPH_ADD_WEIGHTS_YES, IGRAPH_UNDIRECTED);
  fclose(input);

  igraph_vector_init(&v, 2000);
  output = fopen(argv[2], "w");
  igraph_degree(&g, &v, igraph_vss_all(), IGRAPH_OUT, IGRAPH_LOOPS);
  print_vector(&v, output);

//  igraph_write_graph_edgelist(&g, output);
  igraph_destroy(&g);
  igraph_vector_destroy(&v);

  return 0;
}
