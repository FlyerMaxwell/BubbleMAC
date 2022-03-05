#pragma once
#include "function.h"
#include<unordered_map>

int init_simulation(struct Region *region)
{
	struct Item *aItem;
	struct Cell *aCell;
	struct vehicle *aCar;
	int i,j;

	timestamp=0;

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				aCar->role = -1; //each car is given an initial state 0 or 1 (listen or scan)
				aCar->handled = 0;
				aCar->communicate_time = 0;
				aCar->communicated_num = 0;
				aCar->scan_time = -1;
				aCar->choose_car_id = -1;
				duallist_destroy(&aCar->choose_neigh, NULL);
				duallist_destroy(&aCar->known_neigh, NULL);
				aItem = aItem->next;
			}
		}
	}

	return 0;
}

int generate_car_old(struct Region *region) 
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
  
	sprintf(file_path, "/home/lion/Desktop/hfc/vanet1.0_old/vanet1.0/car_position/carposition_%d_%d.txt", traffic_density, bi_num); //traffic density, bi number
	carinfo = fopen(file_path, "r");
	int carnum;
	fscanf(carinfo, "%d", &carnum);
	Car_Number = carnum;
	for (k=0; k< carnum; k++){
		struct vehicle *new_car;
		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));
		fscanf(carinfo, "%d", &new_car->id);
		fscanf(carinfo, "%lf", &new_car->x);
		fscanf(carinfo, "%lf", &new_car->y);
		fscanf(carinfo, "%lf", &new_car->x1);
		fscanf(carinfo, "%lf", &new_car->y1);
		fscanf(carinfo, "%lf", &new_car->v);
		new_car->handled = 2;
		duallist_init(&new_car->history_neigh);
		duallist_init(&new_car->choose_neigh);
		duallist_init(&new_car->neighbours);
		duallist_init(&new_car->real_neigh);
		duallist_init(&new_car->known_neigh);
//if (learning_cycle_num==2) printf("0.1\n");
		flag = false;		
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->box.xmin <= new_car->x && aCell->box.xmax > new_car->x && aCell->box.ymax > new_car->y && aCell->box.ymin <= new_car->y) {flag=true;break;} //find the cell which contains the new car
			}
			if (flag==true) break;
		}
		new_car->belongCell = aCell;
		
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
//if (learning_cycle_num==2) printf("0.2\n");
		if (flag == true) {
			bCar->x = new_car->x;
			bCar->y = new_car->y;
			bCar->x1 = new_car->x1;
			bCar->y1 = new_car->y1;
			bCar->v = new_car->v;
			bCar->belongCell = new_car->belongCell;
			bCar->handled = 1;
			duallist_pick_item(&bCell->cars, bItem);
			duallist_add_to_tail(&aCell->cars, bCar);

			if (bi_num%200 == 0) {
				duallist_destroy(&bCar->history_neigh, NULL);
				duallist_init(&bCar->history_neigh);
				duallist_destroy(&bCar->neighbours, NULL);
				duallist_init(&bCar->neighbours);
			}
//if (learning_cycle_num==2) printf("0.25\n");
			else {
				duallist_destroy(&bCar->neighbours, NULL);
				duallist_init(&bCar->neighbours);
			}

			free(new_car);
		}
//if (learning_cycle_num==2) printf("0.3\n");
		if (flag == false && bi_num%200==0) duallist_add_to_tail(&(aCell->cars), new_car);
		if (flag == false && bi_num%200!=0) free(new_car);
	}
	
