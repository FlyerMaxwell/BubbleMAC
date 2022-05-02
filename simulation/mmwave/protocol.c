#include "protocol.h"
#include "log_result.h"
/***************** different protocol **********************/
int final_bi_protocol_random(struct Region *region)
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num);

	Random_Neighbour_Discovery(region, 0);
	random_Match(region, 0);

		int tmp_com =0; 

		int tmp_pair_num =0;
		int comm_time = BEACON_INTERVAL-2*learning_cycle_num*sector_num-select_waste*SELECT_TIME; //change this
		tmp_pair_num = data_exchange_phase(region, comm_time);
		pair_num = pair_num+tmp_pair_num/2; 


	//printf("time(ms): %d, finished pair number: %d\n", bi_num*5+5, pair_num);

	return 0;
}

int IEEE_AD(struct Region *region)
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	int SP_length, DTI_length = 278; //us
	double path_loss, delta_angle, control_delta_angle, misalign_loss, scan_misalign_loss, gain_loss, tmp_rate;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num);

	//initialize neighbour list & choose whether to be PCP
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				double p = rand()*1.0/RAND_MAX;
				if (p<0.3) aCar->role = 1;
				else aCar->role = 0;
				total_edge+=aCar->neighbours.nItems;
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					neigh->known=0;
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
	}

	total_edge /=2;
	//printf("IEEE 802.11AD, BI number: %d, total edge number: %d\n", bi_num, total_edge);
	init_all_car(region);

	/* clusting into PBSS */
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						duallist_destroy(&aCar->known_neigh, NULL);
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;

							path_loss = 17.7*log10(neigh->dis)+70+15*neigh->dis/1000;  //dB
							delta_angle = abs(neigh->beam_index*min_scan_interval-neigh->angle);
							misalign_loss = 10*(1.2*pow(delta_angle,2)/pow(min_beam_width,2));  //dB

							gain_loss = 8.0;
							neigh->control_SNR = radio_gain-path_loss-scan_misalign_loss-noise_power-gain_loss; //dB

							neigh->known=0;
							nItem = nItem->next;
						}
						aItem = aItem->next;
					}
				}
			}

	
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						if (aCar->role ==1) {aItem = aItem->next; continue;}

						int pcp_num=0;	
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							if (nCar->role ==1 && neigh->control_SNR>6.0 && neigh->packet_num>0) {
								pcp_num++;
							}
							nItem = nItem->next;
						}

						if (pcp_num==0) {aItem = aItem->next; continue;}

						int choose_pcp = rand()%pcp_num+1;
						nItem = aCar->neighbours.head;
						while(choose_pcp !=0) {
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							if (nCar->role ==1 && neigh->control_SNR>6.0) {
								choose_pcp--;
								if (choose_pcp ==0) break;
							}
							nItem = nItem->next;
						}

						bItem = nCar->neighbours.head;
						while (bItem != NULL) {
							bNeigh = (struct neighbour_car*)bItem->datap;
							bCar = (struct vehicle*)bNeigh->carItem->datap;
							if (bCar->id == aCar->id) {duallist_add_to_tail(&nCar->known_neigh, bNeigh); break;}
							bItem = bItem->next;
						}
						aItem = aItem->next;
					}
				}
			}

		/*ARRANGEMENT FOR COMMUNICATION*/
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						if (aCar->role ==0 || aCar->role==2) {aItem = aItem->next; continue;}
						if (aCar->known_neigh.nItems==0) {aItem = aItem->next; continue;}

						SP_length = DTI_length/aCar->known_neigh.nItems;

						nItem = aCar->known_neigh.head;
						while (nItem != NULL) {
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							bItem = nCar->neighbours.head;
							while (bItem != NULL) {
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if (bCar->id == aCar->id) break;
								bItem = bItem->next;
							}
							neigh->packet_num -= neigh->data_rate*SP_length;
							bNeigh->packet_num -= bNeigh->data_rate*SP_length;
							nItem = nItem->next;
						}
		
						aItem = aItem->next;
					}
				}
			}

	return 0;
}

