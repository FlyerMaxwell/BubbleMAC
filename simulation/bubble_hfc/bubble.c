#include <iostream>
#include<unordered_map>
#include<vector>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "function.h"

using namespace std;


/*
    modified by hfc, 2022.3.4.
*/

// argv[0]: name of program, argv[1]: choose a map, argv[2]: define para or not
int main(int argc, char *argv[]) {
 
    FILE *fsource;
    struct Region *aRegion = NULL;
    int seq_num = 0, traffic_density = 40;
    const int UpLocSlot = 150;
    const int SlotPerFrame = 100;
    unordered_map<int, vehicle*> allCars;

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
        
        if (slot % UpLocSlot == 0) {//
            cout<<"Initializing the simulation..."<<endl;
            init_simulation(aRegion);
            cout<<"Initialization have finished."<<endl;

            cout<<"Loading vehilces..."<<endl;
            generate_car(aRegion, slot, allCars);
            cout<<"Vehicles have been loaded."<<endl;
            
            // cout<<"Handling neighbors for each vehicle..."<<endl;
            // handle_neighbour(aRegion);        // 哪些是车辆潜在的neighbors（根据距离，九宫格判断，只需要比较这些车辆，不需要比较整张地图的车辆）
            // cout<<"Neighbors have been handled."<<endl;
        }

        vehicle vehicles[10];
        const int NOT_OCCUPIED = -1;
        const double BRAKE_COE = 0.5;
        double safeDistance(const vehicle& v1, const vehicle& v2){
            float lou = 0.5;
            return ((v1.v * v1.v) - (v2.v * v2.v))/(2 * v1.accelarate * BRAKE_COE));
        }

        /*
        struct vehicle{
            int slot_occupied = 0;
            int slot_condition = 0;
            double commRadius = 0;
            Packet* packet = NULL;
        };
        */

        //Firstly, update vehicles' location every UpLocSlot slots, e.g. 1 packet per frame
        for(auto& veh : vehicles){
            filename = "carposition_" + tostring(density) + "_" + tostring(slot) + ".txt";
            fscanf(carinfo, "%d %lf %lf %lf %lf %lf %d\n", aCar->id, x, y, x1, y1, aCar->v, aCar->belongLane->id);
        }
        //Secondly, determine the slot as well as comm. radius at the beginning of each frame
        for(auto& v : vehicles){    //determine slot
            if(v.slot_occupied != NOT_OCCUPIED){
                int mergeID = -1, divideID = -1;
                for(auto p = v.packet->head; p != NULL; p = p->next){
                    if (p->datap->OHN[slot] == 0){
                        mergeID = p->datap->srcID;
                        merge(v, mergeID);
                    }
                }
                if(Merge){
                        //Actions for merging cases
                }
                else if(BrokenChain){
                        //Actions for broken chain
                }
                else{

                }
            }
            else{
                    v.slot_occupied = rand(0, );
                    v.slot_condition = 1;
            }
        }
        for(auto& v : vehicles){    //determine comm. radius
            v.commRadius = 0;
            for(auto& p = v.neighbours->head; p != NULL; p = p->next){
                double temp = max(safeDistance(v, *(p->datap)), 
                                distance_in_meter(v, *(p->datap)));
                v.commRadius = max(v.commRadius, temp);
            }
        }
        //Thirdly, determine TX & RX
  
    }

    

    return 0;
}