//printf("1\n");
	for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					tItem = aItem;
					aItem = aItem->next;
					if (aCar->handled ==0) {
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

  //printf("\ntotal car number in this BI: %d\n", car_count);
 
  return 0;
}
int generate_car(struct Region *region, const int slot, unordered_map<int, vehicle*>& allCars) 		//new generate_car modified by hfc
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
  
	sprintf(file_path, "/home/lion/Desktop/hfc/vanet1.0_old/vanet1.0/car_position/carposition_%d_%d.txt", traffic_density, slot + 4000); //traffic density, slot number
	carinfo = fopen(file_path, "r");
	int carnum;
	fscanf(carinfo, "%d", &carnum);
	Car_Number = carnum;
	for (k=0; k< carnum; k++){
		struct vehicle *new_car;
		new_car=(struct vehicle*)malloc(sizeof(struct vehicle));
		fscanf(carinfo, "%d", &new_car->id);
		fscanf(carinfo, "%lf", &new_car->x);
		fscanf(carinfo, "%lf", &new_car->y);
		fscanf(carinfo, "%lf", &new_car->x1);
		fscanf(carinfo, "%lf", &new_car->y1);
		fscanf(carinfo, "%lf", &new_car->v);
		fscanf(carinfo, "%lf", &new_car->belongLane);
		allCars[new_car->id] = new_car;
		new_car->handled = 2;
		duallist_init(&new_car->history_neigh);
		duallist_init(&new_car->choose_neigh);
		duallist_init(&new_car->neighbours);
		duallist_init(&new_car->real_neigh);
		duallist_init(&new_car->known_neigh);
//if (learning_cycle_num==2) printf("0.1\n");
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
//if (learning_cycle_num==2) printf("0.2\n");
		if (flag == true) {
			bCar->x = new_car->x;
			bCar->y = new_car->y;
			bCar->x1 = new_car->x1;
			bCar->y1 = new_car->y1;
			bCar->v = new_car->v;
			bCar->belongCell = new_car->belongCell;
			bCar->handled = 1;
			duallist_pick_item(&bCell->cars, bItem);
			duallist_add_to_tail(&aCell->cars, bCar);

			if (bi_num%200 == 0) {
				duallist_destroy(&bCar->history_neigh, NULL);
				duallist_init(&bCar->history_neigh);
				duallist_destroy(&bCar->neighbours, NULL);
				duallist_init(&bCar->neighbours);
			}
//if (learning_cycle_num==2) printf("0.25\n");
			else {
				duallist_destroy(&bCar->neighbours, NULL);
				duallist_init(&bCar->neighbours);
			}

			free(new_car);
		}
//if (learning_cycle_num==2) printf("0.3\n");
		if (flag == false && bi_num%200==0) duallist_add_to_tail(&(aCell->cars), new_car);
		if (flag == false && bi_num%200!=0) free(new_car);
	}
	
//printf("1\n");
	for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					tItem = aItem;
					aItem = aItem->next;
					if (aCar->handled ==0) {
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

  //printf("\ntotal car number in this BI: %d\n", car_count);
 
  return 0;
}


