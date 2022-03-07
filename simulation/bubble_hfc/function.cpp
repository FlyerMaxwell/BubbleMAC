// #pragma once
#include "function.h"
#include <iostream>
// using namespace std;
//--------------YX below--------------------------------------//
int init_simulation(struct Region* region){
	struct Item *aItem;
	struct Cell *aCell;
	struct vehicle *aCar;

	for(int i = 0; i < region->hCells; i++){       
		for(int j = 0; j < region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
//-----------------------需要初始化的内容---------------------------//
                  	aCar->id = -1;               // id of the car
					aCar->handled = 0;                  //  to indicate whether the car has been updated during this time
					aCar->position = 0, aCar->positionNew = 0;	// distance from the head of road before update  
					aCar->v = 0, aCar->vnext = 0;		// the velocity of the vehicle
					aCar->vmax = 0;		// the max velocity of the vehicle
					aCar->a = 0;		// the accelration of the vehicle, regard as constant
					aCar->b = 0;		// the decelration of the vehicle, regard as constant
					aCar->x = 0;
					aCar->y = 0;
					aCar->belongLane = NULL;
					aCar->pathInfo = NULL;
					aCar->currentRoadItem = NULL;

					//varaibles fpr bubble MAC
					aCar->belongLaneID = -1;
					aCar->slot_occupied = 1;  //
					aCar->slot_condition = 0; //0 for no slot occupied, 1 for accessing slots and 2 for occupied already.
					aCar->slot_oneHop = NULL;
					aCar->slot_twoHop = NULL;
					aCar->isExpansion = false;
					aCar->car_role = ROLE_SINGLE;
					aCar->radius_flag = 0;
					aCar->isQueueing = 0;
					aCar->commRadius = 0;
					aCar->dir_x = 0, aCar->dir_y = 0; //车辆的方向矢量(dir_x, dir_y)。可调用IsFront(struct vehicle *aCar, struct vehicle *tCar)判断车辆间位置
					duallist_init(&aCar->packets);
					duallist_init(&aCar->neighbours);
					duallist_init(&aCar->frontV);
					duallist_init(&aCar->rearV);	//store the front neighbors and rear neighbors according to the received packets.
			}
		}
	}
	return 0;
}


//handle neighbors： 处理邻居，将所有车辆的所在的九宫格内的车挂载到其潜在的neighbors中。
void handle_neighbors(struct Region* region){
    struct Cell *aCell, *nCell;
    struct Item *aItem, *nItem;
    struct vehicle* aCar, *nCar;

    for(int i = 0; i<region->hCells; i++){       
		for(int j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			int neighbour_cell[9][2] = {{i-1,j-1}, {i,j-1}, {i+1,j-1}, {i-1,j}, {i,j}, {i+1,j}, {i-1,j+1}, {i,j+1}, {i+1,j+1}};
            if (aCell->cars.head == NULL) continue;
            aItem = aCell->cars.head;
            while(aItem!=NULL){
                aCar = (struct vehicle*)aItem->datap;
                for(int k = 0; i<9 ;k++){
                    if ((neighbour_cell[k][0]<0 || neighbour_cell[k][0]>=region->hCells) || (neighbour_cell[k][1]<0 || neighbour_cell[k][1]>=region->vCells)) continue;
                    int tmpx = neighbour_cell[k][0], tmpy = neighbour_cell[k][1];
                    nCell = aCell = region->mesh + tmpx*region->vCells + tmpy;
                    nItem = nCell->cars.head;
                    while(nItem != NULL){
                        nCar = (struct vehicle*)nItem->datap;
						if (nCar->id == aCar->id) {nItem = nItem->next; continue;} //ncar is the same car with acar
                        duallist_add_to_tail(&(aCar->neighbours), nCar);
                    }
                }
            }
        }
    }
}




// void handle_transmitter(struct Region* region, struct Duallist *Collisions, int slot){
//     struct Cell *aCell;
//     struct Item *aItem, *bItem;
//     struct vehicle *aCar, *bCar;
//     struct neighbour_car* bneigh;