int COMNET_V2(struct Region *region) //compare rate
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num);

	//initialize neighbour list
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				total_edge+=aCar->neighbours.nItems;
				nItem = aCar->neighbours.head;
				duallist_destroy(&aCar->known_neigh, NULL);
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					neigh->known=0;
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
	}

	total_edge /=2;
	//printf("Greedy algorithm, BI number: %d, total edge number: %d\n", bi_num, total_edge);
	init_all_car(region);

			//learning neighbour cars
			for (k=0; k<learning_cycle_num; k++){
				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							aCar->role = rand()%2;
							aItem = aItem->next;
						}
					}
				}

				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							nItem = aCar->neighbours.head;
							while (nItem != NULL){
								neigh = (struct neighbour_car*)nItem->datap;
								nCar = (struct vehicle*)neigh->carItem->datap;
								if (nCar->role != aCar->role && neigh->control_SNR>6.0) {
									if (neigh->known==0){
										neigh->known = 1;
										duallist_add_to_tail(&aCar->known_neigh, neigh);
										known_pair_num++;
										//duallist_add_in_sequence_from_head(&aCar->known_neigh, nCar, (int(*)(struct vehicle*, struct vehicle*))compare_angle);
									}
								}
								nItem = nItem->next;
							}
							aItem = aItem->next;
						}
					}
				}
			} //for (k=0; k<learning_cycle_num; k++)

			//printf("known pair number: %d\n", known_pair_num/2);
			old_known_pair = known_pair_num;


		COMNET_Match(region, 0);

		int tmp_pair_num =0;
		int comm_time = BEACON_INTERVAL-2*learning_cycle_num*sector_num-select_waste*SELECT_TIME;
		tmp_pair_num = data_exchange_phase(region, comm_time);


		//new_pair_num = new_pair_num+tmp_com/2;

		/*int valid =0;
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->cars.head == NULL) continue;

				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					nItem = aCar->known_neigh.head;
					while (nItem != NULL){
						neigh = (struct neighbour_car*)nItem->datap;
						nCar = (struct vehicle*)neigh->carItem->datap;
						valid += DATA_TIME-neigh->packet_num;
						nItem = nItem->next;
						printf("control SNR: %lfdB, real SNR: %lfdB\n", neigh->control_SNR, neigh->SNR);
					}
					printf("\n");
					aItem = aItem->next;
				}		
			}
		}*/

		//printf("time: %d, new pair number: %d, validation: %d\n", timestamp, log_new_pair[timestamp], valid/2);
		//if (timestamp%1000 ==0) {printf("time: %d, LINK number: %d, total pair number: %d  increased pair number: %d\n", timestamp/100+bi_num*100, tmp_com/2, pair_num, pair_num-old_pair_num); old_pair_num = pair_num;}
		if (timestamp == sim_time-1) {printf("time(ms): %d, LINK number: %d, finished pair number: %d  increased pair number: %d\n", bi_num*5+5, new_pair_num, pair_num, pair_num-old_pair_num); old_pair_num = pair_num;}
		//if (pair_num == total_edge) break;


	return 0;
}

			
int JSAC_17(struct Region *region)
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	double r_est;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num);

	//initialize neighbour list & update neighbour weight
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				total_edge+=aCar->neighbours.nItems;
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;

					r_est = 270.0*neigh->control_SNR/log(2);
					neigh->TXweight = -1000.0*(1+abs(aCar->v-nCar->v)/22.2222)/r_est;
					neigh->RXweight = -1000.0*(2-neigh->packet_num*1.0/DATA_TIME)/r_est;

					neigh->known=0;
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
	}

	total_edge /=2;
	printf("JSAC17 algorithm, BI number: %d, total edge number: %d\n", bi_num, total_edge);
	init_all_car(region);

	for (timestamp=0; timestamp<sim_time; timestamp++){
		//learning neighbours at the beginning of every BI;
		if (timestamp % BEACON_INTERVAL ==0) {
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						duallist_destroy(&aCar->known_neigh, NULL);
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							neigh->known=0;
							nItem = nItem->next;
						}
						aItem = aItem->next;
					}
				}
			}

			//learning neighbour cars
			for (k=0; k<learning_cycle_num; k++){
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->role = rand()%2;
						aItem = aItem->next;
					}
				}
			}

			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							if (nCar->role != aCar->role) {
								if (neigh->known==0){
									neigh->known = 1;
									//duallist_add_to_tail(&aCar->known_neigh, neigh);
									known_pair_num++;
									duallist_add_in_sequence_from_head(&aCar->known_neigh, neigh, (int(*)(void*, void*))compare_weight);
								}
							}
							nItem = nItem->next;
						}
						aItem = aItem->next;
					}
				}
			}
			} //for (k=0; k<learning_cycle_num; k++)

			//printf("known pair number: %d\n", known_pair_num/2);
			old_known_pair = known_pair_num;

		}

		//determine which phase time is in, data transmission or target selection
		int phase_num =0;
		if (timestamp%BEACON_INTERVAL >=2*learning_cycle_num*sector_num){
			int start_time = timestamp%BEACON_INTERVAL-2*learning_cycle_num*sector_num;
			int tmp_time = (BEACON_INTERVAL-2*learning_cycle_num*sector_num)/SELECT_NUM;
			if (start_time % tmp_time==0) {
				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							aCar->role = rand()%2;
							aItem = aItem->next;
						}		
					}
				}
			}
			if (start_time % tmp_time ==0) phase_num = 1;
			if (start_time % tmp_time >= select_waste*SELECT_TIME) phase_num =2;
		}

		//printf("timestamp: %d, phase number: %d\n", timestamp, phase_num);
		if (phase_num == 1){
			for (int z=0; z< SELECT_TIME; z++) {
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->handled = 0;
						if (aCar->role !=2) aCar->role = rand()%2;
						aItem = aItem->next;
					}		
				}
			}

			// handle cars with role 1, cars with role 1 select a neighbour from their known neighbour list
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						flag =false; //represent whether aCar establish a link successfully

						if (aCar->role == 1) {
							//printf("car id: %d\n", aCar->id);
							int known_num = aCar->known_neigh.nItems;
							if (known_num == 0) {aItem = aItem->next;continue;}
							bItem = aCar->known_neigh.head;
							/*choose_car_num = rand()%known_num+1;
							for (k=1; k<choose_car_num; k++){
								bItem = bItem->next;
							}*/
							bNeigh = (struct neighbour_car*)bItem->datap;
							bCar = (struct vehicle*)bNeigh->carItem->datap;
							tItem = bCar->neighbours.head;
							while(tItem != NULL){
								tNeigh = (struct neighbour_car*)tItem->datap;
								tCar = (struct vehicle*)tNeigh->carItem->datap;
								if (tCar->id == aCar->id) {duallist_add_to_tail(&bCar->choose_neigh, tNeigh); break;}
								tItem = tItem->next;
							}
						}

						aItem = aItem->next;
					}		
				}
			}


			//handle cars with role 0, cars with role 0 decide whether establish a communication link
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						flag =false; //represent whether aCar establish a link successfully

						if (aCar->role == 2) {
							int chosen_num = aCar->choose_neigh.nItems;
							if (chosen_num ==1) {
								bItem = aCar->choose_neigh.head;
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								tCar = (struct vehicle*)aCar->choose_car_Item->datap;
								
								tItem = bCar->known_neigh.head;
								while(tItem != NULL){ 
									tNeigh = (struct neighbour_car*)tItem->datap;
									ttCar = (struct vehicle*)tNeigh->carItem->datap;
									if (ttCar->id == tCar->id) break;
									tItem = tItem->next;
								}
								if (bNeigh->TXweight > tNeigh->TXweight) {
									bCar->role = 2;
									bCar->choose_car_Item = aItem;
									aCar->choose_car_Item = bNeigh->carItem;
									tCar->role = 0;
									tCar->choose_car_Item = NULL;
									remove_car_from_NeighList(&tCar->known_neigh, aCar);
									
								}
								else {
									remove_car_from_NeighList(&bCar->known_neigh, aCar);
								}
							}  //chosen_num ==1
						}  //aCar->role == 2

						if (aCar->role == 0) {
							int chosen_num = aCar->choose_neigh.nItems;
							//printf("car id: %d, choose number: %d\n", aCar->id, chosen_num);
							if (chosen_num ==1) {
								flag = true;
								new_pair_num++;
								aCar->role =2;
								bItem = aCar->choose_neigh.head;
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								bCar->role =2;
								aCar->choose_car_Item = bNeigh->carItem;
								bCar->choose_car_Item = aItem;
							}  
							//destroy aCar's choose_neighbour duallist
							//duallist_destroy(&aCar->choose_neigh, NULL);
						}

						

						//destroy all Car's choose_neighbour duallist
						duallist_destroy(&aCar->choose_neigh, NULL);

						aItem = aItem->next;
					}		
				}
			}
			}
		} //phase number = 1;

		int tmp_com =0;

		if (phase_num ==2 && timestamp == sim_time-1){
			int tmp_pair_num =0;
			//tmp_pair_num = data_exchange_phase(region);
			pair_num = pair_num+tmp_pair_num/2;
		}

		//new_pair_num = new_pair_num+tmp_com/2;

		int valid =0;
		for(i = 0; i<region->hCells; i++){       
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if (aCell->cars.head == NULL) continue;

				aItem = aCell->cars.head;
				while (aItem != NULL){
					aCar = (struct vehicle*)aItem->datap;
					nItem = aCar->neighbours.head;
					while (nItem != NULL){
						neigh = (struct neighbour_car*)nItem->datap;
						nCar = (struct vehicle*)neigh->carItem->datap;
						valid += DATA_TIME-neigh->packet_num;
						nItem = nItem->next;
					}
					aItem = aItem->next;
				}		
			}
		}

		//printf("time: %d, new pair number: %d, validation: %d\n", timestamp, log_new_pair[timestamp], valid/2);
		//if (timestamp%1000 ==0) {printf("time: %d, LINK number: %d, total pair number: %d  increased pair number: %d\n", timestamp/100+bi_num*100, tmp_com/2, pair_num, pair_num-old_pair_num); old_pair_num = pair_num;}
		if (timestamp == sim_time-1) {printf("time(ms): %d, LINK number: %d, finished pair number: %d  increased pair number: %d\n", bi_num*10+9, new_pair_num, pair_num, pair_num-old_pair_num); old_pair_num = pair_num;}
		//if (pair_num == total_edge) break;

	}
	
	return 0;
}

