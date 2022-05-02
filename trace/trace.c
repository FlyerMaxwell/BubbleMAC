#include<string.h>
#include<stdlib.h>
#include<stdlib.h>
#include"trace.h"
#include"rnorrexp.h"
#include"files.h"

int candroad_has_road(struct Road *aRoad, struct CandRoad *aCandRoad)
{
	return aCandRoad->aRoad == aRoad;
}

int candroad_has_smaller_weight_than(struct CandRoad *aCandRoad, struct CandRoad *bCandRoad)
{
	return aCandRoad->weight < bCandRoad->weight;
} 

void add_candidate_roads(struct Cell *aCell, struct Report *aRep, int angleValid)
{
	struct Item *aItem;
	struct CandRoad *aCandRoad;
	struct Road *aRoad;
	double angle1, angle2;


	aItem = aCell->roads.head;
	while(aItem!=NULL) {
		aRoad = (struct Road*)aItem->datap;
		if(duallist_find(&aRep->candRoads, aRoad, (int(*)(void*,void*))candroad_has_road)) { 
			aItem = aItem->next;
			continue;
		}

		aCandRoad = (struct CandRoad*)malloc(sizeof(struct CandRoad));
		aCandRoad->aRoad = aRoad;
		aCandRoad->distance = distance_point_to_polyline(&(aRep->gPoint), &aRoad->points, &(aCandRoad->gPoint), &(aCandRoad->onSegment));

		if(aCandRoad->distance >= MAX_DISTANCE_ERROR) 
			aCandRoad->distance = MAX_DISTANCE_ERROR;

		if(angleValid) {
			angle1 = aRep->angle;
			angle2 = angle_between(aCandRoad->onSegment.aPoint.x, aCandRoad->onSegment.aPoint.y, aCandRoad->onSegment.bPoint.x, aCandRoad->onSegment.bPoint.y); 
			aCandRoad->angle = inter_angle(angle1, angle2);
			if(aCandRoad->angle > 90 && aRep->speed > HEADING_SPEED_THRESHOLD ) {
				free(aCandRoad);
				aItem = aItem->next;
				continue;
			}
		} else
			aCandRoad->angle = 0;
		if (aCandRoad->angle >= MAX_ANGLE_ERROR) 
			aCandRoad->angle = MAX_ANGLE_ERROR;

		aCandRoad->weight = (1-aCandRoad->distance/MAX_DISTANCE_ERROR) + (1-aCandRoad->angle/MAX_ANGLE_ERROR);
	
		duallist_add_in_sequence_from_head(&aRep->candRoads, aCandRoad, (int(*)(void*, void*))candroad_has_smaller_weight_than);	
		aItem = aItem->next;
	}
}



int trace_has_name(char *name, struct Trace* aTrace)
{
	return !strcmp(name, aTrace->vName);
}


int are_two_reports_duplicated(struct Report *aRep, struct Report *bRep)
{
	if( aRep->timestamp == bRep->timestamp 
	 && aRep->gPoint.x == bRep->gPoint.x
	 && aRep->gPoint.y == bRep->gPoint.y
	 && aRep->speed == bRep->speed
	 && aRep->angle == bRep->angle
	 && aRep->state == bRep->state
	 && aRep->msgType == bRep->msgType
	 && aRep->errorInfo == bRep->errorInfo )
		return 1;
	else 
		return 0;
}

int report_has_earlier_timestamp_than(struct Report* aRep, struct Report *bRep)
{	
	//Function: return a report with earlier timesatamp.
	return aRep->timestamp < bRep->timestamp;
} 

int report_has_later_timestamp_than(struct Report* aRep, struct Report *bRep)
{
	return aRep->timestamp > bRep->timestamp;
} 

