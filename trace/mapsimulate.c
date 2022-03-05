#include "mapsimulate.h"
#include "common.h"
#include "geometry.h"

/* used to find vehicles on the neighbor lanes when changing lane */
struct Item *lfItem, *llItem, *rfItem, *rlItem;

void init_multilane(struct Region *region)
{
    printf("Initialize multilanes...\n");

    struct Item *roadItem, *laneItem, *tempItem, *pointItem;
    struct Road *road, *nextRoad, *currentRoad;
    struct Lane *lane;
    struct crossLane *aCrossLane;
    struct Cross *currentCross;
    struct Point *point;
    double dist, shift;

    roadItem = region->roads.head;
    while(roadItem != NULL){
        road = (struct Road*)roadItem->datap;
        laneItem = road->lanes.head;
	dist = distance_in_latitude(LANE_WIDTH*road->lanes.nItems*2);

        for(int i =0; i < road->lanes.nItems; i++) {
            //initialize lane
            lane = (struct Lane*)laneItem->datap;
            lane->onRoad = road;
            duallist_init(&lane->crossLanes);
            duallist_init(&lane->vehicles);
            
	    shift = road->width / (road->lanes.nItems * 2);
	    shift = road->width/2 - shift * (i*2+1);		
	    shift = distance_in_latitude(shift); 

            //initialize crosslane
            currentCross = (struct Cross*)road->tailEnd;
            tempItem = currentCross->outRoads.head;

            while (tempItem != NULL && (struct Road*)tempItem->datap != road){
            nextRoad = (struct Road*)tempItem->datap;
            aCrossLane = (struct crossLane*)malloc(sizeof(struct crossLane));  
            aCrossLane->fromLane = lane;
            aCrossLane->toRoad = nextRoad;

	    pointItem = road->points.head;
	    pointItem = pointItem->prev;
	    point = (struct Point*)pointItem->datap;
	    aCrossLane->fromPoint.x = point->x - dist * cos(M_PI*road->tailEndAngle/180) - shift * sin(M_PI*road->tailEndAngle/180);
	    aCrossLane->fromPoint.y = point->y - dist * sin(M_PI*road->tailEndAngle/180) + shift * cos(M_PI*road->tailEndAngle/180);

	    pointItem = nextRoad->points.head;
	    point = (struct Point*)pointItem->datap;
	    aCrossLane->toPoint.x = point->x - dist * cos(M_PI*(nextRoad->headEndAngle+180)/180) - shift * sin(M_PI*nextRoad->headEndAngle/180);
	    aCrossLane->toPoint.y = point->y - dist * sin(M_PI*(nextRoad->headEndAngle+180)/180) + shift * cos(M_PI*nextRoad->headEndAngle/180);			
			
	    aCrossLane->length = distance_in_meter(aCrossLane->fromPoint.x, aCrossLane->fromPoint.y, aCrossLane->toPoint.x, aCrossLane->toPoint.y);

            duallist_init(&aCrossLane->vehicles);
            duallist_add_to_head(&lane->crossLanes, aCrossLane);
            tempItem = tempItem->next;  //next outRoad of this cross
            }

            laneItem = laneItem->next; //next lane
        }
        roadItem = roadItem->next; //next road
    }

    printf("Multilane initialization finished!\n");
}


void generate_car(struct Region *region, int carRcdNum)
{
        printf("generating car route information...\n");
  	FILE *carfin;
  	int sCrossNum, dCrossNum, carId, isCrossFind, i=0;
  	double carSpeed;
  	struct Cross *sCross, *dCross;
        struct Item *crossItem;

                carfin = fopen("carPathRcd.txt","w");
  		fprintf(carfin, "%d\n", carRcdNum);
  		i = 0;

  		while (i != carRcdNum) {
			i += 1;
			/* random select a start cross */
			sCrossNum = rand() % region->crosses.nItems + 1;
			crossItem = region->crosses.head;
			while (sCrossNum != 1) {
				crossItem = crossItem->next;
				sCrossNum -= 1;
			} 
			sCross = (struct Cross*)crossItem->datap;
			sCrossNum = sCross->number;
			/* random select a start cross */	

			/* random select an end cross */
			dCrossNum = rand() % region->crosses.nItems + 1;
			crossItem = region->crosses.head;
			while (dCrossNum != 1) {
				crossItem = crossItem->next;
				dCrossNum -= 1;
			} 
			dCross = (struct Cross*)crossItem->datap;
			dCrossNum = dCross->number;
			/* random select an end cross */

			//carSpeed = (double)((rand()%(V_MAX*10)+1)/10);	
			carSpeed = 0;
			fprintf(carfin, "%d %lf %d %d\n",
					i,
					carSpeed,
					sCrossNum,
					dCrossNum);
  		} //while (i != carRcdNum)
  		fclose(carfin); 
                printf("Car information generating success!\n\n");
}