int final_bi_optimal(struct Region *region)
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num);

	//initialize neighbour list
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				total_edge+=aCar->neighbours.nItems;
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					neigh->known=0;
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
	}

	total_edge /=2;
	printf("Optimal algorithm, BI number: %d, total edge number: %d\n", bi_num, total_edge);
	init_all_car(region);

	//start choose neighbour and communicate
	for (timestamp=0; timestamp<sim_time; timestamp++){
		//learning neighbours at the beginning of every BI;
		if (timestamp % BEACON_INTERVAL ==0) {
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						duallist_destroy(&aCar->known_neigh, NULL);
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							neigh->known=0;
							nItem = nItem->next;
						}
						aItem = aItem->next;
					}
				}
			}

			//learning neighbour cars
			for (k=0; k<learning_cycle_num; k++){
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->role = rand()%2;
						aItem = aItem->next;
					}
				}
			}

			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							if (nCar->role != aCar->role) {
								if (neigh->known==0){
									neigh->known = 1;
									duallist_add_to_tail(&aCar->known_neigh, neigh);
									known_pair_num++;
									//duallist_add_in_sequence_from_head(&aCar->known_neigh, nCar, (int(*)(struct vehicle*, struct vehicle*))compare_angle);
								}
							}
							nItem = nItem->next;
						}
						aItem = aItem->next;
					}
				}
			}
			} //for (k=0; k<learning_cycle_num; k++)

			//printf("known pair number: %d\n", known_pair_num/2);
			old_known_pair = known_pair_num;

		}


		//determine which phase time is in, data transmission or target selection
		int phase_num =0;
		if (timestamp%BEACON_INTERVAL >=2*learning_cycle_num*sector_num){
			int start_time = timestamp%BEACON_INTERVAL-2*learning_cycle_num*sector_num;
			int tmp_time = (BEACON_INTERVAL-2*learning_cycle_num*sector_num)/SELECT_NUM;
			if (start_time % tmp_time==0) {
				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							aCar->role = rand()%2;
							aItem = aItem->next;
						}		
					}
				}
			}
			if (start_time % tmp_time ==0) phase_num = 1;
			if (start_time % tmp_time >= select_waste) phase_num =2;
		}

		//printf("timestamp: %d, phase number: %d\n", timestamp, phase_num);
		if (phase_num == 1){
			weighted_blossom(region, Car_Number);
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						if (aCar->role ==2) new_pair_num++;
						aItem = aItem->next;
					}
				}
			}
			new_pair_num /=2;
		} //phase number = 1;

		int tmp_com =0;

		if (phase_num ==2){
			int tmp_pair_num =0;
			//tmp_pair_num = data_exchange_phase(region);
			pair_num = pair_num+tmp_pair_num/2;
		}

		//new_pair_num = new_pair_num+tmp_com/2;

		
		if (timestamp == sim_time-1) {printf("time(ms): %d, LINK number: %d, finished pair number: %d  increased pair number: %d\n", bi_num*10+9, new_pair_num, pair_num, pair_num-old_pair_num); old_pair_num = pair_num;}
		//if (pair_num == total_edge) break;
	}
	
	return 0;
}