void dump_report(FILE *fp, struct Report* aRep, int type)
{
	char time[20];

	ttostr(aRep->timestamp, time);
	if(type == FILE_ORIGINAL_GPS_TAXI)
		fprintf(fp, "%s,%s,%f,%f,%3d,%3d,%3d\n", 
				aRep->fromTrace->vName, 
				time, 
				aRep->gPoint.x, 
				aRep->gPoint.y, 
				aRep->speed, 
				aRep->angle,
				aRep->state);
	else if (type == FILE_MODIFIED_GPS_TAXI)
		fprintf(fp, "%s,%s,%f,%f,%3d,%3d,%3d,%d\n", 
				aRep->fromTrace->vName, 
				time, 
				aRep->gPoint.x, 
				aRep->gPoint.y, 
				aRep->speed, 
				aRep->angle,
				aRep->state,
				aRep->onRoadId);
	else if (type == FILE_ORIGINAL_GPS_BUS)
		fprintf(fp, "%s,%s,%f,%f,%d,%d,%s,%d,%d,%d,%f,%f\n", 
				aRep->fromTrace->vName, 
				time, 
				aRep->gPoint.x, 
				aRep->gPoint.y, 
				aRep->speed, 
				aRep->angle,
				aRep->fromTrace->onRoute, 
				aRep->msgType, 
				aRep->state,
				aRep->errorInfo,
				aRep->routeLeng,
				aRep->gasVol);
	else if (type == FILE_MODIFIED_GPS_BUS)
		fprintf(fp, "%s,%s,%f,%f,%d,%d,%s,%d,%d,%d,%f,%f,%d\n", 
				aRep->fromTrace->vName, 
				time, 
				aRep->gPoint.x, 
				aRep->gPoint.y, 
				aRep->speed, 
				aRep->angle,
				aRep->fromTrace->onRoute, 
				aRep->msgType, 
				aRep->state,
				aRep->errorInfo,
				aRep->routeLeng,
				aRep->gasVol,
				aRep->onRoadId);
}

void trace_dump_func(FILE *fp, struct Trace *aTrace)
{
	struct Item *aItem;
	struct Report *aRep;

	fprintf(fp, "%d\n", aTrace->type);
	aItem = aTrace->reports.head;
	while(aItem!=NULL) {
		aRep = (struct Report*)aItem->datap;
		dump_report(fp, aRep, aTrace->type);
		aItem = aItem->next;
	}
}


struct Trace* load_trace_with_hashtable(int magicNumber, FILE *ftrace, struct Region *aRegion, struct Hashtable *traces, time_t *startAt, time_t *endAt, struct Box *box)
{
	//Function: Load trace from a file in a struct Trace
	//Input: (1)MagicNuber, (2)a pointer to a trace file,(3)address of a region, (4)address of a hashtable to store traces, (5)start time, (6)end time, (7)a Box which can tell you the min and max of the square area
	//Significant variables: MagicNuber, a pointer to a trace file, address of a hashtable to store traces
	//Output: address of a Trace with the whole information of the given trace file


	struct Trace *aTrace = NULL;
	struct Report *newRep;// consider as a global struct variable
	char buf[256], *vName, *onRoute, *strp, *tm;
	struct Item *aItem;
	struct Cell *aCell;
	struct Road *aRoad;
	struct CandRoad *newCandRoad;
	int sameAngle = 1, first = 1, firstangle;
	struct Point aPoint;
	double gasVol, routeLeng;
	int speed, angle, state, errorInfo, msgType, onRoadId; 
	time_t timestamp;
	int needed;

	if(traces == NULL)
		return NULL;