void load_car(struct Region *region)
{
        printf("loading car information\n");
       
        struct Item *crossItem, *roadItem;
        struct Cross *currentCross, *sCross, *dCross;
        struct Road *currentRoad;
        Vehicle *newCar;
	FILE *fout, *carfin;
  	char *cNumOfCar = (char*)malloc(6*sizeof(char)); 
  	char *ptr;
  	int iNumOfCar = 100;
        int carRcdNum, sCrossNum, dCrossNum, carId, isCrossFind;
        double carSpeed;

 	// readin the total num of car info items
        carfin = fopen("carPathRcd.txt","r");
 	fscanf(carfin, "%d", &carRcdNum);
 	int i = carRcdNum;
	srand(time(0));
	while (i != 0) {
		i -= 1;
		//printf("%d\n", i);
		// readin an item of car info
		fscanf(carfin, "%d", &carId);
		fscanf(carfin, "%lf", &carSpeed);
		fscanf(carfin, "%d", &sCrossNum);
		fscanf(carfin, "%d", &dCrossNum);
	
		// find the pointer of sCross and dCross
		crossItem = (struct Item*)region->crosses.head;
		isCrossFind = 0;
		while (isCrossFind != 2 && crossItem != NULL) {
			currentCross = (struct Cross*)crossItem->datap;
			if (currentCross->number == sCrossNum) {
				sCross = currentCross;
				isCrossFind += 1;
			}
			if (currentCross->number == dCrossNum) {
				dCross = currentCross;
				isCrossFind += 1;
			}
			crossItem = crossItem->next;
		} //while (isCrossFind != 2 && crossItem != NULL)
	
		// create the new car
		newCar = (Vehicle*)malloc(sizeof(Vehicle));
		newCar->id = carId;
		newCar->position = CAR_LENGTH;
		newCar->vmax = (double)V_MAX;
		newCar->v = carSpeed;

		/* as to the acceleration and deceleration, we can use the default or random generate them */
		//newCar->a = ACCELERATION;
		//newCar->b = DECELERATION;
		newCar->a = (rand() % 3) + 1;
		newCar->b = (rand() % 6) + 1;
	
		/* search the route of the vehicle */
		newCar->pathInfo = find_shortest_path(region, sCross, dCross);
		
		// can't find a path from sCross to dCross
		if (newCar->pathInfo == NULL) {
			printf("carID:%d\n",newCar->id);
			free(newCar);
			continue;
		}		
		
		/* add this car to the watting list of start road */
		roadItem = (struct Item*)newCar->pathInfo->roads.head;
		currentRoad = (struct Road*)roadItem->datap;
		newCar->currentRoadItem = roadItem;

		duallist_add_to_head(&currentRoad->waittingV, newCar);
		

  	} //while (i != 0)
  	fclose(carfin);
        printf("Information loading success!\n\n");
}

