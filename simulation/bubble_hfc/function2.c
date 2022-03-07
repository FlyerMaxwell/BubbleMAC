#pragma once
#include "function.h"
#include <iostream>

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




void handle_transmitter(struct Region* region, struct Duallist *Collisions, int slot){
    struct Cell *aCell;
    struct Item *aItem, *bItem;
    struct vehicle *aCar, *bCar;
    struct neighbour_car* bneigh;

    for(int i = 0; i<region->hCells; i++){       
        for(int j = 0; j<region->vCells;j++) {
            aCell = region->mesh + i*region->vCells + j;
            aItem = aCell->cars.head;
            while (aItem != NULL){
                aCar = (struct vehicle*)aItem->datap;
                if(aCar->slot_occupied != slot) continue; //忽略掉receiver

                bItem = (struct Item*)aCar->neighbours.head;
                while (bItem != NULL) {
                    bCar =  (struct vehicle*)bItem->datap;
                    if(aCar->commRadius < distance_with_neighbor(aCar, bCar)) continue;   //超出通信半径
                    else{
                        if(bCar->slot_occupied == aCar->slot_occupied){
                            struct collision* coli =  generate_collision(aCar,bCar,0,slot);
                            duallist_add_to_tail(Collisions, coli);
                        }else{
                            struct packet* pkt = generate_packet(aCar,slot);
                            duallist_add_to_tail(&(bCar->packets), pkt);
                        }                           
                    }
                    bItem = bItem->next;
                }
                aItem = aItem->next;
            }
        }
    }
}

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
                
                if(aCar->packets.nItems == 0) {//若当前slot没有收到包，则更新此slot为-1
                    aCar->slot_oneHop[slot] = -1;
                    aCar->slot_twoHop[slot] = -1;
                    continue; //没有收到包
                }else if(aCar->packets.nItems == 1){//只收到一个pkt，则正常收包，update OHN 和THN, frontV, rearV
                    bItem =  (struct Item*)aCar->packets.head;
                    struct packet* pkt= (struct packet*)bItem->datap;
                    int index = pkt->slot, value = pkt->srcVehicle->id;
                    aCar->slot_oneHop[index] = value;//更新OHN时槽使用情况
                    for(int i = 0 ; i < SlotPerFrame; i++){
                        aCar->slot_twoHop[i] = pkt->slot_resource_oneHop_snapShot[i];//将pkt中携带的OHN信息更新到THN中，不考虑覆盖问题
                    }
                    // 将pkt指向的车更新到frontV, rearV
                    




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
	if(aCar->dir_x > 0 && aCar->dir_y > 0)
		return (tCar->x > aCar->x) && (tCar->y > aCar->y);
	else if(aCar->dir_x > 0 && aCar->dir_y < 0)
		return (tCar->x > aCar->x) && (tCar->y < aCar->y);
	else if(aCar->dir_x < 0 && aCar->dir_y > 0)
		return (tCar->x < aCar->x) && (tCar->y > aCar->y);
    else if(aCar->dir_x < 0 && aCar->dir_y < 0)
		return (tCar->x < aCar->x) && (tCar->y < aCar->y);
    
    if(aCar->dir_x != tCar->dir_x || aCar->dir_y != tCar -> dir_y){
		std::cout<<"Error: the compared two cars are not in the same lane."<<endl;
		exit(1);
	}
}

//车辆aCar收到一个pkt，将pkt对应的车辆加入到Front或Rear中
void insertFrontRear(struct vehicle *aCar, struct packet *pkt){
	if(aCar == NULL || pkt == NULL){
		std::cout<<"insertFrontRear Error!"<<endl;
		exit(1);
	}
    if(IsFront(aCar, pkt->srcVehicle)){//pkt来源于前面的一个车
        struct Item* newp = (struct Item*)malloc(sizeof(struct Item));
	    newp->datap = pkt->srcVehicle;
        if(aCar->frontV == NULL){//若当前无frontV
            newp->next = NULL;
            newp->prev = newp;
            aCar->frontV.head = newp;
        }else{
           if(distance_between_vehicle(aCar, aCar->frontV.head->datap) > distance_between_vehicle(aCar, pkt->srcVehicle))
                aCar->frontV.head->datap = pkt->srcVehicle;//此包的车为最近的车
        }

    }else{//pkt来源于后面的一个车

    }
}


//----------------------YX above---------------------//