	while(fgets(buf, 256, ftrace)) {
		if(magicNumber == FILE_ORIGINAL_GPS_TAXI) {//load a line of the data
			vName = strtok(buf, ",");
			tm = strtok(NULL, ",");
			strp = strtok(NULL, ",");
			aPoint.x = atof(strp);
			strp = strtok(NULL, ",");
			aPoint.y = atof(strp);
			strp = strtok(NULL, ",");
			speed = atoi(strp);
			strp = strtok(NULL, ",");
			angle = atoi(strp);
			strp = strtok(NULL, ",");
			state = atoi(strp);
			errorInfo = 0;
			msgType = 0;
			onRoadId = -1;

		} else if(magicNumber == FILE_MODIFIED_GPS_TAXI) {
			vName = strtok(buf, ",");
			tm = strtok(NULL, ",");
			strp = strtok(NULL, ",");
			aPoint.x = atof(strp);
			strp = strtok(NULL, ",");
			aPoint.y = atof(strp);
			strp = strtok(NULL, ",");
			speed = atoi(strp);
			strp = strtok(NULL, ",");
			angle = atoi(strp);
			strp = strtok(NULL, ",");
			state = atoi(strp);
			strp = strtok(NULL, ",");
			onRoadId = atoi(strp);
			errorInfo = 0;
			msgType = 0;

		} else if(magicNumber == FILE_ORIGINAL_GPS_BUS) {
			vName = strtok(buf, ",");
			tm = strtok(NULL, ",");
			strp = strtok(NULL, ",");
			aPoint.x = atof(strp);
			strp = strtok(NULL, ",");
			aPoint.y = atof(strp);
			strp = strtok(NULL, ",");
			speed = atoi(strp);
			strp = strtok(NULL, ",");
			angle = atoi(strp);
			onRoute = strtok(NULL, ",");
			strp = strtok(NULL, ",");
			msgType = atoi(strp);
			strp = strtok(NULL, ",");
			state = atoi(strp);
			strp = strtok(NULL, ",");
			errorInfo = atoi(strp);
			strp = strtok(NULL, ",");
			routeLeng = atof(strp);
			strp = strtok(NULL, ",");
			gasVol = atof(strp);
			onRoadId = -1;

		} else if(magicNumber == FILE_MODIFIED_GPS_BUS) {
			vName = strtok(buf, ",");
			tm = strtok(NULL, ",");
			strp = strtok(NULL, ",");
			aPoint.x = atof(strp);
			strp = strtok(NULL, ",");
			aPoint.y = atof(strp);
			strp = strtok(NULL, ",");
			speed = atoi(strp);
			strp = strtok(NULL, ",");
			angle = atoi(strp);
			onRoute = strtok(NULL, ",");
			strp = strtok(NULL, ",");
			msgType = atoi(strp);
			strp = strtok(NULL, ",");
			state = atoi(strp);
			strp = strtok(NULL, ",");		//apoint is a global variable
			errorInfo = atoi(strp);
			strp = strtok(NULL, ",");
			routeLeng = atof(strp);
			strp = strtok(NULL, ",");
			gasVol = atof(strp);
			strp = strtok(NULL, ",");
			onRoadId = atoi(strp);
		}
		timestamp = strtot(tm);

		if(aRegion == NULL || (aRegion && is_point_in_polygon(&aPoint, aRegion->chosen_polygon)))//weather the choosed point is in the given polygon
			needed = 1;
		else
			needed = 0;

		if(needed && (startAt==NULL || (startAt&&*startAt==0) || (startAt&&*startAt&&timestamp>=*startAt) ))//to choose the useful time(start)
			needed = 1;
		else
			needed = 0;

		if(needed && (endAt==NULL || (endAt&&*endAt==0) || (endAt&&*endAt&&timestamp<=*endAt) ))//to choose the useful time(end)
			needed = 1;
		else
			needed = 0;

		if(needed && (box==NULL || (box&&box->xmin==0) || (box&&box->xmin&&is_point_in_box(&aPoint, box)) ))//judge weather the point in the box or not
			needed = 1;
		else
			needed = 0;

		// If the record line satifie the above conditions, needed = 1, or needed = 0.

		if( needed ) {	
			newRep = (struct Report*)malloc(sizeof(struct Report));
			report_init_func(newRep); //initialize a report.
			newRep->timestamp = timestamp;// set the data loaded from the file to newRep
			newRep->gPoint.x = aPoint.x;
			newRep->gPoint.y = aPoint.y;
			newRep->speed = speed;
			newRep->angle = angle;
			newRep->msgType = msgType;
			newRep->state = state;
			newRep->errorInfo = errorInfo;
			newRep->routeLeng = routeLeng;
			newRep->gasVol = gasVol;
			newRep->onRoadId = onRoadId;
			if(first) {
				firstangle = newRep->angle;
				first --;
			} else if(firstangle != newRep->angle)
				sameAngle = 0;
				

			aItem = hashtable_find(traces, vName);//consider aItem as a global variable. Just use when you need it.
			if(aItem == NULL) {
				//printf("%s\n", vName);
				aTrace = (struct Trace*)malloc(sizeof(struct Trace));
				trace_init_func(aTrace);
				strncpy(aTrace->vName, vName, strlen(vName)+1);//copy the string in Vname to aTrace->vName
				aTrace->type = magicNumber;
				if(magicNumber == FILE_ORIGINAL_GPS_BUS || magicNumber == FILE_MODIFIED_GPS_BUS )
					strncpy(aTrace->onRoute, onRoute, strlen(onRoute)+1);
				
				hashtable_add(traces, vName, aTrace);

				aTrace->maxSpeed = newRep->speed;
				aTrace->startAt = newRep->timestamp;
				aTrace->endAt = newRep->timestamp;
				aTrace->box.xmin = newRep->gPoint.x;
				aTrace->box.xmax = newRep->gPoint.x;
				aTrace->box.ymin = newRep->gPoint.y;
				aTrace->box.ymax = newRep->gPoint.y;
			} else {
				aTrace = (struct Trace*)aItem->datap;
				if(newRep->speed > aTrace->maxSpeed)
					aTrace->maxSpeed = newRep->speed;
				if(newRep->timestamp < aTrace->startAt)
					aTrace->startAt = newRep->timestamp;
				if(newRep->timestamp > aTrace->endAt)
					aTrace->endAt = newRep->timestamp;
				if(newRep->gPoint.x < aTrace->box.xmin)
					aTrace->box.xmin = newRep->gPoint.x;
				if(newRep->gPoint.x > aTrace->box.xmax)
					aTrace->box.xmax = newRep->gPoint.x;
				if(newRep->gPoint.y < aTrace->box.ymin)
					aTrace->box.ymin = newRep->gPoint.y;
				if(newRep->gPoint.y > aTrace->box.ymax)
					aTrace->box.ymax = newRep->gPoint.y;
			}
			newRep->fromTrace = aTrace;
			duallist_add_in_sequence_from_tail(&aTrace->reports, newRep, (int(*)(void*,void*))report_has_earlier_timestamp_than);

			if(aRegion != NULL) {
				aCell = point_in_cell(aRegion, &newRep->gPoint);
				if(aCell != NULL) {
					if(newRep->onRoadId != -1) {
						aItem = aCell->roads.head;
						while(aItem != NULL) {
							aRoad = (struct Road*)aItem->datap;
							if(aRoad->id == newRep->onRoadId) {
								newCandRoad = (struct CandRoad*)malloc(sizeof(struct CandRoad));
								newCandRoad->aRoad = aRoad;
								newCandRoad->distance = distance_point_to_polyline(&(newRep->gPoint), &aRoad->points, &(newCandRoad->gPoint), &(newCandRoad->onSegment));
								duallist_add_to_tail(&newRep->candRoads, newCandRoad);
								newRep->onRoad = newRep->candRoads.head;
								break;
							}
							aItem = aItem->next;
						}
					}
					duallist_add_to_tail(&aCell->reps, newRep);
				}
			}
		}

	}

