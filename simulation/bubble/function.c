// #pragma once
#include "function.h"
// #include <iostream>
// using namespace std;
//--------------YX below--------------------------------------//
int init_simulation(struct Region* region){
	struct Item *aItem;
	struct Cell *aCell;
	struct vehicle *aCar;

	for(unsigned int i = 0; i < region->hCells; i++){       
		for(unsigned int j = 0; j < region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
//-----------------------需要初始化的内容---------------------------//
				aCar->handled = 0;	

//-----------------------需要初始化的内容---------------------------//
                aItem = aItem->next;	
			}
		}
	}
	return 0;
}

void clearPackets(struct Region* region){
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
				duallist_destroy(&(aCar->packets),NULL);
                duallist_destroy(&(aCar->frontV),NULL);
                duallist_destroy(&(aCar->rearV),NULL);
//-----------------------需要初始化的内容---------------------------//
                aItem = aItem->next;	
			}
		}
	}
	// return 0;
}


void update_cars(struct Region *region) 		//new generate_car modified by hfc
{
	double xmin,ymin,xmax,ymax, x, y;
	int flag, i, j, k, l;
	struct Cell *aCell, *bCell;
	struct Item *aItem, *bItem, *tItem, *nItem;
	struct vehicle *bCar, *aCar, *tCar, *nCar;
	struct neighbour_car* tNeigh, *nNeigh, *bNeigh;
	FILE *carinfo;
	char file_path[100];
	int car_count=0;

	xmin = region->chosen_polygon->box.xmin;
	xmax = region->chosen_polygon->box.xmax;
	ymin = region->chosen_polygon->box.ymin;
	ymax = region->chosen_polygon->box.ymax;
    
    printf("Loading vehilces...\n");

	sprintf(file_path, "/home/lion/Yunxiang/vanet1.0/vanet1.0/car_position/carposition_%d_%d.txt", traffic_density, slot/UpLocSlot + 4000); //traffic density, slot number
	carinfo = fopen(file_path, "r");
	int carnum;
	fscanf(carinfo, "%d", &carnum);
	Car_Number = carnum;

	for (k=0; k< carnum; k++){
		struct vehicle *new_car;
		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));

        //读取一行数据到new_Car中
		fscanf(carinfo, "%d", &new_car->id);
		fscanf(carinfo, "%lf", &new_car->x);
		fscanf(carinfo, "%lf", &new_car->y);
		fscanf(carinfo, "%lf", &new_car->a);
		fscanf(carinfo, "%lf", &new_car->b);
		fscanf(carinfo, "%lf", &new_car->v);		
		fscanf(carinfo, "%lf", &new_car->belongLaneID);
		fscanf(carinfo, "%lf", &new_car->dir_x);
		fscanf(carinfo, "%lf", &new_car->dir_y);
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

        //找到new_Car对应的aCell
		flag = false;		
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->box.xmin <= new_car->x && aCell->box.xmax > new_car->x && aCell->box.ymax > new_car->y && aCell->box.ymin <= new_car->y) {
					flag=true;
					break;
				} //find the cell which contains the new car
			}
			if (flag==true) break;
		}
		new_car->belongCell = aCell;


    //查找new_Car是否已经存在， 若存在，flag=true；若不存在，则flag = false
		flag = false;
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				bCell = region->mesh + i*region->vCells + j;
				bItem = (struct Item*)bCell->cars.head;
				while (bItem != NULL){
					bCar = (struct vehicle*)bItem->datap;
					if (bCar->id == new_car->id) {flag = true;break;}
					bItem = bItem->next;
				}
				if (flag == true) break;
			}
			if (flag == true) break;
		}

        //若之前已存在，则更新其运动学信息及所归属的cell
		if (flag == true) {
			bCar->x = new_car->x;
			bCar->y = new_car->y;
			bCar->v = new_car->v;
			bCar->belongCell = new_car->belongCell;
            bCar->handled = 1;//已有的车辆且处理
			duallist_pick_item(&bCell->cars, bItem);
			duallist_add_to_tail(&aCell->cars, bCar);
			free(new_car);
		}
		if (flag == false && slot % (SlotPerFrame*nFrameSta) == 0) duallist_add_to_tail(&(aCell->cars), new_car);
		if (flag == false && slot % (SlotPerFrame*nFrameSta) != 0) free(new_car);
	}

    //处理那些消失掉的车，目前的版本是，在每个updateLoc都清除掉那些消失的车
    for(i = 0; i<region->hCells; i++){       
        for(j = 0; j<region->vCells;j++) {
            aCell = region->mesh + i*region->vCells + j;
            aItem = aCell->cars.head;
            while (aItem != NULL){
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
                            if (nNeigh->car_id ==aCar->id) {duallist_pick_item(&bCar->history_neigh, nItem); break;}
                            nItem = nItem->next;
                        }
                        bItem = bItem->next;
                    }
                    duallist_pick_item(&aCell->cars, tItem); 
                    free(aCar);
                }
                else car_count++;
            }
        }
	}

	fclose(carinfo);

    printf("\ntotal car number in this slot: %d\n", car_count);
    printf("Vehicles have been loaded!\n");
    return;
}