//     for(int i = 0; i<region->hCells; i++){       
//         for(int j = 0; j<region->vCells;j++) {
//             aCell = region->mesh + i*region->vCells + j;
//             aItem = aCell->cars.head;
//             while (aItem != NULL){
//                 aCar = (struct vehicle*)aItem->datap;
//                 if(aCar->slot_occupied != slot) continue; //忽略掉receiver

//                 bItem = (struct Item*)aCar->neighbours.head;
//                 while (bItem != NULL) {
//                     bCar =  (struct vehicle*)bItem->datap;
//                     if(aCar->commRadius < distance_with_neighbor(aCar, bCar)) continue;   //超出通信半径
//                     else{
//                         if(bCar->slot_occupied == aCar->slot_occupied){
//                             struct collision* coli =  generate_collision(aCar,bCar,0,slot);
//                             duallist_add_to_tail(Collisions, coli);
//                         }else{
//                             struct packet* pkt = generate_packet(aCar,slot);
//                             duallist_add_to_tail(&(bCar->packets), pkt);
//                         }                           
//                     }
//                     bItem = bItem->next;
//                 }
//                 aItem = aItem->next;
//             }
//         }
//     }
// }

void handle_receiver(struct Region* region, struct Duallist* Collisions, int slot){
    struct Cell *aCell;
    struct Item *aItem, *bItem;
    struct vehicle *aCar;
    struct neighbour_car* bneigh;

    for(int i = 0; i<region->hCells; i++){       
        for(int j = 0; j<region->vCells;j++) {
            aCell = region->mesh + i*region->vCells + j;
            aItem = aCell->cars.head;
            while (aItem != NULL){
                aCar = (struct vehicle*)aItem->datap;
                if(aCar->slot_occupied == slot) continue; //忽略掉transmitter
                
                if(aCar->packets.nItems == 0) continue; //没有收到包
                else if(aCar->packets.nItems == 1){//只收到一个pkt，则正常收包，update OHN 和THN, frontV, rearV
                    bItem =  (struct Item*)aCar->packets.head;
                    struct packet* pkt= (struct packet*)bItem->datap;
                    int index = pkt->slot, value = pkt->srcVehicle->id;
                    aCar->slot_oneHop[index] = value;//更新OHN时槽使用情况
                    for(int i = 0 ; i < SlotPerFrame; i++){
                        aCar->slot_twoHop[i] = pkt->slot_resource_oneHop_snapShot[i];//将pkt中携带的OHN信息更新到THN中，不考虑覆盖问题
                    }




                }else{//有两个或以上的pkt，产生Collision
                    bItem = (struct Item*)aCar->packets.head;
                    while(bItem!=NULL){
                        struct packet* pkt= (struct packet*)bItem->datap;
                        if(pkt->srcVehicle->slot_condition == 1){//Access Collision
                            struct collision* coli = generate_collision(aCar,pkt->srcVehicle,1,slot);
                            duallist_add_to_tail(Collisions, coli);
                        }else if(pkt->srcVehicle->slot_condition == 2){// MergeCollision
                            struct collision* coli = generate_collision(aCar,pkt->srcVehicle,2,slot);
                            duallist_add_to_tail(Collisions, coli);
                        }
                    } 
                }
            }
        }
    }
}


void log_collisions(struct Region* region, struct Duallist* Collisions){
    char output_collisions_path[100];
    FILE * Collisions_output;
    sprintf(output_collisions_path, "./simulation_result/result_%d_%d.txt", SlotPerFrame, traffic_density);
    
    Collisions_output = fopen(output_collisions_path,"a");
    
    struct Item* aItem = (struct Item*)Collisions->head;
    while( aItem!= NULL){
        struct collision* coli = (struct collision*)aItem->datap;
        fprintf(Collisions_output, "%d %d %d %d\n", coli->type, coli->slot, coli->src->id, coli->dst->id);//TBD
    }
    fclose(Collisions_output);
}