void start_simulate(struct Region* region, struct Hashtable* Traces, int count)
{
	struct Item *blockItem, *fItem, *lItem, *tempItem, *roadItem, *laneItem, *tempItem1;
	Vehicle *fCar, *lCar, *newCar;
	struct Road *currentRoad;
	struct Cross *currentCross;
	struct Lane *currentLane, *nextLane;
	struct Speed *addSpeed;
	double vnext,  distance;
	char c;
	unsigned long index; 
	double currentSpeed,  shift, vsafe;
	int i = 0 , j, lane_num;

        /******** handle block operation ********/
	//update_block(region, tempTraces, count);
 	/******** handle block operation ********/

	/******** update the state of the traffic light ********/
	update_traffic_light(region, count);                      //need to be completed
	/******** update the state of the traffic light ********/

	
	/******** calculate the position ********/
	roadItem = (struct Item*)region->roads.head;
  	while (roadItem != NULL) {
		currentRoad = (struct Road*)roadItem->datap;
		currentCross = currentRoad->tailEnd;
		laneItem = (struct Item*)currentRoad->lanes.head;
                
		/* Generate traffic flow */
		/* pick a vehicle from the watting list to the lane */	
                while(laneItem != NULL){
                      currentLane = (struct Lane*)laneItem->datap;
                      if (currentRoad->waittingV.head != NULL) {
			  if (is_lane_available(&currentLane->vehicles, currentRoad->lanes.nItems)) {
				newCar = (Vehicle*)duallist_pick_head(&currentRoad->waittingV);
				newCar->belongLane = currentLane;
                                newCar->handled = 0;
				duallist_add_to_head(&currentLane->vehicles, newCar);
			  }
                       } 
                       laneItem = laneItem->next;
		} // while(laneItem != NULL)
		/* Generate traffic flow */
   

		/* search if there are vehicles can change lane */
                tempItem = (struct Item*)currentRoad->lanes.head;
                laneItem = tempItem;
		while(tempItem != NULL) {
                        laneItem = tempItem;
                        tempItem = tempItem->next;
			currentLane = (struct Lane*)laneItem->datap;
			fItem = currentLane->vehicles.head;
			if (fItem == NULL) continue;
			if (currentLane->vehicles.nItems <= 1) continue;
			fItem = fItem->prev->prev; printf("success-1.03\n");
			while (fItem->next != NULL) {
				fCar = (Vehicle*)fItem->datap;
				/* if current car is a block or it has been updated, move to the next */
				if (fCar->a == 0 || fCar->handled == 1) {fItem = fItem->prev; continue; }

				fCar->handled = 1;
				lCar = (Vehicle*)fItem->next->datap;
				vsafe = get_safe_velocity(fCar, lCar);
                                printf("successs-1.1\n");
				if (vsafe < fCar->v + fCar->a*DET_TIME) {

					/* the speed is not satisfied, try to change lane */

					/* find the vehicles on the neighbour lanes */
					find_vehicle_on_lane(currentRoad, fItem, laneItem);

					if (is_change_possibile(currentRoad, laneItem, fCar, lfItem, llItem, vsafe, 0) == 1) {

						/* vehicle can change to left lane */
						fItem = change_lane(currentRoad, laneItem, fItem, lfItem, llItem, 0);
						continue;
					} else if (is_change_possibile(currentRoad, laneItem, fCar, rfItem, rlItem, vsafe, 1) == 1) {

						/* vehicle can change to right lane */
						fItem = change_lane(currentRoad, laneItem, fItem, rfItem, rlItem, 1);
						continue;
					}
				}
				fItem = fItem->prev;	
			} //while (fItem->next != NULL)
		} // while(laneItem != NULL)	
		/* search if there are vehicles can change lane */


		/* update all vehicles on the lane */
                tempItem = (struct Item*)currentRoad->lanes.head;
                laneItem = tempItem;
		while(tempItem != NULL) {
                        laneItem = tempItem;
                        tempItem = tempItem->next;
			currentLane = (struct Lane*)laneItem->datap;
			
			/* Move forward */
			fItem = currentLane->vehicles.head;
			currentSpeed = 0;
			
			while (fItem != NULL && fItem->next != NULL) {
				fCar = (Vehicle*)fItem->datap;
				if (fCar->a == 0) {fItem = fItem->next; continue;}
				lCar = (Vehicle*)fItem->next->datap;
				update_vehicle_single_lane(fCar, lCar);
				fCar->position += fCar->vnext * DET_TIME;
				fCar->v = fCar->vnext;
				currentSpeed += fCar->v;
				fItem = fItem->next;
			}
			
			/*currentSpeed = currentSpeed / currentLane->vehicles.nItems;
			addSpeed = (struct Speed*)malloc(sizeof(struct Speed));
			addSpeed->aSpeed = currentSpeed;
			sem_wait(roads_lock+currentRoad->id);
			duallist_add_to_tail(&currentLane->avgSpeed, addSpeed);
			sem_post(roads_lock+currentRoad->id); */
			/* Move forward */
			

			/* update first car */
			if (fItem != NULL) 
				update_first_car(Traces, currentRoad, fItem);
			/* update first car */
			



			/**** Handle the vehicles on the cross from current lane ****/
			tempItem1 = currentLane->crossLanes.head;
			while (tempItem1 != NULL) {
				//printf("handle the cars on the cross\n");
				handle_crossLane((struct crossLane*)tempItem1->datap);
				tempItem1 = tempItem1->next;
			}	
			/**** Handle the vehicles on the cross from current lane ****/



		}	
		/* update all vehicles on the lane */


		roadItem = roadItem->next;
  	} // while(roadItem != NULL)

	/* Calculate the coordinate of the car */
	roadItem = (struct Item*)region->roads.head;
	
  	while(roadItem != NULL) {
		currentRoad = (struct Road*)roadItem->datap;
                laneItem = (struct Item*)currentRoad->lanes.head;
                lane_num = currentRoad->lanes.nItems;

		for(i = 0; i<lane_num; i++) {
                        currentLane = (struct Lane*)laneItem->datap;
                      
			shift = currentRoad->width / (currentRoad->lanes.nItems + 1);
			shift = currentRoad->width/2 - shift * (i+1);
			shift = distance_in_latitude(shift);
			
			/* Coordinate on the lane */
			calculate_coordinate(currentRoad, currentLane, shift,Traces, count);
			/* Coordinate on the lane */


			/* Coordinate in the cross */
			tempItem = currentLane->crossLanes.head;
			while (tempItem != NULL) {
				//printf("coordinate in the cross\n");
				coordinate_crossLane((struct crossLane*)tempItem->datap, Traces, count);
				tempItem = tempItem->next;
				//printf("finish coordinate in the cross\n");
			}	
			/* Coordinate in the cross */
                        laneItem = laneItem->next;
		}
		roadItem = roadItem->next;
	} //while(roadItem != NULL)
	/* Calculate the coordinate of the car */
}

void update_traffic_light(struct Region *region, int count)
{
   struct Item *roadItem;
   struct Road *currentRoad;
   roadItem = region->roads.head;
   while(roadItem != NULL){
       currentRoad = (struct Road*)roadItem->datap;
       for (int i = 0; i<2; i++){
           currentRoad->lights[i].timer--;
           if (currentRoad->lights[i].timer == 0){
              switch(currentRoad->lights[i].state)
              {
                  case 0: currentRoad->lights[i].timer = currentRoad->lights[i].duration[1]; currentRoad->lights[i].state = 1; break;
                  case 1: currentRoad->lights[i].timer = currentRoad->lights[i].duration[2]; currentRoad->lights[i].state = 2; break;
                  case 2: currentRoad->lights[i].timer = currentRoad->lights[i].duration[0]; currentRoad->lights[i].state = 0; break;
              }
           }
       } //for
      
       roadItem = roadItem->next;
   } //while
}

