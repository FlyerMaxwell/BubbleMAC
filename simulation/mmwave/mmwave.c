#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "common.h" 
#include "geometry.h"
#include "trace.h"
#include "blossom.h"
#include "parameters.h"
#include "protocol.h"
#include "log_result.h"
#include "function.h"

/*********** protocol id ************/
//Random:    1 
//JSAC17:    2
//Optimal:   3
//COMNET_V1: 11
//COMNET_V2: 12
//...
/************************************/


int main(int argc, char *argv[]) 
{
  FILE *fsource;
  //struct Region *region = NULL;

  if (argc<2) {
	printf("Usage: %s .map", argv[0]);
	exit(1);
  }

  /******** type in some information ********/
  int use_new_data=0;
//	printf("Use new input or not, input 1 if use new input, input 0 use last input: ");
//	scanf("%d", &use_new_data);
  /******** type in some information ********/
  

  /*********** load map ***********/
  //if((fsource=fopen(argv[1], "rb"))!=NULL) {
	printf("Loading map ...\n");
	//region = region_load_func(fsource, NULL, -1);
	//fclose(fsource);
	printf("Map is loaded successfully!\n");
 // }
  /*********** load map ***********/


  printf("Log experiments data or not? Y:1 N:0: ");
  scanf("%d", &log_data);
  printf("Start simulating ...\n\n");

  
  printf("Input traffic density: ");
  scanf("%d", &traffic_density);
  
  sector_num = (int)(360/scan_theta*1.6+0.5);

  int pro_num = 11;
  printf("Input protocol index: ");
  scanf("%d", &pro_num);
  int i=0;
  printf("Input traffic data index: ");
  scanf("%d", &i);

  printf("\n\n*************** Protocol***************\n");
  for (learning_cycle_num=3; learning_cycle_num<=3; learning_cycle_num++) {
	for (SELECT_TIME=40; SELECT_TIME<=40; SELECT_TIME+=20) {
		//if (SELECT_TIME==40) continue;
			for (SELECT_NUM=1; SELECT_NUM<=1; SELECT_NUM++) {
				//for (int i=0; i<1; i++){
					struct Region *aRegion = NULL;


						fsource=fopen(argv[1], "rb");
						aRegion = region_load_func(fsource, NULL, -1);
						fclose(fsource);


						for (bi_num =(i-1)*2000; bi_num<i*2000; bi_num+=1){
							if (bi_num%BEACON_NUM ==0) {
		  						init_simulation(aRegion);
								// printf("1\n");
								generate_car(aRegion);
								// printf("2\n");
								handle_neighbour(aRegion);
								// printf("3\n");
		  						//IEEE_AD(aRegion);
								final_bi_protocol_random(aRegion);
								//COMNET_V2(aRegion);

								//for (magic_number=1;magic_number<=15;magic_number++){
								//for (int k=0; k<100;k++){
								//	COMNET_Match(aRegion,k);
								//}
								//}
							}
							
								
							else {
								update_trans_rate(aRegion);
								data_exchange_phase(aRegion, BEACON_INTERVAL);
								//AD_exchange_data(aRegion, BEACON_INTERVAL);
								if (bi_num%BEACON_NUM ==BEACON_NUM-1) update_vehicle_info(aRegion);
							}

							if (log_data && bi_num%BEACON_NUM ==BEACON_NUM-1) {
								log_metric1(aRegion, pro_num);	
								log_metric2(aRegion, pro_num);
								log_metric3(aRegion, pro_num);			
							}
							
						}
					//region_free_func(aRegion);
					
			}
  	}
  }
  return 0;
}