double distance_between_vehicle(const struct vehicle* aCar, const struct vehicle* bCar){
    return sqrt((aCar->x - bCar->x)* (aCar->x - bCar->x) + (aCar->y - bCar->y)*(aCar->y - bCar->y));
}

struct packet * generate_packet(struct vehicle *aCar, int slot){
    struct packet* pkt;
    pkt = (struct packet*)malloc(sizeof(struct packet));
    pkt->slot = slot;
    pkt->condition = 0;//还没有发生碰撞
    pkt->srcVehicle = aCar;
    pkt->isQueueing = aCar->isQueueing;
    pkt->slot_resource_oneHop_snapShot = new int[SlotPerFrame];
    for(int i = 0; i < SlotPerFrame; i++){
        pkt->slot_resource_oneHop_snapShot[i] = aCar->slot_oneHop[i];
    }
    return pkt;
}

struct collision* generate_collision(struct vehicle *aCar, struct vehicle *bCar,  int type, int slot){
    struct collision * coli;
    coli = (struct collision*)malloc(sizeof(struct collision));
    coli->type = type;
    coli->slot = slot;
    coli->src = aCar;
    coli->dst = bCar;
    return coli;
}

//if tCar is in front of aCar, return true, if not, return false.
bool IsFront(struct vehicle *aCar, struct vehicle *tCar){
	if(aCar->dir_x != tCar->dir_x || aCar->dir_y != tCar -> dir_y){
		std::cout<<"Error: the compared two cars are not in the same lane."<<endl;
		exit(1);
	}
	if(aCar->dir_x > 0 && aCar->dir_y >0)
		return (tCar->x > aCar->x) && (tCar->y > aCar->y);
	else if(aCar->dir_x > 0 && aCar->dir_y <0)
		return (tCar->x > aCar->x) && (tCar->y < aCar->y);

}


//----------------------YX above---------------------//



//----------------------fc below-------------------//

// int generate_car_old(struct Region *region, unordered_map<int, vehicle*>& allCars) 
// {
//   double xmin,ymin,xmax,ymax, x, y;
//   int flag, i, j, k, l;
//   struct Cell *aCell, *bCell;
//   struct Item *aItem, *bItem, *tItem, *nItem;
//   struct vehicle *bCar, *aCar, *tCar, *nCar;
//   struct neighbour_car* tNeigh, *nNeigh, *bNeigh;
//   FILE *carinfo;
//   char file_path[100];
//   int car_count=0;

//   xmin = region->chosen_polygon->box.xmin;
//   xmax = region->chosen_polygon->box.xmax;
//   ymin = region->chosen_polygon->box.ymin;
//   ymax = region->chosen_polygon->box.ymax;
  
// 	sprintf(file_path, "/home/lion/Desktop/hfc/vanet1.0_old/vanet1.0/car_position/carposition_%d_%d.txt", traffic_density, bi_num); //traffic density, bi number
// 	carinfo = fopen(file_path, "r");
// 	int carnum;
// 	fscanf(carinfo, "%d", &carnum);
// 	Car_Number = carnum;
// 	for (k=0; k< carnum; k++){
// 		struct vehicle *new_car;
// 		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));
// 		fscanf(carinfo, "%d", &new_car->id);
// 		fscanf(carinfo, "%lf", &new_car->x);
// 		fscanf(carinfo, "%lf", &new_car->y);
// 		fscanf(carinfo, "%lf", &new_car->x1);
// 		fscanf(carinfo, "%lf", &new_car->y1);
// 		fscanf(carinfo, "%lf", &new_car->v);
// 		new_car->handled = 2;
// 		duallist_init(&new_car->history_neigh);
// 		duallist_init(&new_car->choose_neigh);
// 		duallist_init(&new_car->neighbours);
// 		duallist_init(&new_car->real_neigh);
// 		duallist_init(&new_car->known_neigh);

		
// //if (learning_cycle_num==2) printf("0.1\n");
// 		flag = false;		
// 		for(i = 0; i<region->hCells; i++){       
// 			for(j = 0; j<region->vCells;j++) {
// 				aCell = region->mesh + i*region->vCells + j;
// 				if (aCell->box.xmin <= new_car->x && aCell->box.xmax > new_car->x && aCell->box.ymax > new_car->y && aCell->box.ymin <= new_car->y) {flag=true;break;} //find the cell which contains the new car
// 			}
// 			if (flag==true) break;
// 		}
// 		new_car->belongCell = aCell;
		