int is_lane_available(struct Duallist* currentLane,int laneNum)
{
	struct Item *aItem;
	Vehicle *aCar;

	aItem = (struct Item*)currentLane->head;
	if (aItem == NULL) return 1;
	
	aCar = (Vehicle*)aItem->datap;
	if (aCar->position > (double)(laneNum*LANE_WIDTH*2+5))
		return 1;
	else 
		return 0;

}

double get_safe_velocity(Vehicle *fCar, Vehicle *lCar)
{
	return lCar->v + (lCar->position-fCar->position - CAR_LENGTH -SAFE_CAR_GAP - lCar->v) / ((lCar->v + fCar->v) / (2 * fCar->b) + DET_TIME);

}

void find_vehicle_on_lane(struct Road *currentRoad, struct Item *fItem, struct Item *laneItem)
{
	Vehicle *fCar = (Vehicle*)fItem->datap;
	Vehicle *aCar;
	struct Lane *currentLane = fCar->belongLane;
	int lfFound = 0, llFound = 0, rfFound = 0, rlFound = 0;

		if (laneItem->prev->next != NULL) {
			// the left side of current lane 

			// first find the prev car 
			lfItem = ((struct Lane*)laneItem->prev->datap)->vehicles.head;
			if (lfItem == NULL) {lfFound = 0; llFound = 0;}  
                        else {
			aCar = (Vehicle*)lfItem->datap;
			if (aCar->position < fCar->position) {
				while (lfItem->next != NULL && ((Vehicle*)(lfItem->next->datap))->position < fCar->position) {
					lfItem = lfItem->next;
				}
				lfFound = 1;
			}
                        
                        //find the next car
			llItem = ((struct Lane*)laneItem->prev->datap)->vehicles.head;
			aCar = (Vehicle*)llItem->datap;
			if (((Vehicle*)(llItem->prev->datap))->position >= fCar->position) {
				llItem = llItem->prev;
				while (llItem->prev->next != NULL && ((Vehicle*)(llItem->prev->datap))->position >= fCar->position) {
					llItem = llItem->prev;
				}
				llFound = 1;
			}
                        } //else
		}

		if (laneItem->next != NULL) {
			// the right side of current lane 
			rfItem = ((struct Lane*)laneItem->next->datap)->vehicles.head;
			if (rfItem == NULL ) {rfFound = 0; rlFound = 0;}
                        else {
			aCar = (Vehicle*)rfItem->datap;
			if (aCar->position < fCar->position) {
				while (rfItem->next != NULL && ((Vehicle*)(rfItem->next->datap))->position < fCar->position) {
					rfItem = rfItem->next;
				}
				rfFound = 1;
			}

			rlItem = ((struct Lane*)laneItem->next->datap)->vehicles.head;
			aCar = (Vehicle*)rlItem->datap;
			if (((Vehicle*)(rlItem->prev->datap))->position >= fCar->position) {
				rlItem = rlItem->prev;
				while (rlItem->prev->next != NULL && ((Vehicle*)(rlItem->prev->datap))->position >= fCar->position) {
					rlItem = rlItem->prev;
				}
				rlFound = 1;
			}
                        } //else
		}

	if (lfFound == 0) { //printf("left follow car is empty\n");
			lfItem = NULL; }
	if (llFound == 0) { //printf("left lead car is empty\n");
			llItem = NULL; }
	if (rfFound == 0) rfItem = NULL;
	if (rlFound == 0) rlItem = NULL;
}

int is_change_possibile(struct Road *currentRoad, struct Item *laneItem, Vehicle *fCar, struct Item *fItem, struct Item *lItem, double vsafe, int direction)
{
	Vehicle *aCar, *bCar;
	double avsafe, bvsafe;
	

	if (lItem == NULL && fItem == NULL) {
		if (direction == 0) {
			// try to turn left
			if (laneItem->prev->next != NULL) return 1;
				else return 0;
		} else {
			// try to turn right
			if (laneItem->next != NULL) return 1;
				else return 0;
		}
	}
	if (lItem == NULL && fItem != NULL) {
		bCar = (Vehicle*)fItem->datap;
		bvsafe = get_safe_velocity(bCar, fCar);
		if (bvsafe >= bCar->v - bCar->b*DET_TIME)
			return 1;
		else return 0;
	}
	if (lItem != NULL && fItem == NULL) {
		aCar = (Vehicle*)lItem->datap;
		avsafe = get_safe_velocity(fCar, (Vehicle*)lItem->datap);
		if (avsafe < aCar->v - aCar->b*DET_TIME) return 0;
		if (avsafe - vsafe > 1) return 1;
			else {
				if (fabs(avsafe - vsafe) < 0.01) {
					if (direction == 0 && laneItem->prev->prev->next == NULL) {
						return 0;
					}
					if (direction == 1 && laneItem->next->next == NULL) {
						return 0;
					}
					return 1;
				}
				return 0;
			}
	}
	if (lItem != NULL && fItem != NULL) {
		aCar = (Vehicle*)lItem->datap;
		bCar = (Vehicle*)fItem->datap;
		avsafe = get_safe_velocity(fCar, (Vehicle*)lItem->datap);
		bvsafe = get_safe_velocity(bCar, fCar);
		if (bvsafe < bCar->v - bCar->b*DET_TIME) return 0;
		if (avsafe < aCar->v - aCar->b*DET_TIME) return 0;
		
		if (avsafe - vsafe > 1) return 1;
			else {
				if (fabs(avsafe - vsafe) < 0.01) {
					if (direction == 0 && laneItem->prev->prev->next == NULL) {
						return 0;
					}
					if (direction == 1 && laneItem->next->next == NULL) {
						return 0;
					}
					return 1;
				}
				return 0;
			}
	}
	
}

