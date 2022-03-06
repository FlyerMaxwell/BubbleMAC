#include <iostream>
#include <unordered_map>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "function.h"
#include "trace.h"

using namespace std;


/*
    modified by hfc, 2022.3.4.
*/

// argv[0]: name of program, argv[1]: choose a map, argv[2]: define para or not
int main(int argc, char *argv[]) {

    FILE *fsource;
    struct Region *aRegion = NULL;
    int seq_num = 0, traffic_density = 40;

    unordered_map<int, struct vehicle*> allCars;

    if (argc == 1) {
	    cout<<"Error: There is no map. Please define a map as:  "<<argv[0]<<" XXX.map"<<endl;
	    exit(1);
    }else if(argc == 2){
        cout<<"Input traffic data sequence number (seq_num) : "<<endl;
        scanf("%d", &seq_num);     //每段数据长度为10s，每5ms记录一次，合计2000条
        cout<<"Input traffic density (traffic density): "<<endl;
        scanf("%d", &traffic_density);
    }

    // Load the map
    cout<<"Loading the map..."<<endl;
    fsource=fopen(argv[1], "rb");
    aRegion = region_load_func(fsource, NULL, -1);
    fclose(fsource);
    cout<<"The map "<< argv[1] <<" has been loaded."<<endl;
    
    // Simulation in sequence style.
	for (slot =(seq_num-1)*2000; slot<seq_num*2000; slot++){
        /*
        ____________________________________Separate Line by Fc Hu_________________________________________________________
        */
        int cur_slot = slot % SlotPerFrame;
        //Firstly, update vehicles' location every UpLocSlot slots, e.g. 1 packet per frame
        if (slot % SlotPerFrame == 0) { //Reload the map per 100ms
            cout<<"Initializing the simulation..."<<endl;
            allCars.clear();
            init_simulation(aRegion);
            cout<<"Initialization have finished."<<endl;

            cout<<"Loading vehilces..."<<endl;
            generate_car(aRegion);
            cout<<"Vehicles have been loaded."<<endl;
            
            // cout<<"Handling neighbors for each vehicle..."<<endl;
            // handle_neighbour(aRegion);        // 哪些是车辆潜在的neighbors（根据距离，九宫格判断，只需要比较这些车辆，不需要比较整张地图的车辆）
            // cout<<"Neighbors have been handled."<<endl;

            //Secondly, determine the slot as well as comm. radius at the beginning of each frame
            for(auto iter = allCars.begin(); iter != allCars.end(); iter++){    //determine slot
                struct vehicle* cur_vehicle = iter->second;
                cur_vehicle->commRadius = 0;
                if(cur_vehicle->slot_occupied != NOT_OCCUPIED){
                    if (cur_vehicle->packets == NULL){
                        cur_vehicle->slot_occupied = rand()%SlotPerFrame;
                        cur_vehicle->slot_condition = 1;
                        cur_vehicle->isQueueing = false;
                    }
                    for(auto p = cur_vehicle->packets->head; p != NULL; p = p->next){
                        struct packet* cur_packet = p->datap;
                        struct vehicle* target_vehicle = cur_packet->srcVehicle;
                        double dis = vehicleDistance(cur_vehicle, target_vehicle);
                        if (cur_vehicle->isQueueing && cur_packet->isQueueing && cur_packet->slot_resource_oneHop_snapShot[cur_packet->slot] == cur_vehicle->id){ 
                            //1:two cars in one queue*****************************筛选不完全
                            //按前后分??
                            if (dis > safeDistance(cur_vehicle, NULL)){
                                if(curInFront(cur_vehicle, target_vehicle)){    //cur_vehicle becomes the new tail of right queue
                                    cur_vehicle->slot_occupied = TAIL;
                                    cur_vehicle->slot_condition = 1;
                                    // cur_vehicle->isQueueing = true;                                   
                                    // cur_vehicle->commRadius = min(safeDistance(cur_vehicle, target_vehicle), dis);
                                    
                                    cur_vehicle->slot_oneHop[cur_packet->slot] = -1;
                                    cur_vehicle->slot_oneHop[TAIL] = cur_vehicle->id;
                                }
                                else{                               //cur_vehicle becomes the new head of left queue
                                    cur_vehicle->slot_occupied = HEAD;
                                    cur_vehicle->slot_condition = 1;
                                    // cur_vehicle->isQueueing = true;                                   
                                    // cur_vehicle->commRadius = min(safeDistance(cur_vehicle, target_vehicle), dis);
                                    
                                    cur_vehicle->slot_oneHop[cur_packet->slot] = -1;
                                    cur_vehicle->slot_oneHop[HEAD] = cur_vehicle->id;                                 
                                } 
                            }
                            else{
                                //update one-hop neighbor 
                                cur_vehicle->slot_oneHop[cur_packet->slot] = target_vehicle->id;
                                cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;
                                cur_vehicle->slot_condition = 2;
                            }
                        }
                        else if (cur_vehicle->isQueueing && cur_packet->isQueueing && target_vehicle->slot_oneHop[cur_slot] != cur_vehicle->id){ //2:two cars in two queues
                            if (cur_vehicle->slot_occupied == 0 ){ //cur_vehicle as a head accessing target_vehicle as a tail
                                cur_vehicle->slot_occupied = randSlot(cur_vehicle->slot_twoHop, 1);
                                cur_vehicle->slot_condition = 1;   
                                // cur_vehicle->isQueueing = true;  
                                cur_vehicle->commRadius = min(cur_vehicle->commRadius, dis);

                                cur_vehicle->slot_oneHop[HEAD] = -1;
                                // cur_vehicle->slot_oneHop[TAIL] = target_vehicle->id;
                                
                            }
                            else{   //target_vehicle as a head accessing cur_vehicle as a tail
                                cur_vehicle->slot_occupied = randSlot(cur_vehicle->slot_twoHop, -1);
                                cur_vehicle->slot_condition = 1;   
                                // cur_vehicle->isQueueing = true;  
                                cur_vehicle->commRadius = max(safeDistance(cur_vehicle, target_vehicle), dis);
                                cur_vehicle->isExpansion = true;
                                // cur_vehicle->slot_oneHop[HEAD] = cur_vehicle->id;
                                
                                cur_vehicle->slot_oneHop[TAIL] = -1;
                            }
                        }
                        else if (cur_vehicle->isQueueing && cur_packet->isQueueing == false){   //3:queue accessing a single vehicle
                            if (cur_vehicle->slot_occupied == 0 ){ //cur_vehicle as a head accessing target_vehicle as a single car
                                cur_vehicle->slot_occupied = randSlot(cur_vehicle->slot_twoHop);
                                cur_vehicle->slot_condition = 1;   
                                // cur_vehicle->isQueueing = true;  
                                cur_vehicle->commRadius = min(cur_vehicle->commRadius, dis);

                                cur_vehicle->slot_oneHop[HEAD] = target_vehicle->id;
                                // cur_vehicle->slot_oneHop[TAIL] = target_vehicle->id;
                                
                            }
                            else{   //target_vehicle as a single car accessing cur_vehicle as a tail
                                cur_vehicle->slot_occupied = randSlot(cur_vehicle->slot_twoHop);
                                cur_vehicle->slot_condition = 1;   
                                // cur_vehicle->isQueueing = true;  
                                cur_vehicle->commRadius = max(safeDistance(cur_vehicle, target_vehicle), dis);
                                cur_vehicle->isExpansion = true;

                                // cur_vehicle->slot_oneHop[HEAD] = cur_vehicle->id;
                                cur_vehicle->slot_oneHop[TAIL] = target_vehicle->id;
                            }
                        } 
                        else if (cur_vehicle->isQueueing == false && cur_packet->isQueueing){   //4:a single vehicle accessing queue
                            if (cur_packet->slot == HEAD ){ //target_vehicle as a head accessing cur_vehicle as a single car
                                cur_vehicle->slot_occupied = HEAD;
                                cur_vehicle->slot_condition = 1;   
                                cur_vehicle->isQueueing = true;  
                                cur_vehicle->commRadius = max(safeDistance(cur_vehicle, NULL), dis);

                                cur_vehicle->slot_oneHop[HEAD] = cur_vehicle->id;
                                // cur_vehicle->slot_oneHop[TAIL] = target_vehicle->id;
                            }
                            else{   //cur_vehicle as a single car accessing target_vehicle as a tail
                                cur_vehicle->slot_occupied = TAIL;
                                cur_vehicle->slot_condition = 1;
                                cur_vehicle->isQueueing = true;
                                cur_vehicle->commRadius = min(safeDistance(cur_vehicle, target_vehicle), dis)

                                // cur_vehicle->slot_oneHop[HEAD] = cur_vehicle->id;
                                cur_vehicle->slot_oneHop[TAIL] = cur_vehicle->id;
                            }
                        }
                        else if (cur_vehicle->isQueueing == false && cur_packet->isQueueing == false){  //5:a single vehicle accessing a single vehicle
                            
                            if(curInFront(cur_vehicle, target_vehicle)){    //cur_vehicle becomes the head of this new queue
                                cur_vehicle->slot_occupied = HEAD;
                                cur_vehicle->slot_condition = 1;
                                cur_vehicle->isQueueing = true;                                   
                                cur_vehicle->commRadius = max(safeDistance(cur_vehicle, NULL), dis);
                                
                                cur_vehicle->slot_oneHop[HEAD] = cur_vehicle->id;
                                cur_vehicle->slot_oneHop[TAIL] = target_vehicle->id;
                            }
                            else{                               //cur_vehicle becomes the new tail of this new queue
                                cur_vehicle->slot_occupied = TAIL;
                                cur_vehicle->slot_condition = 1;
                                cur_vehicle->isQueueing = true;                                   
                                cur_vehicle->commRadius = min(safeDistance(cur_vehicle, target_vehicle), dis);
                                
                                cur_vehicle->slot_oneHop[HEAD] = target_vehicle->id;
                                cur_vehicle->slot_oneHop[TAIL] = cur_vehicle->id; 
                                
                            }
                        }
                    }
                }
                else{
                    v.slot_occupied = randSlot(NULL);        //6:only one lonely vehicle
                    v.slot_condition = 1;
                }
            }
            for(auto& v : vehicles){    //determine comm. radius
                
                for(auto& p = v.neighbours->head; p != NULL; p = p->next){
                    double temp = max(safeDistance(v, *(p->datap)), distance_in_meter(v, *(p->datap)));
                    v.commRadius = max(v.commRadius, temp);
                }
            }
        }
        else if(slot % UpLocSlot == 0){ //Update the vehicle info. per 5 ms
            
            cout<<"Updating vehilces..."<<endl;
            read_and_compare(allCars);
            cout<<"Vehicles have been Updated."<<endl;
        }

        


        

        


        //Thirdly, determine TX & RX
        //-------Communication with selected communication range and slot--------//
        //handle the transmitter 检查通信半径范围内的车，如果 
        struct Duallist AllCollisions;
        handle_transmitter(aRegion, &AllCollisions, slot);
        //handle the receiver
        handle_receiver(aRegion, &AllCollisions, slot);
        
        //log collisions
        if(slot == seq_num*2000-1)                          //如果一次性写入内存爆掉，则放到循环里多次写入
            log_collisions(aRegion,&AllCollisions);
    }  
    return 0;
}

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
                
                if(aCar->packets.nItems == 0) continue; //没有收到包
                else if(aCar->packets.nItems == 1){//只收到一个pkt，则正常收包，update OHN 和THN
                    bItem =  (struct Item*)aCar->packets.head;
                    struct packet* pkt= (struct packet*)bItem->datap;
                    int index = pkt->slot, value = pkt->srcVehicle->id;
                    aCar->slot_oneHop[index] = value;
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