// 		flag = false;
// 		for(i = 0; i<region->hCells; i++){       
// 			for(j = 0; j<region->vCells;j++) {
// 				bCell = region->mesh + i*region->vCells + j;
// 				bItem = (struct Item*)bCell->cars.head;
// 				while (bItem != NULL){
// 					bCar = (struct vehicle*)bItem->datap;
// 					if (bCar->id == new_car->id) {flag = true;break;}
// 					bItem = bItem->next;
// 				}
// 				if (flag == true) break;
// 			}
// 			if (flag == true) break;
// 		}
// //if (learning_cycle_num==2) printf("0.2\n");
// 		if (flag == true) {
// 			bCar->x = new_car->x;
// 			bCar->y = new_car->y;
// 			bCar->x1 = new_car->x1;
// 			bCar->y1 = new_car->y1;
// 			bCar->v = new_car->v;
// 			bCar->belongCell = new_car->belongCell;
// 			bCar->handled = 1;
// 			duallist_pick_item(&bCell->cars, bItem);
// 			duallist_add_to_tail(&aCell->cars, bCar);

// 			if (bi_num%200 == 0) {
// 				duallist_destroy(&bCar->history_neigh, NULL);
// 				duallist_init(&bCar->history_neigh);
// 				duallist_destroy(&bCar->neighbours, NULL);
// 				duallist_init(&bCar->neighbours);
// 			}
// //if (learning_cycle_num==2) printf("0.25\n");
// 			else {
// 				duallist_destroy(&bCar->neighbours, NULL);
// 				duallist_init(&bCar->neighbours);
// 			}

// 			free(new_car);
// 		}
// //if (learning_cycle_num==2) printf("0.3\n");
// 		if (flag == false && bi_num%200==0) duallist_add_to_tail(&(aCell->cars), new_car);
// 		if (flag == false && bi_num%200!=0) free(new_car);
// 	}
	
// //printf("1\n");
// 	for(i = 0; i<region->hCells; i++){       
// 			for(j = 0; j<region->vCells;j++) {
// 				aCell = region->mesh + i*region->vCells + j;
// 				aItem = aCell->cars.head;
// 				while (aItem != NULL){
// 					aCar = (struct vehicle*)aItem->datap;
// 					tItem = aItem;
// 					aItem = aItem->next;
// 					if (aCar->handled ==0) {
// 						bItem = aCar->history_neigh.head;
// 						while (bItem !=NULL) {
// 							bNeigh = (struct neighbour_car*)bItem->datap;
// 							bCar = (struct vehicle*)bNeigh->carItem->datap;
// 							nItem = bCar->history_neigh.head;
// 							while (nItem !=NULL) {
// 								nNeigh = (struct neighbour_car*)nItem->datap;
// 								if (nNeigh->car_id ==aCar->id) {duallist_pick_item(&bCar->history_neigh, nItem); break;}
// 								nItem = nItem->next;
// 							}
// 							bItem = bItem->next;
// 						}
// 						duallist_pick_item(&aCell->cars, tItem); 
// 						free(aCar);
// 					}
// 					else car_count++;
// 				}
// 			}
// 	}

// 	fclose(carinfo);

//   //printf("\ntotal car number in this BI: %d\n", car_count);
 