void update_vehicle_single_lane(Vehicle *fCar, Vehicle *lCar)
{
	double vnext;
	
	if (lCar->position - fCar->position - CAR_LENGTH >= 300 ) {
		// do not use the car-following model
		fCar->vnext = fCar->v + fCar->a * DET_TIME;
		if (fCar->vnext > V_MAX) fCar->vnext = V_MAX;
		fCar->handled = 1;
	} else {
		// use the car-following model
		vnext = car_following_new_v(
				lCar->position - fCar->position - CAR_LENGTH - SAFE_CAR_GAP, 
				fCar->v, 
				lCar->v, 
				fCar->a, 
				fCar->b);
		fCar->vnext = vnext;
		fCar->handled = 1;
	} //if (lCar->position - fCar->position - CAR_LENGTH >= 100 )	
}

void update_first_car(struct Hashtable *tempTraces, struct Road *currentRoad, struct Item *fItem)
{
        char *vName = (char*)malloc(8*sizeof(char));
	Vehicle *fCar = (Vehicle*)fItem->datap;
	struct Cross *currentCross = currentRoad->tailEnd;
	struct Lane *currentLane = fCar->belongLane;
	struct Item *tempItem;
	struct Trace *tempTrace;
	double vnext;
	int index, angle, turn_direction;
        double cross_width = 2*LANE_WIDTH*currentRoad->lanes.nItems;

	if (fCar->a != 0) {
		// the current car is the first car on the road or there is only one car on the road

                              //decide car will turn to which direction
                               if (fCar->currentRoadItem->next != NULL) {
                                     angle = abs(currentRoad->tailEndAngle- ((struct Road*)fCar->currentRoadItem->next->datap)->headEndAngle);
                                     if (angle<30 || angle>330) turn_direction = 1;
                                     else if (angle>=30 && angle <180) turn_direction =2;
                                     else turn_direction = 0; 
                                }
                                else turn_direction = -1;	

		if (fCar->position  <= currentRoad->length - cross_width) {
			//Haven't pass the traffic light line
			if (currentRoad->length - cross_width - fCar->position > 30) {
				//Far from the cross, don't need to slow down
				fCar->v += fCar->a * DET_TIME;
				fCar->handled = 1;
				if (fCar->v > V_MAX) fCar->v = V_MAX;	
     				fCar->position += fCar->v * DET_TIME;
			} else {
				// Near the cross

				if (turn_direction != -1 && currentRoad->lights[turn_direction].state == 2) {
					//Red light
					vnext = car_following_new_v(currentRoad->length - fCar->position - cross_width - 3, fCar->v, 0, fCar->a, fCar->b);
					fCar->v = vnext;
					fCar->handled = 1;
					fCar->position += fCar->v * DET_TIME;
				} else {
					//Green light or there is no traffic light
					//Near the cross, control speed, but don't need to stop
					fCar->v -= fCar->b * DET_TIME;
					fCar->v = 3;
					fCar->handled = 1;
					fCar->position += fCar->v * DET_TIME;
				}
			}

		} else {
			//The car has already passed the traffic light line, don't need to consider traffic light
			//But we will consider wether it has arrived the destination
			if (fCar->currentRoadItem->next == NULL) {
				sprintf(vName, "%d\0", fCar->id);
				index = sdbm((unsigned char*)vName) % MAX_HASH_SIZE;
				tempItem = (struct Item*)hashtable_find(tempTraces, vName);
				if (tempItem != NULL) {
					tempTrace = (struct Trace*)tempItem->datap;
					tempTrace->finished = 1;	
				}			
				fCar = (Vehicle*)duallist_pick_item(&currentLane->vehicles, fItem);
				free(fCar);
			} else {
				//The car has not arrived the destination. add it to the crossLane
				fCar->handled = 1;
				add_to_crossLane(currentRoad, currentLane, fCar);
				//printf("finish add car to cross\n");
			} //if (fCar->currentRoadItem->next == NULL)
	
		} //if (fCar->position <= currentRoad->length - cross_width)
	}		
}


struct Item* change_lane(struct Road *currentRoad, struct Item *laneItem, struct Item *fItem, struct Item *afterItem, struct Item *beforeItem, int direction)
{
	Vehicle *aCar;
	struct Lane *toLane;
	aCar = (Vehicle*)fItem->datap;
	struct Lane *fromLane = aCar->belongLane;
	struct Item *tempItem;
	
