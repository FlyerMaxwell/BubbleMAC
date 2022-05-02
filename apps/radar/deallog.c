#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "common.h"
#include "geometry.h"

#define GPS_LOG 0
#define SIG_LOG 1
#define MAX_DIST 10000

struct Record
{
  char timestamp[32];
  double x;
  double y;
  float speed;
  
  struct Duallist signals;
  struct Duallist derivative_signals;
};

void free_record(struct Record *aRecord)
{
  if (aRecord) {
	duallist_destroy(&aRecord->signals, (void(*)(void*))free);
	free(aRecord);
  }
}

struct Signal
{
  int id;
  float ssi;
  char connected;
  unsigned seen;
  double ssi_2;
};

int signal_has_id(int* id, struct Signal *aSignal)
{
	return *id == aSignal->id;
}

void process_line(struct Duallist *signals, char *line)
{
	int ssi, id;
	struct Item *aItem;
	struct Signal *aSignal;
	int i, j;

	strtok(line, ".");
	strtok(NULL, " ");
	strtok(NULL, " ");
	strtok(NULL, " ");
	ssi = atoi(strtok(NULL, " "));
	id = atoi(strtok(NULL, " "));
	if(ssi<0 || ssi == 99) {
	} else {
		aItem = duallist_find(signals, &id, (int(*)(void*, void*))signal_has_id);
		if(aItem) {
			aSignal = (struct Signal*)aItem->datap;
			aSignal->ssi += ssi;
			aSignal->seen += 1;
			aSignal->connected = 1;
		} else {
			aSignal = (struct Signal*)malloc(sizeof(struct Signal));
			aSignal->id = id;
			aSignal->ssi = ssi;
			aSignal->connected = 1;
			aSignal->seen = 1;
			duallist_add_to_tail(signals, aSignal);
		}
	}
	strtok(NULL, " ");
	strtok(NULL, " ");
	i = atoi(strtok(NULL, " "));
	for (j=0;j<i;j++) {
		id = atoi(strtok(NULL, " "));
		strtok(NULL, " ");
		ssi = atoi(strtok(NULL, " "));
		if (ssi<0 || ssi == 99 )
			continue;
		aItem = duallist_find(signals, &id, (int(*)(void*, void*))signal_has_id);
		if(aItem) {
			aSignal = (struct Signal*)aItem->datap;
			aSignal->ssi += ssi;
			aSignal->seen += 1;
		} else {
			aSignal = (struct Signal*)malloc(sizeof(struct Signal));
			aSignal->id = id;
			aSignal->ssi = ssi;
			aSignal->connected = 0;
			aSignal->seen = 1;
			duallist_add_to_tail(signals, aSignal);
		}
	}
}

