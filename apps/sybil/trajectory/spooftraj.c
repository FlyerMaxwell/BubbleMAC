#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"files.h"
#include"traj.h"

int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	struct Region *rsuRegion;
	char dumpfile[256];

	struct Hashtable trajs;
	struct Duallist spoofingTrajs, *aSpooftraj;
	struct Item *aItem, *aLandmarkItem, *aSpooftrajItem;
	struct Landmark *aLandmark;
	struct Trajectory *aTraj;
	int trajLength = -1, trajNumber = -1, number;

	int i, j, Xmax, length, medium;
	unsigned long index, count;


	if(argc < 3) {
	      printf("%s is used to generate spoofing trajectories of a trace.\n", argv[0]);
	      printf("Usage: %s [-n traj_number] [-l traj_length] .vmap (.trj | .lst ...)\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'n':
			trajNumber = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		case 'l':
			trajLength = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		}
	}

	srand(time(0));

	rsuRegion = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      rsuRegion = region_load_func(fsource, NULL, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}
	hashtable_init(&trajs, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))traj_has_name);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
//			printf("Loading %s file ...\n", argv[1]);
			load_source_file(fsource, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, &trajs, (void*(*)(FILE*, void*))load_traj_with_hashtable, NULL, NULL, NULL,NULL,NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	if(rsuRegion!=NULL && trajs.count != 0) {
	      for (i = 0; i<trajs.size; i++) {
		      aItem = trajs.head[i];
		      while(aItem != NULL ) {
				aTraj = (struct Trajectory*)aItem->datap;

				if(trajLength != -1) 
					length = MIN(aTraj->landmarks.nItems-1, trajLength);
				else if(trajNumber != -1) {
					Xmax = argmax_C_n_x_larger_than_m(aTraj->landmarks.nItems, trajNumber);
					length = Xmax;
				}

				duallist_init(&spoofingTrajs);
				spoof_a_traj_using_stack(&spoofingTrajs, aTraj, rsuRegion, length, -1);
				if(trajLength != -1 && trajNumber != -1 && spoofingTrajs.nItems < trajNumber) {
					//printf("There are no enough %d-signature long trajs can be generated. Required:%d, Achieved:%ld\n", length, trajNumber, spoofingTrajs.nItems);
				} else if(trajLength == -1 && trajNumber != -1 && spoofingTrajs.nItems < trajNumber) {
					number = trajNumber - spoofingTrajs.nItems;
					medium = ceil(((double)aTraj->landmarks.nItems)/2);
					while(number)	{
						spoof_a_traj_using_stack(&spoofingTrajs, aTraj, rsuRegion, length, number);
						length --;
						number = trajNumber - spoofingTrajs.nItems;
					}
					if(spoofingTrajs.nItems < trajNumber) {
						//printf("There are no enough trajs can be generated. Required:%d, Achieved:%ld\n", trajNumber, spoofingTrajs.nItems);
					}	

				}

				j = 0;
				count = MIN(trajNumber, spoofingTrajs.nItems);
				while(count) {
					index = rand()%spoofingTrajs.nItems;
					aSpooftrajItem = spoofingTrajs.head;	
					while(index) {
						index --;
						aSpooftrajItem = aSpooftrajItem->next;
					}
					aSpooftraj = (struct Duallist*)duallist_pick_item(&spoofingTrajs, aSpooftrajItem);
					sprintf(dumpfile, "%s_%d.strj", aTraj->vName, j);
					if( (fdump = fopen(dumpfile, "w"))!=NULL) {
						fprintf(fdump, "%d\n", FILE_TRAJECTORY);
						aLandmarkItem = aSpooftraj->head;
						while(aLandmarkItem!=NULL) {
							aLandmark = (struct Landmark*)((struct Item*)aLandmarkItem->datap)->datap;
							fprintf(fdump, "%s_%d,%d,%ld,0,0,\n", aTraj->vName, j, aLandmark->crossId, aLandmark->timestamp);
							aLandmarkItem = aLandmarkItem->next;
						}
						fflush(fdump);
						fclose(fdump);
					}
					duallist_destroy(aSpooftraj, NULL);
					j++;
					count--;
				}
			      	aItem = aItem->next;
		      }
	      }
	} 
	 
	hashtable_destroy(&trajs, (void(*)(void*))traj_free_func);
	region_free_func(rsuRegion);
	return 0;
}