	if(aTrace) {
		if( sameAngle == 1)
			aTrace->isHeadingValid = 0;
		else
			aTrace->isHeadingValid = 1;

		aTrace->at = aTrace->reports.head;

		*startAt = aTrace->startAt;
		*endAt = aTrace->endAt;
		box->xmin = aTrace->box.xmin;
		box->xmax = aTrace->box.xmax;
		box->ymin = aTrace->box.ymin;
		box->ymax = aTrace->box.ymax;
	}
	
	return aTrace;
}


/*
 * Do the trick to remove dull reports when the vehicle moves
 * under viaducts or in tunnels, and those outliers. Return the 
 * total number of removed reports.
 */
int remove_dull_reports(struct Trace *aTrace)
{ 

	struct Item *aItem, *tempItem;
	struct Report *aRep, *lastRep, *nextRep; 
	int count = 0;
	int maxSpeed;

	if(aTrace->type == FILE_ORIGINAL_GPS_TAXI || aTrace->type == FILE_MODIFIED_GPS_TAXI)
		maxSpeed = TAXI_MAX_SPEED;
	else if(aTrace->type == FILE_ORIGINAL_GPS_BUS || aTrace->type == FILE_MODIFIED_GPS_BUS)
		maxSpeed = BUS_MAX_SPEED;
	aItem = aTrace->reports.head->prev;
	while(aItem != aTrace->reports.head) {
		aRep = (struct Report*)aItem->datap;
		lastRep = (struct Report*)aItem->prev->datap;
		if(aItem->next != NULL) 
			nextRep = (struct Report*)aItem->next->datap;
		else
			nextRep = NULL;

		/* remove dull and duplicated reports */
		if( (aRep->angle == lastRep->angle && 
	  	   equald(aRep->gPoint.x, lastRep->gPoint.x, DELTA) &&
		   equald(aRep->gPoint.y, lastRep->gPoint.y, DELTA) &&
		   aRep->speed != 0 &&
		   aRep->timestamp != lastRep->timestamp) || are_two_reports_duplicated(lastRep, aRep)) {
//			printf("******* dull or duplicated reports *********\n");
			tempItem = aItem->prev;
			report_free_func((struct Report*)duallist_pick_item(&aTrace->reports, aItem));
			count ++;
			aItem = tempItem;
		} else if (nextRep != NULL && distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, lastRep->gPoint.x, lastRep->gPoint.y) + distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, nextRep->gPoint.x, nextRep->gPoint.y) > (nextRep->timestamp - lastRep->timestamp)*maxSpeed/3.6 && distance_in_meter(nextRep->gPoint.x, nextRep->gPoint.y, lastRep->gPoint.x, lastRep->gPoint.y) < (nextRep->timestamp - lastRep->timestamp)*maxSpeed/3.6 ) {
//			printf("******* Outlier reports *********\n");
			tempItem = aItem->prev;
			report_free_func((struct Report*)duallist_pick_item(&aTrace->reports, aItem));
			count ++;
			aItem = tempItem;
		} else if(aRep->timestamp==lastRep->timestamp && distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, lastRep->gPoint.x, lastRep->gPoint.y)>0) {
//			printf("******* Outlier reports *********\n");
			tempItem = aItem->prev;
			report_free_func((struct Report*)duallist_pick_item(&aTrace->reports, aItem));
			count ++;
			aItem = tempItem;

		} else
			aItem = aItem->prev;

	} 
	return count;
}