void make_records(struct Duallist *records, char *sig_logfile, char* gps_logfile, char* speed_logfile)
{
	FILE *fsource;
	char buf[1024], line[1024], lasttime[32], *currtime;
	struct Duallist signals;
	int n, first, ndup;
	struct Item *pItem;
	struct Record *aRecord;
	struct Signal *aSignal;
	double x;

	if(records==NULL)
		return;
	duallist_init(records);
	if((fsource=fopen(sig_logfile, "r"))!=NULL) {
		n = fgets(buf, 1024, fsource);
		strncpy(line, buf, 1024);
		currtime = strtok(buf, ".");
		first = 1;
		while (n)
		{
			if(first) {
				duallist_init(&signals);
				strncpy(lasttime, currtime, 32);
				ndup = 0;
				first = 0;
			}
			if (!strncmp(lasttime, currtime, 32)) {
				process_line(&signals, line);
				n = fgets(buf, 1024, fsource);
				strncpy(line, buf, 1024);
				currtime = strtok(buf, ".");
				ndup += 1;
			} else {
				aRecord = (struct Record*)malloc(sizeof(struct Record));
				strncpy(aRecord->timestamp, lasttime, 32);
				aRecord->x = 0;
				aRecord->y = 0;
				duallist_init(&aRecord->signals);
				while(signals.nItems) {
					aSignal = (struct Signal*)duallist_pick_head(&signals);
					if(aSignal->seen * 2 > ndup) {
						aSignal->ssi /= aSignal->seen;
						duallist_add_to_tail(&aRecord->signals, aSignal);
					} else {
						free(aSignal);
					}
				}
				duallist_add_to_tail(records, aRecord);		
				first = 1;
			}
		}
		fclose(fsource);
	}
	
	if((fsource=fopen(gps_logfile, "r"))!=NULL) {
		n = fgets(buf, 1024, fsource);
		currtime = strtok(buf, ".");
		pItem = records->head;
		aRecord = (struct Record*)pItem->datap;
		while (strncmp(aRecord->timestamp, currtime, 32)>0) {
			n = fgets(buf, 1024, fsource);
			currtime = strtok(buf, ".");
		}
		while (strncmp(aRecord->timestamp, currtime, 32)<0) {
			pItem = pItem->next;
			aRecord = (struct Record*)pItem->datap;
		}

		first = 1;
		while (n)
		{
			if(first) {
				strncpy(lasttime, currtime, 32);
				ndup = 0;
				first = 0;
			}
			if (!strncmp(lasttime, currtime, 32)) {
				strtok(NULL, " ");
				x = atof(strtok(NULL, " "));
				aRecord->x += x;
				aRecord->y += atof(strtok(NULL, " "));
				n = fgets(buf, 1024, fsource);
				currtime = strtok(buf, ".");
				if (x) ndup += 1;
			} else {
				if(ndup) {
					aRecord->x /= ndup;
					aRecord->y /= ndup;
				} else {
					aRecord->x = 0;
					aRecord->y = 0;
				}
				pItem = pItem->next;
				if (!pItem) break; 
				aRecord = (struct Record*)pItem->datap;
				first = 1;
			}
		}
		fclose(fsource);
      }
}

