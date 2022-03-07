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


// argv[0]: name of program, argv[1]: choose a map, argv[2]: define para or not
int main(int argc, char *argv[]) {

    FILE *fsource;
    struct Region *aRegion = NULL;
    int seq_num = 0, traffic_density = 40;
    struct Duallist AllCollisions;
    
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
    printf("The map %s has been loaded.\n",argv[1]);

    // Simulation in sequence style.
	for (slot =(seq_num-1)*2000; slot<seq_num*2000; slot++){
      
        int cur_slot = slot % SlotPerFrame;
        //Firstly, update vehicles' location every UpLocSlot slots, e.g. 1 packet per frame
        if (slot % SlotPerFrame == 0) { //Reload the map per 100ms
            printf("Initializing the simulation...\n");
            // allCars.clear();
            //init_simulation(aRegion);           //所有的车在frame的一开始需要进行哪些初始化就在这里做。并非要全部都做
            //printf("Initialization have finished.\n");

            printf("Loading vehilces...\n");
            generate_car(aRegion);              //读取文件，更新车辆位置信息
            printf("Vehicles have been loaded.\n");
            
            printf("Handling potential neighbors for each vehicle...\n");
            handle_neighbours(aRegion);          // 哪些是车辆潜在的neighbors（根据距离，九宫格判断，只需要比较这些车辆，不需要比较整张地图的车辆）
            printf("Neighbors have been handled.\n");

            //Secondly, determine the slot as well as comm. radius at the beginning of each frame
            bubble_mac_protocol(aRegion);
        }
        else if(slot % UpLocSlot == 0){ //Update the vehicle info. per 5 ms
            
            printf("Updating vehilces...\n");
            generate_car(aRegion);
            printf("Vehicles have been Updated.\n");
        }
        

        //-------Communication with selected communication range and slot--------//
        //handle the transmitter 检查通信半径范围内的车，如果 
        handle_transmitter(aRegion, &AllCollisions, slot);
        //handle the receiver
        handle_receiver(aRegion, &AllCollisions, slot);
        
        //log collisions
        if(slot == seq_num*2000-1)                          //如果一次性写入内存爆掉，则放到循环里多次写入
            log_collisions(aRegion,&AllCollisions);
    }  
    return 0;
}

