/* -*- mode: C -*-  */
/* vim:set ts=2 sts=2 sw=2 et: */
/* 
   IGraph library.
   Copyright (C) 2011-2012  Gabor Csardi <csardi.gabor@gmail.com>
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
   02110-1301 USA

*/

#include "igraph_stack.h"
#include "igraph_datatype.h"
#include "igraph_iterators.h"
#include "igraph_interrupt_internal.h"
#include "igraph_vector.h"
#include "igraph_interface.h"
#include "igraph_adjlist.h"

int igraph_i_eccentricity(const igraph_t *graph,
			  igraph_vector_t *res,
			  igraph_vs_t vids,
			  igraph_neimode_t mode,
			  const igraph_adjlist_t *adjlist) {

  int no_of_nodes=igraph_vcount(graph);
  igraph_stack_t stack;
  igraph_vit_t vit;
  igraph_vector_int_t counted;
  int i, mark=1;
  igraph_vector_t vneis, *neis=&vneis;

  IGRAPH_CHECK(igraph_stack_init(&stack, 100));
  IGRAPH_FINALLY(igraph_stack_destroy, &stack);
  
  IGRAPH_CHECK(igraph_vit_create(graph, vids, &vit));
  IGRAPH_FINALLY(igraph_vit_destroy, &vit);

  IGRAPH_CHECK(igraph_vector_int_init(&counted, no_of_nodes));
  IGRAPH_FINALLY(igraph_vector_int_destroy, &counted);

  if (!adjlist) {
    IGRAPH_VECTOR_INIT_FINALLY(neis, 0);
  }

  IGRAPH_CHECK(igraph_vector_resize(res, IGRAPH_VIT_SIZE(vit)));
  igraph_vector_fill(res, -1);

  for (i=0, IGRAPH_VIT_RESET(vit); 
       !IGRAPH_VIT_END(vit); 
       IGRAPH_VIT_NEXT(vit), mark++, i++) {

    int source;
    source=IGRAPH_VIT_GET(vit);
    IGRAPH_CHECK(igraph_stack_push(&stack, 0));
    IGRAPH_CHECK(igraph_stack_push(&stack, source));
    VECTOR(counted)[source]=mark;
    
    IGRAPH_ALLOW_INTERRUPTION();
    
    while (!igraph_stack_empty(&stack)) {
      int act=igraph_stack_pop(&stack);
      int dist=igraph_stack_pop(&stack);
      int j, n;

      if (dist > VECTOR(*res)[i]) {
	VECTOR(*res)[i]=dist;
      }

      if (adjlist) {
	neis=igraph_adjlist_get(adjlist, act);
      } else {
	IGRAPH_CHECK(igraph_neighbors(graph, neis, act, mode));
      }
      n=igraph_vector_size(neis);
      for (j=0; j<n; j++) {
	int nei=VECTOR(*neis)[j];
	if (VECTOR(counted)[nei] != mark) {
	  VECTOR(counted)[nei]=mark;
	  IGRAPH_CHECK(igraph_stack_push(&stack, dist+1));
	  IGRAPH_CHECK(igraph_stack_push(&stack, nei));
	}
      }      
    } /* while !igraph_stack_empty(stack) */

  } /* for IGRAPH_VIT_NEXT(vit) */

  if (!adjlist) {
    igraph_vector_destroy(neis);
    IGRAPH_FINALLY_CLEAN(1);
  }
  igraph_vector_int_destroy(&counted);
  igraph_vit_destroy(&vit);
  igraph_stack_destroy(&stack);
  IGRAPH_FINALLY_CLEAN(3);
  
  return 0;
}

/**
 * \function igraph_eccentricity
 * Eccentricity of some vertices
 * 
 * The eccentricity of a vertex is calculated by measuring the shortest
 * distance from (or to) the vertex, to (or from) all vertices in the
 * graph, and taking the maximum.
 * 
 * </para><para>
 * This implementation ignores vertex pairs that are in different
 * components. Isolated vertices have eccentricity zero.
 * 
 * \param graph The input graph, it can be directed or undirected.
 * \param res Pointer to an initialized vector, the result is stored
 *    here.
 * \param vids The vertices for which the eccentricity is calculated.
 * \param mode What kind of paths to consider for the calculation:
 *    \c IGRAPH_OUT, paths that follow edge directions;
 *    \c IGRAPH_IN, paths that follow the opposite directions; and
 *    \c IGRAPH_ALL, paths that ignore edge directions. This argument
 *    is ignored for undirected graphs.
 * \return Error code.
 * 
 * Time complexity: O(v*(|V|+|E|)), where |V| is the number of
 * vertices, |E| is the number of edges and v is the number of
 * vertices for which eccentricity is calculated.
 * 
 * \sa \ref igraph_radius().
 * 
 * \example examples/simple/igraph_eccentricity.c
 */

int igraph_eccentricity(const igraph_t *graph, 
			igraph_vector_t *res,
			igraph_vs_t vids,
			igraph_neimode_t mode) {

  return igraph_i_eccentricity(graph, res, vids, mode, /*adjlist=*/ 0);
}

/**
 * \function igraph_radius
 * Radius of a graph
 * 
 * The radius of a graph is the defined as the minimum eccentricity of 
 * its vertices, see \ref igraph_eccentricity().
 * 
 * \param graph The input graph, it can be directed or undirected.
 * \param radius Pointer to a real variable, the result is stored
 *   here.
 * \param mode What kind of paths to consider for the calculation:
 *    \c IGRAPH_OUT, paths that follow edge directions;
 *    \c IGRAPH_IN, paths that follow the opposite directions; and
 *    \c IGRAPH_ALL, paths that ignore edge directions. This argument
 *    is ignored for undirected graphs.
 * \return Error code.
 * 
 * Time complexity: O(|V|(|V|+|E|)), where |V| is the number of
 * vertices and |E| is the number of edges.
 * 
 * \sa \ref igraph_eccentricity().
 * 
 * \example examples/simple/igraph_radius.c
 */

int igraph_radius(const igraph_t *graph, igraph_real_t *radius, 
		  igraph_neimode_t mode) {

  int no_of_nodes=igraph_vcount(graph);

  if (no_of_nodes==0) {
    *radius = IGRAPH_NAN;
  } else {
    igraph_adjlist_t adjlist;
    igraph_vector_t ecc;
    IGRAPH_CHECK(igraph_adjlist_init(graph, &adjlist, mode));
    IGRAPH_FINALLY(igraph_adjlist_destroy, &adjlist);
    IGRAPH_VECTOR_INIT_FINALLY(&ecc, igraph_vcount(graph));
    IGRAPH_CHECK(igraph_i_eccentricity(graph, &ecc, igraph_vss_all(), 
				       mode, &adjlist));
    *radius = igraph_vector_min(&ecc);
    igraph_vector_destroy(&ecc);
    igraph_adjlist_destroy(&adjlist);
    IGRAPH_FINALLY_CLEAN(2);
  }
  
  return 0;
}
