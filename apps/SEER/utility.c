#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<sys/time.h>
#include"utility.h"


void setup_road_scrap(struct Road *aRoad, int nSlides)
{
	struct Slide *newSlide;
	int k;


	if(nSlilds!=0) {
		for (k = 0; k<nSlides; k++) {
			newSlide = (struct Slide*)malloc(sizeof(struct Slide));
			duallist_init(&newSlide->samples);
			newSlide->condition = 0;
			duallist_add_to_tail(&aRoad->slides, newSlide);
		}
	}

}

void install_log_to_cell(struct Log *aLog)
{
	int i,j;
	struct Cell *aCell;
	struct LogList *newEntry;
	
	i = (aLog->gPoint.x - region->box.xmin)/region->cellSize;
	j = (aLog->gPoint.y - region->box.ymin)/region->cellSize;
	aCell = region->mesh + i*region->height + j;

	newEntry = (struct LogList*)malloc(sizeof(struct LogList));
	newEntry->aLog = aLog;
	newEntry->next = NULL;
	newEntry->prev = NULL;
	if(aCell->alls == NULL) {
		aCell->alls = newEntry;
		newEntry->prev = newEntry;
	} else {
		aCell->alls->prev->next = newEntry;
		newEntry->prev = aCell->alls->prev;
		aCell->alls->prev = newEntry;
	}	
}

/* different from the log list in the traces, 
here logs are double-chained with the prev 
pointer of the first log pointing to the last log
*/
void install_log_to_cell_slide(struct Log *aLog)
{
	int i,j,whichSlide;
	struct Cell *aCell;
	struct LogList *newEntry;
	struct LogList *aSlide;
	
	i = (aLog->gPoint.x - region->box.xmin)/region->cellSize;
	j = (aLog->gPoint.y - region->box.ymin)/region->cellSize;
	if (i < 0) i = 0;
	if (j < 0) j = 0;
	if (i > region->width ) i = region->width;
	if (j > region->height) j = region->height;
	aCell = region->mesh + i*region->height + j;

	if(region->interval == 0) return;
	whichSlide = (aLog->timestamp - region->starttime)/region->interval;
	if(whichSlide < 0 || whichSlide >= (region->endtime-region->starttime)/region->interval) return;
	aSlide = (aCell->slides)[whichSlide];

	newEntry = (struct LogList*)malloc(sizeof(struct LogList));
	newEntry->aLog = aLog;
	newEntry->next = NULL;
	newEntry->prev = NULL;
	if(aSlide == NULL) {
		(aCell->slides)[whichSlide] = newEntry;
		newEntry->prev = newEntry;
	} else {
		aSlide->prev->next = newEntry;
		newEntry->prev = aSlide->prev;
		aSlide->prev = newEntry;
	}	
}

void install_log_to_road_slide(struct Log *aLog)
{
	int whichSlide;
	struct SampleList *newEntry, *tmp;	
	struct Slide *aSlide;
	struct ScrapList *aScrap;
	
	if(aLog->onRoad==NULL||aLog->onRoad!=NULL&&aLog->onRoad->aRoad==NULL) return;
	if(region->interval == 0) return;


	if(region->interval == 0) return;
	whichSlide = (aLog->timestamp - region->starttime)/region->interval;
	if(whichSlide < 0 || whichSlide >= (region->endtime-region->starttime)/region->interval) return;
	aScrap = aLog->onRoad->aRoad->scraps;
	while(aScrap!=NULL) {
		if(is_point_in_box(&(aLog->onRoad->gPoint),&(aScrap->box))) { 
			if(aLog->onRoad->aRoad->bigEnd == aLog->onRoad->toCross) {
				aSlide = aScrap->s2b+whichSlide;
				aLog->onRoad->aRoad->nSamplesS2B ++;
			} else {
				aSlide = aScrap->b2s+whichSlide;
				aLog->onRoad->aRoad->nSamplesB2S ++;
			}
			newEntry = (struct SampleList*)malloc(sizeof(struct SampleList));
			newEntry->speed = aLog->speed;
			newEntry->next = NULL;
			if(aSlide->samples == NULL) {
				aSlide->samples = newEntry;
				tmp = newEntry;
			} else {
				newEntry->next = aSlide->samples;
				aSlide->samples = newEntry;
			}	
			aSlide->nSamples ++;
			break;
		}
		aScrap = aScrap->next;
	}
}

struct Road* find_road_by_id(struct RoadList *head, int id)
{
	struct RoadList *aRoad;
	aRoad = head;
	while(aRoad!=NULL) {
		if(aRoad->aRoad->number == id)
			return aRoad->aRoad;
		aRoad = aRoad->next;
	}
	return NULL;
}