//handle neighbors： 处理邻居，将所有车辆的所在的九宫格内的车挂载到其潜在的neighbors中,即每个车辆的neighbors就是当前九宫格内的邻居。
void handle_neighbours(struct Region* region){
    struct Cell *aCell, *nCell;
    struct Item *aItem, *nItem;
    struct vehicle* aCar, *nCar;

    for(int i = 0; i<region->hCells; i++){       
		for(int j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
            printf("Current Cell is : %d %d\n", aCell->xNumber, aCell->yNumber);

			int neighbour_cell[9][2] = {{i-1,j-1}, {i,j-1}, {i+1,j-1}, {i-1,j}, {i,j}, {i+1,j}, {i-1,j+1}, {i,j+1}, {i+1,j+1}};
            if (aCell->cars.head == NULL) continue;
            aItem = aCell->cars.head;
            
            while(aItem != NULL){
                aCar = (struct vehicle*)aItem->datap;
                duallist_destroy(&(aCar->neighbours), NULL);//先把之前的清空掉

                for(int k = 0; k < 9 ; k++){
                    if ((neighbour_cell[k][0]<0 || neighbour_cell[k][0]>=region->hCells) || (neighbour_cell[k][1]<0 || neighbour_cell[k][1]>=region->vCells)) continue;

                    int tmpx = neighbour_cell[k][0], tmpy = neighbour_cell[k][1];
                    nCell = region->mesh + tmpx*region->vCells + tmpy;//找到当前的cell
    
                    nItem = nCell->cars.head;//遍历当前cell中的每个车
                    while(nItem != NULL){
                        nCar = (struct vehicle*)nItem->datap;
						if (nCar->id != aCar->id)  duallist_add_to_tail(&(aCar->neighbours), nCar);//将id不同的车加入到neighbor list。
                        if(aCar->id == 129 && nCar->id == 135) printf("--------nCar:%d %d\n", nCar->id, slot);
                        nItem = nItem->next;
                    }
                }
                aItem = aItem->next;
            }
        }
    }
}

//找到原因了，重复添加了neighbor。

double distance_between_vehicle(const struct vehicle* aCar, const struct vehicle* bCar){
    return sqrt((aCar->x - bCar->x)*(aCar->x - bCar->x) + (aCar->y - bCar->y)*(aCar->y - bCar->y));
}

//生成Collision， 记录type, slot, 两个车及两车对应的当前slot使用情况
struct collision* generate_collision(struct vehicle *aCar, struct vehicle *bCar,  int type, int slot){
    struct collision * coli;
    coli = (struct collision*)malloc(sizeof(struct collision));
    coli->type = type;
    coli->slot = slot;
    coli->src = aCar;
    coli->dst = bCar;