void trace_init_func(struct Trace *aTrace)
{
	//Function: to initial a trace
	//Input: address of a Struct Trace
	//Output: None. 
	int randint;

	if(aTrace == NULL)
		return;
	memset(aTrace->vName, '\0', NAME_LENGTH);
	memset(aTrace->onRoute, '\0', NAME_LENGTH);
	randint = rand();
	aTrace->color.integer = randint;
	duallist_init(&aTrace->reports);
}

void trace_free_func(struct Trace *aTrace)
{
	if(aTrace == NULL) 
		return;
	duallist_destroy(&aTrace->reports, (void(*)(void*))report_free_func);
	free(aTrace);
}


void report_init_func(struct Report *aRep)
{
	//Function: 
	if(aRep == NULL)
		return;
	aRep->shown = 0;
	duallist_init(&aRep->candRoads);
	aRep->onRoad = NULL;
	aRep->onPath = NULL;
	aRep->state = 0;
	aRep->msgType = 0;
}


void report_free_func(struct Report *aRep)
{
	if(aRep == NULL) 
		return;
	duallist_destroy(&aRep->candRoads, free);
	if(aRep->onPath != NULL)
		path_free_func(aRep->onPath);
	free(aRep);
}

int is_report_in_upway(struct Report *aRep)
{
	if(aRep && (aRep->state & 0x10)==0)
		return 1;
	return 0;
}

int is_trace_mixed(struct Trace *aTrace)
{
	long errorCount;
	struct Duallist subTraces;
	struct Item *aItem, *bItem;
	struct Trace *newTrace, *curTrace, *bTrace, *candTrace;
	struct Report *aRep, *bRep;
	double dist, angle, maxDist, maxAngle, minValue;
	int maxSpeed;

	if(aTrace == NULL) 
		return -1;

	if(aTrace->type == FILE_ORIGINAL_GPS_TAXI || aTrace->type == FILE_MODIFIED_GPS_TAXI)
		maxSpeed = TAXI_MAX_SPEED;
	else if(aTrace->type == FILE_ORIGINAL_GPS_BUS || aTrace->type == FILE_MODIFIED_GPS_BUS)
		maxSpeed = BUS_MAX_SPEED;
	errorCount = 0;
	aItem = aTrace->reports.head;
	duallist_init(&subTraces);
	while(aItem != NULL) {
		aRep = (struct Report*)aItem->datap;
		if(is_duallist_empty(&subTraces)) {
			newTrace = (struct Trace*)malloc(sizeof(struct Trace));
			curTrace = newTrace;
			duallist_init(&newTrace->reports);
			duallist_add_to_tail(&newTrace->reports, aItem->datap);
			duallist_add_to_tail(&subTraces, newTrace);
		} else {
			bItem = subTraces.head;
			maxDist = maxAngle = -1;
			while(bItem!=NULL) {
				bTrace = (struct Trace*)bItem->datap;
				bTrace->var1 = -1;
				bRep = (struct Report*)bTrace->reports.head->prev->datap;
				dist = distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, bRep->gPoint.x, bRep->gPoint.y);
				if(dist <= maxSpeed*(aRep->timestamp-bRep->timestamp)/3.6 && aRep->timestamp - bRep->timestamp < 60*30) {
					/* Trace.var1 field is used to store distance value*/
					bTrace->var1 = dist;
					/* Trace.var2 field is used to store angle value */
					angle =angle_between(aRep->gPoint.x, aRep->gPoint.y, bRep->gPoint.x, bRep->gPoint.y);
					bTrace->var2 = inter_angle(angle, aRep->angle);
					if (bTrace->var1 > maxDist)
						maxDist = bTrace->var1;
					if (bTrace->var2 > maxAngle)
						maxAngle = bTrace->var2;
				}
				bItem = bItem->next;
			}
			if(maxDist == -1) {	
				newTrace = (struct Trace*)malloc(sizeof(struct Trace));
				curTrace = newTrace;
				duallist_init(&newTrace->reports);
				duallist_add_to_tail(&newTrace->reports, aItem->datap);
				duallist_add_to_tail(&subTraces, newTrace);
				errorCount ++;
			} else {
				minValue = 3;
				bItem = subTraces.head;
				while(bItem!=NULL) {
					bTrace = (struct Trace*)bItem->datap;
					bTrace->var3 = 3;
					if(bTrace->var1 != -1) {
						bTrace->var3 = 0;
						if(maxDist> 0)
							bTrace->var3 += bTrace->var1/maxDist;
						if(maxAngle>0)
							bTrace->var3 += bTrace->var2/maxAngle;
						if(bTrace->var3 < minValue) {
							minValue = bTrace->var3;
							candTrace = bTrace;
						}
					}
					bItem = bItem->next;
				}
				if (candTrace == curTrace) {
					duallist_add_to_tail(&curTrace->reports, aItem->datap);
				} else {
					curTrace = candTrace;
					duallist_add_to_tail(&curTrace->reports, aItem->datap);
					errorCount ++;
				}	
			}
		}
		aItem = aItem->next;
	}

	while(!is_duallist_empty(&subTraces)) {
		bTrace = (struct Trace*)duallist_pick_head(&subTraces);
		duallist_destroy(&bTrace->reports, NULL);
		free(bTrace);
	}
	if (errorCount > MIXED_TRACE_THRESHOLD) 
		return 1;
	else
		return 0;
}