	if (direction == 0) {
		toLane = (struct Lane*)laneItem->prev->datap;
	} else {
		toLane = (struct Lane*)laneItem->next->datap;
	}

	if (beforeItem == NULL) {
		if (afterItem == NULL) {
			tempItem = fItem->prev;
			aCar = (Vehicle*)duallist_pick_item(&fromLane->vehicles, fItem);
			aCar->handled = 1;
			aCar->belongLane = toLane;
			duallist_add_to_head(&toLane->vehicles, aCar);
		} else {
			tempItem = fItem->prev;
			aCar = (Vehicle*)duallist_pick_item(&fromLane->vehicles, fItem);
			aCar->handled = 1;
			aCar->belongLane = toLane;
			duallist_add_to_tail(&toLane->vehicles, aCar);

		}
	} else {
		tempItem = fItem->prev;
		aCar = (Vehicle*)duallist_pick_item(&fromLane->vehicles, fItem);
		aCar->handled = 1;
		aCar->belongLane = toLane;
		//toLane = ((struct Vehicle*)beforeItem->datap)->belongLane;
		duallist_add_before_item(&toLane->vehicles, beforeItem->prev, beforeItem, aCar);
	}
	
	return tempItem;
}

double car_following_new_v(double g, double vf, double vl, double a, double b)
{
	float vnext, vsafe;
	if (vf < V_MAX - a*DET_TIME) {
		vnext = vf + a*DET_TIME;
	} else {
		vnext = V_MAX;
	}		
	vsafe = vl + (g - vl) / ((vl + vf) / (2 * b) + DET_TIME);
	if (vnext > vsafe) vnext = vsafe;
	if (vnext < 0) vnext = 0;
	return vnext;
}

void add_to_crossLane(struct Road *currentRoad, struct Lane *currentLane, Vehicle *fCar)
{
	struct Road *nextRoad = (struct Road*)fCar->currentRoadItem->next->datap;

	struct crossLane *nextLane;
	struct Item *aItem, *firstItem;
	Vehicle *firstCar;
	double vnext, cross_width = 2*LANE_WIDTH*currentRoad->lanes.nItems;;
	char c;

	aItem = currentLane->crossLanes.head;
	while (aItem != NULL) {
		nextLane = (struct crossLane*)aItem->datap;
		if (nextLane->toRoad == nextRoad) {
			firstItem = nextLane->vehicles.head;
			if (firstItem == NULL) {
				// there is no car in the cross lane
				fCar = (Vehicle*)duallist_pick_tail(&currentLane->vehicles);
				fCar->v += fCar->a*DET_TIME;
				fCar->position = fCar->position + fCar->v*DET_TIME- currentRoad->length + cross_width;
				duallist_add_to_head(&nextLane->vehicles, fCar);
				return;
			} else {
				// There already has a car in the cross lane
				firstCar = (Vehicle*)firstItem->datap;
				if (firstCar->position > CAR_LENGTH + SAFE_CAR_GAP) {
					fCar = (Vehicle*)duallist_pick_tail(&currentLane->vehicles);
					fCar->position = 0;
					fCar->v = fCar->v+fCar->a*DET_TIME;
					duallist_add_to_head(&nextLane->vehicles, fCar);
					return;
				} else {
					// can not go into cross, wait on current road
					fCar->position = currentRoad->length - cross_width;
					fCar->v = 0;
					return;
				}
			}
		}
		aItem = aItem->next;
	}
}

void handle_crossLane(struct crossLane* currentLane)
{
	struct Item *fItem, *lItem, *firstItem;
	Vehicle *fCar, *lCar, *firstCar;
	struct Lane *nextLane;
	double vnext;
	char c;
        double cross_width = 2*LANE_WIDTH*currentLane->toRoad->lanes.nItems;

	fItem = currentLane->vehicles.head;
	if (fItem == NULL) return;
	fCar = (Vehicle*)fItem->datap;
	
	while (fItem != NULL && fItem->next != NULL) {
		lItem = (struct Item*)fItem->next;
		lCar = (Vehicle*)lItem->datap; 
		vnext = car_following_new_v(lCar->position - fCar->position - CAR_LENGTH - SAFE_CAR_GAP, fCar->v, lCar->v, fCar->a, fCar->b);
		fCar->v = vnext;
		fCar->handled = 1;
		fCar->position += fCar->v * DET_TIME;
		fItem = fItem->next;	
		fCar = (Vehicle*)fItem->datap;
	}

	/* First vehicle on this cross lane */
	if (fCar->position + (fCar->v + fCar->a*DET_TIME)*DET_TIME > currentLane->length) {
		// the car will drive on the next road
		nextLane = find_available_lane(currentLane, currentLane->toRoad, fCar);
		if (nextLane != NULL) {
			// found a lane to add in
			fCar = (Vehicle*)duallist_pick_tail(&currentLane->vehicles);
			fCar->position = cross_width;
			fCar->handled = 1;
			fCar->v += fCar->v+fCar->a*DET_TIME;
			fCar->currentRoadItem = fCar->currentRoadItem->next;
			fCar->belongLane = nextLane;
			fCar->handled = 1;
			duallist_add_to_head(&nextLane->vehicles, fCar);
			return;
		} else {
			/* can not find an available lane */
			fCar->position = currentLane->length - CAR_LENGTH;
			fCar->v = 0;
			fCar->handled = 1;
			return;
		}
	} else {
		/* can not go away from the cross, still move in the cross */
		fCar->v += fCar->a*DET_TIME;
		if (fCar->v > 3) fCar->v = 3;
		fCar->handled = 1;
		fCar->position += fCar->v*DET_TIME;
	}
}