    coli->src_oneHop = (int*)malloc(sizeof(int)*SlotPerFrame);
    coli->src_twoHop = (int*)malloc(sizeof(int)*SlotPerFrame);
    coli->dst_oneHop = (int*)malloc(sizeof(int)*SlotPerFrame);
    coli->dst_twoHop = (int*)malloc(sizeof(int)*SlotPerFrame);
    
    for(int i = 0; i < SlotPerFrame; i++){
        coli->src_oneHop[i] = aCar->slot_oneHop[i];
        coli->src_twoHop[i] = aCar->slot_twoHop[i];
        coli->dst_oneHop[i] = bCar->slot_oneHop[i];
        coli->dst_twoHop[i] = bCar->slot_twoHop[i];
    }

    return coli;
}

//generate_packet
struct packet * generate_packet(struct vehicle *aCar, struct vehicle *bCar, int slot){
    struct packet* pkt;
    pkt = (struct packet*)malloc(sizeof(struct packet));
    pkt->slot = slot;
    pkt->condition = 0;//还没有发生碰撞
    pkt->srcVehicle = aCar;
    pkt->dstVehicle = bCar;
    pkt->isQueueing = aCar->isQueueing;
    pkt->slot_resource_oneHop_snapShot = (int*)malloc(sizeof(int)*SlotPerFrame);
    for(int i = 0; i < SlotPerFrame; i++){
        pkt->slot_resource_oneHop_snapShot[i] = aCar->slot_oneHop[i];
    }
    return pkt;
}


void log_packet(struct packet * aPkt, int slot){
    char output_collisions_path[100];
    FILE * Collisions_output;
    sprintf(output_collisions_path, "./simulation_result/bubble_packet_%d_%d.txt", SlotPerFrame, traffic_density);
    //printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%d\n",traffic_density);
    Collisions_output = fopen(output_collisions_path,"a");
    
    fprintf(Collisions_output,"No: %d, slot: %d, condition: %d\n", cnt_pkt++, slot, aPkt->condition);
    
    // src
    fprintf(Collisions_output,"src id: %d\n",aPkt->srcVehicle->id);
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",aPkt->srcVehicle->slot_oneHop[i]);
    // }
    // fprintf(Collisions_output,"\n");
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",aPkt->srcVehicle->slot_twoHop[i]);
    // }
    // fprintf(Collisions_output,"\n");

    //dst
    fprintf(Collisions_output,"dst id: %d\n",aPkt->dstVehicle->id);
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",aPkt->dstVehicle->slot_oneHop[i]);
    // }
    // fprintf(Collisions_output,"\n");
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",aPkt->dstVehicle->slot_twoHop[i]);
    // }
    fprintf(Collisions_output,"\n");
    
    fclose(Collisions_output);
}


void log_collision(struct collision* coli){
    char output_collisions_path[100];
    FILE * Collisions_output;
    sprintf(output_collisions_path, "./simulation_result/bubble_collision_%d_%d.txt", SlotPerFrame, traffic_density);
    
    Collisions_output = fopen(output_collisions_path,"a");
    
    fprintf(Collisions_output, "No:%d, Type: %d, Slot: %d, Distance:%lf\n", cnt_coli++,coli->type, coli->slot, distance_between_vehicle(coli->src, coli->dst));
        
    //src
    fprintf(Collisions_output,"src ID: %d, x: %lf, y: %lf, comm.R: %lf \n",coli->src->id, coli->src->x, coli->src->y, coli->src->commRadius);
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",coli->src_oneHop[i]);
    // }
    // fprintf(Collisions_output,"\n");
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",coli->src_twoHop[i]);
    // }
    // fprintf(Collisions_output,"\n");

    //dst
    fprintf(Collisions_output,"dst ID: %d, x: %lf, y: %lf ,comm.R: %lf \n",coli->dst->id, coli->dst->x, coli->dst->y, coli->dst->commRadius);
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",coli->dst_oneHop[i]);
    // }
    // fprintf(Collisions_output,"\n");
    // for(int i = 0; i< SlotPerFrame; i++){
    //     fprintf(Collisions_output,"%d ",coli->dst_twoHop[i]);
    // }
    fprintf(Collisions_output,"\n");

    fclose(Collisions_output);
    
    //释放当前结构体内存
    free(coli->src_oneHop);
    free(coli->src_twoHop);
    free(coli->dst_oneHop);
    free(coli->dst_twoHop);
    free(coli);
}