void load_road_condition(char *roadinfo)
{
	FILE *froadinfo;
	char line[256], *buffer, *p, *q;
	int totalSlides, nSlides, rId, rDi, i, pass;
	struct Road *aRoad;
	struct ScrapList *aScrap;
	time_t origtm, endtm;
	char ch;

	if(region->interval == 0) return;
	nSlides = ceil((double)(region->endtime-region->starttime)/region->interval);


	if((froadinfo=fopen(roadinfo, "r"))==NULL) {
		printf("Cannot open %s to read!\n", roadinfo);
	} else {
		fgets(line, 256, froadinfo);
		fgets(line, 256, froadinfo);
		origtm = strtot(line);
		fgets(line, 256, froadinfo);
		endtm = strtot(line);

		pass = (region->starttime-origtm)/region->interval;
		totalSlides = (endtm-origtm)/region->interval;
		buffer = (char*)malloc(32*(totalSlides+1));

		fgets(line, 256, froadinfo);
		fgets(line, 256, froadinfo);
		fgets(line, 256, froadinfo);
		sscanf(line, "%lf %lf", &(region->lower), &(region->upper));
		for (i=0;i<4;i++)
			fgets(line, 256, froadinfo);

		while (fgetc(froadinfo)!=EOF)
		{
			fseek(froadinfo, -1, SEEK_CUR);

			fgets(line, 256, froadinfo);
			p = strtok(line, " ");
			if(atoi(p)!=0) {
				printf("Wrong road infomation dump file!\n");
				free(buffer);
				return;
			}
			p = strtok(NULL, " ");
			rId = atoi(p);
			p = strtok(NULL, " ");
			rDi = atoi(p);

			aRoad = find_road_by_id(region->roads.head, rId);
			if(aRoad!=NULL) {
				if(aRoad->scraps==NULL) {
					while( (ch=fgetc(froadinfo)) == 49) {
						fgets(buffer, 32*(totalSlides+1), froadinfo);
					} 
					if(ch == EOF) break;
					fseek(froadinfo, -1, SEEK_CUR);
				} else {
					aScrap = aRoad->scraps;
					while(aScrap!=NULL) {
						fgets(buffer, 32*(totalSlides+1), froadinfo);
						p = strtok(buffer, " ");
						if(atoi(p)!=1) {
							printf("Wrong road infomation dump file!\n");
							free(buffer);
							return;
						}
						for(i=0;i<pass;i++) {
							p = strtok(NULL, " ");
							q = strtok(NULL, " ");
						}
							
						for(i=0;i<nSlides; i++) {
							p = strtok(NULL, " ");
							q = strtok(NULL, " ");
							if(p[0]=='\n') 
								break;
							if(rDi==0) {
								aScrap->s2b[i].condition = atof(p);
								aScrap->s2b[i].nSamples = atof(q);
							} else {
								aScrap->b2s[i].condition = atof(p);
								aScrap->b2s[i].nSamples = atof(q);
							}
						}
						aScrap = aScrap->next;
					}
				}
			} else {
				while( (ch=fgetc(froadinfo)) == 49) {
					fgets(buffer, 32*(totalSlides+1), froadinfo);
				} 
				if(ch == EOF) break;
				fseek(froadinfo, -1, SEEK_CUR);
			}
		}
	fclose(froadinfo);
	}
}

