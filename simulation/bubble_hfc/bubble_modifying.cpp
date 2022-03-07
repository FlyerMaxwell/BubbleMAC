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
            
            cout<<"Handling neighbors for each vehicle..."<<endl;
            // handle_neighbour(aRegion);        // 哪些是车辆潜在的neighbors（根据距离，九宫格判断，只需要比较这些车辆，不需要比较整张地图的车辆）
            cout<<"Neighbors have been handled."<<endl;

            //Secondly, determine the slot as well as comm. radius at the beginning of each frame
            for(auto iter = allCars.begin(); iter != allCars.end(); iter++){    //determine slot
                struct vehicle* cur_vehicle = iter->second;
                cur_vehicle->commRadius = 0;
                //*********************************************separate line*********************************************

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
                            else{
                                for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){
                                    struct packet* cur_packet = (struct packet*)p->datap;
                                    struct vehicle* target_vehicle = cur_packet->srcVehicle;
                                    double dis = vehicleDistance(cur_vehicle, target_vehicle);
                                    if(curInFront(cur_vehicle, target_vehicle)){    //当前车在前，申请成为新车头
                                        cur_vehicle->slot_occupied = HEAD_SLOT;
                                        cur_vehicle->car_role = ROLE_HEAD;
                                        cur_vehicle->slot_condition = 1;
                                        cur_vehicle->commRadius = max(safeDistance(cur_vehicle, NULL), dis);
                                        cur_vehicle->radius_flag = true;

                                        cur_vehicle->slot_oneHop[HEAD_SLOT] = cur_vehicle->id;
                                        cur_vehicle->slot_oneHop[TAIL_SLOT] = target_vehicle->id;
                                    }
                                    else{                               //当前车在后，申请成为新车尾
                                        cur_vehicle->slot_occupied = TAIL_SLOT;
                                        cur_vehicle->car_role = ROLE_TAIL;
                                        cur_vehicle->slot_condition = 1;
                                        cur_vehicle->commRadius = min(safeDistance(cur_vehicle, target_vehicle), dis);
                                        cur_vehicle->radius_flag = true;

                                        cur_vehicle->slot_oneHop[HEAD_SLOT] = cur_vehicle->id;
                                        cur_vehicle->slot_oneHop[TAIL_SLOT] = target_vehicle->id;
                                    }        

                                }
                            }
                        }
                        
                        break;
                    }
                    case ROLE_HEAD:{
                        if(cur_vehicle->slot_condition == 1){   //车头申请时槽

                        }
                        else{   //车头已确认时槽

                        }
                        break;
                    }
                    case ROLE_TAIL:{
                        if(cur_vehicle->slot_condition == 1){//车尾申请时槽

                        }
                        else{//车尾已确认时槽

                        }
                        break;
                    }
                    case ROLE_MID:{
                        if(cur_vehicle->slot_condition == 1){//车中申请时槽

                        }
                        else{//车中已确认时槽

                        }
                        break;
                    }
                    default: {printf("default\n");break;}
                }

                //*********************************************separate line*********************************************
                if(cur_vehicle->slot_occupied != NOT_OCCUPIED){
                    if (cur_vehicle->packets.head == NULL){
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