int handle_neighbour(struct Region *region) //need to be changed;
{
	struct Cell *aCell, *nCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar;
	struct neighbour_car *bneigh, *aNeigh, *tNeigh;
	int x, y, i, j;


	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				bItem = aCar->neighbours.head;
				while (bItem != NULL) {
					bneigh = (struct neighbour_car*)bItem->datap;
					bneigh->state = 0;
					bItem = bItem->next;
				}
				aItem = aItem->next;
			}
		}
	}

	//printf("11111111\n");
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			int neighbour_cell[9][2] = {{i-1,j-1}, {i,j-1}, {i+1,j-1}, {i-1,j}, {i,j}, {i+1,j}, {i-1,j+1}, {i,j+1}, {i+1,j+1}};
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				for (int k=0; k<9; k++){
					if ((neighbour_cell[k][0]<0 || neighbour_cell[k][0]>=region->hCells) || (neighbour_cell[k][1]<0 || neighbour_cell[k][1]>=region->vCells)) continue;
					int tmpx = neighbour_cell[k][0], tmpy = neighbour_cell[k][1];
					nCell = aCell = region->mesh + tmpx*region->vCells + tmpy;
					nItem = nCell->cars.head;
					while (nItem != NULL){
						nCar = (struct vehicle*)nItem->datap;
						if (nCar->id == aCar->id) {nItem = nItem->next; continue;} //ncar is the same car with acar

						double dis, angle;
						dis = calculate_dis(aCar->x1, aCar->y1, nCar->x1, nCar->y1);
						if (dis > com_dis) {nItem = nItem->next; continue;}
						angle = angle_between(aCar->x1, aCar->y1, nCar->x1, nCar->y1);						

						bItem = aCar->neighbours.head;
						int flag = true;
						while(bItem != NULL){
							bneigh = (struct neighbour_car*)bItem->datap;
							bCar = (struct vehicle*)bneigh->carItem->datap;

							if (calculate_angle_diff(angle, bneigh->angle) > neighbour_angle) {
								bItem=bItem->next;continue;
							}

							if (bneigh->dis < dis) {flag=false;bItem = bItem->next;continue;}
							
							tItem = bItem;
							bItem = bItem->next;
							duallist_pick_item(&aCar->neighbours, tItem);
						}

						if (flag){
							struct neighbour_car *new_neigh;
							new_neigh = (struct neighbour_car*)malloc(sizeof(struct neighbour_car));
							new_neigh->carItem = nItem;
							new_neigh->state = 1;
							new_neigh->cell_x = neighbour_cell[k][0];
							new_neigh->cell_y = neighbour_cell[k][1];
							new_neigh->car_id = nCar->id;
							new_neigh->dis = dis;
							new_neigh->angle = angle;

							bItem = aCar->history_neigh.head;
							while (bItem != NULL) {
								bneigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bneigh->carItem->datap;
								if (bCar->id == new_neigh->car_id) {//printf("neigh id: %d, packet time: %d\n", bCar->id, bneigh->packet_num);
									new_neigh->packet_num = bneigh->packet_num;
									break;
								}
								bItem = bItem->next;
							}
							if (bItem == NULL || bi_num%200 == 0) new_neigh->packet_num=DATA_TIME;
							duallist_add_to_tail(&(aCar->neighbours), new_neigh);
							//if (bItem == NULL) duallist_add_to_tail(&(aCar->history_neigh), new_neigh);
						}

						nItem = nItem->next;
					}
				}
				aItem = aItem->next; 
			}
		}
	}
	
	//printf("22222222\n");
	struct Item *bnItem;
	// validate whether each neighbour car of aCar also treat aCar as its neighbour
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					int flag = false;
					bItem = ((struct neighbour_car*)nItem->datap)->carItem;
					bCar = (struct vehicle*)bItem->datap;
					bnItem = bCar->neighbours.head;
					while (bnItem != NULL){
						bneigh = (struct neighbour_car*)bnItem->datap;
						if (bneigh->car_id == aCar->id) {flag = true; break;}
						bnItem = bnItem->next;
					}
					if (flag == false) {tItem = nItem; nItem=nItem->next; duallist_pick_item(&aCar->neighbours, tItem); continue;}
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}
		}
	}

	//printf("33333333\n");
	int total_edge=0;
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				total_edge+=aCar->neighbours.nItems;
				aItem = aItem->next;
			}		
		}
	}

	total_edge /=2;
	//printf("BI number: %d, total edge number: %d\n", bi_num, total_edge);

	/*if (bi_num%100 ==99) { //real neighbour for calculating metric 1,2,3
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->cars.head == NULL) continue;

				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					duallist_destroy(&aCar->real_neigh, NULL);
					duallist_copy_by_reference(&aCar->real_neigh, &aCar->neighbours);
					aItem = aItem->next;
				}		
			}
		}
	}*/

	for(i = 0; i<region->hCells; i++){       //delete neighbours whose data has all been transmitted & beam index
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					aNeigh = (struct neighbour_car*)nItem->datap;
					aNeigh->beam_index = (int)(aNeigh->angle/(min_scan_interval)+0.5);
					aNeigh->scan_beam_index = (int)(aNeigh->angle/scan_theta+0.5);
					if (aNeigh->packet_num <=0) {tItem = nItem; nItem = nItem->next; duallist_pick_item(&aCar->neighbours, tItem); continue;}
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}
		}
	}

	//calculate_control_SNR(region);
	calculate_rate(region);

	show_graph_degree(region);

	return 0;
}

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
	calculate_rate(region);

	fclose(carinfo);

	return;
}