/***************** different protocol **********************/



/***************** different neighbour discovery ******************/
int COMNET_ND(struct Region *region, int circ_num) //compare rate
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0, iter_round=200;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num+circ_num);

	//initialize neighbour list
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				total_edge+=aCar->neighbours.nItems;
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					neigh->known=0;
					neigh->beam_index = (int)neigh->angle/scan_theta;
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
	}

	//total_edge /=2;
	//printf("Greedy algorithm, BI number: %d, total edge number: %d\n", bi_num, total_edge);

			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						duallist_destroy(&aCar->known_neigh, NULL);
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							neigh->known=0;
							nItem = nItem->next;
						}
						aItem = aItem->next;
					}
				}
			}

			//learning neighbour cars
			for (k=0; k<iter_round; k++){
				if (k%24==0) {
					for(i = 0; i<region->hCells; i++){       
						for(j = 0; j<region->vCells;j++) {
							aCell = region->mesh + i*region->vCells + j;
							if (aCell->cars.head == NULL) continue;

							aItem = aCell->cars.head;
							while (aItem != NULL){
								aCar = (struct vehicle*)aItem->datap;
								aCar->role = rand()%2;
								aItem = aItem->next;
							}
						}
					}
				}

				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							nItem = aCar->neighbours.head;
							while (nItem != NULL){
								neigh = (struct neighbour_car*)nItem->datap;
								nCar = (struct vehicle*)neigh->carItem->datap;
								if (nCar->role != aCar->role && neigh->control_SNR>6.0 && neigh->beam_index == k%24) {
									if (neigh->known==0){
										neigh->known = 1;
										//duallist_add_in_sequence_from_head(&aCar->known_neigh, neigh, (int(*)(void*, void*))compare_rate);
										known_pair_num++;
										//duallist_add_in_sequence_from_head(&aCar->known_neigh, nCar, (int(*)(struct vehicle*, struct vehicle*))compare_angle);
									}
								}
								nItem = nItem->next;
							}
							aItem = aItem->next;
						}
					}
				}
				double ratio = known_pair_num*1.0/total_edge;
				log_ND(region, 1000+circ_num, ratio);
			} //for (k=0; k<learning_cycle_num; k++)


	return 0;
}