//处理发包过程
void handle_transmitter(struct Region* region, int slot){
    struct Cell *aCell;
    struct Item *aItem, *bItem;
    struct vehicle *aCar, *bCar;
    struct neighbour_car* bneigh;

    int cnt_now_car = 0;

    for(int i = 0; i < region->hCells; i++){       
        for(int j = 0; j<region->vCells; j++) {
            aCell = region->mesh + i*region->vCells + j;
            aItem = aCell->cars.head;
            //printf("test0\n");

            while (aItem != NULL){
                aCar = (struct vehicle*)aItem->datap;
                //printf("slot: %d, Vehicle No: %d, slot_condition: %d, slot_occupied: %d \n",slot, aCar->id,aCar->slot_condition, aCar->slot_occupied);


                if(aCar->slot_occupied != slot){
                    aItem = aItem->next; //忽略掉receiver
                    continue;
                }  

                printf("Current Slot: %d, Current Transmitter: %d\n", slot, aCar->id);//对当前时槽正好发射的节点进行操作

                bItem = (struct Item*)aCar->neighbours.head;//遍历当前transmitter的邻居节点
                while (bItem != NULL) {
                    bCar =  (struct vehicle*)bItem->datap;
                    printf("bCar Id: %d\n", bCar->id);
                    //printf("%d 's neighbor is %d\n", aCar->id, bCar->id);
                    double distanceAB = distance_between_vehicle(aCar, bCar);
                    if(aCar->commRadius < distanceAB){
                        //printf("%d 's Comm Range is: %lf, OutRange neighbor is %d, distance is %lf\n",aCar->id,  aCar->commRadius, bCar->id,distanceAB);
                        bItem = bItem->next;
                    }else{
                        //printf("%d 's Comm Range is: %lf, InRange neighbor is %d, distance is %lf\n", aCar->id,aCar->commRadius, bCar->id, distanceAB);
                        if(bCar->slot_occupied == aCar->slot_occupied){//若此时目标车辆也在发送，则产生collision
                            struct collision* coli =  generate_collision(aCar,bCar,0,slot);
                            log_collision(coli);
                            bItem = bItem->next;
                        }else{
                            struct packet* pkt = generate_packet(aCar,bCar,slot);
                            duallist_add_to_tail(&(bCar->packets), pkt);
                            printf("A packet! cnt_pkt: %d, src: %d, dst:%d ,slot:%d \n", cnt_pkt, aCar->id, bCar->id,slot);
                            log_packet(pkt,slot);
                            bItem = bItem->next;
                        }                           
                    }
                }
                aItem = aItem->next;
            }
        }
    }
}

