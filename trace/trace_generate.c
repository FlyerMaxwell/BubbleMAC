#include "common.h"
#include "geometry.h"
#include "mapsimulate.h"
#include "trace.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <sys/io.h>

int main(int argc, char *argv[])
{
   
   
   /***************load map********************/
    FILE *fsource, *fOutput;
    struct Region *region = NULL;
    
    if(argc < 2) {
	printf("Usage: %s .map output.txt\n", argv[0]);
	exit(1);
    }
    
    if((fsource=fopen(argv[1], "rb"))!=NULL) {
	printf("************Loading map ...************\n");
	region = region_load_func(fsource, NULL, -1);
	fclose(fsource);
    }
    printf("************Map loading success!************\n\n");
    
    if (access(argv[2],0)==-1) {
       printf("Use default Output File...\n");
       fOutput = fopen("trace.txt", "w");
    }
    else fOutput = fopen(argv[2],"w"); 
   /***************load map********************/


  /*************initialize multilane**************/
  init_multilane(region);
  /*************initialize multilane**************/


  /******** type in some information ********/
    int SIMULATE_CAR_NUM;
    int SIMULATE_TIME;
    int use_new_data;
	printf("Use new input or not, input 1 if use new input, input 0 use last input: ");
	scanf("%d", &use_new_data);
        if (use_new_data == 1){
		printf("Please input the num of simulating vehciles: ");
		scanf("%d", &SIMULATE_CAR_NUM);
        }
	printf("Please input the num of simulating time: ");
	scanf("%d", &SIMULATE_TIME);
  /******** type in some information ********/
  

  /******** generate car route information ********/
  if (use_new_data == 1) generate_car(region, SIMULATE_CAR_NUM);
  /******** generate car route information ********/


  /******** load car information ********/
  load_car(region);  
  /******** load car information ********/

  /******** Initialize the hashtable of trace ********/
  struct Hashtable* Traces;
	printf("Initialize the hashtable of trace\n");
	hashtable_init(Traces, MAX_HASH_SIZE, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
        printf("Initialisation success!\n\n");
  /******** Initialize the hashtable of trace ********/

  



  /*********start simulate***************/
   int count = SIMULATE_TIME;
   printf("Starting simulating...\n");
   while(count != 0){
     start_simulate(region, Traces, count);
     count--;
  }
  printf("Simulation finished!\n\n");
  /*********start simulate***************/




  /*********write traces to file**********/
  printf("Writing traces into file...\n");
  trace_write(Traces, fOutput);
  fclose(fOutput);
  printf("Finished!\n\n");  
  /*********write traces to file**********/
  
  return 1;
}