int Random_Neighbour_Discovery(struct Region *region, int circ_num)
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0;
	int iter_round = 24*learning_cycle_num, direction_num = 24;
	int total_edge = 0;
	double path_loss, misalign_loss, scan_misalign_loss, gain_loss, control_SNR, tmp_rate;
	double mis_angle1, mis_angle2, angle1, angle2;
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar, *bCar, *nCar;
	struct neighbour_car *neigh;

	srand((unsigned int)time(0)+bi_num+circ_num);

	//initialize neighbour list
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				total_edge+=aCar->neighbours.nItems;
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					neigh->known=0;
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
	}
	total_edge/=2;

			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						duallist_destroy(&aCar->known_neigh, NULL);
						nItem = aCar->neighbours.head;
						while (nItem != NULL){
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							neigh->known=0;
							nItem = nItem->next;
						}
						aItem = aItem->next;
					}
				}
			}

			//learning neighbour cars
			for (k=0; k<iter_round; k++){     
				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							aCar->role = rand()%2;
							aCar->match_id = rand()%direction_num;   //sector_num;
							aItem = aItem->next;
						}
					}	
				}

				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							nItem = aCar->neighbours.head;
							while (nItem != NULL){
								neigh = (struct neighbour_car*)nItem->datap;
								bCar = (struct vehicle*)neigh->carItem->datap;
								if (bCar->role != aCar->role && neigh->known==0) {
									int a_beam_index = (int)(neigh->angle/scan_theta+0.5);
									int b_beam_index; 
									if (a_beam_index+12<direction_num) b_beam_index=a_beam_index+12;
									else b_beam_index=a_beam_index-12;

									if (abs(aCar->match_id-a_beam_index)>1 || abs(bCar->match_id-b_beam_index)>1) {nItem = nItem->next; continue;}

									path_loss = 17.7*log10(neigh->dis)+70+15*neigh->dis/1000;  //dB

									angle1 = neigh->angle;
									if (angle1+180 <360) angle2 = angle1+180;
									else angle2 = angle1-180;
								
	 								mis_angle1 = abs(aCar->match_id*scan_theta-angle1);
									mis_angle2 = abs(bCar->match_id*scan_theta-angle2);

									if (aCar->role ==1) scan_misalign_loss = 10*(1.2*pow(mis_angle1,2)/pow(scan_alpha,2)+1.2*pow(mis_angle2,2)/pow(scan_beta,2));  //dB
									else scan_misalign_loss = 10*(1.2*pow(mis_angle2,2)/pow(scan_alpha,2)+1.2*pow(mis_angle1,2)/pow(scan_beta,2));

									gain_loss = 10*log10(scan_alpha/min_beam_width)+10*log10(scan_beta/min_beam_width);
									control_SNR = radio_gain-path_loss-scan_misalign_loss-noise_power-gain_loss; //dB

									if (control_SNR>=6){
										neigh->known = 1;
										duallist_add_to_tail(&aCar->known_neigh, neigh);
										known_pair_num++;
									}
								
								}
								nItem = nItem->next;
							}
							aItem = aItem->next;
						}
					}
				}
				double ratio = known_pair_num/(2.0*total_edge);
				//log_ND(region, 2000+circ_num, ratio);
			} //for (k=0; k<learning_cycle_num; k++)


	return 0;
}


/***************** different neighbour discovery ******************/



