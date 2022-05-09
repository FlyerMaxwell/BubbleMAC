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


// argv[0]: name of program, argv[1]: choose a map, argv[2]: define para or not
int main(int argc, char *argv[]) {
    srand(0);       //固定随机数种子，获取稳定的结果

    int slot_start = 0;
    int slot_end = 10;
    int slot_step = 1;

    struct Duallist ALL_Vehicles;// 每个slot下的全部车辆，于一个链表中中
    duallist_init(&ALL_Vehicles);

    // Simulation in sequence style.
    for(int slot = slot_start; slot < slot_end; slot += slot_step){
        printf("slot = %d\n",slot);
        
        // init_simulation();          //在每个时刻需要初始化的东西
        
        if(slot % UpLocSlot == 0) { //每UpLocSlot更新运动位置
            init_simulation(&ALL_Vehicles);
            updateLocation(&ALL_Vehicles, slot);
            handle_neighbours(&ALL_Vehicles);
            printf("The location of vehicles has been updated, Current slot = %d\n", slot);
        }

        // Determine the slot and comm. range at the beginning of each frame
        // if(slot % SlotPerFrame == 0){
        //     bubble_mac_protocol();
        //     clearPackets();
        // }
        //handle the transmitter at each slot
        //handle_transmitter(aRegion, slot);
        //handle the receiver at each slot
        //handle_receiver(aRegion, slot);
    }
    // printf("Car/km:%d\n Total Cars: %d\n Total Transmited Packets: %d\n Total Received Packet: %d\n Total Collisions: %d\n ", traffic_density,Car_Number,cnt_pkt, cnt_received, cnt_coli);
        
    return 0;
}