void calculate_road_condition(double *lower, double *upper)
{
	struct RoadList *aRoad;
	struct ScrapList *aScrap;
	int i,nSlides;
	double nSamples;
	struct SampleList *aSample;
	struct Slide *aSlide;
	double sum;

	if(region->interval > 0)
		nSlides = (region->endtime-region->starttime)/region->interval;
	else
		nSlides = 0;
	aRoad = region->roads.head;
	while(aRoad != NULL) {
		aScrap = aRoad->aRoad->scraps;
		while(aScrap!=NULL) {
			for (i =0 ; i<nSlides; i++) {
				aSlide = aScrap->s2b + i;
				nSamples = 0;
				aSlide->condition = 0;
				sum = 0;
				aSample = aSlide->samples;
				while(aSample!=NULL) {
					if(aSample->speed!=0) {
						sum = sum + aSample->speed;
						nSamples +=1;
					}
					aSample = aSample->next;
				}

				if(aSlide->nSamples>2&&sum != 0) {
					aSample = aSlide->samples;
					while(aSample!=NULL) {
						aSlide->condition = aSlide->condition + aSample->speed*aSample->speed/sum;
						aSample = aSample->next;
						aSlide->nSamples = nSamples;
					}
				} else {
					if (aSlide->nSamples>2) {
						aSlide->condition = 0;
					} else {
						aSlide->condition = -1;
					}
				}
					
				if(lower!=NULL && *lower == -1 && aSlide->condition >=0) {
					*lower = aSlide->condition;
				} else if(lower!=NULL && aSlide->condition < *lower&& aSlide->condition >=0) {
					*lower =  aSlide->condition;
				}
				if(upper!=NULL && *upper == -1&& aSlide->condition >=0) {
					*upper = aSlide->condition;
				} else if(upper!=NULL && aSlide->condition > *upper&& aSlide->condition >=0) {
					*upper =  aSlide->condition;
				}

				aSlide = aScrap->b2s + i;
				nSamples = 0;
				aSlide->condition = 0;
				sum = 0;
				aSample = aSlide->samples;
				while(aSample!=NULL) {
					if(aSample->speed!=0) {
						sum = sum + aSample->speed;
						nSamples +=1;
					}
					aSample = aSample->next;
				}

				if(aSlide->nSamples>2&&sum != 0) {
					aSample = aSlide->samples;
					while(aSample!=NULL) {
						aSlide->condition = aSlide->condition + aSample->speed*aSample->speed/sum;
						aSample = aSample->next;
						aSlide->nSamples = nSamples;
					}
				} else {
					if (aSlide->nSamples>2) {
						aSlide->condition = 0;
					} else {
						aSlide->condition = -1;
					}
				}
					
				if(lower!=NULL && *lower == -1 && aSlide->condition >=0) {
					*lower = aSlide->condition;
				} else if(lower!=NULL && aSlide->condition < *lower&& aSlide->condition >=0) {
					*lower =  aSlide->condition;
				}
				if(upper!=NULL && *upper == -1&& aSlide->condition >=0) {
					*upper = aSlide->condition;
				} else if(upper!=NULL && aSlide->condition > *upper&& aSlide->condition >=0) {
					*upper =  aSlide->condition;
				}
			}
			aScrap = aScrap->next;
		}
		aRoad = aRoad->next;
	}
}

void dump_every_sample_on_road(int rId)
{
	struct ScrapList *aScrap;
	int i,nSlides;
	double nSamples;
	struct SampleList *aSample;
	struct Slide *aSlide;
	struct RoadList *aRoad;
	double maxs, mins;

	aRoad = region->roads.head;
	while(aRoad != NULL) {
		if(rId != -1 && rId != aRoad->aRoad->number) {	
			aRoad = aRoad->next;
			continue;
		}

		if(region->interval > 0)
			nSlides = (region->endtime-region->starttime)/region->interval;
		else
			nSlides = 0;
		aScrap = aRoad->aRoad->scraps;
		for (i =0 ; i<nSlides; i++) {
			aSlide = aScrap->s2b + i;
			maxs = 0; 
			mins = MAXVALUE;
			aSample = aSlide->samples;
			while(aSample!=NULL) {
				if(aSample->speed>maxs ) {
					maxs = aSample->speed;
				}
				if (aSample->speed<mins) {
					mins= aSample->speed;
				} 
				aSample = aSample->next;
			}
			if(aSlide->nSamples > 1)
				printf("%0.2lf ", maxs-mins);
		}

		for (i =0 ; i<nSlides; i++) {
			aSlide = aScrap->b2s + i;
			maxs = 0; 
			mins = MAXVALUE;
			aSample = aSlide->samples;
			while(aSample!=NULL) {
				if(aSample->speed>maxs ) {
					maxs = aSample->speed;
				}
				if (aSample->speed<mins) {
					mins= aSample->speed;
				} 
				aSample = aSample->next;
			}
			if(aSlide->nSamples > 1)
				printf("%0.2lf ", maxs-mins);
		}
		aRoad = aRoad->next;
	}
}

void dump_road_slides(struct RoadList *aRoad, FILE *fdump)
{
	struct ScrapList *aScrap;
	int i,nSlides;
	double nSamples;
	struct SampleList *aSample;
	struct Slide *aSlide;
	double sum;

	fprintf(fdump, "0 %d %d %.2lf %0.2lf\n",	aRoad->aRoad->number,
							aRoad->direction,
						    	aRoad->aRoad->length,
					    		aRoad->density);
	if(region->interval > 0)
		nSlides = (region->endtime-region->starttime)/region->interval;
	else
		nSlides = 0;
	aScrap = aRoad->aRoad->scraps;
	while(aScrap!=NULL) {
		fprintf(fdump, "1 ");
		for (i =0 ; i<nSlides; i++) {
			if(aRoad->direction==0)
				aSlide = aScrap->s2b + i;
			else
				aSlide = aScrap->b2s + i;
			fprintf(fdump, "%.2lf %.0lf  ", aSlide->condition, aSlide->nSamples);
		}
		fprintf(fdump, "\n");
		aScrap = aScrap->next;
	}
}


