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

int init_simulation(struct Duallist *ALL_Vehicles){
	struct Item *aItem;
	struct vehicle *aCar;

	
	aItem = ALL_Vehicles->head;
	while (aItem != NULL){
		aCar = (struct vehicle*)aItem->datap;
//---需要初始化的内容---//
	    aCar->handled = 0;	
//---需要初始化的内容---//
        aItem = aItem->next;	
	}

	return 0;
}


void updateLocation(struct Duallist *ALL_Vehicles, int slot){
    FILE *fin = NULL;
    int flag;
    int timestep;
    struct Item *aItem, *bItem, *tItem,*nItem;
    struct neighbour_car* tNeigh, *nNeigh, *bNeigh;
    struct vehicle *aCar, *bCar;
    int car_count = 0;
    
    printf("Loading vehilces...\n");

    sprintf(fin, "/media/lion/Elements SE/Maxwell/BubbleMAC/SumoSimu/MyCross/transformed/carposition_%d.txt", slot);
    fin = fopen(fin, "r");

    //读取文件，添加车辆
    while(fscanf(fin, "%d", &timestep)!=-1){
        struct vehicle *new_car;
		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));
        
        //读取一行数据到new_Car中
        fscanf(fin, "%s", new_car->id);
        fscanf(fin, "%lf", &new_car->x);
        fscanf(fin, "%lf", &new_car->y);
        fscanf(fin, "%lf", &new_car->angle);
        fscanf(fin, "%s", new_car->type);
        fscanf(fin, "%lf", &new_car->speed);
        fscanf(fin, "%lf", &new_car->pos);
        fscanf(fin, "%s", new_car->lane);
        fscanf(fin, "%lf", &new_car->slope);
        fscanf(fin, "%lf", &new_car->flow);
        fscanf(fin, "%lf", &new_car->speed2);

        new_car->slot_oneHop = (int*)malloc(sizeof(int)*SlotPerFrame);
		new_car->slot_twoHop = (int*)malloc(sizeof(int)*SlotPerFrame);

        for(int ii = 0; ii < SlotPerFrame; ii++){
            new_car->slot_oneHop[ii] = -1;
            new_car->slot_twoHop[ii] = -1;
        }

        new_car->handled = 2;//新车

        duallist_init(&new_car->packets);
		duallist_init(&new_car->neighbours);
		duallist_init(&new_car->frontV);
		duallist_init(&new_car->rearV);


        //查找new_Car是否已经存在， 若存在，flag=true；若不存在，则flag = false;遍历一次ALL_Vehicles双链表，看是否已经存在（id是否相等），若相等则flag=true；若不相等，则flag=false
		flag = false;
        bItem = (struct Item*)ALL_Vehicles->head;
        while(bItem != NULL){
            bCar = (struct vehicle*)bItem->datap;
            if (!strcmp(bCar->id, new_car->id)) {flag = true;break;}
            bItem = bItem->next;
        }

        //若之前已存在，则更新其运动学信息
        if(flag == true){
            bCar->x = new_car->x;
            bCar->y = new_car->y;
			bCar->speed = new_car->speed;
            bCar->handled = 1;//已有的车辆且处理
            duallist_pick_item(ALL_Vehicles, bItem);//这样做也没错，只是没必要。。先不改了
			duallist_add_to_tail(ALL_Vehicles, bCar);//添加更新后的节点
			free(new_car);
        }
        //若之前不存在，如果处于要更新车辆的时刻，则更新车辆，否则不更新车辆
        // if (flag == false && slot % (SlotPerFrame*nFrameSta) == 0) duallist_add_to_tail(ALL_Vehicles, new_car);
		// if (flag == false && slot % (SlotPerFrame*nFrameSta) != 0) free(new_car);
        if (flag == false)
            duallist_add_to_tail(ALL_Vehicles, new_car);
    }
    
    
    //处理那些消失掉的车，目前的版本是，在每个updateLoc都清除掉那些消失的车
    aItem = ALL_Vehicles->head;
    while(aItem != NULL){
        aCar = (struct vehicle*)aItem->datap;
        tItem = aItem;
		aItem = aItem->next;
        if (aCar->handled == 0) {
            bItem = aCar->history_neigh.head;
            while (bItem !=NULL) {
                bNeigh = (struct neighbour_car*)bItem->datap;
                bCar = (struct vehicle*)bNeigh->carItem->datap;
                nItem = bCar->history_neigh.head;
                while (nItem !=NULL) {
                    nNeigh = (struct neighbour_car*)nItem->datap;
                    if (strcmp(nNeigh->car_id ,aCar->id)) {duallist_pick_item(&bCar->history_neigh, nItem); break;}
                    nItem = nItem->next;
                    }
                    bItem = bItem->next;
            }
            duallist_pick_item(ALL_Vehicles, tItem); 
            free(aCar);
        }
        else car_count++;
    }

    fclose(fin);

    printf("\ntotal car number in this slot: %d\n", car_count);
    printf("Vehicles have been loaded!\n");
    return;
};