void set_trace_table_time(struct Hashtable *traces, time_t atClock)
{
	int first;
	struct Item *aItem;
	struct Item *bItem;
	struct Report *aReport;
	struct Trace *aTrace;
	unsigned long i;

	for(i = 0; i<traces->size; i++) {
		aItem = traces->head[i];
		while (aItem!=NULL)
		{
			first = 1;
			aTrace = (struct Trace*)aItem->datap;
			aTrace->countdown = -1;
			bItem = aTrace->reports.head;
			while(bItem != NULL) {
				aReport = (struct Report*)bItem->datap;
				aReport->shown = 0;
				if(first && aReport->timestamp >= atClock) {
					aTrace->at = bItem;
					aTrace->countdown = difftime(aReport->timestamp, atClock);
					first --;
				}
				bItem = bItem->next;
			}
			aItem = aItem->next;
		}
	}
}

void set_selected_traces_time(struct Duallist *selectedTraces, time_t atClock)
{
	struct Item *aItem;
	struct Item *bItem;
	struct Trace *aSelectedTrace;
	struct Report *aReport;

	aItem = selectedTraces->head;
	while(aItem!=NULL) {
		aSelectedTrace = (struct Trace*)aItem->datap;
		aSelectedTrace->countdown = -1;
		bItem = aSelectedTrace->reports.head;
		while(bItem != NULL) {
			aReport = (struct Report*)bItem->datap;
			aReport->shown = 1;
			if(aReport->timestamp >= atClock) {
				aSelectedTrace->at = bItem;
				aSelectedTrace->countdown = difftime(aReport->timestamp, atClock);
				break;
			}
			bItem = bItem->next;
		}
		while(bItem != NULL) {
			aReport = (struct Report*)bItem->datap;
			aReport->shown = 0;
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
}

/* the reports here must have onRoad set up */
struct Path *get_path_between_two_reports(struct Region *aRegion, struct Report *aRep, struct Report *bRep, double lengthreshold)
{
	struct Road *aRoad, *bRoad;
	struct Path *aPath;

	if(aRegion == NULL || aRep == NULL || bRep == NULL || aRep->onRoad == NULL || bRep->onRoad == NULL) {
		return NULL;
	}

	aRoad = ((struct CandRoad*)aRep->onRoad->datap)->aRoad;
	bRoad = ((struct CandRoad*)bRep->onRoad->datap)->aRoad;

	aPath = NULL;
	if(lengthreshold == -1 || (lengthreshold!=-1&&distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, bRep->gPoint.x, bRep->gPoint.y) < lengthreshold)) {
		if(aRoad == bRoad) {
			aPath = (struct Path*)malloc(sizeof(struct Path));
			duallist_init(&aPath->roads);
			duallist_add_to_tail(&aPath->roads, aRoad);
		} else if(bRoad->headEnd != aRoad->tailEnd) {
			aPath = find_shortest_path(aRegion, aRoad->tailEnd, bRoad->headEnd);
			if (aPath != NULL) {
				duallist_add_to_head(&aPath->roads, aRoad);
				duallist_add_to_tail(&aPath->roads, bRoad);
			}

		} else {
			aPath = (struct Path*)malloc(sizeof(struct Path));
			duallist_init(&aPath->roads);
			duallist_add_to_head(&aPath->roads, aRoad);
			duallist_add_to_tail(&aPath->roads, bRoad);
		}
	}
	return aPath;
}

struct Trace* insert_reports(struct Region *aRegion, struct Trace *aTrace, int insertMode, int outputMode, int interval)
{
	struct Item *aItem, *nextItem, *bItem;
	struct Report *aRep, *nextRep, *bRep;
	struct Trace *rtTrace;
	struct Path *aPath;
	struct Duallist *aList;
	struct Cell *lastCell, *aCell;
	time_t clock;

	if(aRegion==NULL || aTrace == NULL)
		return NULL;

	printf("Interpolating Trace: %s ...\n", aTrace->vName);

	rtTrace = (struct Trace*) malloc(sizeof(struct Trace));
	sprintf(rtTrace->vName, "%si", aTrace->vName);
	strncpy(rtTrace->onRoute, aTrace->onRoute, strlen(aTrace->onRoute)+1);
	rtTrace->color.integer = aTrace->color.integer + 40;
	rtTrace->type = aTrace->type;
	duallist_init(&rtTrace->reports);

	lastCell = NULL;
	aItem = aTrace->reports.head;
	while(aItem) {
		while(aItem!=NULL && (!is_point_in_polygon(&((struct Report*)aItem->datap)->gPoint, aRegion->chosen_polygon) || ((struct Report*)aItem->datap)->onRoad == NULL) ) {
			aItem = aItem->next;
		}
		if(aItem && is_point_in_polygon(&((struct Report*)aItem->datap)->gPoint, aRegion->chosen_polygon)) {
			nextItem = aItem->next;
			while(nextItem!=NULL && is_point_in_polygon(&((struct Report*)nextItem->datap)->gPoint, aRegion->chosen_polygon) && ((struct Report*)nextItem->datap)->onRoad == NULL) {
				nextItem = nextItem->next;
			}
			if(nextItem && is_point_in_polygon(&((struct Report*)nextItem->datap)->gPoint, aRegion->chosen_polygon) ) {
				aRep = (struct Report*)aItem->datap;
				nextRep = (struct Report*)nextItem->datap;
				aPath = get_path_between_two_reports(aRegion, aRep, nextRep, -1);
				aList = insert_reports_in_path(aPath, aRep, nextRep, insertMode);
				if(aList) {
					if(outputMode == OUTPUT_MODE_CELL) {
						bItem = aList->head;
						while(bItem) {
							bRep = (struct Report*)bItem->datap;
							aCell = point_in_cell(aRegion, &bRep->gPoint);
							if(lastCell == NULL || (lastCell&&lastCell!=aCell)) {
								lastCell = aCell;
								bRep->fromTrace = rtTrace;
								duallist_add_to_tail(&rtTrace->reports, bRep);
							} else {
								report_free_func(bRep);
							}
								
							bItem = bItem->next;
						}
						
					} else if(outputMode == OUTPUT_MODE_INTERVAL) {
						bItem = aList->head;
						bRep = (struct Report*)bItem->datap;
						clock = bRep->timestamp;
						while(bRep->timestamp < nextRep->timestamp) {
							if(bRep->timestamp == clock) {
								bRep->fromTrace = rtTrace;
								duallist_add_to_tail(&rtTrace->reports, bRep);
								clock += interval;
							} else {
								report_free_func(bRep);
							}
							bItem = bItem->next;
							bRep = (struct Report*)bItem->datap;
						}
					}
					duallist_destroy(aList, NULL);
					free(aList);
				}

			}
			aItem = aItem->next;
		}
	}

	return rtTrace;
}


/* inserted reports are not associated with roads */
struct Duallist *insert_reports_in_path(struct Path *aPath, struct Report *aRep, struct Report* nextRep, int insertMode)
{
	struct Duallist* rtList;
	struct Duallist *polyline;
	double length, pace;
	struct CandRoad *newCandRoad;
	time_t period, clock;
	char buf[32];
	struct Report *newRep;
	struct Item *aItem;
	struct Point *aPoint, *bPoint;
	int angle;
	double d, walked, walking;
	
	if(!aPath || !aRep || !nextRep)
		return NULL;
	polyline = polyline_on_path(aPath, &aRep->gPoint, &nextRep->gPoint);
	length = polyline_length(polyline);
	period = nextRep->timestamp - aRep->timestamp;

	if(insertMode == INSERT_MODE_AVGSPEED) {
		if(period)
			pace = length*MINIMUM_TIME_UNIT/period;
		else if(length) {
			ttostr(aRep->timestamp, buf);	
			printf("GPS reports at different locations with the same timestamp found.\n Trace:%s, Rep timestamp:%s\n", aRep->fromTrace->vName, buf);
			duallist_destroy(polyline, (void(*)(void*))point_free_func);
			free(polyline);
			return NULL;
		} else 
			pace = 0;

		rtList = (struct Duallist*)malloc(sizeof(struct Duallist));
		duallist_init(rtList);

		newRep = (struct Report*)malloc(sizeof(struct Report));
		newRep->gPoint.x = aRep->gPoint.x;
		newRep->gPoint.y = aRep->gPoint.y;
		newRep->timestamp = aRep->timestamp;
		newRep->speed = aRep->speed;
		newRep->angle = aRep->angle;
		newRep->state = aRep->state;
		newRep->msgType = aRep->msgType;
		newRep->routeLeng = aRep->routeLeng;
		newRep->gasVol = aRep->gasVol;
		newRep->errorInfo = aRep->errorInfo;
		duallist_init(&newRep->candRoads);
		newCandRoad = (struct CandRoad*)malloc(sizeof(struct CandRoad));
		newCandRoad->aRoad = ((struct CandRoad*)aRep->onRoad->datap)->aRoad;
		copy_segment(&newCandRoad->onSegment, &((struct CandRoad*)aRep->onRoad->datap)->onSegment);
		duallist_add_to_head(&newRep->candRoads, newCandRoad);
		newRep->onRoad = newRep->candRoads.head;
		newRep->onRoadId = newCandRoad->aRoad->id;
		newRep->onPath = NULL;
		duallist_add_to_tail(rtList, newRep);
		
		aItem = polyline->head;
		walked = 0;
		clock = aRep->timestamp;
		while(aItem && aItem->next) {
			aPoint = (struct Point*)aItem->datap;
			bPoint = (struct Point*)aItem->next->datap;
			d = distance_in_meter(aPoint->x, aPoint->y, bPoint->x, bPoint->y);
			angle = angle_between(aPoint->x, aPoint->y, bPoint->x, bPoint->y);
			walking = d;
			while(walked + walking >= pace) {
				walking = walked + walking - pace;
				walked = 0;
				clock += MINIMUM_TIME_UNIT;

				newRep = (struct Report*)malloc(sizeof(struct Report));
				newRep->gPoint.x = (walking*aPoint->x + (d-walking)*bPoint->x)/d;
				newRep->gPoint.y = (walking*aPoint->y + (d-walking)*bPoint->y)/d;
				newRep->timestamp = clock;
				newRep->speed = length/period;
				newRep->angle = angle;
				newRep->state = 0;
				if(aRep->fromTrace->type == FILE_MODIFIED_GPS_BUS) 
					newRep->state = aRep->state;
				newRep->msgType = 0;
				newRep->routeLeng = 0;
				newRep->gasVol = 0;
				newRep->errorInfo = 0;
				duallist_init(&newRep->candRoads);
				newRep->onRoad = NULL;
				newRep->onRoadId = -1;
				newRep->onPath = NULL;
				duallist_add_to_tail(rtList, newRep);
			}
			walked += walking;
			aItem = aItem->next;
		}

		newRep = (struct Report*)malloc(sizeof(struct Report));
		newRep->gPoint.x = nextRep->gPoint.x;
		newRep->gPoint.y = nextRep->gPoint.y;
		newRep->timestamp = nextRep->timestamp;
		newRep->speed = nextRep->speed;
		newRep->angle = nextRep->angle;
		newRep->state = nextRep->state;
		newRep->msgType = nextRep->msgType;
		newRep->routeLeng = nextRep->routeLeng;
		newRep->gasVol = nextRep->gasVol;
		newRep->errorInfo = nextRep->errorInfo;
		duallist_init(&newRep->candRoads);
		newCandRoad = (struct CandRoad*)malloc(sizeof(struct CandRoad));
		newCandRoad->aRoad = ((struct CandRoad*)nextRep->onRoad->datap)->aRoad;
		copy_segment(&newCandRoad->onSegment, &((struct CandRoad*)nextRep->onRoad->datap)->onSegment);
		duallist_add_to_head(&newRep->candRoads, newCandRoad);
		newRep->onRoad = newRep->candRoads.head;
		newRep->onRoadId = newCandRoad->aRoad->id;
		newRep->onPath = NULL;
		duallist_add_to_tail(rtList, newRep);
		

	} else if(insertMode == INSERT_MODE_TRAFFIC) {
		// to be implemented
	}
	
	duallist_destroy(polyline, (void(*)(void*))point_free_func);
	free(polyline);
	return rtList;
}