void dump_road_condition()
{
	char t1[32], t2[32], filename[256];
	struct RoadList *aRoad;
	struct RoadList *roadList;
	int count=0;

	FILE *fdump;
	ttostr(region->starttime,t1);
	ttostr(region->endtime, t2);
	
	sprintf(filename, "%s_%s_%d_%d.dump", t1, t2, region->scrap, region->interval);
	fdump = fopen(filename, "w");
	if(fdump == NULL) {
		printf("Cannot open %s to write!\n", filename);
		return;
	}

	printf("%lf %lf %lf %lf\n", region->box.xmin, region->box.ymin,
					   region->box.xmax, region->box.ymax);
	fprintf(fdump, "%lf, %lf %lf %lf\n", region->box.xmin, region->box.ymin, region->box.xmax, region->box.ymax);
	printf("%s\n",  t1);
	fprintf(fdump,"%s\n", t1);
	printf("%s\n",  t2);
	fprintf(fdump,"%s\n", t2);
	printf("%d\n", region->interval);
	fprintf(fdump, "%d\n", region->interval);
	printf("%d\n", region->scrap);
	fprintf(fdump, "%d\n", region->scrap);
	fprintf(fdump, "%0.2lf %.2lf\n", region->lower, region->upper);
	printf("The number of roads: %d\n", region->roads.nRoads);
	fprintf(fdump, "The number of roads: %d\n", region->roads.nRoads);
	printf("The number of crosses: %d\n", region->crosses.nCrosses);
	fprintf(fdump, "The number of crosses: %d\n", region->crosses.nCrosses);
	printf("The number of traces: %d\n", traces.nTraces);
	fprintf(fdump, "The number of traces: %d\n", traces.nTraces);
	printf("------------\n");
	fprintf(fdump,"------------\n");
	printf("Start to dump...\n");

	roadList = sort_road_by_density(region->roads.head);
	aRoad = roadList;
	while(aRoad!=NULL) {
		dump_road_slides(aRoad, fdump);
		count ++;
		aRoad = aRoad->next;
	}	
	fflush(fdump);
	fclose(fdump);
	free_roadlist(roadList);
	printf("Done.\n");
}

struct RoadList * sort_road_by_density(struct RoadList *aList)
{
	struct RoadList *rtList=NULL, *aRoad, *bRoad, *dRoad, *newp;

	if(aList == NULL) return NULL;
		
	aRoad = aList;
	while(aRoad !=NULL) {
		newp = NULL;
		if(aRoad->aRoad->nSamplesS2B > 0 && aRoad->aRoad->length != 0) {
			newp=(struct RoadList*)malloc(sizeof(struct RoadList));
			newp->aRoad = aRoad->aRoad;
			newp->direction = 0;
			newp->density = aRoad->aRoad->nSamplesS2B/aRoad->aRoad->length;	
			newp->next = NULL;
		}
		if(aRoad->aRoad->nSamplesB2S > 0 && aRoad->aRoad->length != 0) {
			newp=(struct RoadList*)malloc(sizeof(struct RoadList));
			newp->aRoad = aRoad->aRoad;
			newp->direction = 1;
			newp->density = aRoad->aRoad->nSamplesB2S/aRoad->aRoad->length;	
			newp->next = NULL;
		}
		if(newp!=NULL) {
			if (rtList == NULL) {
				rtList = newp;
			} else {
				bRoad = rtList;
				while(bRoad->next!=NULL && bRoad->density >= newp->density) {
					dRoad = bRoad;
					bRoad = bRoad->next;
				}
				if(bRoad->density < newp->density) {
					newp->next = bRoad;
					if(bRoad == rtList) {
						rtList = newp;
					} else {
						dRoad->next = newp;
					}
				} else {
					newp->next = bRoad->next;
					bRoad->next = newp;
				}
			}
		}
		aRoad = aRoad->next;
	}
	return rtList;
}



