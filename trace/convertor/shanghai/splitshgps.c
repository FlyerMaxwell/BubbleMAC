/*
 * Seperate individual taxi or bus from the all-mixed GPS file and convert the original data format to .ogd format.
 *
 * This program should use after program "pickshgps" 
 */
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include"trace.h"
#include"common.h"
#include"geometry.h"
#include"files.h"
#include"shgps.h"

#define TO_BE_ACTIVE 500


int main(int argc, char *argv[])
{
  FILE *fp, *ftext, *frmv = NULL;
  char buf[256], name[128], fname[32], *change_to_date=NULL;
  long dullReps;
  int i, vtype = VEHICLE_TYPE_TAXI;

  struct Hashtable traces;
  struct Item *aItem, *bItem, *atItem;
  struct Trace *aTrace;
  struct Report *newRep;
  struct Record aRcd;

  int active = TO_BE_ACTIVE;
  

  if(argc < 2) {
	printf("Usage: %s [-bus][-n active_threshold][-d change_date_to \"yyyy-mm-dd\"]  GPSfile [remove_list]\n", argv[0]);
	exit(1);
  } 

  while(argv[1][0] == '-') {
      switch(argv[1][1]) {
      case 'b':
	      vtype = VEHICLE_TYPE_BUS;
	      argc-=1;
	      argv+=1;
	      break;
      case 'n':
	      active = atoi(argv[2]);
	      argc-=2;
	      argv+=2;
	      break;
      case 'd':
	      change_to_date = argv[2];
	      argc-=2;
	      argv+=2;
	      break;
	     
      default:
	      printf("Usage: %s [-bus][-n active_threshold]  GPSfile [remove_list]\n", argv[0]);
      }
  }

  if((fp=fopen(argv[1], "r"))== NULL) {
  	printf("cannot open GPS data.\n");
  	exit(2);
  }

  if(argc > 2) {
	frmv = fopen(argv[2], "r");
  }

  hashtable_init(&traces, 10000,(unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
  while(fgets(buf, 256, fp)) {
    	if(read_a_record(buf, vtype, change_to_date, &aRcd))
		continue;
	newRep = (struct Report*)malloc(sizeof(struct Report));
	memset(newRep, 0, sizeof(struct Report));

	if (vtype == VEHICLE_TYPE_TAXI) {
		newRep->gPoint.x = atof(aRcd.longitude);
		newRep->gPoint.y = atof(aRcd.lattitude);
		newRep->timestamp = strtot(aRcd.timestamp);
		newRep->speed = atoi(aRcd.speed);
		newRep->angle = normal_angle_taxi(atoi(aRcd.heading));
		newRep->state = atoi(aRcd.D7);

		sprintf(name, "t%s", aRcd.vId);
	} else {
		newRep->msgType = atoi(aRcd.msgType);
		newRep->gPoint.x = atof(aRcd.longitude);
		newRep->gPoint.y = atof(aRcd.lattitude);
		newRep->timestamp = strtot(aRcd.timestamp);
		newRep->speed = atoi(aRcd.speed);
		newRep->angle = normal_angle_bus(atoi(aRcd.heading));
		newRep->state = 0;
		if (atoi(aRcd.D7) == 1) newRep->state |= 0x80;
		if (atoi(aRcd.D6) == 1) newRep->state |= 0x40;
		if (atoi(aRcd.D5) == 1) newRep->state |= 0x20;
		if (atoi(aRcd.D4) == 1) newRep->state |= 0x10;
		if (atoi(aRcd.D3) == 1) newRep->state |= 0x8;
		if (atoi(aRcd.D2) == 1) newRep->state |= 0x4;
		newRep->errorInfo = atoi(aRcd.errorInfo); 
		newRep->routeLeng = atof(aRcd.routeLeng);
		newRep->gasVol = atof(aRcd.gasVol);
		
		sprintf(name, "b%s", aRcd.vId);
	}
	
	aItem = hashtable_find(&traces, name);
	if(aItem == NULL) {
		aTrace = (struct Trace*)malloc(sizeof(struct Trace));
		trace_init_func(aTrace);
		strncpy(aTrace->vName, name, strlen(name));
		newRep->fromTrace = aTrace;
		if(vtype == VEHICLE_TYPE_BUS) {
			strncpy(aTrace->onRoute, aRcd.routeId, strlen(aRcd.routeId)+1);
			aTrace->type = FILE_ORIGINAL_GPS_BUS;
		} else {
			aTrace->type = FILE_ORIGINAL_GPS_TAXI;
		}
			
		aTrace->var1 = 0;
		atItem = duallist_add_in_sequence_from_tail(&aTrace->reports, newRep, (int(*)(void*, void*))report_has_earlier_timestamp_than);
		hashtable_add(&traces, name, aTrace);
	} else {
		aTrace = (struct Trace*)aItem->datap;
		newRep->fromTrace = aTrace;
		atItem = duallist_add_in_sequence_from_tail(&aTrace->reports, newRep, (int(*)(void*, void*))report_has_earlier_timestamp_than);
	}
	if(are_two_reports_active((struct Report*)atItem->datap, (struct Report*)atItem->prev->datap)) {
		aTrace->var1 ++;
	}
  }
  fclose(fp);

  if(frmv != NULL) {
	while(fgets(buf, 128, frmv)) {
		sscanf(buf, "%s", name);
		trace_free_func((struct Trace*)hashtable_pick(&traces, name));
	}
	fclose(frmv);
  }	

  for(i = 0; i < traces.size; i++) {
	aItem = traces.head[i];
	while (aItem != NULL){
		bItem = aItem->next;
		aTrace = (struct Trace*)aItem->datap;
		/* First remove dull reports from trace */
		dullReps = remove_dull_reports(aTrace);
		//printf("Trace: %s has %ld reports removed, %.0lf reports active\n", aTrace->vName, dullReps, aTrace->var1);
		if(aTrace->var1 < active) {
		//	printf("------> Trace %s is removed for not being active.\n", aTrace->vName);
			trace_free_func((struct Trace*)hashtable_pick(&traces, aTrace->vName));
		} else {
			/* then distiguish mixed vehicles from the trace */
			if(is_trace_mixed(aTrace)) {
		//		printf("******> Trace %s considered as mixed trace.\n", aTrace->vName);
				trace_free_func((struct Trace*)hashtable_pick(&traces, aTrace->vName));
			} else {
				if(vtype == VEHICLE_TYPE_TAXI) {
					sprintf(fname, "%s.ogd", aTrace->vName);
					if((ftext=fopen(fname, "w"))!=NULL) {
						trace_dump_func(ftext, aTrace);
						fflush(ftext);
						fclose(ftext);	
					} else {
						printf("Cannot open %s to write!\n", fname);
					}
				} else {
					sprintf(fname, "%s.ogd", aTrace->vName);
					if((ftext=fopen(fname, "w"))!=NULL) {
						trace_dump_func(ftext, aTrace);
						fflush(ftext);
						fclose(ftext);	
					} else {
						printf("Cannot open %s to write!\n", fname);
					}
				}
			}
		}
		aItem = bItem;
	}
  }

  hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
  return 0;
}


