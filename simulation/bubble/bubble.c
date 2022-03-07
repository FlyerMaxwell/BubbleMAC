// #include <iostream>
// #include <unordered_map>
// #include <vector>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "function.h"
#include "trace.h"

// using namespace std;

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))



// argv[0]: name of program, argv[1]: choose a map, argv[2]: define para or not
int main(int argc, char *argv[]) {

    FILE *fsource;
    struct Region *aRegion = NULL;
    int seq_num = 0, traffic_density = 40;

    
    if (argc == 1) {
	    printf("Error: There is no map. Please define a map as:  ./%s XXX.map\n",argv[0]);
	    exit(1);
    }else if(argc == 2){
        printf("Input traffic data sequence number (seq_num) : \n");
        scanf("%d", &seq_num);     //每段数据长度为10s，每5ms记录一次，合计2000条
        printf("Input traffic density (traffic density): \n");
        scanf("%d", &traffic_density);
    }

    // Load the map
    printf("Loading the map...\n");
    fsource=fopen(argv[1], "rb");
    aRegion = region_load_func(fsource, NULL, -1);
    fclose(fsource);
    printf("The map %s has been loaded.\n");

    // Simulation in sequence style.
	for (slot =(seq_num-1)*2000; slot<seq_num*2000; slot++){
        /*
        ____________________________________Separate Line by Fc Hu_________________________________________________________
        */
        int cur_slot = slot % SlotPerFrame;
        //Firstly, update vehicles' location every UpLocSlot slots, e.g. 1 packet per frame
        if (slot % SlotPerFrame == 0) { //Reload the map per 100ms
            printf("Initializing the simulation...\n");
            // allCars.clear();
            init_simulation(aRegion);
            printf("Initialization have finished.\n");

            printf("Loading vehilces...\n");
            generate_car(aRegion);
            printf("Vehicles have been loaded.\n");
            
            printf("Handling neighbors for each vehicle...\n");
            handle_neighbours(aRegion);        // 哪些是车辆潜在的neighbors（根据距离，九宫格判断，只需要比较这些车辆，不需要比较整张地图的车辆）
            printf("Neighbors have been handled.\n");

            //Secondly, determine the slot as well as comm. radius at the beginning of each frame

            struct Cell *aCell, *nCell;
            struct Item *aItem, *nItem;
            struct vehicle* cur_vehicle;

            for(int i = 0; i<aRegion->hCells; i++){         //determine slot
                for(int j = 0; j<aRegion->vCells;j++) {
                    aCell = aRegion->mesh + i*aRegion->vCells + j;
                    if (aCell->cars.head == NULL) continue;
                    aItem = aCell->cars.head;
                    while(aItem!=NULL){
                        cur_vehicle = (struct vehicle*)aItem->datap;
                        switch(cur_vehicle->car_role){
                            case ROLE_SINGLE:{
                                if(cur_vehicle->slot_condition == 0){   //单车无申请
                                    cur_vehicle->slot_occupied = rand()%(SlotPerFrame - 1) + 1;
                                    cur_vehicle->slot_condition = 1;
                                }
                                else if(cur_vehicle->slot_condition == 1){  //单车申请时槽
                                    if(cur_vehicle->packets.head == NULL){
                                        cur_vehicle->slot_occupied = rand()%(SlotPerFrame - 1) + 1; //无人回复，单车随机一个新时槽
                                    }
                                    else{           //Merge
                                        for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){
                                            struct packet* cur_packet = (struct packet*)p->datap;
                                            struct vehicle* target_vehicle = cur_packet->srcVehicle;
                                            double dis = vehicleDistance(cur_vehicle, target_vehicle);
                                            
                                            if(curInFront(cur_vehicle, target_vehicle)){ //当前车在前，申请成为新车头
                                                cur_vehicle->slot_occupied = HEAD_SLOT;
                                                cur_vehicle->slot_condition = 1;
                                                cur_vehicle->car_role = ROLE_HEAD;
                                                cur_vehicle->commRadius = max(safeDistance(cur_vehicle, NULL), dis);
                                                cur_vehicle->radius_flag = true;

                                                cur_vehicle->slot_oneHop[HEAD_SLOT] = cur_vehicle->id;

                                                if(target_vehicle->car_role == ROLE_SINGLE){
                                                    cur_vehicle->slot_oneHop[TAIL_SLOT] = target_vehicle->id;
                                                }

                                            }
                                            else{                                       //当前车在后，申请成为新车尾
                                                cur_vehicle->slot_occupied = TAIL_SLOT;
                                                cur_vehicle->slot_condition = 1;
                                                cur_vehicle->car_role = ROLE_TAIL;
                                                cur_vehicle->commRadius = min(safeDistance(cur_vehicle, target_vehicle), dis);
                                                cur_vehicle->radius_flag = true;
                                            
                                                cur_vehicle->slot_oneHop[TAIL_SLOT] = cur_vehicle->id;

                                                if(target_vehicle->car_role == ROLE_SINGLE){
                                                    cur_vehicle->slot_oneHop[HEAD_SLOT] = target_vehicle->id;
                                                }

                                            }
                                            break;                                       
                                        }
                                    }
                                }
                                
                                break;
                            }
                            case ROLE_HEAD:{
                                if(cur_vehicle->packets.head = NULL){
                                    degrade(cur_vehicle);
                                    break;
                                }
                                if(cur_vehicle->slot_condition == 1){   //车头申请时槽
                                    applyForSlot(cur_vehicle);
                                }
                                else{   //车头已确认时槽
                                    for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){        //申请成功
                                        struct packet* cur_packet = (struct packet*)p->datap;
                                        struct vehicle* target_vehicle = cur_packet->srcVehicle;
                                        double dis = vehicleDistance(cur_vehicle, target_vehicle);

                                        if(dis > safeDistance(cur_vehicle, NULL)){  //断链
                                            degrade(cur_vehicle);
                                        }
                                        else if(cur_packet->slot_resource_oneHop_snapShot[HEAD_SLOT] != cur_vehicle->id){   //Merge
                                            cur_vehicle->slot_occupied = randSlot(cur_vehicle->slot_oneHop, -1);
                                            cur_vehicle->slot_condition = 1;
                                            cur_vehicle->car_role = ROLE_MID;
                                            cur_vehicle->commRadius = min(safeDistance(cur_vehicle, target_vehicle), dis);
                                            cur_vehicle->radius_flag = true;
                                            
                                            cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;

                                            if(target_vehicle->car_role == ROLE_SINGLE){
                                                cur_vehicle->slot_oneHop[HEAD_SLOT] = target_vehicle->id;
                                            }                                            
                                            // function(1, 2, 3, 4, 5, 6)
                                        }
                                    }
                                }
                                break;
                            }
                            case ROLE_TAIL:{
                                if(cur_vehicle->packets.head = NULL){
                                    degrade(cur_vehicle);
                                    break;
                                }
                                if(cur_vehicle->slot_condition == 1){//车尾申请时槽
                                    applyForSlot(cur_vehicle);
                                }
                                else{//车尾已确认时槽
                                    for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){        //申请成功
                                        struct packet* cur_packet = (struct packet*)p->datap;
                                        struct vehicle* target_vehicle = cur_packet->srcVehicle;
                                        double dis = vehicleDistance(cur_vehicle, target_vehicle);

                                        if(dis > safeDistance(cur_vehicle, NULL)){  //断链
                                            degrade(cur_vehicle);                                     
                                        }
                                        else if(cur_packet->slot_resource_oneHop_snapShot[HEAD_SLOT] != cur_vehicle->id){   //Merge
                                            cur_vehicle->slot_occupied = randSlot(cur_vehicle->slot_twoHop, 1);    //车尾扩张通信半径，用twoHop
                                            cur_vehicle->slot_condition = 1;
                                            cur_vehicle->car_role = ROLE_MID;
                                            cur_vehicle->commRadius = max(safeDistance(cur_vehicle, target_vehicle), dis);
                                            cur_vehicle->radius_flag = true;
                                            
                                            cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;

                                            if(target_vehicle->car_role == ROLE_SINGLE){
                                                cur_vehicle->slot_oneHop[TAIL_SLOT] = target_vehicle->id;
                                            }                                            
                                        }
                                    }
                                }
                                break;
                            }
                            case ROLE_MID:{
                                if(cur_vehicle->packets.head = NULL){
                                    degrade(cur_vehicle);
                                    break;
                                }
                                if(cur_vehicle->slot_condition == 1){//车中申请时槽
                                    applyForSlot(cur_vehicle);
                                }
                                else{//车中已确认时槽
                                        for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){        //申请成功
                                        struct packet* cur_packet = (struct packet*)p->datap;
                                        struct vehicle* target_vehicle = cur_packet->srcVehicle;
                                        double dis = vehicleDistance(cur_vehicle, target_vehicle);

                                        if(dis > safeDistance(cur_vehicle, NULL)){  //断链
                                            if(curInFront(cur_vehicle, target_vehicle)){
                                                cur_vehicle->slot_occupied = TAIL_SLOT;     
                                                cur_vehicle->slot_condition = 1;
                                                cur_vehicle->car_role = ROLE_TAIL;

                                                struct vehicle* front = (struct vehicle*)cur_vehicle->frontV.head->datap;
                                                
                                                cur_vehicle->commRadius = min(safeDistance(cur_vehicle, front),vehicleDistance(cur_vehicle, front));
                                                cur_vehicle->radius_flag = true;

                                                cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;
                                            }
                                            else{
                                                cur_vehicle->slot_occupied = HEAD_SLOT;
                                                cur_vehicle->slot_condition = 1;
                                                cur_vehicle->car_role = ROLE_HEAD;

                                                struct vehicle* rear = (struct vehicle*)cur_vehicle->frontV.head->datap;
                                                
                                                cur_vehicle->commRadius = max(safeDistance(cur_vehicle, rear), vehicleDistance(cur_vehicle, rear));
                                                cur_vehicle->radius_flag = true;

                                                cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            default: {printf("default\n");break;}
                        }
                    
                    }
                }
            }
            
            for(int i = 0; i<aRegion->hCells; i++){         //determine comm. radius
                for(int j = 0; j<aRegion->vCells;j++) {
                    aCell = aRegion->mesh + i*aRegion->vCells + j;
                    if (aCell->cars.head == NULL) continue;
                    aItem = aCell->cars.head;
                    while(aItem!=NULL){
                        cur_vehicle = (struct vehicle*)aItem->datap;
                        
                        if(cur_vehicle->radius_flag == true){
                            cur_vehicle->radius_flag = false;
                        }
                        else{
                            switch (cur_vehicle->car_role){
                                case ROLE_SINGLE:{
                                    cur_vehicle->commRadius = safeDistance(cur_vehicle, NULL);
                                    break;
                                }
                                case ROLE_HEAD:{
                                    cur_vehicle->commRadius = safeDistance(cur_vehicle, NULL);
                                    break;
                                }
                                case ROLE_TAIL:{
                                    if(cur_vehicle->frontV.head != NULL){
                                        cur_vehicle->commRadius = vehicleDistance(cur_vehicle, (struct vehicle*)cur_vehicle->frontV.head->datap);
                                    }
                                    break;
                                }
                                case ROLE_MID:{
                                    if(cur_vehicle->frontV.head != NULL && cur_vehicle->rearV.head != NULL){
                                        cur_vehicle->commRadius = max(vehicleDistance(cur_vehicle, (struct vehicle*)cur_vehicle->frontV.head->datap),
                                                                        vehicleDistance(cur_vehicle, (struct vehicle*)cur_vehicle->rearV.head->datap));
                                    }
                                    break;
                                }
                            }
                        }
            
            
                    }
                }
            }
            
        }
        else if(slot % UpLocSlot == 0){ //Update the vehicle info. per 5 ms
            
            printf("Updating vehilces...\n");
            generate_car(aRegion);
            printf("Vehicles have been Updated.\n");
        }
        

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