struct Lane* find_available_lane(struct crossLane* currentLane, struct Road* nextRoad, Vehicle *fCar)
{
	int lane;
	struct Lane *nextLane;
	struct Item *firstItem, *laneItem;
	Vehicle *firstCar;
        double cross_width = 2*LANE_WIDTH*nextRoad->lanes.nItems;

	/* find an availale lane that a vehicle can go from cross on this lane */
        laneItem = nextRoad->lanes.head;
	while(laneItem->next != NULL) {
		nextLane = (struct Lane*)laneItem->datap;
		
		if (nextLane->vehicles.head == NULL) {
			return nextLane;
		} 
		firstItem = (struct Item*)nextLane->vehicles.head;
		firstCar = (Vehicle*)firstItem->datap;

		if (firstCar->position > cross_width + CAR_LENGTH + SAFE_CAR_GAP) {
			return nextLane;
		}
                laneItem = laneItem->next;
	} //  if (is_road_available != 1)
	
	return NULL;
}

void calculate_coordinate(struct Road* currentRoad, struct Lane* currentLane, double shift, struct Hashtable *tempTraces, int count)
{
	struct Item *pointItem_0, *pointItem_1, *fItem, *tempItem;
	Vehicle *fCar;
	struct Point *point_0, *point_1;
	struct Report *tempRep;
	struct Trace *tempTrace;
	struct Speed *speed;
	struct Cross *currentCross;
	double dis_between_points, currentSpeed, dis_start_to_point0, ratio;
	int index;
        char *vName = (char*)malloc(8*sizeof(char));

	fItem = (struct Item*)currentLane->vehicles.head;
	pointItem_0 = (struct Item*)currentRoad->points.head;
       	pointItem_1 = (struct Item*)pointItem_0->next;
	point_0 = (struct Point*)pointItem_0->datap;
	point_1 = (struct Point*)pointItem_1->datap;
       	dis_between_points = distance_in_meter(point_0->x, point_0->y, point_1->x, point_1->y);
	dis_start_to_point0 = 0;
	currentSpeed = 0;		
	
	while (fItem != NULL) {
		fCar = (Vehicle*)fItem->datap;
		fCar->handled = 0;			
		currentSpeed += fCar->v*DET_TIME;
		
		while (fCar->position > dis_between_points + dis_start_to_point0) {
			pointItem_0 = pointItem_1;
			if (pointItem_1->next != NULL) {
				pointItem_1 = pointItem_1->next;
				point_0 = (struct Point*)pointItem_0->datap;
				point_1 = (struct Point*)pointItem_1->datap;
			} else {
				currentCross = currentRoad->tailEnd;
				point_0 = (struct Point*)pointItem_0->datap;
				point_1 = &currentCross->gPoint;
			}
			dis_start_to_point0 += dis_between_points;
			dis_between_points = distance_in_meter(point_0->x, point_0->y, point_1->x, point_1->y);			
		} //while (fCar->position > dis_between_points + dis_start_to_point0)
		
		ratio = (fCar->position - dis_start_to_point0) / dis_between_points;
		
		fItem = (struct Item*)fItem->next;
		
		tempRep = (struct Report*)malloc(sizeof(struct Report));
		report_init_func(tempRep);
	
		// create a new report
		//tempRep->timestamp = strtot(timestamp);
		tempRep->speed = (short)fCar->v;
		tempRep->angle = (short)angle_between(point_0->x, point_0->y, point_1->x, point_1->y);
		fCar->x = point_0->x + (point_1->x - point_0->x)*ratio - shift * sin(M_PI*tempRep->angle/180);
		fCar->y = point_0->y + (point_1->y - point_0->y)*ratio + shift * cos(M_PI*tempRep->angle/180);
		tempRep->gPoint.x = fCar->x;
		tempRep->gPoint.y = fCar->y;
		tempRep->state = '0';
		
		sprintf(vName, "%d\0", fCar->id);
		index = sdbm((unsigned char*)vName) % MAX_HASH_SIZE;
		//sem_wait(hash_entry + index);
		tempItem = (struct Item*)hashtable_find(tempTraces, vName);
			if(tempItem == NULL) {
			//printf("%s\n", vName);
			tempTrace = (struct Trace*)malloc(sizeof(struct Trace));
			trace_init_func(tempTrace);
			strncpy(tempTrace->vName, vName, strlen(vName)+1);
			if (fCar->a == 0) {
				tempTrace->type = 'b';
			} else {
				tempTrace->type = '0';
			}
			hashtable_add(tempTraces, vName, tempTrace);
	
			tempTrace->oldx = 0;
			tempTrace->oldy = 0;
			tempTrace->startCount = count;
			tempTrace->finished = 0;
			/*tempTrace->maxSpeed = tempRep->speed;
			tempTrace->startAt = tempRep->timestamp;
			tempTrace->endAt = tempRep->timestamp;
			tempTrace->box.xmin = tempRep->gPoint.x;
			tempTrace->box.xmax = tempRep->gPoint.x;
			tempTrace->box.ymin = tempRep->gPoint.y;
			tempTrace->box.ymax = tempRep->gPoint.y;*/
		} else {
			tempTrace = (struct Trace*)tempItem->datap;
			//sem_wait(&roadlock[fCar->id]);
			/*if(tempRep->speed > tempTrace->maxSpeed)
				tempTrace->maxSpeed = tempRep->speed;
			if(tempRep->timestamp < tempTrace->startAt)
				tempTrace->startAt = tempRep->timestamp;
			if(tempRep->timestamp > tempTrace->endAt)
				tempTrace->endAt = tempRep->timestamp;
			if(tempRep->gPoint.x < tempTrace->box.xmin)
				tempTrace->box.xmin = tempRep->gPoint.x;
			if(tempRep->gPoint.x > tempTrace->box.xmax)
				tempTrace->box.xmax = tempRep->gPoint.x;
			if(tempRep->gPoint.y < tempTrace->box.ymin)
				tempTrace->box.ymin = tempRep->gPoint.y;
			if(tempRep->gPoint.y > tempTrace->box.ymax)
				tempTrace->box.ymax = tempRep->gPoint.y;*/
		} //if(tempItem == NULL)
		tempRep->fromTrace = tempTrace;
		duallist_add_to_tail(&tempTrace->reports, tempRep);
		//sem_post(hash_entry + index);
	
	} //while (fItem != NULL)
}