/* 
* mouse-on-screen operations
*
*
***************************************************************************
***************************************************************************/
void build_polygon(struct Polygon ** chosen_polygon, struct Point *aPoint)
{
  struct PointList *newp;

  newp = (struct PointList*)malloc(sizeof(struct PointList));
  newp->x = aPoint->x, newp->y = aPoint->y, newp->next = NULL;
  if(*chosen_polygon == NULL) {
	*chosen_polygon = (struct Polygon*)malloc(sizeof(struct Polygon));
	(*chosen_polygon)->points = newp;
	(*chosen_polygon)->currentPoint = newp;
	(*chosen_polygon)->nPoints = 1;
	(*chosen_polygon)->mouseAt = newp;
	(*chosen_polygon)->state = 0;
	(*chosen_polygon)->box.xmin = (*chosen_polygon)->box.xmax = newp->x;
	(*chosen_polygon)->box.ymin = (*chosen_polygon)->box.ymax = newp->y;
  } else {
	(*chosen_polygon)->currentPoint->next = newp;
	(*chosen_polygon)->currentPoint = newp;
	(*chosen_polygon)->nPoints ++;
	(*chosen_polygon)->mouseAt = newp;
	(*chosen_polygon)->box.xmin = MIN((*chosen_polygon)->box.xmin, newp->x);
	(*chosen_polygon)->box.xmax = MAX((*chosen_polygon)->box.xmax, newp->x);
	(*chosen_polygon)->box.ymin = MIN((*chosen_polygon)->box.ymin, newp->y);
	(*chosen_polygon)->box.ymax = MAX((*chosen_polygon)->box.ymax, newp->y);
  }
}

void close_polygon(struct Polygon *chosen_polygon)
{
  
  struct PointList *newp, *p;

  newp = (struct PointList*)malloc(sizeof(struct PointList));
  newp->x = chosen_polygon->points->x, newp->y = chosen_polygon->points->y;
  newp->next = NULL;

  chosen_polygon->currentPoint->next = newp;
  chosen_polygon->mouseAt = NULL;
  /* the polygon is completed */
  chosen_polygon->state = 1;
  chosen_polygon->scale = 1;
}

void free_polygon(struct Polygon * chosen_polygon)
{
  struct PointList *apoint;

  if(chosen_polygon != NULL) 
	while(chosen_polygon->points != NULL) {
		apoint = chosen_polygon->points->next;
		free(chosen_polygon->points);
		chosen_polygon->points = apoint;
	}
	free(chosen_polygon);
}
			

int is_legal(struct Polygon *chosen_polygon, struct Point *aPoint)
{
  struct PointList *previous, *p;
  struct Segment seg1, seg2;
  previous = chosen_polygon->points;

  if(chosen_polygon->nPoints < 2) 
	if (equald(chosen_polygon->points->x, aPoint->x, DELTA1) && equald(chosen_polygon->points->y, aPoint->y, DELTA1)) 
		return 0;
	else 
		return 1;

  while(previous->next != chosen_polygon->currentPoint) previous = previous->next; 
  if( (equald(aPoint->x, previous->x, DELTA1) && equald(aPoint->y, previous->y, DELTA1)) 
   || (equald(chosen_polygon->currentPoint->x, aPoint->x, DELTA1) && equald(chosen_polygon->currentPoint->y, aPoint->y, DELTA1)))
	return 0;

  seg1.x1 = aPoint->x, seg1.y1 = aPoint->y;
  seg1.x2 = chosen_polygon->currentPoint->x, seg1.y2 = chosen_polygon->currentPoint->y;
  p = chosen_polygon->points;
  while(p!=previous) {
	seg2.x1 = p->x, seg2.y1 = p->y;
	seg2.x2 = p->next->x, seg2.y2 = p->next->y;
 	if(are_segments_intersected(&seg1, &seg2)) return 0;
 	p = p->next;
  }
  return 1;
}


int is_polygon(struct Polygon *chosen_polygon, struct Point *aPoint)
{
  struct PointList *previous, *p;
  previous = chosen_polygon->points;

  if(chosen_polygon->nPoints < 2) return 0;

  while(previous->next != chosen_polygon->currentPoint) previous = previous->next; 
  if( (equald(aPoint->x, previous->x, DELTA1) && equald(aPoint->y, previous->y, DELTA1))
   || (equald(chosen_polygon->currentPoint->x,aPoint->x, DELTA1) && equald(chosen_polygon->currentPoint->y, aPoint->y, DELTA1)))
	return 0;

  p = chosen_polygon->points;
  while(p!=previous) {
 	if(equald(aPoint->x,p->x,DELTA1) && equald(aPoint->y, p->y, DELTA1)) return 1;
 	p = p->next;
  }
  return 0;
}