void normalize_signals(struct Duallist *records)
{
	struct Duallist signals;
	struct Item *aItem, *bItem, *cItem;
	struct Record *aRecord;
	struct Signal *aSignal, *bSignal;

	duallist_init(&signals);
	aItem = records->head;
	while(aItem) {
		aRecord = (struct Record*)aItem->datap;
		bItem = aRecord->signals.head;
		while(bItem) {
			aSignal = (struct Signal*)bItem->datap;
			cItem = duallist_find(&signals, &aSignal->id, (int(*)(void*, void*))signal_has_id);
			if(cItem) {
				bSignal = (struct Signal*)cItem->datap;
				bSignal->ssi += aSignal->ssi;
				bSignal->seen += 1;
				bSignal->ssi_2 += aSignal->ssi * aSignal->ssi;
			} else {
				bSignal = (struct Signal*)malloc(sizeof(struct Signal));
				bSignal->id = aSignal->id;
				bSignal->ssi = aSignal->ssi;
				bSignal->seen = 1;
				bSignal->ssi_2 = aSignal->ssi * aSignal->ssi;
				duallist_add_to_tail(&signals, bSignal);
			}
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
	aItem = signals.head;
	while(aItem) {
		aSignal = (struct Signal*)aItem->datap;
		aSignal->ssi = aSignal->ssi/aSignal->seen;
		aSignal->ssi_2 = sqrt(aSignal->ssi_2/aSignal->seen-aSignal->ssi*aSignal->ssi);
		aItem = aItem->next;
	}
	aItem = records->head;
	while(aItem) {
		aRecord = (struct Record*)aItem->datap;
		bItem = aRecord->signals.head;
		while(bItem) {
			aSignal = (struct Signal*)bItem->datap;
			cItem = duallist_find(&signals, &aSignal->id, (int(*)(void*, void*))signal_has_id);
			bSignal = (struct Signal*)cItem->datap;
			aSignal->ssi = (aSignal->ssi - bSignal->ssi)/bSignal->ssi_2;
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
}

double sig_distance(struct Record *aRecord, struct Record *bRecord)
{
	struct Item *aItem, *bItem;
	struct Signal *aSignal, *bSignal;
	double dist = 0;
	int num = 0;

	aItem = aRecord->signals.head;
	while(aItem) {
		aSignal = (struct Signal*)aItem->datap;
		bItem = bRecord->signals.head;
		while(bItem) {
			bSignal = (struct Signal*)bItem->datap;
			if(aSignal->id == bSignal->id) {
				dist += (aSignal->ssi - bSignal->ssi)*(aSignal->ssi-bSignal->ssi);
				num += 1;
			}
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
	if(num)
		return dist/num;
	else
		return MAX_DIST;

}

void align_gps_check_signals(struct Duallist *aRecords, struct Duallist *bRecords)
{
	struct Item *aItem, *bItem, *cItem, *pItem;
	struct Signal *aSignal;
	struct Record *aRecord, *bRecord, *mRecord;
	double mindist, dist;

	aItem = aRecords->head;
	while(aItem) {
		aRecord = (struct Record*)aItem->datap;
		if(aRecord->x == 0) {
			aItem = aItem->next;
			continue;
		}
		mindist = 100;
		bItem = bRecords->head;
		while(bItem) {
			bRecord = (struct Record*)bItem->datap;
			if (bRecord->x == 0) {
				bItem = bItem->next;
				continue;
			}
			dist = distance_in_meter(aRecord->x, aRecord->y, bRecord->x, bRecord->y);
			if(dist < mindist) {
				mindist = dist;
				pItem = bItem;
				mRecord = (struct Record*)pItem->datap;
			}
			bItem = bItem->next;
		}
		printf("%s %s %.2lf *", aRecord->timestamp, mRecord->timestamp, mindist);
		cItem = aRecord->signals.head;
		while(cItem) {
			aSignal = (struct Signal*)cItem->datap;
			printf("%d %.1lf ", aSignal->id, aSignal->ssi);
			cItem = cItem->next;
		}
		printf("* ");
		cItem = mRecord->signals.head;
		while(cItem) {
			aSignal = (struct Signal*)cItem->datap;
			printf("%d %.1lf ", aSignal->id, aSignal->ssi);
			cItem = cItem->next;
		}
		printf("\n");

		aItem = aItem->next;
	}	


}

double DTW (struct Duallist *aRecords, struct Duallist *bRecords, unsigned long window)
{
	unsigned long n, m, i, j, k;
	double *dist, rst;
	struct Item *aItem, *bItem;
	struct Record *aRecord, *bRecord;

	n = aRecords->nItems;
	m = bRecords->nItems;
	dist = (double*)malloc(sizeof(double)*(n+1)*(m+1));
	for (i=0; i<n+1; i++)
		for(j=0; j<m+1; j++)
			*(dist+i*(m+1)+j) = MAX_DIST;
	*dist = 0;
	aItem = aRecords->head;
	for (i=1; i<=n; i++) {
		aRecord = (struct Record*)aItem->datap;
		bItem = bRecords->head;
		k = MAX(1, i-window);
		for(j=1;j<k;j++)
			bItem = bItem->next;
		for(j=k; j<MIN(m, i+window)+1; j++) {
			bRecord = (struct Record*)bItem->datap;
			*(dist+i*(m+1)+j) = sig_distance(aRecord, bRecord);
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
	rst = *(dist+(n+1)*(m+1));
	free(dist);
	return rst;
}


int main(int argc, char**argv)
{
	struct Duallist aRecords, bRecords;
	char *fromTime;
	int segmentDistance, slidingDistance;
	double dist;

	if(argc < 5) {
		printf("Usage: %s [-t fromTime segmentDistance] [-s slidingDistance] sig_log_A GPS_log_A speed_log_A sig_log_B GPS_log_B speed_log_B\n", argv[0]);
		exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 't':
			fromTime = argv[2];
			segmentDistance = atoi(argv[3]);
			argc-=3;
			argv+=3;
			break;
		case 's':
			slidingDistance = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		}
	}

	make_records(&aRecords, argv[1], argv[2], argv[3]);
	make_records(&bRecords, argv[4], argv[5], argv[6]);
	normalize_signals(&aRecords);
	normalize_signals(&bRecords);

	//select_segment(&aSegment, &aRecords, fromTime, segmentDistance);

	align_gps_check_signals(&aRecords, &bRecords);
	
	duallist_destroy(&aRecords, (void(*)(void*))free_record);
	duallist_destroy(&bRecords, (void(*)(void*))free_record);
	return 0;
}