//   return 0;
// }
int generate_car(struct Region *region) 		//new generate_car modified by hfc
{
	double xmin,ymin,xmax,ymax, x, y;
	int flag, i, j, k, l;
	struct Cell *aCell, *bCell;
	struct Item *aItem, *bItem, *tItem, *nItem;
	struct vehicle *bCar, *aCar, *tCar, *nCar;
	struct neighbour_car* tNeigh, *nNeigh, *bNeigh;
	FILE *carinfo;
	char file_path[100];
	int car_count=0;

	xmin = region->chosen_polygon->box.xmin;
	xmax = region->chosen_polygon->box.xmax;
	ymin = region->chosen_polygon->box.ymin;
	ymax = region->chosen_polygon->box.ymax;
  
	sprintf(file_path, "/home/lion/Desktop/hfc/vanet1.0_old/vanet1.0/car_position/carposition_%d_%d.txt", traffic_density, slot + 4000); //traffic density, slot number
	carinfo = fopen(file_path, "r");
	int carnum;
	fscanf(carinfo, "%d", &carnum);
	Car_Number = carnum;
	for (k=0; k< carnum; k++){
		struct vehicle *new_car;
		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));
		fscanf(carinfo, "%d", &new_car->id);
		fscanf(carinfo, "%lf", &new_car->x);
		fscanf(carinfo, "%lf", &new_car->y);
		fscanf(carinfo, "%lf", &new_car->a);
		fscanf(carinfo, "%lf", &new_car->b);
		fscanf(carinfo, "%lf", &new_car->v);		
		fscanf(carinfo, "%lf", &new_car->belongLaneID);
		fscanf(carinfo, "%lf", &new_car->dir_x);
		fscanf(carinfo, "%lf", &new_car->dir_y);

		new_car->handled = 2;
		duallist_init(&new_car->history_neigh);
		duallist_init(&new_car->choose_neigh);
		duallist_init(&new_car->neighbours);
		duallist_init(&new_car->real_neigh);
		duallist_init(&new_car->known_neigh);
//if (learning_cycle_num==2) printf("0.1\n");
		flag = false;		
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->box.xmin <= new_car->x && aCell->box.xmax > new_car->x && aCell->box.ymax > new_car->y && aCell->box.ymin <= new_car->y) {
					flag=true;
					break;
				} //find the cell which contains the new car
			}
			if (flag==true) break;
		}
		new_car->belongCell = aCell;
		allCars[new_car->id] = new_car;	//*************************modified by hfc
		flag = false;
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				bCell = region->mesh + i*region->vCells + j;
				bItem = (struct Item*)bCell->cars.head;
				while (bItem != NULL){
					bCar = (struct vehicle*)bItem->datap;
					if (bCar->id == new_car->id) {flag = true;break;}
					bItem = bItem->next;
				}
				if (flag == true) break;
			}
			if (flag == true) break;
		}
//if (learning_cycle_num==2) printf("0.2\n");
		if (flag == true) {
			bCar->x = new_car->x;
			bCar->y = new_car->y;
			bCar->v = new_car->v;
			bCar->belongCell = new_car->belongCell;
			bCar->handled = 1;
			duallist_pick_item(&bCell->cars, bItem);
			duallist_add_to_tail(&aCell->cars, bCar);

			if (bi_num%200 == 0) {
				duallist_destroy(&bCar->history_neigh, NULL);
				duallist_init(&bCar->history_neigh);
				duallist_destroy(&bCar->neighbours, NULL);
				duallist_init(&bCar->neighbours);
			}
//if (learning_cycle_num==2) printf("0.25\n");
			else {
				duallist_destroy(&bCar->neighbours, NULL);
				duallist_init(&bCar->neighbours);
			}

			free(new_car);
		}
//if (learning_cycle_num==2) printf("0.3\n");
		if (flag == false && bi_num%200==0) duallist_add_to_tail(&(aCell->cars), new_car);
		if (flag == false && bi_num%200!=0) free(new_car);
	}
	
//printf("1\n");
	for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					tItem = aItem;
					aItem = aItem->next;
					if (aCar->handled ==0) {
						bItem = aCar->history_neigh.head;
						while (bItem !=NULL) {
							bNeigh = (struct neighbour_car*)bItem->datap;
							bCar = (struct vehicle*)bNeigh->carItem->datap;
							nItem = bCar->history_neigh.head;
							while (nItem !=NULL) {
								nNeigh = (struct neighbour_car*)nItem->datap;
								if (nNeigh->car_id ==aCar->id) {duallist_pick_item(&bCar->history_neigh, nItem); break;}
								nItem = nItem->next;
							}
							bItem = bItem->next;
						}
						duallist_pick_item(&aCell->cars, tItem); 
						free(aCar);
					}
					else car_count++;
				}
			}
	}

	fclose(carinfo);

  //printf("\ntotal car number in this BI: %d\n", car_count);
 
  return 0;
}