/**************** different Matching ****************************/
int random_Match(struct Region *region, int circ_num) 
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num+circ_num);

		for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->role = 0;
						aCar->choose_car_Item = NULL;
						aCar->choice_car = NULL;
						duallist_destroy(&aCar->choose_neigh, NULL);

						//duallist_destroy(&aCar->known_neigh, NULL);
						//duallist_copy_by_reference(&aCar->known_neigh, &aCar->neighbours);  //delete when used in protocol
						

						aItem = aItem->next;
					}		
				}
		}

			for (int z=0; z< SELECT_TIME; z++) {
			// every car chooses a target
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->handled = 0;
						aCar->choice_car = NULL;

						int tmp_num=0;
						int tmp_index = 0;
						if (aCar->role != 2) {
							if (aCar->known_neigh.nItems == 0) { aItem = aItem->next;continue;}
							
							int tmp_index = rand()%aCar->known_neigh.nItems+1;
							bItem = aCar->known_neigh.head;
							for (k=1; k<tmp_index; k++){
								bItem = bItem->next;
							}
							bNeigh = (struct neighbour_car*)bItem->datap;
							aCar->choice_car = (struct Item*)bNeigh->carItem;
							bCar = (struct vehicle*)bNeigh->carItem->datap;
							tItem = bCar->known_neigh.head;
							while(tItem != NULL){
								tNeigh = (struct neighbour_car*)tItem->datap;
								tCar = (struct vehicle*)tNeigh->carItem->datap;
								if (tCar->id == aCar->id) {duallist_add_to_tail(&bCar->choose_neigh, tNeigh); break;}
								tItem = tItem->next;
							}
						}

						aItem = aItem->next;
					}		
				}
			}

			//every car decides whether establish a link //need to be changed
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						flag =false; //represent whether aCar establish a link successfully

						if (aCar->choice_car ==NULL || aCar->role ==2) {duallist_destroy(&aCar->choose_neigh, NULL);aItem = aItem->next;continue;}

						tCar = (struct vehicle*)aCar->choice_car->datap;
						bItem = aCar->choose_neigh.head;
						while (bItem != NULL) {
							bNeigh = (struct neighbour_car*)bItem->datap;
							bCar = (struct vehicle*)bNeigh->carItem->datap;
							if (tCar->id == bCar->id) {
								flag = true;
								aCar->role =2;
								aCar->choose_car_Item = bNeigh->carItem;
								break;
							}
							bItem = bItem->next;
						}

						//destroy all Car's choose_neighbour duallist
						duallist_destroy(&aCar->choose_neigh, NULL);

						aItem = aItem->next;
					}		
				}
			}

			if (z==SELECT_TIME-1) {
				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							if (aCar->role ==2) new_pair_num++;
							aItem = aItem->next;
						}		
					}
				}
				new_pair_num/=2;
				//printf("total pair number: %d\n", new_pair_num);
			}
			
			//log_throughput(region, 2000+circ_num);
			}


	return 0;
}



int COMNET_Match(struct Region *region, int circ_num) //compare rate
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num+circ_num);

		for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->role = 0;
						aCar->choose_car_Item = NULL;
						aCar->choice_car = NULL;
						aCar->match_id = rand()%10000+1;
						duallist_destroy(&aCar->choose_neigh, NULL);

						//duallist_destroy(&aCar->known_neigh, NULL);
						//duallist_copy_by_reference(&aCar->known_neigh, &aCar->neighbours);  //delete when used in protocol

						aItem = aItem->next;
					}		
				}
		}

			for (int z=0; z< SELECT_TIME; z++) {
			// every car chooses a target
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->handled = 0;
						aCar->choice_car = NULL;

						int tmp_num=0;
						int tmp_index = 0;
						if (aCar->role != 2) {
							if (aCar->known_neigh.nItems == 0) {aCar->choice_car = NULL; aItem = aItem->next;continue;}
							
							bItem = aCar->known_neigh.head;
							while (bItem != NULL) {
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if ((bCar->match_id+aCar->match_id)%magic_number == z%magic_number) tmp_num++; //break for best neigh;
								bItem = bItem->next;
							}

							//new
							if (tmp_num==0) {aCar->choice_car = NULL; aItem = aItem->next;continue;}
							tmp_index = rand()%tmp_num+1;
							bItem = aCar->known_neigh.head;
							while(tmp_index!=0){
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if ((bCar->match_id+aCar->match_id)%magic_number == z%magic_number) {tmp_index--; if (tmp_index==0) break;}
								bItem = bItem->next;
							}

							aCar->choice_car = (struct Item*)bNeigh->carItem;
							tItem = bCar->known_neigh.head;
							while(tItem != NULL){
								tNeigh = (struct neighbour_car*)tItem->datap;
								tCar = (struct vehicle*)tNeigh->carItem->datap;
								if (tCar->id == aCar->id) {duallist_add_to_tail(&bCar->choose_neigh, tNeigh); break;}
								tItem = tItem->next;
							}
						}
						if (aCar->role == 2) {
							if (aCar->known_neigh.nItems == 0) {aCar->choice_car = NULL; aItem = aItem->next;continue;}
							tItem = aCar->known_neigh.head;
							while(tItem != NULL){
								tNeigh = (struct neighbour_car*)tItem->datap;
								tCar = (struct vehicle*)tNeigh->carItem->datap;
								if (tCar->id == ((struct vehicle*)aCar->choose_car_Item->datap)->id)  break;
								tItem = tItem->next;
							}

							bItem = aCar->known_neigh.head;
							while (bItem != NULL) {
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if ((bCar->match_id+aCar->match_id)%magic_number == z%magic_number) tmp_num++; //break for best neigh;
								bItem = bItem->next;
							}

							//new
							if (tmp_num==0) {aCar->choice_car = NULL; aItem = aItem->next;continue;}
							tmp_index = rand()%tmp_num+1;
							bItem = aCar->known_neigh.head;
							while(tmp_index!=0){
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if ((bCar->match_id+aCar->match_id)%magic_number == z%magic_number) {tmp_index--; if (tmp_index==0) break;}
								bItem = bItem->next;
							}

							if (bNeigh->data_rate < tNeigh->data_rate || bCar->role==2) {aCar->choice_car = NULL; aItem = aItem->next;continue;}
							aCar->choice_car = (struct Item*)bNeigh->carItem;
							tItem = bCar->known_neigh.head;
							while(tItem != NULL){
								tNeigh = (struct neighbour_car*)tItem->datap;
								tCar = (struct vehicle*)tNeigh->carItem->datap;
								if (tCar->id == aCar->id) {duallist_add_to_tail(&bCar->choose_neigh, tNeigh); break;}
								tItem = tItem->next;
							}
						}

						aItem = aItem->next;
					}		
				}
			}

			//every car decides whether establish a link
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						flag =false; //represent whether aCar establish a link successfully

						if (aCar->choice_car ==NULL) {duallist_destroy(&aCar->choose_neigh, NULL);aItem = aItem->next;continue;}

						tCar = (struct vehicle*)aCar->choice_car->datap;
						bItem = aCar->choose_neigh.head;
						while (bItem != NULL) {
							bNeigh = (struct neighbour_car*)bItem->datap;
							bCar = (struct vehicle*)bNeigh->carItem->datap;
							if (tCar->id == bCar->id) {
								flag = true;
								aCar->role =2;
								aCar->choose_car_Item = bNeigh->carItem;
								break;
							}
							bItem = bItem->next;
						}
						aCar->choice_car = NULL;
						//destroy all Car's choose_neighbour duallist
						duallist_destroy(&aCar->choose_neigh, NULL);

						aItem = aItem->next;
					}		
				}
			}

			//break old link
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						flag =false; //represent whether aCar establish a link successfully

						if (aCar->role !=2) {aCar->choose_car_Item = NULL; aItem = aItem->next;continue;}

						tCar = (struct vehicle*)aCar->choose_car_Item->datap;
						ttCar = (struct vehicle*)tCar->choose_car_Item->datap;
						if (ttCar->id != aCar->id) {
							aCar->role =0;
							aCar->choose_car_Item = NULL;
						}

						aItem = aItem->next;
					}		
				}
			}

			if (z==SELECT_TIME-1) {
				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							if (aCar->role ==2) new_pair_num++;
							aItem = aItem->next;
						}		
					}
				}
				new_pair_num/=2;
				//printf("total pair number: %d\n", new_pair_num);
			}
			
			//log_throughput(region, 10000*magic_number+circ_num);
			}


	return 0;
}

