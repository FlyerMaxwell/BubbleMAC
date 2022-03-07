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
    struct Duallist AllCollisions;
    int seq_num = 3, traffic_density = 40;

    char usedMap[20]="three_lane.map"; 
    
    // if (argc == 1) {
	//     printf("Error: There is no map. Please define a map as:  ./%s XXX.map\n",argv[0]);
	//     exit(1);
    // }else if(argc == 2){
    //     strcpy(usedMap,argv[1]);
    //     printf("Input traffic data sequence number (seq_num) : \n");
    //     scanf("%d", &seq_num);     //每段数据长度为10s，每5ms记录一次，合计2000条
    //     printf("Input traffic density (traffic density): \n");
    //     scanf("%d", &traffic_density);
    // }

    // Load the map
    printf("Loading the map...\n");
    fsource=fopen(usedMap, "rb");
    aRegion = region_load_func(fsource, NULL, -1);
    fclose(fsource);
    printf("The map %s has been loaded.\n",usedMap);
    printf("%lf\n",aRegion->cellSize);//cellsize错了，剑钢说暂不影响

    // Simulation in sequence style.
	for (slot = 0; slot < 20000; slot++){//carposition从4000-5999，合计2000个文件，10秒
        //Update vehicles' location.
        if(slot % (SlotPerFrame*10) == 0){  //每1s对参与统计的车辆进行更新
            generate_car(aRegion);
            handle_neighbours(aRegion);              
        } else if(slot % UpLocSlot == 0) { //5ms更新位置
            updateLoc(aRegion);
            handle_neighbours(aRegion);
        }

        // Determine the slot and comm. range at the beginning of each frame
        if(slot % SlotPerFrame == 0){
            bubble_mac_protocol(aRegion);
        }

        //handle the transmitter at each slot
        handle_transmitter(aRegion, &AllCollisions, slot);
        //handle the receiver at each slot
        handle_receiver(aRegion, &AllCollisions, slot);
        
    }  
        // Log collisions. 如果一次性写入文件太大，则放进循环多次写入;或直接写入文件
        log_collisions(aRegion,&AllCollisions);
    return 0;
}

