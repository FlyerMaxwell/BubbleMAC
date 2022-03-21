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
    srand((unsigned int)time(0));
    FILE *fsource;
    struct Region *aRegion = NULL;
    srand(0);

    //默认参数
    int seq_num = 3, traffic_density = 40;//traffic_density是指单向车道上的车辆密度，双向车道上车道数*2
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
    printf("%lf\n",aRegion->cellSize);//cellsize错了，剑钢说暂不影响,three_lane.map的region尺寸为1 Cell*7 Cell。其中每个Cell为150m *150m

    // Simulation in sequence style.
	for (slot = 0; slot < 3; slot++){//carposition从4000-5999，合计2000个文件，10秒
        //Update vehicles' location.
        // printf("the slot is %d\n", slot);
        if(slot % UpLocSlot == 0) { //每UpLocSlot更新位置,每SlotPerFrame*nFrameSta添加新车
            init_simulation(aRegion);
            update_cars(aRegion);
            handle_neighbours(aRegion);
            printf("The simulator has been init, the neighbors have been handle, Current slot = %d\n", slot);
        }
        
        // // Determine the slot and comm. range at the beginning of each frame
        if(slot % SlotPerFrame == 0){
            bubble_mac_protocol(aRegion);
            clearPackets(aRegion);
        }

        //printf("Handling Tx and Rx..\n");
        //handle the transmitter at each slot
        handle_transmitter(aRegion, slot);
        //handle the receiver at each slot
        handle_receiver(aRegion, slot);
    }  
        
    return 0;
}

