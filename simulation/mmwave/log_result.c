#include "log_result.h"
#include "math.h"
/********************* write result ***********************/
void log_ND(struct Region *region, int pro, double ratio)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar, *nCar, *tCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j, total_pair_num=0;
	int total_rate=0;

	sprintf(CDF_output_path, "./simulation_result/Neighbour_Discovery/density_%d_%d.csv", traffic_density, pro); //BEACON_NUM, car_number, timestamp, ith car position, jth protocol

	CDF_fOutput = fopen(CDF_output_path, "a");

	fprintf(CDF_fOutput, "%lf\n", ratio);
	fclose(CDF_fOutput);
}

void log_rate(struct Region *region)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j, total_pair_num=0;
	double percent;

	sprintf(CDF_output_path, "./simulation_result/CDF/speed_%d.csv", traffic_density); //BEACON_NUM, car_number, timestamp, ith car position, jth protocol

	CDF_fOutput = fopen(CDF_output_path, "w");

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->neighbours.nItems ==0) {aItem=aItem->next;continue;}
				nItem = aCar->neighbours.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					fprintf(CDF_fOutput, "%d\n", neigh->data_rate);
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}
		}
	}

	fclose(CDF_fOutput);
}

void log_throughput(struct Region *region, int pro)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar, *nCar, *tCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j, total_pair_num=0;
	int total_rate=0, car_num=0;

	sprintf(CDF_output_path, "./simulation_result/Matching/density_%d_%d.csv", traffic_density, pro); //BEACON_NUM, car_number, timestamp, ith car position, jth protocol

	CDF_fOutput = fopen(CDF_output_path, "a");

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				car_num++;
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->role != 2) {aItem=aItem->next;continue;}
				tCar = (struct vehicle*)aCar->choose_car_Item->datap;
				nItem = aCar->known_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					if (nCar->id == tCar->id) {total_rate+=neigh->data_rate; break;}
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}
		}
	}

	fprintf(CDF_fOutput, "%lf\n", total_rate*1.0/car_num);
	fclose(CDF_fOutput);
}

void log_neighbour(struct Region *region)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j, total_pair_num=0;
	double percent;

	sprintf(CDF_output_path, "./simulation_result/CDF/car_number_%d_1.csv", traffic_density); //BEACON_NUM, car_number, timestamp, ith car position, jth protocol

	CDF_fOutput = fopen(CDF_output_path, "a");

	int total_num = 0;
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->neighbours.nItems ==0) {aItem=aItem->next;continue;}
				total_num++;
				/*nItem = aCar->neighbours.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					fprintf(CDF_fOutput, "%lf, %lf\n", neigh->dis, neigh->angle);
					nItem = nItem->next;
				}*/
				total_pair_num += aCar->neighbours.nItems;	
				aItem = aItem->next;
			}
		}
	}
	percent = total_pair_num*1.0/total_num;
	fprintf(CDF_fOutput, "%lf\n", percent);

	fclose(CDF_fOutput);
}

void log_metric1(struct Region *region, int pro)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j, total_pair_num=0;
	double percent;

	sprintf(CDF_output_path, "./simulation_result/CDF/metric1/result_%d_%d_%d_%d_%d_%d_%d_%d.csv", bi_num%200, BEACON_NUM, traffic_density, pro, learning_cycle_num, SELECT_TIME, BEACON_NUM, DATA_TIME/100); //BEACON_NUM, car_number, timestamp, ith car position, jth protocol

	CDF_fOutput = fopen(CDF_output_path, "a");

	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->history_neigh.nItems ==0) {aItem=aItem->next;continue;}
				nItem = aCar->history_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					if (neigh->packet_num <=0) total_pair_num++;
					nItem = nItem->next;
				}
				percent = total_pair_num*1.0/aCar->history_neigh.nItems;
				fprintf(CDF_fOutput, "%lf\n", percent);
				total_pair_num = 0;
				aItem = aItem->next;
			}
		}
	}

	fclose(CDF_fOutput);
}