//handle neighbors： 处理邻居，将所有车辆的所在的九宫格内的车挂载到其潜在的neighbors中,即每个车辆的neighbors就是当前九宫格内的邻居。暴力，两层循环。这里暴力是为了后面每次遍历的时候能少遍历一点
void handle_neighbours(struct Duallist *ALL_Vehicles){
    struct Cell *aCell, *nCell;
    struct Item *aItem, *nItem;
    struct vehicle* aCar, *nCar;

    aItem =ALL_Vehicles-> head;
    while(aItem != NULL){
        aCar = (struct vehicle*)aItem->datap;
        duallist_destroy(&(aCar->neighbours), NULL);//先把之前的清空掉
        nItem = ALL_Vehicles-> head;
        while(nItem != NULL){
            nCar = (struct vehicle*)nItem->datap;
            //id不相等且处于两千米以内，则将其加入到neighbors
            if(strcmp(nCar->id, aCar->id)!=0 && distance_between_vehicle(aCar,nCar) < 2000){
                duallist_add_to_tail(&(aCar->neighbours), nCar);//将id不同的车加入到neighbor list。
            }
            nItem = nItem->next;
        }
    }
};



// argv[0]: name of program, argv[1]: choose a map, argv[2]: define para or not
int main(int argc, char *argv[]) {
    srand(0);       //固定随机数种子，获取稳定的结果

    int slot_start = 0;
    int slot_end = 100;
    int slot_step = 1;

    struct Duallist ALL_Vehicles;// 每个slot下的全部车辆，于一个链表中中

    // Simulation in sequence style.
    for(int slot = slot_start; slot < slot_end; slot += slot_step){
        printf("slot = %d\n",slot);
        
        // init_simulation();          //在每个时刻需要初始化的东西
        
        if(slot % UpLocSlot == 0) {     //每UpLocSlot更新位置,每SlotPerFrame*nFrameSta添加新车(这没问题，防止丢车)
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


//输出某个节点的neighbors
        // struct Cell* aCell;
        // struct Item* aItem, *bItem;
        // struct vehicle* aCar,*bCar;
        //  for(int i = 0; i < aRegion->hCells; i++){       
        //     for(int j = 0; j<aRegion->vCells; j++) {
        //         aCell = aRegion->mesh + i*aRegion->vCells + j;
        //         aItem = aCell->cars.head;

        //     while (aItem != NULL){
        //         aCar = (struct vehicle*)aItem->datap;
                
        //         bItem = (struct Item*)aCar->neighbours.head;//遍历当前transmitter的邻居节点
        //         while (bItem != NULL) {
        //             bCar = (struct vehicle*)bItem->datap;
        //             if(slot==14 && aCar->id == 129 && bCar->id == 135) printf("Current bCar%d\n",bCar->id);
        //             bItem = bItem->next;
        //         }
        //         aItem = aItem->next;
        //     }
        //     }
        //  }