void coordinate_crossLane(struct crossLane* currentLane, struct Hashtable *tempTraces, int count)
{
	struct Item *fItem, *tempItem;
	Vehicle *fCar;
	double ratio;
	struct Report *tempRep;
	int index;
	struct Trace *tempTrace;	
	char c, *vName = (char*)malloc(8*sizeof(char));
;

	fItem = currentLane->vehicles.head;
	if (fItem == NULL) return;
	

	while (fItem != NULL) {
		fCar = (Vehicle*)fItem->datap;
		fCar->handled = 0;
		ratio = fCar->position/currentLane->length;
	
		fCar->x = currentLane->fromPoint.x + (currentLane->toPoint.x - currentLane->fromPoint.x)*ratio;
		fCar->y = currentLane->fromPoint.y + (currentLane->toPoint.y - currentLane->fromPoint.y)*ratio;
		
		// create a new report
		tempRep = (struct Report*)malloc(sizeof(struct Report));
		report_init_func(tempRep);	
		tempRep->speed = (short)fCar->v;
		tempRep->angle = (short)angle_between(currentLane->fromPoint.x, currentLane->fromPoint.y, currentLane->toPoint.x, currentLane->toPoint.y);
		tempRep->gPoint.x = fCar->x;
		tempRep->gPoint.y = fCar->y;
		tempRep->state = '0';
		
		sprintf(vName, "%d\0", fCar->id);
		index = sdbm((unsigned char*)vName) % MAX_HASH_SIZE;
		//sem_wait(hash_entry + index);
		tempItem = (struct Item*)hashtable_find(tempTraces, vName);
		if(tempItem == NULL) {
			//printf("%s\n", vName);
			tempTrace = (struct Trace*)malloc(sizeof(struct Trace));
			trace_init_func(tempTrace);
			strncpy(tempTrace->vName, vName, strlen(vName)+1);
			hashtable_add(tempTraces, vName, tempTrace);
	
			tempTrace->oldx = 0;
			tempTrace->oldy = 0;
			tempTrace->startCount = count;
			tempTrace->finished = 0;
		} else {
			tempTrace = (struct Trace*)tempItem->datap;
	
		} //if(tempItem == NULL)
		tempRep->fromTrace = tempTrace;
		duallist_add_to_tail(&tempTrace->reports, tempRep);
		//sem_post(hash_entry + index);
		
		fItem = fItem->next;
	}
}

void trace_write(struct Hashtable *Traces, FILE *fOutput)
{
        if (Traces == NULL) return;
        struct Item *headItem, *reportItem;
	struct Trace *aTrace;
        struct Report *aReport;
	Vehicle *aCar;

        for (int i =0; i<Traces->size; i++){
		headItem = Traces->head[i];
        	while (headItem != NULL) {
		aTrace = (struct Trace*)headItem->datap;
                reportItem = aTrace->reports.head;
                while (reportItem != NULL) {
			aReport = (struct Report*)reportItem->datap;
                 	fprintf(fOutput, "%s %hd %hd %lf %lf\n",
			aTrace->vName,
			aReport->speed,
			aReport->angle,
			aReport->gPoint.x,
			aReport->gPoint.y);
			reportItem = reportItem->next; 	
		} // while(reportItem != NULL)
		headItem = headItem->next;
		} //while (headItem != NULL) 
	} //for
	
	return;
}