int simple_weighted_Match(struct Region *region, int circ_num) //compare rate
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

	srand((unsigned int)time(0)+bi_num+circ_num);

		for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->role = 0;
						aCar->choose_car_Item = NULL;
						aCar->choice_car = NULL;
						aCar->match_id = rand()%1000+1;
						duallist_destroy(&aCar->choose_neigh, NULL);

						duallist_destroy(&aCar->known_neigh, NULL);
						duallist_copy_by_reference(&aCar->known_neigh, &aCar->neighbours);  //delete when used in protocol

						aItem = aItem->next;
					}		
				}
		}

			for (int z=0; z< SELECT_TIME; z++) {
			// every car chooses a target
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						aCar->handled = 0;
						aCar->choice_car = NULL;
						int tmp_num=0;
						int tmp_index = 0;
						if (aCar->role != 2) {
							if (aCar->known_neigh.nItems == 0) {aCar->choice_car = NULL; aItem = aItem->next;continue;}
							
							bItem = aCar->known_neigh.head;
							double tmp_rate=0;
							while (bItem != NULL) {
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if (bNeigh->data_rate > tmp_rate && bCar->role !=2) {tmp_rate=bNeigh->data_rate; neigh = bNeigh;}
								bItem = bItem->next;
							}

							bCar = (struct vehicle*)neigh->carItem->datap;
							aCar->choice_car = neigh->carItem;
							tItem = bCar->known_neigh.head;
							while(tItem != NULL){
								tNeigh = (struct neighbour_car*)tItem->datap;
								tCar = (struct vehicle*)tNeigh->carItem->datap;
								if (tCar->id == aCar->id) {duallist_add_to_tail(&bCar->choose_neigh, tNeigh); break;}
								tItem = tItem->next;
							}
						}

						aItem = aItem->next;
					}		
				}
			}

			//every car decides whether establish a link //need to be changed
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						flag =false; //represent whether aCar establish a link successfully

						if (aCar->choice_car ==NULL || aCar->role ==2) {duallist_destroy(&aCar->choose_neigh, NULL);aItem = aItem->next;continue;}

						tCar = (struct vehicle*)aCar->choice_car->datap;
						bItem = aCar->choose_neigh.head;
						while (bItem != NULL) {
							bNeigh = (struct neighbour_car*)bItem->datap;
							bCar = (struct vehicle*)bNeigh->carItem->datap;
							if (tCar->id == bCar->id) {
								flag = true;
								aCar->role =2;
								aCar->choose_car_Item = bNeigh->carItem;
								break;
							}
							bItem = bItem->next;
						}

						//destroy all Car's choose_neighbour duallist
						duallist_destroy(&aCar->choose_neigh, NULL);

						aItem = aItem->next;
					}		
				}
			}

			if (z==SELECT_TIME-1) {
				for(i = 0; i<region->hCells; i++){       
					for(j = 0; j<region->vCells;j++) {
						aCell = region->mesh + i*region->vCells + j;
						if (aCell->cars.head == NULL) continue;

						aItem = aCell->cars.head;
						while (aItem != NULL){
							aCar = (struct vehicle*)aItem->datap;
							if (aCar->role ==2) new_pair_num++;
							aItem = aItem->next;
						}		
					}
				}
				new_pair_num/=2;
				//printf("total pair number: %d\n", new_pair_num);
			}
			
			log_throughput(region, 3000+circ_num);
			}


	return 0;
}

