/*
 * getRoutesRelationGraph.c
 *
 *  Created on: Jun 20, 2012
 *      Author: archee
 */


#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include "busroute.h"
#include "common.h"
#include "files.h"
#include "trace.h"
#include "geometry.h"

struct EndVertexinRRG
{
	char name[NAME_LENGTH];
	struct Duallist cross;
	struct EndVertexinRRG * nextevp;//next end vertex
	double weight;

};////End Vertex in the Route Relation Graph

struct HeadVertexinRRG//RRG=ROUTE RELATION GRAPH
{
  char name[NAME_LENGTH];
  struct EndVertexinRRG * headvp;//point to routes(end vertex) which are connected to it
};//Head vertex in the Route Relation Graph


int main(int argc, char **argv)
{
    FILE *fsource=NULL;
    FILE *fout=NULL;
    FILE *afout=NULL;
    FILE *bfout=NULL;
    FILE *cfout=NULL;
    struct Region *region;
    struct Hashtable routes;
    long routeTableSize = 500;
    unsigned long index;
    struct Item *p, *pa;

    unsigned long index1;
    struct Duallist routeList;
    struct Duallist *cellList;
	struct Item *aRoadItem, *bRoadItem, *cRoadItem, *aCrossItem, *aRouteItem;
	struct Item *aCellItem;
	struct Busroute *Headroutep, *Endroutep,*aBusroute;
	struct Cell *aCell;
	int reptimes;

    if(argc < 3) {
      printf("%s is used to get the relations between bus routes.\n", argv[0]);
      printf("Usage: %s .map (.lst |.bus ...)\n", argv[0]);
      exit(1);
    }


	region = NULL;

	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}

	hashtable_init(&routes, routeTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);
	duallist_init(&routeList);

	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			load_source_file(fsource, region, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, &routes, (void*(*)(FILE*, struct Region*, void *))load_route_with_hashtable, NULL, NULL, NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	for (index1=0; index1<routes.size; index1++){ 
		p=routes.head[index1];
		while (p!=NULL) {
			duallist_add_to_tail(&routeList, p->datap);
			p = p->next;
		}
	}				




	struct HeadVertexinRRG RRG[300];//array for storing Route relation graph


	struct Road *aRoad,*bRoad,*cRoad;
	struct EndVertexinRRG *newp, *pf;
	int cn,i,Vetexcounter;
	struct Cross *aCross;
	double weightfactor,afactor;
	double maxlength=0;


	int na, ma, nb, mb;

	char aroutename[NAME_LENGTH];
	char broutename[NAME_LENGTH];
	char croutename[NAME_LENGTH];

	for(i = 0; i< 300; i++)
		RRG[i].headvp = NULL;

	p=routeList.head;

    fout=fopen("RRGnew.txt", "w");
    afout=fopen("Vertex.txt", "w");
    bfout=fopen("Vertex1.txt", "w");
    cfout=fopen("routelength.txt", "w");

    printf( "routelist.nItems = %ld \n", routeList.nItems );

	Vetexcounter=0;

	for(index=0; index<routeList.nItems; index++){

		Headroutep=(struct Busroute *)p->datap;
		strncpy(RRG[index].name, Headroutep->name, strlen(Headroutep->name)+1);
		if (Headroutep->path->roads.nItems == 0) 
			continue;

		for(pa=routeList.head;pa!=NULL; pa=pa->next){
					Endroutep=(struct Busroute*)pa->datap;
					cn=0;

					if(!strcmp(RRG[index].name, Endroutep->name) || Endroutep->path->roads.nItems == 0) continue;//

					if(Headroutep->path->roads.head!=NULL && Endroutep->path->roads.head!=NULL){

						aRoadItem=Headroutep->path->roads.head;
						while(aRoadItem!=NULL)
						{
							aRoad=(struct Road *)aRoadItem->datap;
							bRoadItem=Endroutep->path->roads.head;
							while(bRoadItem!=NULL)
							{
								bRoad=(struct Road *)bRoadItem->datap;
								if (aRoad->headEnd->number==bRoad->headEnd->number||(aRoadItem->next==NULL && aRoad->tailEnd->number==bRoad->headEnd->number)) {
									if(cn==0){
										newp = (struct EndVertexinRRG*)malloc(sizeof(struct EndVertexinRRG));
										duallist_init(&newp->cross);
										strncpy(newp->name, Endroutep->name, strlen(Endroutep->name)+1);
										newp->nextevp=RRG[index].headvp;
										RRG[index].headvp=newp;
										newp->weight=0;
										cn++;
								    	}
									duallist_add_to_tail(&newp->cross, bRoad->headEnd);
								}

								if(bRoadItem->next==NULL && aRoad->headEnd->number==bRoad->tailEnd->number){
									if(cn==0){
										newp = (struct EndVertexinRRG*)malloc(sizeof(struct EndVertexinRRG));
										duallist_init(&newp->cross);
										strncpy(newp->name, Endroutep->name, strlen(Endroutep->name)+1);
										newp->nextevp=RRG[index].headvp;
										RRG[index].headvp=newp;
										newp->weight=0;
										cn++;
									}
									duallist_add_to_tail(&newp->cross, bRoad->tailEnd);
								}


								if(aRoadItem->next==NULL && bRoadItem->next==NULL && aRoad->tailEnd->number==bRoad->tailEnd->number){
									if(cn==0) {
										newp = (struct EndVertexinRRG*)malloc(sizeof(struct EndVertexinRRG));
										duallist_init(&newp->cross);
										strncpy(newp->name, Endroutep->name, strlen(Endroutep->name)+1);
										newp->nextevp=RRG[index].headvp;
										RRG[index].headvp=newp;
										newp->weight=0;
										cn++;
									}

									duallist_add_to_tail(&newp->cross, bRoad->tailEnd);

								}
								bRoadItem=bRoadItem->next;
							}
							aRoadItem=aRoadItem->next;
						}

					} else {
						if(Headroutep->path->roads.head!=NULL)
							printf("path %s is empty!\n", Headroutep->name);
						if(Endroutep->path->roads.head!=NULL)
							printf("path %s is empty!\n", Endroutep->name);
					}

				}

		/*		for(pf=RRG[index].headvp;pf!=NULL;pf=pf->nextevp){
					if(strcmp(RRG[index].name, pf->name)<=0)
						printf("%s, %s\n", RRG[index].name, pf->name);
					else
						printf("%s, %s\n", pf->name, RRG[index].name);

					for(aCrossItem=pf->cross.head;aCrossItem!=NULL;aCrossItem=aCrossItem->next){
						aCross=(struct Cross*)aCrossItem->datap;
						printf("%d,", aCross->number);
					}
					printf("\n");
				}*/



				for(na=0; na<NAME_LENGTH && RRG[index].name[na]!='_'; na++)
							{
								aroutename[na]=RRG[index].name[na];
							}
						        aroutename[na]='\0';

							   if(RRG[index].name[na]=='_' && RRG[index].name[na+1]=='d')
							{
								for(ma=na+1;ma>0;ma--)
								{
									aroutename[ma]=aroutename[ma-1];
								}
								aroutename[0]='-';
							}

				//UPWAY positive, DOWNWAY negative, e.g. 573_UPWAY->573;573_DOWNWAY->-573
				for(pf=RRG[index].headvp;pf!=NULL;pf=pf->nextevp){


					for(nb=0; nb<NAME_LENGTH && pf->name[nb]!='_'; nb++)
					{
			           broutename[nb]=pf->name[nb];
					}
					   broutename[nb]='\0';

					   if(pf->name[nb]=='_' && pf->name[nb+1]=='d')
					{
						for(mb=nb+1;mb>0;mb--)
						{
							broutename[mb]=broutename[mb-1];
						}
						broutename[0]='-';
					}

					int routenumber=-atoi(aroutename);
					convert_routeid_int_to_char(routenumber,croutename);

					afactor=0;
					reptimes=0;
			if(Headroutep->path->roads.head!=NULL && Endroutep->path->roads.head!=NULL){
		            cRoadItem=Headroutep->path->roads.head;
		            aRouteItem=duallist_find(&routeList, croutename, (int(*)(void*,void*))route_has_name);
		            aBusroute=(struct Busroute *)aRouteItem->datap;
		            weightfactor=1/(Headroutep->path->length);
		            if((Headroutep->path->length)>maxlength)maxlength=Headroutep->path->length;
		            aCrossItem=(struct Item*)pf->cross.head;
		            	while(cRoadItem!=NULL){
		            		if(aCrossItem==NULL)break;
		            		if(((struct Road*)cRoadItem->datap)->headEnd!=((struct Cross*)aCrossItem->datap))
		            			cRoadItem=cRoadItem->next;
		            		else if(((struct Road*)cRoadItem->datap)->headEnd==((struct Cross*)aCrossItem->datap)){
		            			cRoadItem=cRoadItem->next;
		            			aCrossItem=aCrossItem->next;
		            			if(aCrossItem==NULL || cRoadItem==NULL){
		            				reptimes++;
		            				break;
		            			}
                                if(((struct Road*)cRoadItem->datap)->headEnd!=((struct Cross*)aCrossItem->datap)) reptimes++;
                                else{
                                    while(((struct Road*)cRoadItem->datap)->headEnd==((struct Cross*)aCrossItem->datap)){
                                    cRoad=(struct Road*)cRoadItem->datap;
                                    afactor=afactor+cRoad->length;
                                    cRoadItem=cRoadItem->next;
                                    aCrossItem=aCrossItem->next;
                                    if(aCrossItem==NULL||cRoadItem==NULL) break;
                                    }
                                 }
		            		}
		            	}

		            	pf->weight=1-(weightfactor*100*reptimes+pow(weightfactor*afactor, (double)1/sqrt(afactor/100)));

					fprintf(fout, "%s %s %f %f %d %ld\n", aroutename, broutename, pf->weight, afactor, reptimes, pf->cross.nItems);
					//fprintf(cfout, "%s %f\n", broutename, weightfactor);//show pairwise routes and weight (contact cross)
				}
			}
				Vetexcounter++;
				fprintf(afout, "%s, %s\n",RRG[index].name, aroutename);
				p=p->next;

			    if(RRG[index].headvp!=NULL) fprintf(bfout, "%s\n",RRG[index].name);
			}

	       int aNumber,count=0;
	        char buf[128];

            for(p=routeList.head;p!=NULL;p=p->next)
            {
    			aBusroute = (struct Busroute*)p->datap;
    			convert_routeid_char_to_int(aBusroute->name,&aNumber);
    			cellList = get_route_coverage(region, aBusroute);
    			if(cellList) {
    				aCellItem = cellList->head;
    				   while(aCellItem) {
    					   count++;
    					   aCell = (struct Cell*)aCellItem->datap;
    					   sprintf(buf, "1%03d%03d", aCell->xNumber, aCell->yNumber);
    			           fprintf(fout, "%d %s %d\n", aNumber, buf,(int)ceil((100*(aBusroute->path->length)/(2*maxlength))));
    			           //fprintf(cfout, "%d %s %f\n", aNumber, buf,(aBusroute->path->length)/2);
    			   	      // fprintf(cfout, "%s %f\n", aBusroute->name, aBusroute->path->length);
    			           aCellItem = aCellItem->next;
    				   }
    			    }
			    duallist_destroy(cellList, NULL);
			    fprintf(cfout, "%s %f\n", aBusroute->name, aBusroute->path->length);
            }
    		printf("the number of (route -> cell) edges is %d\n", count);

	        fclose(fout);
		    fclose(afout);
		    fclose(bfout);
		    fclose(cfout);


            printf("cell size is %f!\n hcell is %ld\n hcell is %ld\n",region->cellSize,region->hCells, region->vCells);
			printf(	"number of vertex is %d\n",	Vetexcounter);
			printf("max length is %f\n", maxlength);
			printf("finish!");
			duallist_destroy(&routeList, (void(*)(void*))NULL);
			hashtable_destroy(&routes, (void(*)(void*))route_free_func);
			region_free_func(region);
			return 0;

		}