void calculate_rate(struct Region *region)
{

  int flag, i, j, k, l;
  struct Cell *aCell, *bCell;
  struct Item *aItem, *bItem, *tItem, *nItem;
  struct vehicle *bCar, *aCar, *tCar, *nCar;
  struct neighbour_car* tNeigh, *nNeigh, *bNeigh;
  double path_loss, delta_angle, control_delta_angle, misalign_loss, scan_misalign_loss, gain_loss, SNR_dB, tmp_rate;
  int SNR;
  int avg_rate;

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				//if (aCar->role !=2) {aItem = aItem->next; continue;}
				bItem = aCar->neighbours.head;
				avg_rate =0;
				while (bItem != NULL){
					bNeigh = (struct neighbour_car*)bItem->datap;
					bCar = (struct vehicle*)bNeigh->carItem->datap;

					bNeigh->dis = calculate_dis(aCar->x1, aCar->y1, bCar->x1, bCar->y1);
					bNeigh->angle = angle_between(aCar->x1, aCar->y1, bCar->x1, bCar->y1);

					/*calculate Control SNR & Link SNR*/
					path_loss = 17.7*log10(bNeigh->dis)+70+15*bNeigh->dis/1000;  //dB

					delta_angle = abs(bNeigh->beam_index*min_scan_interval-bNeigh->angle);
 					control_delta_angle = abs(bNeigh->scan_beam_index*scan_theta-bNeigh->angle);

					misalign_loss = 10*(2.4*pow(delta_angle,2)/pow(min_beam_width,2));  //dB
					scan_misalign_loss = 10*(1.2*pow(control_delta_angle,2)/pow(scan_alpha,2)+1.2*pow(control_delta_angle,2)/pow(scan_beta,2));  //dB

					SNR_dB = radio_gain-path_loss-misalign_loss-noise_power; //dB
					gain_loss = 10*log10(scan_alpha/min_beam_width)+10*log10(scan_beta/min_beam_width);
					bNeigh->control_SNR = radio_gain-path_loss-scan_misalign_loss-noise_power-gain_loss; //dB
					bNeigh->SNR = SNR_dB;
					SNR = (int)SNR_dB;
					//tmp_rate = 270.0*log(1+SNR)/log(2); //Shannon formula

					if (SNR >= 21) bNeigh->data_rate = 606;         //MCS12 (10Byte/10us)
					else if (SNR >= 20) bNeigh->data_rate = 505;    //MCS11
					else if (SNR >= 19) bNeigh->data_rate = 404;    //MCS10
					else if (SNR >= 15) bNeigh->data_rate = 328;    //MCS9
					else if (SNR >= 13) bNeigh->data_rate = 303;    //MCS8
					else if (SNR >= 12) bNeigh->data_rate = 252;    //MCS7
					else if (SNR >= 11) bNeigh->data_rate = 202;    //MCS6
					//else if (SNR >= 12) bNeigh->data_rate = 156;    //MCS5
					else if (SNR >= 10) bNeigh->data_rate = 151;    //MCS4
					else if (SNR >= 9) bNeigh->data_rate = 126;    //MCS3	
					else if (SNR >= 7)  bNeigh->data_rate = 101;    //MCS2
					else if (SNR >= 6)  bNeigh->data_rate = 50;     //MCS1
					else bNeigh->data_rate = 0;
					
					//printf("angle: %.1lf, dis: %.1lf, path loss: %.1lf, misalign loss: %.1lf, beam index:%d, SNR: %.1lfdB, data rate: %d\n",bNeigh->angle, bNeigh->dis, path_loss, misalign_loss, bNeigh->beam_index, SNR_dB, bNeigh->data_rate);
					avg_rate += bNeigh->data_rate;
					bItem = bItem->next;
				}
				if (aCar->neighbours.nItems !=0){
					avg_rate = avg_rate/aCar->neighbours.nItems;
					//printf("neighbour num: %d, average data rate: %d\n", aCar->neighbours.nItems, avg_rate);
				}
				aItem = aItem->next;
			}
		}

	}

	if (bi_num %200==0) {log_neighbour(region);}
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