void log_metric2(struct Region *region, int pro)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j, total_pair_num=0;
	double percent;

	sprintf(CDF_output_path, "./simulation_result/CDF/metric2/result_%d_%d_%d_%d_%d_%d_%d_%d.csv", bi_num%200, BEACON_NUM, traffic_density, pro, learning_cycle_num, SELECT_TIME, BEACON_NUM, DATA_TIME/100); //BEACON_NUM, car_number, timestamp, ith car position, jth protocol
	
	CDF_fOutput = fopen(CDF_output_path, "a");
	//TOTAL_fOutput = fopen(TOTAL_output_path, "w");
	
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->history_neigh.nItems ==0) {aItem=aItem->next;continue;}
				nItem = aCar->history_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					if (neigh->packet_num>=0) total_pair_num+=(DATA_TIME-neigh->packet_num);
					else total_pair_num+=DATA_TIME;
					nItem = nItem->next;
				}
				percent = total_pair_num*1.0/(aCar->history_neigh.nItems*DATA_TIME);
				fprintf(CDF_fOutput, "%lf\n", percent);
				total_pair_num = 0;
				aItem = aItem->next;
			}
		}
	}

	fclose(CDF_fOutput);
}

void log_metric3(struct Region *region, int pro)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j;
	double average_num = 0;
	double mse=0;

	sprintf(CDF_output_path, "./simulation_result/CDF/metric3/result_%d_%d_%d_%d_%d_%d_%d_%d.csv", bi_num%200, BEACON_NUM, traffic_density, pro, learning_cycle_num, SELECT_TIME, BEACON_NUM, DATA_TIME/100); //BEACON_NUM, car_number, timestamp, ith car position, jth protocol
	
	CDF_fOutput = fopen(CDF_output_path, "a");
	//TOTAL_fOutput = fopen(TOTAL_output_path, "w");
	
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->history_neigh.nItems ==0) {aItem=aItem->next;continue;}
				nItem = aCar->history_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					//if (neigh->packet_num <0) printf("bi_num:%d, car id:%d, neigh num: %d, packet time: %d !!! neighbour id: %d\n",bi_num, aCar->id, aCar->real_neigh.nItems, neigh->packet_num, neigh->car_id);
					if (neigh->packet_num>=0) average_num= average_num+(DATA_TIME-neigh->packet_num)*1.0/DATA_TIME;
					else average_num= average_num+1.0;
					nItem = nItem->next;
				}

				average_num /= aCar->history_neigh.nItems;

				nItem = aCar->history_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					if (neigh->packet_num>=0) mse= mse+((DATA_TIME-neigh->packet_num)*1.0/DATA_TIME-average_num)*((DATA_TIME-neigh->packet_num)*1.0/DATA_TIME-average_num);
					else mse= mse+(1.0-average_num)*(1.0-average_num);
					//printf("car id: %d, neighbour num: %d, neighbour id: %d, bi num: %d, packet_num: %d\n", aCar->id, aCar->real_neigh.nItems, neigh->car_id, bi_num, neigh->packet_num);
					nItem = nItem->next;
				}
				mse = sqrt(mse/aCar->history_neigh.nItems);
				fprintf(CDF_fOutput, "%lf\n", mse);
				mse=0;
				average_num=0;
				aItem = aItem->next;
			}
		}
	}
	fclose(CDF_fOutput);
}