void update_vehicle_info(struct Region *region)
{
	double xmin,ymin,xmax,ymax, x, y;
	int flag, i, j, k, l;
  	struct Cell *aCell, *bCell;
  	struct Item *aItem, *bItem, *tItem, *nItem;
  	struct vehicle *bCar, *aCar, *tCar, *nCar;
  	struct neighbour_car* tNeigh, *nNeigh, *bNeigh;

	for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->cars.head == NULL) continue;

				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;

					tItem = aCar->neighbours.head;
					while (tItem != NULL) {
						tNeigh = (struct neighbour_car*)tItem->datap;
						tCar = (struct vehicle*)tNeigh->carItem->datap;
						nItem = aCar->history_neigh.head;
						while (nItem !=NULL) {
							nNeigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)nNeigh->carItem->datap;
							if (nCar->id == tCar->id) {nNeigh->packet_num=tNeigh->packet_num; break;}
							nItem = nItem->next;
						}
						if (nItem == NULL) duallist_add_to_tail(&aCar->history_neigh, tNeigh);
						tItem = tItem->next;
					}

					aItem = aItem->next;
				}
			}
	}
}

void update_trans_rate(struct Region *region)
{
  double xmin,ymin,xmax,ymax, x, y;
  int flag, i, j, k, l;
  struct Cell *aCell, *bCell;
  struct Item *aItem, *bItem, *tItem, *nItem;
  struct vehicle *bCar, *aCar, *tCar, *nCar;
  struct neighbour_car* tNeigh, *nNeigh, *bNeigh;
  FILE *carinfo;
  char file_path[100];
  int car_count=0;

	// update position
	sprintf(file_path, "/home/lion/Desktop/hfc/vanet1.0_old/vanet1.0/car_position/carposition_%d_%d.txt", traffic_density, bi_num); //traffic density, bi number
	carinfo = fopen(file_path, "r");
	int carnum;
	fscanf(carinfo, "%d", &carnum);
	//Car_Number = carnum;
	for (k=0; k< carnum; k++){
		int flag=0;
		struct vehicle *new_car;
		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));
		fscanf(carinfo, "%d", &new_car->id);
		fscanf(carinfo, "%lf", &new_car->x);
		fscanf(carinfo, "%lf", &new_car->y);
		fscanf(carinfo, "%lf", &new_car->x1);
		fscanf(carinfo, "%lf", &new_car->y1);
		fscanf(carinfo, "%lf", &new_car->v);

		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->cars.head == NULL) continue;

				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					if (aCar->id == new_car->id) {
						aCar->x = new_car->x;
						aCar->y = new_car->y;
						aCar->x1 = new_car->x1;
						aCar->y1 = new_car->y1;
						aCar->v = new_car->v;
						flag=1;
						break;
					}
					aItem = aItem->next;		
				}
				if (flag==1) break;
			}
			if (flag==1) break;
		}

		free(new_car);
	}

	//update transmission rate
	calculate_rate(region);

	fclose(carinfo);

	return;
}


int show_graph_degree(struct Region *region)
{
	int i, j;
	struct Cell *aCell;
	struct Item *aItem;
	struct vehicle *aCar;
	int degree=0;

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->neighbours.nItems > degree) degree = aCar->neighbours.nItems;
				aItem = aItem->next;
			}
		}
	}
	
	//printf("Graph Degree: %d\n", degree);
	return 0;
}

double calculate_dis(double x1, double y1, double x2, double y2)
{
	double dis = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	return dis;
}

double calculate_angle_diff(double angle1, double angle2)
{
	double diff=0;
	if (fabs(angle1-angle2) > 180) {
		if (angle1>angle2) diff = fabs(angle2+360-angle1);
		else diff = fabs(angle1+360-angle2);
	}
	else diff = fabs(angle1-angle2);

	return diff;
}