void handle_receiver(struct Region* region, int slot){
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
                
                if(aCar->slot_occupied == slot) {
                    aItem = aItem->next;
                    // printf("There is a transmitter!\n");
                    continue; //忽略掉transmitter
                }
                
                //printf("Current Slot: %d, Current Receiver: %d\n", slot, aCar->id);//对当前时槽正好发射的节点进行操作
                //到目前时间一直都没有收到包
                if(aCar->packets.nItems == 0){
                    aItem = aItem->next;
                    // printf("There is no packets!\n");
                    continue; //忽略掉transmitter
                }
                
                //数一下当前slot收到了多少个包---->packet只有当一个完整的frame才刷新
                int cnt_cur_pkt = 0;
                bItem =  (struct Item*)aCar->packets.head;
                while(bItem != NULL){
                    struct packet* pkt= (struct packet*)bItem->datap;
                    if(pkt->slot == slot) cnt_cur_pkt++;
                    bItem = bItem->next;
                }
                // 若当前slot收到0个包
                if(cnt_cur_pkt == 0){
                    aCar->slot_oneHop[slot] = -1;
                    aCar->slot_twoHop[slot] = -1;//??这个要更新吗??
                }else if(cnt_cur_pkt == 1){//只收到一个pkt，则正常收包，update OHN 和THN, frontV, rearV 有问题
                    cnt_received++;
                    struct packet* pkt= (struct packet*)duallist_pick_tail(&(aCar->packets));
                    int index = pkt->slot, value = pkt->srcVehicle->id;
                    aCar->slot_oneHop[index] = value;//更新OHN时槽使用情况
                    for(int i = 0 ; i < SlotPerFrame; i++){
                        aCar->slot_twoHop[i] = pkt->slot_resource_oneHop_snapShot[i];//将pkt中携带的OHN信息更新到THN中，不考虑覆盖问题？？？？
                    }
                    // 将pkt指向的车更新到frontV, rearV
                    insertFrontRear(aCar, pkt);
                }else{//有两个或以上的pkt，产生Collision
                    bItem = (struct Item*)aCar->packets.head;
                    while(bItem!=NULL){
                        struct packet* pkt= (struct packet*)bItem->datap;
                        if(pkt->slot == slot){
                            if(pkt->srcVehicle->slot_condition == 1){
                                //printf("hello!!!!_____________________________________________!!!!!!\n");
                                //printf("%d %d\n",aCar->id,pkt->srcVehicle->id);
                                struct collision* coli = generate_collision(aCar,pkt->srcVehicle,1,slot);
                                log_collision(coli);
                            }else if(pkt->srcVehicle->slot_condition == 2){
                                //printf("hello!!!!_____________________________________________~~~~~~~\n");
                                struct collision* coli = generate_collision(aCar,pkt->srcVehicle,2,slot);
                                log_collision(coli);
                            }
                            printf("hello!!!!_____________________________________________\n");
                        }
                        bItem = bItem->next;  
                    }
                }
                printf("hello\n");
                aItem = aItem->next;
            }
        }
    }
}

//if tCar is in front of aCar, return true, if not, return false.
bool IsFront(struct vehicle *aCar, struct vehicle *tCar){//先不考虑车道
	// if(aCar->dir_x != tCar->dir_x || aCar->dir_y != tCar -> dir_y){
	// 	printf("Error: the compared two cars are not in the same lane.\n");
	// 	exit(1);
	// }
	if(aCar->dir_x > 0 && aCar->dir_y > 0)
		return (tCar->x > aCar->x) && (tCar->y > aCar->y);
	else if(aCar->dir_x > 0 && aCar->dir_y < 0)
		return (tCar->x > aCar->x) && (tCar->y < aCar->y);
	else if(aCar->dir_x < 0 && aCar->dir_y > 0)
		return (tCar->x < aCar->x) && (tCar->y > aCar->y);
    else if(aCar->dir_x < 0 && aCar->dir_y < 0)
		return (tCar->x < aCar->x) && (tCar->y < aCar->y);
    
    // if(aCar->dir_x != tCar->dir_x || aCar->dir_y != tCar -> dir_y){
	// 	printf("Error: the compared two cars are not in the same lane.\n");
	// 	exit(1);
	// }
}