void log_metric4(struct Region *region, int pro)
{
	struct Cell *aCell;
	struct Item *aItem, *nItem;
	struct vehicle *aCar;
	struct neighbour_car *neigh;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j;
	double average_num = 0;
	double mse=0;

	sprintf(CDF_output_path, "./simulation_result/CDF/metric4/result_%d_%d_%d_%d_%d_%d.csv", traffic_density, pro, learning_cycle_num, SELECT_TIME, SELECT_NUM, DATA_TIME/100); //car_number, timestamp, ith car position, jth protocol
	
	CDF_fOutput = fopen(CDF_output_path, "a");
	//TOTAL_fOutput = fopen(TOTAL_output_path, "w");
	
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				if (aCar->history_neigh.nItems ==0) {aItem=aItem->next;continue;}
				nItem = aCar->history_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					//if (neigh->packet_num <0) printf("bi_num:%d, car id:%d, neigh num: %d, packet time: %d !!! neighbour id: %d\n",bi_num, aCar->id, aCar->real_neigh.nItems, neigh->packet_num, neigh->car_id);
					average_num= average_num+(DATA_TIME-neigh->packet_num)/DATA_TIME;
					nItem = nItem->next;
				}

				average_num /= aCar->history_neigh.nItems;

				nItem = aCar->history_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					mse= mse+((DATA_TIME-neigh->packet_num)/DATA_TIME-average_num)*((DATA_TIME-neigh->packet_num)/DATA_TIME-average_num);
					//printf("car id: %d, neighbour num: %d, neighbour id: %d, bi num: %d, packet_num: %d\n", aCar->id, aCar->real_neigh.nItems, neigh->car_id, bi_num, neigh->packet_num);
					nItem = nItem->next;
				}
				mse = sqrt(mse/aCar->history_neigh.nItems);
				fprintf(CDF_fOutput, "%lf\n", mse);
				mse=0;
				average_num=0;
				aItem = aItem->next;
			}
		}
	}
	fclose(CDF_fOutput);
}


void write_CDF_result(struct Region *region)
{
	struct Cell *aCell;
	struct Item *aItem;
	struct vehicle *aCar;
	FILE *CDF_fOutput, *TOTAL_fOutput;
	char CDF_output_path[100];
	char TOTAL_output_path[100];
	int i,j, total_pair_num=0;
	double percent;

	sprintf(CDF_output_path, "./simulation_result/CDF/result_%d_%d_%d_%d.csv", Car_Number, timestamp, 1, 4); //car_number, timestamp, ith car position, jth protocol
	sprintf(TOTAL_output_path, "./simulation_result/CDF/total_link_%d_%d_%d_%d.csv", Car_Number, timestamp, 1, 4); //car_number, timestamp, ith car position, jth protocol	
	CDF_fOutput = fopen(CDF_output_path, "w");
	//TOTAL_fOutput = fopen(TOTAL_output_path, "w");
	
	for(i = 0; i<region->hCells; i++){       
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				percent = (double)aCar->communicated_num/aCar->neighbours.nItems;
				fprintf(CDF_fOutput, "%lf\n", percent);
				total_pair_num += aCar->communicated_num;
				aItem = aItem->next;
			}
		}
	}

	//fprintf(TOTAL_fOutput, "%d\n", total_pair_num);
}

void write_total_link_result(int total_link[], int num, int pro, int total_edge_num)
{
	struct Cell *aCell;
	struct Item *aItem;
	struct vehicle *aCar;
	FILE *TOTAL_fOutput;

	char TOTAL_output_path[100];
	
	sprintf(TOTAL_output_path, "./simulation_result/TOTAL_LINK/total_link_%d_%d_%d_%d_%d_%d_%d.csv", Car_Number, num, pro, SELECT_TIME, learning_cycle_num, SELECT_NUM, DATA_TIME); //car_number, ith car position, jth protocol, select time, learning cycle number, select number, DATA TIME
	TOTAL_fOutput = fopen(TOTAL_output_path, "w");
	
	for (int i=0; i<10000; i++){
		if (total_link[i] ==-1) {fclose(TOTAL_fOutput);return;}
		//if (i==0) printf("zzz: %d\n", total_link[i]/10);
		fprintf(TOTAL_fOutput, "%d\n", total_link[i]/10);
		//if (total_link[i]/10 == total_edge_num) {fclose(TOTAL_fOutput);return;}
	}

	fclose(TOTAL_fOutput);
}

void write_sector_num_result(int avg_time[], int num)
{
	FILE *fOutput;
	char output_path[100];

	sprintf(output_path, "./simulation_result/SECTOR_NUM/result_%d_%d.csv", Car_Number, num); //car_number, timestamp, ith car position	
	fOutput = fopen(output_path, "w");
	for (int i=0; i<sector_num; i++){
		fprintf(fOutput, "%d\n", avg_time[i]);
	}
	fclose(fOutput);
}

/********************* write result ***********************/