/*int routenumber=-atoi(aroutename);
convert_routeid_int_to_char(routenumber,croutename);

afactor=0;

cRoadItem=Headroutep->path->roads.head;
aRouteItem=duallist_find(&routeList, croutename, (int(*)(void*,void*))route_has_name);
aBusroute=(struct Busroute *)aRouteItem->datap;
weightfactor=1/(2*((Headroutep->path->length)+(aBusroute->path->length)));
aCrossItem=(struct Item*)pf->cross.head;
	while(cRoadItem!=NULL)
	{
		if(aCrossItem!=NULL)
		{
			aCross=(struct Cross*)aCrossItem->datap;
			if(((struct Road*)cRoadItem->datap)->headEnd!=aCross){
				cRoad=(struct Road*)cRoadItem->datap;
				afactor=afactor+cRoad->length;
				cRoadItem=cRoadItem->next;
			}
			else if(((struct Road*)cRoadItem->datap)->headEnd==aCross){
				pf->weight=pf->weight+weightfactor*afactor*afactor;
				afactor=0;
				cRoadItem=cRoadItem->next;
				aCrossItem=aCrossItem->next;
			}
		}
		else {
			cRoad=(struct Road*)cRoadItem->datap;
			afactor=afactor+cRoad->length;
			cRoadItem=cRoadItem->next;
		}
	}
	afactor=afactor+(aBusroute->path->length);
	pf->weight=pf->weight+weightfactor*afactor*afactor;

fprintf(fout, "%s %s %f\n", aroutename, broutename, pf->weight);
//fprintf(fout, "%s %s %d\n", broutename, aroutename, 1);//show pairwise routes and weight (contact cross)
}
Vetexcounter++;
fprintf(afout, "%s, %s\n",RRG[index].name, aroutename);
p=p->next;

if(RRG[index].headvp!=NULL) fprintf(bfout, "%s\n",RRG[index].name);
}

int aNumber,count=0;
char buf[128];

for(p=routeList.head;p!=NULL;p=p->next)
{
aBusroute = (struct Busroute*)p->datap;
convert_routeid_char_to_int(aBusroute->name,&aNumber);
cellList = get_route_coverage(region, aBusroute);
if(cellList) {
aCellItem = cellList->head;
   while(aCellItem) {
	   count++;
	   aCell = (struct Cell*)aCellItem->datap;
	   sprintf(buf, "1%03d%03d", aCell->xNumber, aCell->yNumber);
       fprintf(fout, "%d %s %f\n", aNumber, buf,(aBusroute->path->length)/2);
       aCellItem = aCellItem->next;
   }
}
duallist_destroy(cellList, NULL);
}
printf("the number of (route -> cell) edges is %d\n", count);

fclose(fout);
fclose(afout);
fclose(bfout);


printf("cell size is %f!\n hcell is %ld\n hcell is %ld\n",region->cellSize,region->hCells, region->vCells);
printf(	"number of vertex is %d\n",	Vetexcounter);
printf("finish!");
duallist_destroy(&routeList, (void(*)(void*))NULL);
hashtable_destroy(&routes, (void(*)(void*))route_free_func);
region_free_func(region);
return 0;

}*/