/**************** different Matching ****************************/


int compare_rate(struct neighbour_car *aNeigh, struct neighbour_car *bNeigh)
{
	if (aNeigh->control_SNR < bNeigh->control_SNR) return 1;
	else return 0;
}

int compare_weight(struct neighbour_car *aNeigh, struct neighbour_car *bNeigh)
{
	if (aNeigh->RXweight < bNeigh->RXweight) return 1;
	else return 0;
}

void remove_car_from_NeighList(struct Duallist *duallist, struct vehicle *aCar)
{
	struct Item *bItem;
	struct vehicle *bCar;

	bItem = duallist->head;
	while (bItem !=NULL) {
		bCar = (struct vehicle*)bItem->datap;
		if (bCar->id == aCar->id) {duallist_pick_item(duallist, bItem); break;}
		bItem = bItem->next;
	}
	return;
}



int data_exchange_phase(struct Region *region, int data_change_time)
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

			int tmp_pair_num =0;
			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						if (aCar->role ==2) {
							
							bItem = aCar->known_neigh.head;
							while (bItem != NULL){
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if (bCar->id == ((struct vehicle*)aCar->choose_car_Item->datap)->id) break;
								bItem = bItem->next;
							}

							bNeigh->packet_num -= bNeigh->data_rate*data_change_time; //....
							//if (bNeigh->packet_num<0) printf("car id: %d, neigh id: %d, trans rate: %d, com time: %d\n", aCar->id, bCar->id, bNeigh->data_rate, bNeigh->packet_num);
							
							if (bNeigh->packet_num <=0) {
								tmp_pair_num++;
								bNeigh->known = 2;
								aCar->choose_car_Item = NULL;
								aCar->role = rand()%2;
								duallist_pick_item(&aCar->known_neigh, bItem);
							}
						} 
						aItem = aItem->next;
					}
				}
			}

	return tmp_pair_num;	
}

int AD_exchange_data(struct Region *region, int data_change_time)
{
	int pair_num=0, old_pair_num=0, i, j, k, choose_car_num, flag, known_pair_num=0, old_known_pair=0, new_pair_num=0;
	int total_edge = 0;
	struct Cell *aCell;
	struct Item *aItem, *bItem, *nItem, *tItem;
	struct vehicle *aCar, *bCar, *nCar, *tCar, *ttCar;
	struct neighbour_car *neigh, *bNeigh, *tNeigh;

			for(i = 0; i<region->hCells; i++){       
				for(j = 0; j<region->vCells;j++) {
					aCell = region->mesh + i*region->vCells + j;
					if (aCell->cars.head == NULL) continue;

					aItem = aCell->cars.head;
					while (aItem != NULL){
						aCar = (struct vehicle*)aItem->datap;
						if (aCar->role ==0 || aCar->role==2) {aItem = aItem->next; continue;}
						if (aCar->known_neigh.nItems==0) {aItem = aItem->next; continue;}

						int SP_length = data_change_time/aCar->known_neigh.nItems;

						nItem = aCar->known_neigh.head;
						while (nItem != NULL) {
							neigh = (struct neighbour_car*)nItem->datap;
							nCar = (struct vehicle*)neigh->carItem->datap;
							bItem = nCar->neighbours.head;
							while (bItem != NULL) {
								bNeigh = (struct neighbour_car*)bItem->datap;
								bCar = (struct vehicle*)bNeigh->carItem->datap;
								if (bCar->id == aCar->id) break;
								bItem = bItem->next;
							}
							neigh->packet_num -= neigh->data_rate*SP_length;
							bNeigh->packet_num -= bNeigh->data_rate*SP_length;
							nItem = nItem->next;
						}
		
						aItem = aItem->next;
					}
				}
			}
	return 0;
}



int init_all_car(struct Region *region)
{
	struct Item *aItem;
	struct Cell *aCell;
	struct vehicle *aCar;
	int i, j;

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				aCar->role = rand()%2; //each car is given an initial state 0 or 1 (listen or scan)
				aCar->communicate_time = 0;
				aCar->communicated_num = 0;
				aCar->scan_time = 1;
				aItem = aItem->next;
			}
		}
	}

	return 0;
}
