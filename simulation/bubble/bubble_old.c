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
    vehicle test;
    FILE *fsource;
    struct Region *aRegion = NULL;
    int seq_num = 0, traffic_density = 40;
    const int UpLocSlot = 10;
    const int SlotPerFrame = 200;
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
                struct vehicle* cur_v = iter->second;
                cur_v->commRadius = 0;
                if(cur_v->slot_occupied != NOT_OCCUPIED){
                    for(auto p = cur_v->packets->head; p != NULL; p = p->next){
                        struct packet* cur_p = p->datap;
                        struct vehicle* target_v = cur_p->srcVehicle;
                        double dis = vehicleDistance(cur_v, target_v);
                        if (cur_v->isQueueing && cur_p->isQueueing && target_v->slot_oneHop[cur_slot] == cur_v->id){ //1:two cars in one queue
                            dis = vehicleDistance(cur_v, target_v);
                            if (dis > THRESHOLD){
                                //断链 
                            }
                            continue;
                        }
                        else if (cur_v->isQueueing && cur_p->isQueueing && target_v->slot_oneHop[cur_slot] != cur_v->id){ //2:two cars in two queues
                            if (cur_v->slot_occupied == 0 ){
                                cur_v->commRadius = max(cur_v->commRadius, dis);
                                
                            }
                        }0
                        else if (cur_v->isQueueing && cur_p->isQueueing == false){   //3:head accessing a single vehicle
                            //
                        } 
                        else if (cur_v->isQueueing == false && cur_p->isQueueing){   //4:a single vehicle accessing tail
                            if (target_v->slot_occupied == 0 ){ //target_c accessing cur_v
                                cur_v->commRadius = max(cur_v->commRadius, dis);
                                
                            }
                        }
                        else if (cur_v->isQueueing == false && cur_p->isQueueing == false){  //5:a single vehicle accessing a single vehicle
                            
                            if(curInFront(cur_v, target_v)){    //cur_v becomes the head of this new queue
                                cur_v->slot_occupied = 0;
                                cur_v->slot_condition = 2;
                                cur_v->isQueueing = true;                                   
                                cur_v->commRadius = safeDistance(cur_v, NULL);
                                
                                cur_v->slot_oneHop[SlotPerFrame - 1] = target_v->id; 
                            }
                            else{                               //cur_v becomes the new tail of this new queue
                                cur_v->slot_occupied = SlotPerFrame - 1;
                                cur_v->slot_condition = 2;
                                cur_v->isQueueing = true;                                   
                                cur_v->commRadius = max(safeDistance(cur_v, target_v), dis);
                                
                                cur_v->slot_oneHop[0] = target_v->id; 
                            }
                        }
                    }
                }
                else{
                    v.slot_occupied = rand()%(SlotPerFrame - 1) + 1;        //6:only one lonely vehicle
                    v.slot_condition = 1;
                }
            }
            for(auto& v : vehicles){    //determine comm. radius
                
                for(auto& p = v.neighbours->head; p != NULL; p = p->next){
                    double temp = max(safeDistance(v, *(p->datap)), 
                                    distance_in_meter(v, *(p->datap)));
                    v.commRadius = max(v.commRadius, temp);
                }
            }
        }
        else if(slot % UpLocSlot == 0){ //Update the vehicle info. per 5 ms
            
            cout<<"Updating vehilces..."<<endl;
            read_and_compare(allCars);
            cout<<"Vehicles have been Updated."<<endl;
        }

        int cur_slot = slot % SlotPerFrame;
        const int NOT_OCCUPIED = -1;
        const double BRAKE_COE = 0.5;
        

        //Thirdly, determine TX & RXasdadas
  
    }

    

    return 0;
}