//车辆aCar收到一个pkt，将pkt对应的车辆加入到Front或Rear中
void insertFrontRear(struct vehicle *aCar, struct packet *pkt){
	if(aCar == NULL || pkt == NULL){
		printf("insertFrontRear Error!\n");
		exit(1);
	}
    if(IsFront(aCar, pkt->srcVehicle)){//pkt来源于前面的一个车
        struct Item* newp = (struct Item*)malloc(sizeof(struct Item));
	    newp->datap = pkt->srcVehicle;
        if(aCar->frontV.head == NULL){//若当前无frontV
            newp->next = NULL;
            newp->prev = newp;
            aCar->frontV.head = newp;
        }else{
           if(distance_between_vehicle(aCar, (struct vehicle*)aCar->frontV.head->datap) > distance_between_vehicle(aCar, pkt->srcVehicle))
                aCar->frontV.head->datap = pkt->srcVehicle;//此包的车为最近的车
        }

    }else{//pkt来源于后面的一个车
        struct Item* newp = (struct Item*)malloc(sizeof(struct Item));
	    newp->datap = pkt->srcVehicle;
        if(aCar->rearV.head == NULL){//若当前无rearV
            newp->next = NULL;
            newp->prev = newp;
            aCar->rearV.head = newp;
        }else{
           if(distance_between_vehicle(aCar, (struct vehicle*)aCar->rearV.head->datap) > distance_between_vehicle(aCar, pkt->srcVehicle))
                aCar->rearV.head->datap = pkt->srcVehicle;//此包的车为最近的车
        }

    }
}

//----------------------YX above---------------------//

int randSlot(int* occupied, int div){
	int len = SlotPerFrame - 2, start = 0;
	int ret = 0;
	if (div = -1){					//取前一半可用槽
		len >>= 1;
	}
	else if(div == 1){
		start = len;
	}

	int index = 0;
	for(int i = start; i < len; i++){
		if(occupied[i] == -1){
			index++;
		}
	}
	index = rand()%index;
    for(int i = start; i < len; i++){
		if(occupied[i] == -1){
			index--;
			if(index == 0)
				return i;
		}
	}
	
	return ret;
}
// int randSlot(int* occupied){
// 	return randSlot(occupied, 0);
// }






void update_vehicle_info(struct Region *region)
{
	double xmin,ymin,xmax,ymax, x, y;
	int flag, i, j, k, l;
  	struct Cell *aCell, *bCell;
  	struct Item *aItem, *bItem, *tItem, *nItem;
  	struct vehicle *bCar, *aCar, *tCar, *nCar;
  	struct neighbour_car* tNeigh, *nNeigh, *bNeigh;

	for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->cars.head == NULL) continue;

				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;

					tItem = aCar->neighbours.head;
					while (tItem != NULL) {
						tNeigh = (struct neighbour_car*)tItem->datap;
						tCar = (struct vehicle*)tNeigh->carItem->datap;
						nItem = aCar->history_neigh.head;
						while (nItem !=NULL) {
							nNeigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)nNeigh->carItem->datap;
							if (nCar->id == tCar->id) {nNeigh->packet_num=tNeigh->packet_num; break;}
							nItem = nItem->next;
						}
						if (nItem == NULL) duallist_add_to_tail(&aCar->history_neigh, tNeigh);
						tItem = tItem->next;
					}

					aItem = aItem->next;
				}
			}
	}
}

void update_trans_rate(struct Region *region)
{
  double xmin,ymin,xmax,ymax, x, y;
  int flag, i, j, k, l;
  struct Cell *aCell, *bCell;
  struct Item *aItem, *bItem, *tItem, *nItem;
  struct vehicle *bCar, *aCar, *tCar, *nCar;
  struct neighbour_car* tNeigh, *nNeigh, *bNeigh;
  FILE *carinfo;
  char file_path[100];
  int car_count=0;

	// update position
	sprintf(file_path, "/home/lion/Desktop/hfc/vanet1.0_old/vanet1.0/car_position/carposition_%d_%d.txt", traffic_density, bi_num); //traffic density, bi number
	carinfo = fopen(file_path, "r");
	int carnum;
	fscanf(carinfo, "%d", &carnum);
	//Car_Number = carnum;
	for (k=0; k< carnum; k++){
		int flag=0;
		struct vehicle *new_car;
		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));
		fscanf(carinfo, "%d", &new_car->id);
		fscanf(carinfo, "%lf", &new_car->x);
		fscanf(carinfo, "%lf", &new_car->y);
		fscanf(carinfo, "%lf", &new_car->x1);
		fscanf(carinfo, "%lf", &new_car->y1);
		fscanf(carinfo, "%lf", &new_car->v);

		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->cars.head == NULL) continue;

				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					if (aCar->id == new_car->id) {
						aCar->x = new_car->x;
						aCar->y = new_car->y;
						aCar->x1 = new_car->x1;
						aCar->y1 = new_car->y1;
						aCar->v = new_car->v;
						flag=1;
						break;
					}
					aItem = aItem->next;		
				}
				if (flag==1) break;
			}
			if (flag==1) break;
		}

		free(new_car);
	}

	//update transmission rate
	// calculate_rate(region);

	fclose(carinfo);

	return;
}