int car_legal(struct Region *region, struct vehicle *aCar, struct Cell *aCell)
{
	int x = aCell->xNumber, y = aCell->yNumber;
	int neighbour_cell[9][2] = {{x-1,y-1}, {x,y-1}, {x+1,y-1}, {x-1,y}, {x,y}, {x+1,y}, {x-1,y+1}, {x,y+1}, {x+1,y+1}};
	struct Cell *nCell;
	struct Item *nItem; //n represents neighbour
	struct vehicle *nCar;
	int flag = true;

	for (int i=0; i<9; i++){
		if ((neighbour_cell[i][0]<0 || neighbour_cell[i][0]>=region->hCells) || (neighbour_cell[i][1]<0 || neighbour_cell[i][1]>=region->vCells)) continue;
		int tmpx = neighbour_cell[i][0], tmpy = neighbour_cell[i][1];
		nCell = aCell = region->mesh + tmpx*region->vCells + tmpy;
		nItem = nCell->cars.head;
		while (nItem != NULL){
			nCar = (struct vehicle*)nItem->datap;
			if (calculate_dis(aCar->x1, aCar->y1, nCar->x1, nCar->y1) < safe_dis) {flag =false;break;}  // new car  doesn't have enough distance with another car
			nItem = nItem->next;
		}
		if (flag==false) break;
	}

	return flag;
}


double safeDistance(const struct vehicle* v1, const struct vehicle* v2){
	if(v2 == NULL){
		return ((v1->v * v1->v)/(2 * v1->b));
	}
	return (v1->v * v1->v - v2->v * v2->v)/(2 * v1->b * BRAKE_COE);
}

double vehicleDistance(const struct vehicle* v1, const struct vehicle* v2){
	double x = v1->x - v2->x, y = v1->y - v2->y;
	return sqrt(x*x + y*y);
}

bool curInFront(const struct vehicle* cur, const struct vehicle* tar){
	return ((cur->x - tar->x) * cur->dir_x > 0) || (((cur->x - tar->x) * cur->dir_x == 0) && (cur->y - tar->y) * cur->dir_y > 0);
}

int randSlot(int* occupied, int div){
	int len = 0;
	int ret = 0;
	unordered_map<int, int> tab;
	for(int i = 0; i < SlotPerFrame; i++){
		if(occupied[i] == -1){
			tab[len++] = i;
		}
	}
	if (div = -1){					//取后一半可用槽
		len >>= 1;
		ret = tab[len + rand()%len];
	}
	else{
		len >>= div;
		ret = tab[rand()%len];
	}
	tab.clear();
	return ret;
}
int randSlot(int* occupied){
	return randSlot(occupied, 0);
}

void Degrade(struct vehicle* cur_vehicle){

	cur_vehicle->slot_occupied = rand()%(SlotPerFrame - 1) + 1;
	cur_vehicle->slot_condition = 1;
	cur_vehicle->car_role = ROLE_SINGLE;
	cur_vehicle->commRadius = safeDistance(cur_vehicle, NULL);
	cur_vehicle->radius_flag = true;  
	// for(int i = 0; i < SlotPerFrame; i++){	//清空onehop
	// 	cur_vehicle->slot_oneHop[i] = -1;
	// 	cur_vehicle->slot_twoHop[i] = -1;
	// }

}

void applyForSlot(struct vehicle* cur_vehicle){
	for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){        //申请成功
		struct packet* cur_packet = (struct packet*)p->datap;
		struct vehicle* target_vehicle = cur_packet->srcVehicle;
		double dis = vehicleDistance(cur_vehicle, target_vehicle);

		if(cur_packet->slot_resource_oneHop_snapShot[cur_vehicle->slot_occupied] == cur_vehicle->id){
			cur_vehicle->slot_condition = 2;
			cur_vehicle->slot_oneHop[cur_packet->slot] = target_vehicle->id;
		}
	}
}