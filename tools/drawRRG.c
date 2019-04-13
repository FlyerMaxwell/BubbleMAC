/*
 * drawRRG.c
 *
 *  Created on: Jun 26, 2012
 *      Author: archee
 */


#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<gtk/gtk.h>
#include "busroute.h"
#include "common.h"
#include "files.h"
#include "trace.h"
#include "geometry.h"


struct EVertexRRG
{
	char name[NAME_LENGTH];
	struct Duallist cross;
	struct EVertexRRG * nextevp;//next end vertex

};////End Vertex in the Route Relation Graph

struct HVertexRRG//RRG=ROUTE RELATION GRAPH
{
  char name[NAME_LENGTH];
  struct EVertexRRG * headvp;//point to routes(end vertex) which are connected to it
};//Head vertex in the Route Relation Graph

int main(int argc, char *argv[])
{

   /* struct HVertexRRG RRG[300];
    struct EVertexRRG *newEVp;
    FILE *fInput=NULL;


    struct Item *aItem;
    struct Road *aRoad;
    int id;
    unsigned long i, nItems;*/

    GtkWidget *window;
    gtk_init (&argc, &argv); /* 初始化显示环境 */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL); /* 创建一个新的窗口*/
    //gtk_widget_set_title (GTK_WINDOW(window), "Hello, World");
    gtk_widget_show (window); /*显示窗口*/
    gtk_main (); /*进入睡眠状态，等待事件激活*/


 /*   if(argc < 2) {
      printf("%s is used to show the relation graph of bus routes.\n", argv[0]);
      exit(1);
    }

	fInput=fopen(argv[1], "r");
	if(fInput==NULL) {
		printf("The .txt file was not opened\n");
		exit(1);
	}


    while()
    {

    }


    newHVp = (struct Busroute*)malloc(sizeof(struct Busroute));

    newp = (struct EndVertexinRRG*)malloc(sizeof(struct EndVertexinRRG));
    duallist_init(&newp->cross);

    for

    fscanf(fInput, "%s", newRoute->name);
    fscanf(fInput, "%lf %lf %lf %lf\n", &newRoute->box.xmin, &newRoute->box.xmax, &newRoute->box.ymin, &newRoute->box.ymax);

    fscanf(fInput, "%lu\n", &nItems);
    if(nItems){
    	for(i=0;i<nItems;i++) {
    		fscanf(fInput, "%d\n", &id);
    		if ((aItem = duallist_find(&aRegion->roads, &id, (int(*)(void*,void*))road_has_id))!=NULL) {
    			aRoad = (struct Road*)aItem->datap;
    			duallist_add_to_tail(&newRoute->path->roads, aRoad);
    		}
    	}
  		fscanf(fInput, "%lf\n", &newRoute->path->length);
    	fscanf(fInput, "%d\n", &newRoute->path->turns);
    }

    fscanf(fInput, "%lu\n", &nItems);
    for(i=0;i<nItems;i++) {
    	fscanf(fInput, "%d\n", &id);
    	duallist_add_to_tail(&newRoute->stops, (int*)id);
    }

    	return newRoute;



*/
    return(0);

}