/*	for(pa=p->next;pa!=NULL; pa=pa->next){
					Endroutep=(struct Busroute*)pa->datap;
					cn=0;

					if(Headroutep->path->roads.head!=NULL && Endroutep->path->roads.head!=NULL){
						for(aRoadItem=Headroutep->path->roads.head; aRoadItem!=NULL; aRoadItem=aRoadItem->next){
							aRoad=(struct Road *)aRoadItem->datap;

							for(bRoadItem=Endroutep->path->roads.head; bRoadItem!=NULL; bRoadItem=bRoadItem->next){
								bRoad=(struct Road *)bRoadItem->datap;
								if (aRoad->headEnd->number==bRoad->headEnd->number||(aRoadItem->next==NULL && aRoad->tailEnd->number==bRoad->headEnd->number)) {
									if(cn==0){
										newp = (struct EndVertexinRRG*)malloc(sizeof(struct EndVertexinRRG));
										duallist_init(&newp->cross);
										strncpy(newp->name, Endroutep->name, strlen(Endroutep->name)+1);
										newp->nextevp=RRG[index].headvp;
										RRG[index].headvp=newp;
										duallist_init(&newp->cross);
										cn++;
								    	}
									duallist_add_to_head(&newp->cross, bRoad->headEnd);
								}

								if(bRoadItem->next==NULL && aRoad->headEnd->number==bRoad->tailEnd->number){
									if(cn==0){
										newp = (struct EndVertexinRRG*)malloc(sizeof(struct EndVertexinRRG));
										duallist_init(&newp->cross);
										strncpy(newp->name, Endroutep->name, strlen(Endroutep->name)+1);
										newp->nextevp=RRG[index].headvp;
										RRG[index].headvp=newp;
										duallist_init(&newp->cross);
										cn++;
									}
									duallist_add_to_head(&newp->cross, bRoad->tailEnd);
								}


								if(aRoadItem->next==NULL && bRoadItem->next==NULL && aRoad->tailEnd->number==bRoad->tailEnd->number){
									if(cn==0) {
										newp = (struct EndVertexinRRG*)malloc(sizeof(struct EndVertexinRRG));
										duallist_init(&newp->cross);
										strncpy(newp->name, Endroutep->name, strlen(Endroutep->name)+1);
										newp->nextevp=RRG[index].headvp;
										RRG[index].headvp=newp;
										duallist_init(&newp->cross);
										cn++;
									}

									duallist_add_to_head(&newp->cross, bRoad->tailEnd);

								}
							}
						}

					} else {
						printf("path is empty!");
						exit(1);
					}

				}

				for(pf=RRG[index].headvp;pf!=NULL;pf=pf->nextevp){
					if(strcmp(RRG[index].name, pf->name)<=0)
						printf("%s, %s\n", RRG[index].name, pf->name);
					else
						printf("%s, %s\n", pf->name, RRG[index].name);

					for(aCrossItem=pf->cross.head;aCrossItem!=NULL;aCrossItem=aCrossItem->next){
						aCross=(struct Cross*)aCrossItem->datap;
						printf("%d,", aCross->number);
					}
					printf("\n");
				}



				for(na=0; na<NAME_LENGTH && RRG[index].name[na]!='_'; na++)
							{
								aroutename[na]=RRG[index].name[na];
							}
						        aroutename[na]='\0';

							   if(RRG[index].name[na]=='_' && RRG[index].name[na+1]=='d')
							{
								for(ma=na+1;ma>0;ma--)
								{
									aroutename[ma]=aroutename[ma-1];
								}
								aroutename[0]='-';
							}

				//UPWAY positive, DOWNWAY negative, e.g. 573_UPWAY->573;573_DOWNWAY->-573
				for(pf=RRG[index].headvp;pf!=NULL;pf=pf->nextevp){

					crosscount=0;

					for(nb=0; nb<NAME_LENGTH && pf->name[nb]!='_'; nb++)
					{
			           broutename[nb]=pf->name[nb];
					}
					   broutename[nb]='\0';

					   if(pf->name[nb]=='_' && pf->name[nb+1]=='d')
					{
						for(mb=nb+1;mb>0;mb--)
						{
							broutename[mb]=broutename[mb-1];
						}
						broutename[0]='-';
					}

		            for(aCrossItem=pf->cross.head;aCrossItem!=NULL;aCrossItem=aCrossItem->next)
		            //{crosscount++;}
		            {

		            }

					fprintf(fout, "%s %s %d\n", aroutename, broutename, 1);
					fprintf(fout, "%s %s %d\n", broutename, aroutename, 1);//show pairwise routes and weight (contact cross)
				}
				Vetexcounter++;
				fprintf(afout, "%s, %s\n",RRG[index].name, aroutename);
				p=p->next;

			    if(RRG[index].headvp!=NULL) fprintf(bfout, "%s\n",RRG[index].name);
			}



		    fclose(fout);
		    fclose(afout);
		    fclose(bfout);




			printf(	"number of vertex is %d\n",	Vetexcounter);
			printf("finish!");
			duallist_destroy(&routeList, (void(*)(void*))NULL);
			hashtable_destroy(&routes, (void(*)(void*))route_free_func);
			region_free_func(region);
			return 0;

		}*/