int show_graph_degree(struct Region *region)
{
	int i, j;
	struct Cell *aCell;
	struct Item *aItem;
	struct vehicle *aCar;
	int degree=0;

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->neighbours.nItems > degree) degree = aCar->neighbours.nItems;
				aItem = aItem->next;
			}
		}
	}
	
	//printf("Graph Degree: %d\n", degree);
	return 0;
}

double calculate_dis(double x1, double y1, double x2, double y2)
{
	double dis = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	return dis;
}

double calculate_angle_diff(double angle1, double angle2)
{
	double diff=0;
	if (fabs(angle1-angle2) > 180) {
		if (angle1>angle2) diff = fabs(angle2+360-angle1);
		else diff = fabs(angle1+360-angle2);
	}
	else diff = fabs(angle1-angle2);

	return diff;
}

int car_legal(struct Region *region, struct vehicle *aCar, struct Cell *aCell)
{
	int x = aCell->xNumber, y = aCell->yNumber;
	int neighbour_cell[9][2] = {{x-1,y-1}, {x,y-1}, {x+1,y-1}, {x-1,y}, {x,y}, {x+1,y}, {x-1,y+1}, {x,y+1}, {x+1,y+1}};
	struct Cell *nCell;
	struct Item *nItem; //n represents neighbour
	struct vehicle *nCar;
	int flag = true;

	for (int i=0; i<9; i++){
		if ((neighbour_cell[i][0]<0 || neighbour_cell[i][0]>=region->hCells) || (neighbour_cell[i][1]<0 || neighbour_cell[i][1]>=region->vCells)) continue;
		int tmpx = neighbour_cell[i][0], tmpy = neighbour_cell[i][1];
		nCell = aCell = region->mesh + tmpx*region->vCells + tmpy;
		nItem = nCell->cars.head;
		while (nItem != NULL){
			nCar = (struct vehicle*)nItem->datap;
			if (calculate_dis(aCar->x1, aCar->y1, nCar->x1, nCar->y1) < safe_dis) {flag =false;break;}  // new car  doesn't have enough distance with another car
			nItem = nItem->next;
		}
		if (flag==false) break;
	}

	return flag;
}


double safeDistance(const struct vehicle* v1, const struct vehicle* v2){
	if(v2 == NULL){
		return ((v1->v * v1->v)/(2 * v1->b));
	}
	return (v1->v * v1->v - v2->v * v2->v)/(2 * v1->b * BRAKE_COE);
}

double vehicleDistance(const struct vehicle* v1, const struct vehicle* v2){
	double x = v1->x - v2->x, y = v1->y - v2->y;
	return sqrt(x*x + y*y);
}

bool curInFront(const struct vehicle* cur, const struct vehicle* tar){
	return ((cur->x - tar->x) * cur->dir_x > 0) || (((cur->x - tar->x) * cur->dir_x == 0) && (cur->y - tar->y) * cur->dir_y > 0);
}



void degrade(struct vehicle* cur_vehicle){		//退化为单车

	cur_vehicle->slot_occupied = rand()%(SlotPerFrame - 2) + 1;
	cur_vehicle->slot_condition = 1;
	cur_vehicle->car_role = ROLE_SINGLE;
	cur_vehicle->commRadius = safeDistance(cur_vehicle, NULL);
	cur_vehicle->radius_flag = true;  
}

void applyForSlot(struct vehicle* cur_vehicle){
    int i = 0;
	for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){        //申请成功
        printf("times: %d", i++);
		struct packet* cur_packet = (struct packet*)p->datap;
		struct vehicle* target_vehicle = cur_packet->srcVehicle;
		double dis = vehicleDistance(cur_vehicle, target_vehicle);

		if(cur_packet->slot_resource_oneHop_snapShot[cur_vehicle->slot_occupied] == cur_vehicle->id){	//确认了我的存在
			cur_vehicle->slot_condition = 2;
			cur_vehicle->slot_oneHop[cur_packet->slot] = target_vehicle->id;
			cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;
		}
	}
}




//bubblle MAC, to determine slot and communication range.
void bubble_mac_protocol(struct Region* aRegion){
    
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
                            cur_vehicle->slot_occupied = rand()%(SlotPerFrame - 2) + 1;
                            cur_vehicle->slot_condition = 1;
                        }
                        else if(cur_vehicle->slot_condition == 1){  //单车申请时槽
                            if(cur_vehicle->packets.head == NULL){
                                cur_vehicle->slot_occupied = rand()%(SlotPerFrame - 2) + 1; //无人回复，单车随机一个新时槽
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


                                    }
                                    else{   //当前车在后，申请成为新车尾
                                        cur_vehicle->slot_occupied = TAIL_SLOT;
                                        cur_vehicle->slot_condition = 1;
                                        cur_vehicle->car_role = ROLE_TAIL;
                                        cur_vehicle->commRadius = dis; //min(safeDistance(cur_vehicle, target_vehicle), dis);
                                        cur_vehicle->radius_flag = true;

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
                        }
                        else if(cur_vehicle->slot_condition == 1){   //车头申请时槽
                            applyForSlot(cur_vehicle);
                        }
                        else{   //车头已确认时槽
                            for(struct Item* p = cur_vehicle->packets.head; p != NULL; p = p->next){        //申请成功
                                struct packet* cur_packet = (struct packet*)p->datap;
                                struct vehicle* target_vehicle = cur_packet->srcVehicle;
                                double dis = vehicleDistance(cur_vehicle, target_vehicle);	//遍历包之后再决定

                                if(dis > safeDistance(cur_vehicle, NULL)){  //断链——————————————————————————————————————————————问题：多包情况
                                    degrade(cur_vehicle);

                                }
                                else if(cur_packet->slot_resource_oneHop_snapShot[HEAD_SLOT] != cur_vehicle->id){   //cur_vehicle与前车merge
                                    cur_vehicle->slot_occupied = randSlot(cur_vehicle->slot_oneHop, -1);
                                    cur_vehicle->slot_condition = 1;
                                    cur_vehicle->car_role = ROLE_MID;
                                    cur_vehicle->commRadius = dis; //min(safeDistance(cur_vehicle, target_vehicle), dis);
                                    cur_vehicle->radius_flag = true;

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
                                        
                                        cur_vehicle->commRadius = vehicleDistance(cur_vehicle, front); //min(safeDistance(cur_vehicle, front),vehicleDistance(cur_vehicle, front));
                                        cur_vehicle->radius_flag = true;

                                        // cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;
                                    }
                                    else{
                                        cur_vehicle->slot_occupied = HEAD_SLOT;
                                        cur_vehicle->slot_condition = 1;
                                        cur_vehicle->car_role = ROLE_HEAD;

                                        struct vehicle* rear = (struct vehicle*)cur_vehicle->frontV.head->datap;
                                        
                                        cur_vehicle->commRadius = max(safeDistance(cur_vehicle, rear), vehicleDistance(cur_vehicle, rear));
                                        cur_vehicle->radius_flag = true;

                                        // cur_vehicle->slot_oneHop[cur_vehicle->slot_occupied] = cur_vehicle->id;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default: {printf("default\n");break;}

                }
                aItem = aItem->next;	
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
    
                aItem = aItem->next;	
            }
        }
    }

}
